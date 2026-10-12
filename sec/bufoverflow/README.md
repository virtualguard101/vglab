# bof-lab —— 栈缓冲区溢出完整攻防实验

> 教学用实验，复刻 Aleph One 经典论文 *Smashing The Stack For Fun And Profit*
> (Phrack 49-14)，并移植到 **x86_64 / 现代 glibc / GCC 16**。
> 目标：让初学者亲手走完一条**完整攻击链**，再逐层拆解**防御**为什么有效。

⚠️ **仅限授权学习环境**：所有靶子都在本机、无特权、自己攻击自己的进程。
不要把这里的技术用于任何未授权的系统。

---

## 0. 这个实验在教什么

一句话：**把"数据"写成了"代码的下一步地址"**。

```
   正常：  调用函数 → 执行 → ret 回到调用点继续
   溢出：  输入超长 → 淹掉栈上的返回地址 → ret 跳到攻击者指定的地方
   利用：  那个"地方"正是攻击者塞进缓冲区的机器码 → 拿到 shell
```

五个阶段，从"看见现象"到"亲手利用"再到"逐层防住"：

| 阶段 | 文件 | 学到什么 |
|------|------|----------|
| 01 | `src/part1_unsafe.c` | 哪些 C 函数不做边界检查、溢出长什么样 |
| 02 | `src/part2_dissect.c` | 栈帧布局 `[局部变量\|saved RBP\|返回地址]`，手改返回地址 |
| 03 | `src/part3_vulnerable.c` | 一个真实形态的脆弱目标（`strcpy(buf, argv[1])`） |
| 04 | `src/part4_exploit.c` | 完整 payload：NOP 雪橇 + shellcode + 地址猜解 + 自动爆破 |
| 05 | `src/part5_defense.c`, `src/canary_probe.c` | 四层防御：安全 API / Canary / NX / ASLR |

---

## 1. 快速开始

需要：`gcc`、`just`（或用下面的手动命令）、`gdb`/`objdump`（阶段 02 选学）。

```bash
cd bof-lab
just              # 列出所有命令
just build        # 编译全部靶子与工具 → build/

just demo-unsafe  # 01 看溢出现象
just demo-dissect # 02 看栈布局 + 改返回地址
just demo         # 04 完整攻击链（payload 布局 + 自动爆破）
just defense      # 05 四层防御逐一拦截
```

> **没有 `just`？** 每条配方就是普通 gcc 命令，直接看 `Justfile` 照抄即可。

攻击阶段要真正弹出 shell，需要**关闭 ASLR**（见 §4）：

```bash
just demo-noaslr  # 在 setarch -R 子环境里跑，命中率最高
```

---

## 2. 攻击链逐步拆解（阶段 04 的内部）

目标 `part3_vulnerable.c` 的要害只有一行：

```c
char buffer[128];
strcpy(buffer, argv[1]);   // 不检查长度 → 可写穿
```

栈上 `buffer` 之后紧跟着保存的 RBP 和返回地址：

```
低地址                                                        高地址
┌───────────────── buffer[128] ─────────────────┬─ saved RBP ─┬─ return addr ─┐
│ B+0                                       B+127│ B+128..135  │  B+136..143   │
└────────────────────────────────────────────────┴─────────────┴───────────────┘
                                                                 ▲
                                               我们要覆盖的就是这 8 个字节
```

**payload 布局（共 142 字节，全串无 `\x00`）：**

```
 [ NOP × 94 ][ shellcode × 42 ][ 猜解地址低 6 字节 ]
   0 .. 93     94 .. 135          136 .. 141
   ↑雪橇       ↑execve("/bin/sh")  ↑正好压在 return addr 的字节 0..5
```

四个关键设计，对应论文的四个难点在 64 位下的翻版：

1. **NOP 雪橇**（论文 exploit3）：地址猜不准没关系，只要落在 94 个 `0x90`
   里任意一个，CPU 就一路"滑"到 shellcode。把"猜一个精确地址"降级成
   "猜一个区间"。

2. **shellcode 无 NUL**（论文 "Shell Code" 一节）：`strcpy` 遇 `\x00` 截断，
   所以 shellcode 里的 `/bin/sh` 结束符由代码在运行时补写，不预置进数据。
   详见 `src/shellcode.h` 顶部的逐字节注释。

3. **partial overwrite**（论文没有、64 位特有）：用户态地址形如
   `0x00007fff_xxxxxxxx`，高 2 字节恒为 `\x00`。整 8 字节塞进 `strcpy`
   会被截断，所以只覆盖返回地址的**低 6 字节**，让 `strcpy` 自动补的结尾
   `\x00` 恰好落在本就为 0 的第 7 字节上——数据完整，地址也正确。

4. **命中检测**（真实 exploit 框架的做法）：自动模式 `fork`+`exec` 目标，
   用管道把 `echo <marker>; exit` 喂给它的 stdin，再从 stdout 收。
   **读到 marker = 真的弹出了 shell**（只看退出码不行，正常退出和 shell
   退出都是 0）。实现见 `run_once()`。

**自动爆破**（论文 exploit4）：以 `get_sp()` 为基准，`±step` 地向两侧扫描
返回地址猜测，直到命中。ASLR 关闭时几十发内必中；开启时几乎打不中——
**这正是 ASLR 的防御意义**。

---

## 3. 防御：为什么现代系统默认打不穿（阶段 05）

四层防御互相独立、层层设防。实验用"刻意矛盾"的编译标志把它们分别开关，
好让你看清每一层**单独**挡住了什么。

