# 栈内存布局 (进程入口 _start 处, 由内核构造):
#   高地址
#   +-----------------+
#   | ...环境变量串...|
#   | "<str>\0"       |  <-- argv[1] 指向这里 (字符串本体, 以 \0 结尾)
#   | "./stack\0"     |  <-- argv[0] 指向这里
#   +-----------------+
#   | NULL            |  envp 数组结尾
#   | envp[0] ...     |
#   | NULL            |  [rsp+24]  argv 数组结尾哨兵
#   | argv[1]         |  [rsp+16]  指针, 指向 "hello\0"
#   | argv[0]         |  [rsp+8]   指针, 指向程序名
#   | argc            |  [rsp]     参数个数 (整数, 不是指针)
#   +-----------------+  <-- rsp 初始指向这里
#   低地址

.intel_syntax noprefix

.global _start

.section .text

_start:
    # ---- 阶段 1: 用 pop 依次消费栈上的启动数据 ----
    ; # pop reg 等价于两步: mov reg, [rsp] # add rsp, 8
    pop rbx                     # rbx = argc, rsp 前进到 argv[0] (见上图, 即 [rsp+8] -> [rsp])
                                # 选 rbx 是因为 syscall 不会破坏它 (只破坏 rcx/r11/rax)
    pop rax                     # rax = argv[0] (程序名指针), 丢弃, rsp 前进到 argv[1]
    dec rbx                     # rbx = argc - 1, 记录剩余待输出的参数个数

.Lnext_arg:
    test rbx, rbx               # 参数输出完了吗? (没传参时 argc-1 == 0, 直接结束)
    jz .Lexit
    pop rsi                     # rsi = argv[i] (指针, 指向字符串本体), rsp 指向下一个 argv
                                # 这是第 1 次解引用的 pop 版本: 读栈 + 移动 rsp 一条指令完成
                                # rsi 之后将直接作为 write 的 buf

    xor rdx, rdx                # rdx = 0, 长度计数器, 同时是 write 的 count 参数

# 手写 strlen: 从 argv[i] 起逐字节扫描, 直到遇到 '\0'
.Lstrlen:
    cmp byte ptr [rsi+rdx], 0   # 第 2 次解引用: 按字节读字符串本体 (基址 rsi + 变址 rdx)
                                # byte ptr 必不可少: 立即数 0 无法推断访问宽度
    je .Lwrite                  # 遇到 '\0', rdx 恰为字符串长度
    inc rdx
    jmp .Lstrlen

.Lwrite:
    mov rax, 1                  # rax = SYS_write
    mov rdi, 1                  # arg0 fd = stdout
                                # arg1 rsi = buf, arg2 rdx = len, 均已就位
    syscall                     # write(1, argv[i], len)# rax 被返回值覆盖, rsi/rbx 不变

    # ---- 阶段 2: 用 push / pop 借栈临时存放一个 '\n' ----
    push 10                     # rsp -= 8, 再把 8 字节 0x000000000000000a 写入 [rsp]
                                # 小端序: 最低地址的第 1 个字节正是 0x0a ('\n')
    mov rax, 1                  # rax = SYS_write (被上次 syscall 覆盖, 需重设)
    mov rdi, 1                  # fd = stdout
    mov rsi, rsp                # buf = 栈顶地址, 即刚压入的 '\n' 所在处
                                # 这里要的是地址本身, 所以是 mov 而不是解引用
    mov rdx, 1                  # count = 1, 只输出最低字节
    syscall                     # write(1, rsp, 1)
    pop rax                     # 弹出并丢弃那 8 字节, rsp 恢复原值
                                ; # push/pop 必须成对, 否则 rsp 会错位, 后面读到的就不是 argv 了

    dec rbx                     # 剩余参数个数减 1
    jmp .Lnext_arg              # 回到循环头, 此时 rsp 正好指向下一个 argv[i+1]

.Lexit:
    mov rdi, 0                  # 退出码 0
    mov rax, 60                 # rax = SYS_exit
    syscall                     # 不返回. _start 不是函数, 不能用 ret
                                # (入口处栈顶是 argc 而不是返回地址)
