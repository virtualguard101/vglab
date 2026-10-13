# 源码详解

| 文件 | 阶段 | 构建产物 | 一句话 |
|------|------|----------|--------|
| `common.h` | 共享 | — | 阶段间共享常量与函数指针类型 |
| `shellcode.h` | 共享 | — | x86_64 `execve("/bin/sh")` shellcode（42B, 无 `\x00`） |
| `unsafe.c` | 01 | `unsafe` / `unsafe_canary` | 五种经典不安全函数的溢出行为 |
| `dissect.c` | 02 | `dissect` | 栈布局观测 + 修改返回地址跳过语句 |
| `vulnerable.c` | 03 | `vulnerable` | 被攻击目标: `argv[1]` → `strcpy`（CVE-2019-17542 形态） |
| `exploit.c` | 04 | `exploit` | 攻击端: NOP 雪橇 + 地址猜解 + 自动爆破 |
| `defense.c` | 05 | `defense` | 同一漏洞的安全版本（4 种写法, 含 1 反例） |
| `canary_probe.c` | 05 | `canary_probe` / `canary_probe_off` | 观测 canary 值与触发 `__stack_chk_fail` |

---

## 共享头文件

### `common.h`
阶段间共享的常量与类型, **不含逻辑**. 关键定义: 

| 宏 | 值 | 用途 |
|----|----|------|
| `DEFAULT_BUF_SIZE` | 128 | `vulnerable.c` 的栈缓冲区大小; `exploit.c` 据此算 ret 槽偏移 |
| `DEFAULT_BSIZE` | 256 | payload 总长上限 |
| `DEFAULT_OFFSET` | 0 | 单发模式默认猜解偏移 |
| `DEFAULT_STEP` | 16 | 自动爆破步长（按对齐宽度跳） |
| `DEFAULT_MAX_DISP` | 2048 | 自动爆破搜索半径（±2048） |

三个 `typedef`（`overflow_fn_t` / `vuln_entry_t` / `exploit_entry_t`）表达
“目标与攻击端是**独立进程**”这一现代利用的标准形态: 攻击者无法在目标进程里
顺手帮忙. 

### `shellcode.h`
42 字节、无 `\x00` 的 x86_64 `execve("/bin/sh")` shellcode, 被 `exploit.c`
`#include` 进来当数据用. 文件顶部有一张**逐字节注释表**（offset 0..40 每条
指令）, 建议对照阅读. 三个必须理解的点: 

1. **为什么用负向 RIP 相对寻址** `lea rdi,[rip-14]`: 正向位移
   `(0x1a,0,0,0)` 含 3 个 NUL, 会被 `strcpy` 截断; 负向位移的补码
   `(0xf2,0xff,0xff,0xff)` 天然无 NUL. 
2. **为什么 `/bin/sh` 的结束 NUL 由代码运行时补写**（`mov [rdi+7],sil`）: 
   同理, 预置进数据会被 `strcpy` 截断, shellcode 送不全. 
3. **硬性前提**（缺一不可）: `-z execstack`（否则 NX 拦）、
   `-fno-stack-protector`（否则 canary 拦）、`-no-pie`（地址可预测）. 

---

## 01 · `unsafe.c` —— 不安全函数现象

**构建两份对照**（`just build` 自动生成）: 
- `unsafe` : 脆弱标志（无 canary）, 溢出 → 返回地址被 `0x41` 覆盖 → **SIGSEGV**
- `unsafe_canary` : `-fstack-protector-all`, 同样输入 → canary 被踩 → **SIGABRT**

**用法**: `./build/unsafe [1-5]`（或 `just unsafe` 跑默认 case 1 + 对照）

| 参数 | 函数 | 演示的不安全点 | 触发方式 |
|------|------|----------------|----------|
| `1` | `demo_strcpy` | `strcpy` 无界拷贝（论文 example2.c 原样复现） | 自动: 256B 灌进 16B 栈缓冲 |
| `2` | `demo_gets` | `gets` 无法限长, C11 已从标准移除 | 交互: 输入 >7 字符回车 |
| `3` | `demo_sprintf` | `sprintf` 无界格式化进固定缓冲 | 自动: 128B 源 → 32B 目标 |
| `4` | `demo_scanf` | `scanf("%s")` 无宽度 == `gets`; `%15s` 才安全 | 交互: 本例已用安全的 `%15s` |
| `5` | `demo_getchar_loop` | 论文点名的“读到分隔符”循环无界 | 交互: 本例已加 `i < sizeof-1` 上限 |

**原理与教学点**: 
- case 1/3 是**无界写**的直接演示, 必然溢出. 
- case 4/5 故意写成**安全版本**（`%15s`、循环带上限）, 作为“正确写法长什么样”
  的正面对照——注释里标出了对应的不安全写法. 
- `gets` 在现代 `<stdio.h>` 已不再声明, 文件顶部用 `extern char *gets(char *);`
  手动声明来演示这个“被语言标准除名”的历史反例. 