| 层 | 机制 | 挡住了 payload 的哪一步 | 本实验怎么看 |
|----|------|------------------------|--------------|
| 源码 | 边界检查 / `snprintf` | 根本不让溢出发生 | `part5_defense.c` |
| 编译器 | **Canary** `-fstack-protector` | 覆盖返回地址前先踩坏哨兵 → abort | `canary_probe.c` |
| 链接器/CPU | **NX** `-z noexecstack` | 栈不可执行 → shellcode 跳进去就段错误 | 见下方 |
| 内核 | **ASLR** | 栈地址随机 → 固定猜解几乎必不中 | `demo` vs `demo-noaslr` |
| 语言 | 内存安全语言 (Rust/Go) | 从根上消除这类 bug | 讨论 |

**亲手验证每一层：**

```bash
# Canary：有保护 → SIGABRT(134)，无保护 → SIGSEGV(139)
./build/canary_probe     smash 200   # *** stack smashing detected ***
./build/canary_probe_off smash 200   # 段错误
./build/canary_probe show            # 连跑两次，哨兵值不同（随机）

# NX：把目标用可执行栈 vs 不可执行栈各编一份，拿 exploit 打，对比结果
gcc -O0 -fno-stack-protector -no-pie -z noexecstack src/part3_vulnerable.c -o build/vuln_nx
setarch $(uname -m) -R ./build/exploit -z -t build/vuln_nx    # NX 下 shellcode 执行即崩，打不通

# ASLR：对比开/关
cat /proc/sys/kernel/randomize_va_space   # 2=完全开启（默认）
just demo          # ASLR 开：几乎打不中
just demo-noaslr   # ASLR 关：几十发内命中
```

**重点认知**：没有任何单层是"银弹"。
- Canary 挡经典返回地址覆盖，但挡不住信息泄露后精确重写、或不经过 `ret`
  的控制流劫持。
- NX 让 shellcode 失效，于是攻击者转向 **ROP**（复用已有代码片段）。
- ASLR 靠随机性，但一次信息泄露就可能把它废掉。

现代利用是"**泄露 → 绕 ASLR → ROP 绕 NX → 精确重写绕 Canary**"的组合拳。
纵深防御的意义在于**抬高门槛、增加需要串联的漏洞数量**。

---

## 4. 实验前提与编译标志（务必理解）

攻击阶段的靶子（`part3`）用了一组**刻意关闭现代防御**的标志，否则初学者
第一步就被 canary/NX/ASLR 挡住，看不到溢出的本质：

```
-O0                   不优化，栈布局可预测、好对照
-fno-stack-protector  关 canary（否则溢出被 __stack_chk_fail 拦）
-no-pie               非 PIE，代码/栈地址可预测（教学简化）
-z execstack          栈可执行（否则 shellcode 跳进去就被 NX 杀）
```

再加上运行时关闭 ASLR（`setarch -R` 或临时 `sysctl`），攻击才能稳定复现。
**把这四件事反过来做，就是 §3 的四层防御**——这不是巧合，而是本实验的主线。

> 临时全局关 ASLR（需要 root，实验后记得恢复）：
> ```bash
> echo 0 | sudo tee /proc/sys/kernel/randomize_va_space   # 关
> echo 2 | sudo tee /proc/sys/kernel/randomize_va_space   # 恢复
> ```
> 推荐优先用 `setarch $(uname -m) -R <cmd>`，只影响单个进程、无需 root。

---

## 5. 与论文的对应关系

| 论文 (i386, 1996) | 本实验 (x86_64, 2026) |
|-------------------|----------------------|
| `example2.c` 无界 `strcpy` | `part1` 的 `demo_strcpy` |
| `example3.c` 改 ret 跳过语句 | `part2` 的 `ret_redirect` |
| `int 0x80` 系统调用 | `syscall` 指令 |
| JMP/CALL/POP 取字符串地址 | `lea rdi,[rip-14]` 负向 RIP 相对寻址 |
| 4 字节返回地址直塞 | 6 字节 partial overwrite（高 2 字节是 NUL） |
| `exploit3.c` NOP 雪橇 | `build_payload()` 的 94 字节雪橇 |
| `exploit4.c` 偏移猜解 | `-z` 自动爆破 + shell 命中检测 |
| `sp.c` 探栈顶 | `get_sp()` / `VULN_DEBUG=1` 泄露 |

论文发表时这些防御都还不存在；读完本实验的 §3，你就能理解**这 30 年里
系统安全到底加固了什么**。

---

## 6. 目录结构

```
bof-lab/
├── Justfile            构建 + 分阶段演示（编译标志的"刻意矛盾"在此）
├── README.md           本文件
├── src/
│   ├── common.h        阶段间共享定义
│   ├── shellcode.h     x86_64 execve("/bin/sh")，42B 无 NUL，逐字节注释
│   ├── part1_unsafe.c  01 不安全函数
│   ├── part2_dissect.c 02 栈布局 + 改返回地址
│   ├── part3_vulnerable.c 03 脆弱目标
│   ├── part4_exploit.c 04 攻击端
│   ├── part5_defense.c 05 安全重写
│   ├── canary_probe.c  05 canary 观测
│   └── README.md       src 分区说明
└── build/              构建产物（gitignore）
```

## 7. 延伸阅读

- Aleph One, *Smashing The Stack For Fun And Profit*, Phrack 49-14（本实验底本，见 `../stack_smashing.pdf`）
- Nergal, *The advanced return-into-lib(c) exploits*（ROP 前身，绕 NX）
- *The Geometry of Innocent Flesh on the Bone* (Shacham, 2007)（ROP 奠基）
- Linux `mitigations`: `checksec`、`scanmem`、`pwntools`（进一步实战工具链）
