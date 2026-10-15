# bufoverflow

> 教学用实验, Aleph One 经典论文 [*Smashing The Stack For Fun And Profit*](stack_smashing.pdf) 的 **x86_64 / 现代 glibc / GCC 16**复刻.

## 原理概述

简单来说就是**把"数据"写成了"代码的下一步地址"**.

```
   正常:  调用函数 → 执行 → ret 回到调用点继续
   溢出:  输入超长 → 淹掉栈上的返回地址 → ret 跳到攻击者指定的地方
   利用:  那个"地方"正是攻击者塞进缓冲区的机器码 → 拿到 shell
```

源码概览:

| 阶段 | 文件 | 构建产物 | 学到什么 |
|------|------|----------|----------|
| 01 | `src/unsafe.c` | `unsafe` / `unsafe_canary` | 哪些 C 函数不做边界检查, 溢出的具体表现 |
| 02 | `src/dissect.c` | `dissect` | 栈帧布局 `[局部变量\|saved RBP\|返回地址]`, 手改返回地址 |
| 03 | `src/vulnerable.c` | `vulnerable` | 一个真实形态的脆弱目标(`strcpy(buf, argv[1])`) |
| 04 | `src/exploit.c` | `exploit` | 完整 payload: NOP 雪橇 + shellcode + 地址猜解 + 自动爆破 |
| 05 | `src/defense.c`, `src/canary_probe.c` | `defense` / `canary_probe` / `canary_probe_off` | 四层防御: 安全 API / Canary / NX / ASLR |

详解见[`src/README.md`](src/README.md).

---

## 快速开始

```bash
just              # 列出所有命令
just build        # 编译全部靶子与工具 → build/

just unsafe       # 01 看溢出现象
just dissect      # 02 看栈布局 + 改返回地址
just exploit      # 04 完整攻击链(payload 布局 + 自动爆破)
just defense      # 05 四层防御逐一拦截
```

攻击阶段要真正弹出 shell, 需要**关闭 ASLR**:

```bash
just exploit  # 在 setarch -R 子环境里跑, 命中率最高
```

---

## 攻击链详解

目标 `vulnerable.c` 的要害只有一行:

```c
char buffer[128];
strcpy(buffer, argv[1]);   // 不检查长度 → 可写穿
```

栈上 `buffer` 之后紧跟着保存的 RBP 和返回地址:

```
低地址                                                                    高地址
┌───────────────── buffer[128] ─────────────────┬─ saved RBP ─┬─ return addr ─┐
│ B+0                                      B+127│ B+128..135  │  B+136..143   │
└───────────────────────────────────────────────┴─────────────┴───────────────┘
                                                                ▲
                                返回地址槽占 8 字节; 我们只覆盖其低 6 字节
```

**payload 布局(142 Bytes, 全串无 `\x00`):**

```
 [ NOP × 78 ][ shellcode × 42 ][ scratch × 16 ][ 猜解地址低 6 字节 ]
   0 .. 77      78 .. 119         120 .. 135       136 .. 141
 ↑NOP Sled    ↑execve("/bin/sh") ↑给shellcode   ↑正好压在return addr
                                  预留的push缓冲  的字节0..5
 ( strcpy 末尾自动补的 \x00 落到字节 142(ret 槽第 7 字节), 本就为 0, 数据完整 )
```

几个关键设计如下, 对应论文的难点在x86_64下的翻版:

1. **NOP 雪橇**(论文 exploit3): 地址猜不准没关系, 只要落在 78 个 `0x90` 里任意一个, CPU 就一路"滑"到 shellcode. 把"猜一个精确地址"降级成 "猜一个区间". (紧挨 shellcode 之后还留了 16 字节 `scratch`, 防止 shellcode 的 `push` 往低地址踩到即将执行的指令).

2. **shellcode 无 NUL**(论文 "Shell Code" 一节): `strcpy` 遇 `\x00` 截断, 所以 shellcode 里的 `/bin/sh` 结束符由代码在运行时补写, 不预置进数据. 详见 `src/shellcode.h` 顶部的逐字节注释. 

3. **partial overwrite**(论文没有, x86_64特有): 用户态地址形如 `0x00007fff_xxxxxxxx`, 高 2 字节恒为 `\x00`. 整 8 字节塞进 `strcpy` 会被截断, 所以只覆盖返回地址的**低 6 字节**, 让 `strcpy` 自动补的结尾 `\x00` 恰好落在本就为 0 的第 7 字节上——数据完整, 地址也正确. 

4. **命中检测**(真实 exploit 框架的做法): 自动模式 `fork`+`exec` 目标, 用管道把 `echo <marker>; exit` 喂给它的 stdin, 再从 stdout 返回. **读到 marker = 真的弹出了 shell**(只看退出码不行, 正常退出和 shell 退出都是 0). 注意 marker 在自动模式下被父进程经管道**内部捕获**, 不会显示在终端; 加 `-v` 可看到 `[*] marker received; shell confirmed` 这行确认. 实现见 `run_once()`. 

5. **自动爆破**(论文 exploit4): 以 `get_sp()` 为基准, `±step` 地向两侧扫描返回地址猜测, 直到命中. ASLR 关闭时几十发内必中; 开启时几乎打不中——这正是 ASLR 的防御意义. 

---

## 现代操作系统的防御措施