- **重要认知**: 是否崩溃、崩在哪里, **依赖编译器如何摆放数组**. 带 canary 构建
  下某些 case 会“侥幸存活”（溢出恰好落在同帧另一数组里, 没碰到 canary）. 
  这本身是教学点: 溢出后果是**布局相关、不确定**的. canary 的**确定性**演示
  在 `canary_probe.c`. 

---

## 02 · `dissect.c` —— 栈布局与改返回地址

**用法**: `./build/dissect [layout|redirect]`, 或环境变量
`DISSECT_MODE=redirect`（`just dissect` 两种都跑）. 

### 模式 A: `layout`（栈布局观测）
打印 `buffer[16]` / `saved RBP` / `return addr` 三者的真实地址与内容, 并算出
`buffer → RET 槽`的字节偏移. 对应论文用 `buffer1+12` 定位 RET 的思路, 只是
i386 的 4B 指针换成 x86_64 的 8B 指针. 

### 模式 B: `redirect`（改返回地址跳过语句, 论文 example3 重演）
`ret_redirect()` 把自己的返回地址 `+RET_SKIP_DELTA`, 使 `main` 中 `call` 之后
的赋值语句 `g_x = 1;` 被**跳过**, 最终 `g_x` 仍为 0. 

**两个偏移量是构建产物, 必须理解会漂移**: 
- `RET_OFFSET_FROM_BUF 21`: `buffer1` 起点到 RET 槽的距离（gcc 16 `-O0` 实测）. 
- `RET_SKIP_DELTA 20`: 要跳过的两条 `movl` 指令的总字节数（论文原稿写 +8
  是笔误, 自注 “should be 10”）. 

> **遇到 `g_x != 0` 怎么办**（换了编译器/版本/优化级别后极可能发生）: 
> 用 objdump 重算, 文件注释里给了命令: 
> ```bash
> objdump -d build/dissect | sed -n '/<ret_redirect>:/,/ret/p'
> ```
> 找 `lea -0xN(%rbp)`（buffer1 地址）→ `RET_OFFSET = 8 + N`; 
> 数 `call` 返回点之后两条赋值指令的字节数定 `RET_SKIP_DELTA`. 
> **这个“偏移要自己量”正是本阶段的核心技能**——论文时代如此, 现在依然如此. 

---

## 03 · `vulnerable.c` —— 被攻击的目标

**用法**: `./build/vulnerable <字符串>`; `VULN_DEBUG=1 ./build/vulnerable x`
会打印栈布局（`just leak`）. 

要害一行（真实世界对应 VVC 解码器 **CVE-2019-17542** 的代码形态: 
`const char *src → strcpy` 进固定栈数组）: 

```c
char buffer[128];
strcpy(buffer, argv[1]);   // 不检查 strlen(argv[1]) vs 128
```

**教学点**: 
- `VULN_DEBUG=1` 打印 `buffer` / `rsp` / `&ret slot` 地址, 模拟真实攻击里的
  **信息泄露**（等价于论文先跑 `sp.c` 探栈顶）. 真实攻击中地址靠泄露或爆破
  获得——这正是阶段 04 要解决的问题. 
- 函数末尾 `return 0;`: 若返回地址已被覆盖, `ret` 就把控制流交给 payload. 
- **为什么 x86_64 要 partial overwrite**: 文件顶部注释详细推导——用户态地址
  高 2 字节恒为 `\x00`, 整 8 字节会被 `strcpy` 截断, 故只覆盖低 6 字节, 
  让末尾 NUL 落在本就为 0 的第 7 字节. 

---

## 04 · `exploit.c` —— 攻击端

**用法**（公共参数 `-t 目标路径`）: 

| 命令 | 模式 | 作用 |
|------|------|------|
| `./build/exploit -x` | hex | 只打印 payload 布局与十六进制, 不执行（`just exploit` 先跑这个） |
| `./build/exploit -l` | leak | 借 `VULN_DEBUG=1` 向目标要真实地址, 用真地址打一发（`just leak` 近似） |
| `./build/exploit -a 0x7ffd...` | 单发 | 指定猜解地址, 打一发交互 shell |
| `./build/exploit -o 400` | 单发 | 用 `get_sp()-400` 作猜解地址, 打一发（`just getshell 400`） |
| `./build/exploit -z` | 自动爆破 | `offset` 从 0 向 ±max 扫, 步长 step, 命中即停（`just exploit` / `just noaslr`） |
| `-v` | 修饰 | 打印 `[*] exec ...` 与 `[*] marker received; shell confirmed` |

其余参数: `-s 步长`（默认 16）、`-m 搜索半径`（默认 ±2048）. 

**三块核心逻辑**: 

1. **`build_payload(out, guess)`** —— 组装 142 字节、无 NUL 的 payload: 
   `[NOP×78][shellcode×42][scratch×16][guess 低 6 字节(小端)]`. 组完会**自检
   有无 NUL**, 若猜解地址低 6 字节里出现 `0x00`（例如地址恰好对齐）则这一发
   跳过——终端上的 `[!] payload byte N is NUL` 是**设计内的跳过**, 不是错误. 

