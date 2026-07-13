# RISC-V / 计算机系统学习建议

> 面向：希望系统学习 RISC-V、计算机系统与体系结构的初学者  
> 三线协同：**CSAPP**（程序员视角的系统）· **CS61C**（RISC-V 组成与组织）· **一生一芯 YSYX**（从门电路到处理器实现）  
> 辅线：Logisim-Evolution 开源对照项目  
> 更新日期：2026-07-14

---

## 1. 三条线各自解决什么问题

| 资源 | 视角 | 强项 | 弱项 / 注意 |
|------|------|------|-------------|
| **[CSAPP](https://csapp.cs.cmu.edu/)**《深入理解计算机系统》 | 程序员 → 机器 | 位运算、机器级程序、链接、异常、虚存、并发；配套 lab 成熟 | 汇编以 **x86-64** 为主；处理器章（Y86）可略读，实现深度不如 YSYX |
| **[CS61C](https://cs61c.org/)** Great Ideas in Computer Architecture | 组成原理 + RISC-V | **RISC-V 汇编**、Logisim CPU、Cache、并行；与 YSYX ISA 同族 | 课程作业框架随学期变；自学用公开讲义/往期 proj |
| **[一生一芯](https://ysyx.oscc.cc/)** | 做出能跑的处理器 | F→A 贯通：Logisim → NEMU → RTL/NPC → SoC → 流水线；强调独立调试 | 坡度陡；以当期 [讲义](https://ysyx.oscc.cc/docs/) 为准 |

**一句话分工：**

- **CSAPP**：搞懂「我写的 C 在系统里怎么跑、怎么崩、怎么优化」。  
- **CS61C**：搞懂「RISC-V 机器长什么样、CPU/Cache 怎么组织」。  
- **YSYX**：亲手把「指令 → 电路 → 仿真 → SoC」做出来。

不要三门课「整本并行从头刷完」。按下面阶段，**同一主题用不同材料互补**；同时打开的主任务 ≤ 2 个。

---

## 2. 总原则

1. **实现主线 = 一生一芯**；理论与软件系统 = CSAPP；RISC-V 组织与 Logisim CPU = CS61C。  
2. **汇编优先学 RISC-V（CS61C）**，CSAPP Ch3 用来迁移概念（栈帧、调用约定、指针）；不必先啃完 x86 再碰 RISC-V。  
3. **Logisim / 课设仓库是对照物**，不替代 YSYX F5/F6 或 CS61C Proj3 的自学实现。  
4. **先能跑、再能解释、再能自己改**；每个阶段能口述一条指令或一次函数调用的完整路径。  
5. **STFW / RTFM / 科学提问**（YSYX F1）；CSAPP lab 与仿真调试同理。

---

## 3. 主题映射总表

| 主题 | CSAPP | CS61C | 一生一芯 | 建议顺序 |
|------|-------|-------|----------|----------|
| 位级 / 数据类型 | Ch2 + Data Lab | 数制、C 位运算 | F3 组合逻辑基础 | CSAPP Ch2 ↔ 并行 F3 前半 |
| C 与 Linux 工具 | Ch1；全书贯穿 | C 复习、工具链 | E1、E3 | 贯穿 |
| 机器级程序 | Ch3（x86 概念） | **RISC-V 汇编 + Proj2** | E4、D2 | **先 CS61C RISC-V，再对照 CSAPP Ch3** |
| 处理器实现 | Ch4 Y86（略读） | Lab5–6、**Proj3** Logisim CPU | **F5–F6、D4、C2、B5** | YSYX F 自研 → CS61C Proj3 巩固 → YSYX RTL |
| 优化 / 流水线直觉 | Ch5 | 流水线 lab、冒险 | B5 | CS61C / YSYX 做完再读 CSAPP Ch5 |
| 存储器层次 | Ch6 + Cache Lab | Cache 讲义/lab | B4 | 三者同主题对齐 |
| 链接 / ELF | Ch7 | 目标文件相关 | C4、E4 | CSAPP Ch7 ↔ YSYX C4 |
| 异常 / 进程 / I/O | Ch8、Ch10 | 中断/系统概念（较浅） | C5、D5 | CSAPP 为主，YSYX 做硬件侧 |
| 虚存 / malloc | Ch9 + Malloc Lab | VM 讲义 | A 阶段虚存等 | CSAPP 先建立模型，YSYX A 再落地 |
| 并发 | Ch12 + Shell Lab | 并行 / SIMD / WSC | （非核心） | 处理器主线稳定后再做 |

---

## 4. 分阶段学习路径

### 阶段 0：环境与心态（约 1 周）

| 做什么 | 资源 |
|--------|------|
| 读 YSYX 理念与路线 | [项目概述](https://ysyx.oscc.cc/project/intro.html) · [报名](https://ysyx.oscc.cc/signup/)（可选） |
| 科学提问 | YSYX F1 |
| 装 Logisim-Evolution | [F2](https://ysyx.oscc.cc/docs/2407/f/2.html) |
| 浏览 CSAPP / CS61C 目录 | [CSAPP](https://csapp.cs.cmu.edu/) · [CS61C](https://cs61c.org/)（选一学期公开材料，如 [SP26 labs](https://cs61c.org/sp26/labs/lab05/)） |

本仓库 `csa/` 可继续练组合逻辑。

**完成标准**：Logisim 能仿真；知道三线各自负责什么。

---

### 阶段 1：数据表示 + 数字逻辑地基（约 3～5 周）

**目标**：位与门电路不再是两张皮。

| 优先级 | 内容 | 产出 |
|--------|------|------|
| 主 | YSYX **F2–F3**（Logisim + 数字逻辑必做） | 组合/时序/FSM 习题 |
| 主 | CSAPP **Ch2** + **Data Lab** | `bits.c` 过测试；理解补码、浮点直觉 |
| 辅 | CS61C 数制与 C 位运算讲义（若有） | 与 Ch2 对照笔记一页即可 |

**刻意并行**：每天可「上午 Data Lab 一题 + 晚上 F3 一小节」，主题都是「信息如何编码」。

**完成标准**：Data Lab 核心题完成；能解释时序电路与组合电路差异。

---

### 阶段 2：机器级程序（RISC-V 优先）（约 4～6 周）

**目标**：会读会写 RISC-V 汇编，并理解调用约定与栈；x86 只作概念迁移。

| 优先级 | 内容 | 产出 |
|--------|------|------|
| 主 | CS61C：**RISC-V 汇编**（Venus 等）+ **Project 2**（或同等 asm 大作业） | 手写函数、调用约定、简单程序 |
| 主 | YSYX **E1**（C）**E3**（Linux）**E4**（C→二进制 / 模拟器思想） | 工具链能编 RISC-V；理解 ELF 粗轮廓 |
| 辅 | CSAPP **Ch3**（选读：过程、栈、数组/结构、缓冲溢出思想） | 用 RISC-V 例子重画 CSAPP 插图 |
| 可选 | CSAPP Bomb Lab / Attack Lab | **用 x86 练调试与逆向**；时间紧可推迟到阶段 5 |

**完成标准**：能把一段含指针/递归的 C 对应到 RISC-V；会用 `objdump`/`gdb`（或 Venus）单步。

---

### 阶段 3：从「理解 CPU」到「画出 CPU」（约 6～10 周）— 本路径核心

**目标**：建立「指令控制状态机」并做出可运行处理器（先图形化，再巩固）。

#### 3A — 一生一芯 F 阶段（必须自己做）

| 任务 | 讲义 | 要点 |
|------|------|------|
| F4 | 状态机模型 | 程序 = 指令驱动状态转移 |
| F5 | [sCPU](https://ysyx.oscc.cc/docs/2407/f/5.html) | 译码、GPR、ALU、分支 |
| F6 | [minirv](https://ysyx.oscc.cc/docs/2407/f/6.html) | 8 条：`add` `addi` `lui` `lw` `lbu` `sw` `sb` `jalr` |

#### 3B — CS61C CPU 轨道（巩固与对照）

| 内容 | 用途 |
|------|------|
| [Lab 5 Logisim](https://cs61c.org/sp26/labs/lab05/) | 子电路、splitter、隧道；与 F2/F3 互补 |
| [Lab 6](https://cs61c.org/sp26/labs/lab06/) | 立即数、控制信号、流水线直觉 |
| **Project 3**（CS61CPU） | ALU / RegFile / 数据通路 / 两级流水；中文整理见 [CS 自学社区](https://www.learncs.site/docs/curriculum-resource/cs61c/projects/proj3) |

**建议顺序**：先独立完成 **YSYX F5–F6**，再做 CS61C Lab5–6 / Proj3（指令更全、测试更全，适合查漏补缺）。若 F3 很吃力，可先做 Lab5 再回 F5。

#### 3C — Logisim 开源对照（F6 完成后再看）

| 优先级 | 仓库 | 用途 |
|--------|------|------|
| ★★★ | [meiniKi/RV32I_SC_Logisim](https://github.com/meiniKi/RV32I_SC_Logisim) | 极简单周期全貌 |
| ★★★ | [daleydeng/CircuitRISCV](https://github.com/daleydeng/CircuitRISCV) | RV32IM + 外设；[B 站](https://www.bilibili.com/video/BV1Lc411B7zD/) |
| ★★ | [leijurv/Logisim-RISC-V](https://github.com/leijurv/Logisim-RISC-V) | C → ROM image |
| ★ | SAP-1 / 8-bit 教学机 | 仅当取指–执行直觉仍弱时 |

**不要在此阶段深挖**：华科五级流水线课设（留给阶段 6）。

#### 3D — 读物

- CSAPP **Ch4** 略读（数据通路思想即可，不必纠结 Y86 细节）。  
- 《计算机组成与设计：硬件/软件接口》（Patterson/Hennessy，**RISC-V 版**）与 CS61C 同步最合适。

**完成标准**：minirv 能跑；能讲清至少一条 load/store 与一条 ALU 指令的通路；CS61C Proj3 或同等实现至少完成到可跑子集指令。

---

### 阶段 4：预学习收束 — 仿真器与入学（约 4～8 周）

| 内容 | 对齐 |
|------|------|
| YSYX **E2** HDL、**E5** Verilator 仿真 | 为 RTL 做准备 |
| YSYX **E6 PA1** | 入学答辩关键 |
| CSAPP：复习 Ch1–3 薄弱点；**Ch7 链接**预习（为 C4） | 软件侧补强 |

**完成标准**：按当期 YSYX 预学习清单勾选完毕，准备答辩。

---

### 阶段 5：NEMU / RTL / 单周期 NPC + 系统软件入门（约 8～14 周）

| YSYX | CSAPP / CS61C 配套 |
|------|-------------------|
| D1 RV32IM NEMU | 把 CS61C 汇编知识写进模拟器；DiffTest 的「黄金模型」 |
| D2–D3 机器级表示、AM | CSAPP Ch3 过程调用 + Ch7 对象文件直觉 |
| D4 [RTL minirv](https://ysyx.oscc.cc/docs/2407/d/4.html) | 把 F6 电路「翻译」成 HDL |
| D5 设备 I/O | CSAPP Ch10 系统级 I/O（选读） |
| C2 单周期 NPC | 与 CS61C 单周期/Proj3 对照 |
| C3–C5 调试、ELF、异常 / RT-Thread | **CSAPP Ch7 精读**；Ch8 异常控制流；Bomb Lab（若未做）练调试 |

**完成标准**：NEMU + NPC 跑通阶段程序；会差分测试；能解释一条系统调用/异常在软硬件两侧大概发生什么。

---

### 阶段 6：存储器层次、流水线、SoC（约 10～16 周）

| 主题 | 学法 |
|------|------|
| Cache | CSAPP **Ch6 + Cache Lab** ∥ CS61C Cache ∥ YSYX **B4** |
| 流水线 | YSYX **B5** 为主；CS61C Lab6 / Proj3 流水部分复习；CSAPP Ch4/Ch5 补性能直觉 |
| 总线 / SoC | YSYX **B1–B3** |
| 流水线对照仓库 | [HUST_RISCV-CPU](https://github.com/walkalone20/HUST_RISCV-CPU)、[HUST 硬件综合训练](https://github.com/fly-lovest/HUST-CS-Comprehensive-hardware-training)、[poorpool/riscv-cpu](https://github.com/poorpool/riscv-cpu) — **B5 前后打开** |

**完成标准**：能口述命中/缺失与写策略；能说明数据冒险与转发；YSYX B 阶段考核按官方清单。

---

### 阶段 7：虚存、OS 接口与进阶（持续）

| 内容 | 对齐 |
|------|------|
| CSAPP **Ch9 + Malloc Lab** | 虚存与分配器必须亲手做 |
| CSAPP **Ch8 Shell Lab**（可选 Ch12） | 进程、信号、并发入门 |
| YSYX **A 阶段**（虚存、特权级、xv6、Linux 等） | 把 CSAPP 模型落到真实 ISA/OS |
| CS61C 并行 / WSC（选学） | 扩展视野，非处理器主线必需 |

**完成标准**：Malloc Lab 可用；能解释 VA→PA 与缺页；YSYX A 按自选认证等级推进。

---

## 5. 建议时间表（弹性）

假设每周 **10～15 小时**、基础偏弱：

| 周期 | 焦点 | 产出物 |
|------|------|--------|
| 第 1 周 | 阶段 0 + F2 | 环境；提问读后感草稿 |
| 第 2–5 周 | 阶段 1：F3 + CSAPP Ch2/Data Lab | bits 练习 + 数字逻辑习题 |
| 第 6–10 周 | 阶段 2：CS61C RISC-V + YSYX E1/E3/E4 | 汇编小项目；工具链 |
| 第 11–18 周 | 阶段 3：F4–F6 → CS61C Lab/Proj3 | minirv `.circ`；可选对照一仓库 |
| 第 19–24 周 | 阶段 4：E2/E5/PA1 | 入学答辩 |
| 之后 | 阶段 5→6→7 | 按 YSYX D/C/B/A + CSAPP 相应章 |

基础好可压缩阶段 1–2；**不要跳过 F6** 直接抄流水线仓库。

---

## 6. 若时间冲突：砍什么、留什么

| 若只能保一条实现线 | 保留 | 可砍 / 推迟 |
|--------------------|------|-------------|
| 以「做出芯片/CPU」为目标 | **YSYX 全流程** | CSAPP Attack；CS61C 并行/WSC；Bomb 可后做 |
| 以「系统程序员」为目标 | **CSAPP 全书 + labs** | YSYX B/A 深流水与流片；仍建议做完 F6 + 一点 RISC-V |
| 以「组成原理课」为目标 | **CS61C + YSYX F/D/C** | CSAPP Ch10–12；Malloc 可与 A 阶段再对齐 |

**最低配（仍算入门完整）**：CSAPP Ch2–3、Ch6–7、Ch9（选） + CS61C RISC-V 与 Proj3 + YSYX F 全程与 PA1。

---

## 7. 配套书籍与工具

**书**

| 书 | 用途 |
|----|------|
| CSAPP（深入理解计算机系统） | 阶段 1/2/5/6/7 主教材之一 |
| Computer Organization and Design（**RISC-V** 版，P&H） | 与 CS61C / YSYX 组成部分同步 |
| 《RISC-V 开放架构设计之道》 | ISA 与设计思想（YSYX 亦推荐） |
| 《计算机系统——基于 RISC-V+Linux 平台》（袁春风等） | 与 YSYX 生态接近 |
| K&R《C 程序设计语言》 | E1 / CS61C C 底子 |

**工具**

- Logisim-Evolution  
- RISC-V GNU 工具链 / Venus（CS61C）  
- Verilator、NEMU、AM、NPC（YSYX）  
- `gdb` / `objdump`；CSAPP lab 自带驱动脚本  

---

## 8. 检查清单

**阶段 1–2**

- [ ] Data Lab（或同等位运算练习）完成核心题  
- [ ] 能读写 RISC-V 汇编并说明调用约定  
- [ ] Linux + C 达到 YSYX E 必做水平  

**阶段 3–4**

- [ ] F5 sCPU、F6 minirv 独立完成  
- [ ] CS61C Lab5–6 或 Proj3（至少子集）完成  
- [ ] 对照过 ≤1 个单周期参考 `.circ` 并写差异笔记  
- [ ] PA1 完成并准备入学答辩  

**阶段 5–6**

- [ ] NEMU / NPC 跑通规定程序；会 DiffTest  
- [ ] CSAPP Ch7 链接能讲清重定位与符号  
- [ ] Cache Lab（或 CS61C Cache + YSYX B4）能解释命中过程  
- [ ] 流水线冒险与转发能口述  

**阶段 7**

- [ ] Malloc Lab（或同等分配器）  
- [ ] 虚存与 YSYX A 阶段目标对齐推进  

---

## 9. 常见误区

| 误区 | 建议 |
|------|------|
| CSAPP、CS61C、YSYX 三门同时从头刷 | 按阶段主题对齐；同时主任务 ≤ 2 |
| 先把 CSAPP x86 汇编学「完美」再碰 RISC-V | **先 RISC-V**；Ch3 当概念书 |
| 用开源流水线 `.circ` 代替 F6 / Proj3 | 先自研再对照 |
| 只看书不做 lab / 不仿真 | CSAPP lab、CS61C proj、YSYX 讲义任务三者都要有可运行产物 |
| 阶段 3 就啃华科五级流水 + Cache + 总线 | 推到阶段 6，与 B4/B5 对齐 |

---

## 10. 本仓库落点

| 路径 | 建议用途 |
|------|----------|
| `csa/` | F2–F3、CS61C Lab 风格小电路；逐步加 ALU/RegFile |
| `docs/scratch/` | 本规划、周计划、三线对照笔记 |
| `cca/`（若启用） | 流水线/Cache 笔记与示意图 |
| （自建）`csapp-labs/`、`cs61c/` | 本地 lab 工作区，勿与 YSYX 实验目录混在一起 |

---

## 11. 本周可执行的下一步

1. 收藏：[YSYX 课程主页](https://ysyx.oscc.cc/docs/) · [CSAPP](https://csapp.cs.cmu.edu/) · [CS61C](https://cs61c.org/)（选定一学期公开 lab/proj）。  
2. 完成 YSYX F2；开始 F3。  
3. 同步开始 CSAPP Ch2，并开 Data Lab 第一题。  
4. 阶段 1 结束后再进入 CS61C RISC-V，不要提前并行第三条大线。  

---

## 参考链接速查

| 类型 | 链接 |
|------|------|
| 一生一芯 | https://ysyx.oscc.cc/ · https://ysyx.oscc.cc/docs/ |
| YSYX F2 / F5 / F6 / D4 | https://ysyx.oscc.cc/docs/2407/f/2.html 等 |
| CSAPP | https://csapp.cs.cmu.edu/ · [Labs](https://csapp.cs.cmu.edu/3e/labs.html) |
| CS61C | https://cs61c.org/ · Lab5/Lab6/Proj3 以当期站点为准 |
| CS61C Proj3 中文整理 | https://www.learncs.site/docs/curriculum-resource/cs61c/projects/proj3 |
| Logisim-Evolution | https://github.com/logisim-evolution/logisim-evolution |

*本文为个人学习规划草稿。一生一芯考核以当期官方讲义为准；CS61C 作业以所选学期公开材料为准；CSAPP lab 注意版权与获取渠道。*