| 层 | 机制 | 挡住了 payload 的哪一步 | 本实验怎么看 |
|----|------|------------------------|--------------|
| 源码 | 边界检查 / `snprintf` | 根本不让溢出发生 | `defense.c` |
| 编译器 | **Canary** `-fstack-protector` | 覆盖返回地址前先踩坏哨兵 → abort | `canary_probe.c` |
| 链接器/CPU | **NX** `-z noexecstack` | 栈不可执行 → shellcode 跳进去就段错误 | 见下方 |
| 内核 | **ASLR** | 栈地址随机 → 固定猜解几乎必不中 | `exploit` 开/关 ASLR 对比 |
| 语言 | 内存安全语言 (Rust/Go) | 从根上消除这类 bug | 讨论 |

快速验证:

```bash
# Canary:有保护 → SIGABRT(134), 无保护 → SIGSEGV(139)
./build/canary_probe     smash 200   # *** stack smashing detected ***
./build/canary_probe_off smash 200   # 段错误
./build/canary_probe show            # 连跑两次, 哨兵值不同(随机)

# NX:把目标用可执行栈 vs 不可执行栈各编一份, 拿 exploit 打, 对比结果
gcc -O0 -fno-stack-protector -no-pie -z noexecstack src/vulnerable.c -o build/vuln_nx
setarch $(uname -m) -R ./build/exploit -z -t build/vuln_nx    # NX 下 shellcode 执行即崩, 打不通

# ASLR:对比开/关
cat /proc/sys/kernel/randomize_va_space   # 2=完全开启(默认)
just exploit          # ASLR 开:几乎打不中
just noaslr   # ASLR 关:几十发内命中
```

**重点认知**: 没有任何单层是"银弹". 
- Canary 挡经典返回地址覆盖, 但挡不住信息泄露后精确重写、或不经过 `ret`
  的控制流劫持. 
- NX 让 shellcode 失效, 于是攻击者转向 **ROP**(复用已有代码片段). 
- ASLR 靠随机性, 但一次信息泄露就可能把它废掉. 

在现代操作系统上想要利用, 就需要"**泄露 → 绕 ASLR → ROP 绕 NX → 精确重写绕 Canary**"的组合拳. 

纵深防御的意义在于**抬高门槛、增加需要串联的漏洞数量**. 

---

## 实验前提与编译标志

攻击阶段的靶程序(`vulnerable.c`)用了一组**刻意关闭现代防御**的标志:

```
-O0                   不优化, 栈布局可预测、好对照
-g                    带调试信息, 便于 objdump/gdb 对照
-fno-stack-protector  关 canary(否则溢出被 __stack_chk_fail 拦)
-no-pie               非 PIE, 代码/栈地址可预测
-z execstack          栈可执行(否则 shellcode 跳进去就被 NX 杀)
```

再加上运行时关闭 ASLR(`setarch -R` 或临时 `sysctl`), 攻击才能稳定复现.

临时全局关 ASLR 可用以下命令:
```bash
echo 0 | sudo tee /proc/sys/kernel/randomize_va_space   # 关
echo 2 | sudo tee /proc/sys/kernel/randomize_va_space   # 恢复
```

推荐优先使用 `setarch $(uname -m) -R <cmd>`, 只影响单个进程、无需 root. 

---

## 论文环境对比

| 论文 (i386, 1996) | 本实验 (x86_64, 2026) |
|-------------------|----------------------|
| `example2.c` 无界 `strcpy` | `unsafe.c` 的 `demo_strcpy` |
| `example3.c` 改 ret 跳过语句 | `dissect.c` 的 `ret_redirect` |
| `int 0x80` 系统调用 | `syscall` 指令 |
| JMP/CALL/POP 取字符串地址 | `lea rdi,[rip-14]` 负向 RIP 相对寻址 |
| 4 字节返回地址直塞 | 6 字节 partial overwrite(高 2 字节是 NUL) |
| `exploit3.c` NOP 雪橇 | `build_payload()` 的 78 字节雪橇 |
| `exploit4.c` 偏移猜解 | `-z` 自动爆破 + shell 命中检测 |
| `sp.c` 探栈顶 | `get_sp()` / `VULN_DEBUG=1` 泄露 |

---

## 目录结构

```
bufoverflow/
├── Justfile            构建 + 分阶段演示(编译标志的"刻意矛盾"在此)
├── README.md           本文件
├── LICENSE             免责声明
├── stack_smashing.pdf  Aleph One 论文底本
├── src/
│   ├── common.h        阶段间共享定义
│   ├── shellcode.h     x86_64 execve("/bin/sh"), 42B 无 NUL, 逐字节注释
│   ├── unsafe.c  01    不安全函数(5 个测试用例)
│   ├── dissect.c 02    栈布局 + 改返回地址
│   ├── vulnerable.c 03 脆弱目标
│   ├── exploit.c 04    攻击端
│   ├── defense.c 05    安全重写(4 个测试用例)
│   ├── canary_probe.c  05 canary 观测
│   └── README.md       src 逐文件用法与原理详解
└── build/              构建产物(gitignore; just build 生成 8 个可执行文件)
```

## 参考文献

- [Aleph One, *Smashing The Stack For Fun And Profit*, Phrack 49-14](stack_smashing.pdf)

- Nergal, *The advanced return-into-lib(c) exploits*(ROP 前身, 绕 NX)

- *The Geometry of Innocent Flesh on the Bone* (Shacham, 2007)(ROP 奠基)

- Linux `mitigations`: `checksec`、`scanmem`、`pwntools`