2. **`run_once(payload, len, target, probe, verbose)`** —— 单发执行, 两种模式: 
   - `probe=true`（自动爆破用）: `fork`+`exec` 目标, 用两条管道接管子进程的
     stdin/stdout, 父进程写 `echo BOF_SHELL_OK; exit\n`, 再从 stdout 读. 
     **读到 marker = 真的弹出了 shell**（只看退出码不行, 正常返回和 shell 退出
     都是 0）. 父进程写完立刻 `close` 写端, shell 读到 EOF 即退出, 避免卡死. 
     ⚠️ marker 被父进程**内部捕获**, 不显示在终端; 加 `-v` 看确认行. 
   - `probe=false`（单发交互）: 子进程**不做 dup2**, shell 的 stdio 直连你当前
     终端——命中后这个 shell 就是你的, 可以真正交互. 

3. **自动爆破循环**（`main` 里 `-z` 分支）: 以 `get_sp()` 为基准, 
   `d = 0, step, 2*step, ...` 向 `+d/-d` 两侧扫描猜解地址, 逐发 `run_once`, 
   命中打印 `[+] PWNED! offset ..., guess 0x...`. 
   **ASLR 关闭时几十发内必中; 开启时几乎打不中——这正是 ASLR 的意义. **

---

## 05 · `defense.c` —— 源码层防御（4 种写法）

**用法**: `./build/defense [1-4] <字符串>`（`just defense` 用 300 个 'A' 跑
case 1/2/3）. 构建用 `-fstack-protector-strong`（本阶段讨论的是**源码层**, 
canary 只是兜底）. 

| 参数 | 函数 | 写法 | 结论 |
|------|------|------|------|
| `1` | `safe_strncpy` | `strncpy(buf, src, sizeof(buf)-1)` + 手动补 `\0` | 安全, 但 `strncpy` 语义有坑（不自动补 NUL） |
| `2` | `safe_snprintf` | `snprintf(buf, sizeof(buf), "%s", src)` | **推荐**: 永远 NUL 结尾, 返回值可检测截断 |
| `3` | `safe_reject` | 先 `strlen` 检查, 超长直接拒绝 | 最保守: 宁可不处理也不截断（对应真实服务返回 400） |
| `4` | `still_vulnerable` | `strncpy(buf, src, strlen(src))` | **反面教材**: 长度取成源长度, 照样溢出 |

**核心教学点（case 4）**: 边界必须是**目标容量**（`sizeof(buffer)`）, 
**永远不能是源长度**（`strlen(src)`）. case 4 的 `strncpy` 看起来“带了长度”, 
实则和裸 `strcpy` 没区别——编译器不报错, 只有 canary 才拦得住. 这说明
“用了 `strncpy` 就安全”是错觉. 

---

## 05 · `canary_probe.c` —— Canary 的确定性演示

**构建两份对照**（`just build` 自动生成）: 
- `canary_probe` : `-fstack-protector-all`, 有哨兵
- `canary_probe_off` : `-fno-stack-protector`, 无哨兵

**用法**: `./build/canary_probe <show|smash [n]>`

| 命令 | 作用 | 有保护构建 | 无保护构建 |
|------|------|-----------|-----------|
| `show` | 打印本次运行的 canary 值 | 每次运行值不同（随机） | 读到的是未初始化槽, 演示意义有限 |
| `smash 200` | 向 64B 缓冲写 200B | 校验失败 → **SIGABRT(134)** | 踩穿到返回地址 → **SIGSEGV(139)** |

**原理**: 
- canary 由编译器 prologue 从 `%fs:0x28`（TLS）取出放进栈帧, **低字节恒为
  `0x00`**——专门阻止 `strcpy` 这类以 NUL 判结束的拷贝顺手泄露/伪造它. 
  `show_canary()` 直接读 `%fs:0x28` 打印, 就是栈帧里的那个值. 
- `smash(n)` 往 64B 缓冲写 n 字节; 溢出必然先踩坏返回地址前的 canary, 
  函数返回前校验失败 → `__stack_chk_fail` → `*** stack smashing detected ***`
  → `abort()`. 这解释了 `unsafe_canary` 为何是 SIGABRT 而非 SIGSEGV. 
- **canary 挡不住什么**（文件顶部注释）: 覆盖返回地址前就被用的局部指针、
  不经过 `ret` 的控制流劫持（如 GOT 改写）、信息泄露先读出 canary 再精确重写. 
  所以它是“提高门槛”, 要和 NX/ASLR 叠加. 

---

## 偏移会漂移, 结论不会

阶段 02 的 `RET_OFFSET_FROM_BUF` / `RET_SKIP_DELTA`、阶段 04 命中时的 `offset`, 
都随编译器、版本、优化级别、运行时栈布局变化. **数字不用背**: README §4 与各
文件注释都给了重算/重扫的方法. 会漂的是坐标, 不漂的是原理——这正是实验要你
内化的部分. 
