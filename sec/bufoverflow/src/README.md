# `src/` — 源码分区

| 文件 | 阶段 | 说明 |
|------|------|------|
| `common.h` | 共享 | 阶段间的函数指针表（目标/攻击端进程解耦） |
| `shellcode.h` | 共享 | x86_64 `execve("/bin/sh")` shellcode（42B，无 `\x00`） |
| `part1_unsafe.c` | 01 | 五种经典不安全函数的溢出行为演示 |
| `part2_dissect.c` | 02 | 栈布局观测 + 修改返回地址跳过语句（论文 example2/3 重演） |
| `part3_vulnerable.c` | 03 | 目标程序：`argv[1]` → `strcpy`（CVE-2019-17542 形态） |
| `part4_exploit.c` | 04 | 攻击端：NOP 雪橇 + 地址猜解 + 自动爆破驱动 |
| `part5_defense.c` | 05 | 同一漏洞的安全版本（`fgets` + 显式边界） |
| `canary_probe.c` | 05 | 观测 canary 值与触发 `__stack_chk_fail` |

构建产物：`../build/`（gitignore 内）。用 `just build` 一键编译，
各目标的编译标志见仓库根 `../Justfile`（脆弱目标与防御目标标志不同是刻意的）。
