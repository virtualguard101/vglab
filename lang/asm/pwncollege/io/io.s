.intel_syntax noprefix

.global _start

.section .rodata
path:
    .asciz "out.txt" 

.section .text
_start:
    sub rsp, 256                # 在栈上预留 256 字节作为输入缓冲区

    # 1. read(0, buf, 256): 从终端 (stdin) 读取用户输入
    xor rax, rax                # SYS_read (0), xor 异或运算清零 rax 寄存器
    xor rdi, rdi                # stdin (0)
    mov rsi, rsp                # 缓冲区 = 栈顶
    mov rdx, 256                # 最多读取 256 字节
    syscall                     # rax = 实际读取的字节数
    mov r12, rax                # 把长度保存到 r12: syscall 只破坏 rax, rcx, r11,
                                # r12 不会被改写, 后面两次 write 都要用到它

    # 2. open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644): 创建/清空并以只写方式打开文件
    mov rax, 2                  # SYS_open
    lea rdi, [rip + path]       # 文件名的地址 (RIP 相对寻址, 必须用 lea 取地址)
    mov rsi, 0x241              # flags = O_WRONLY (0x1) | O_CREAT (0x40) | O_TRUNC (0x200)
    mov rdx, 0644               # mode = 0644 (八进制), 仅在文件被新建时生效: rw-r--r--
    syscall                     # rax = 新的 fd (通常是 3)
    mov r13, rax                # 保存文件的 fd, 写入和关闭时都要用

    # 3. write(fd, buf, len): 把用户输入写入文件
    mov rdx, r12                # 长度 = read 实际读取的字节数, 而不是上限 256
    mov rax, 1                  # SYS_write
    mov rdi, r13                # 目标是刚打开的文件, 不是 stdout
    mov rsi, rsp                # 数据仍然在栈上的缓冲区里
    syscall

    # 4. write(1, buf, len): 把同样的数据输出到终端 (stdout)
    mov rdx, r12                # 长度不变; rdx 可能已被内核改写, 所以重新设置
    mov rax, 1                  # SYS_write
    mov rdi, 1                  # stdout (1)
    mov rsi, rsp                # 同一块缓冲区
    syscall

    # 5. close(fd): 关闭文件, 释放文件描述符
    mov rax, 3                  # SYS_close
    mov rdi, r13                # 要关闭的 fd
    syscall

    # 6. exit(0)
    mov rax, 60                 # SYS_exit
    xor rdi, rdi                # 退出码 0
    syscall
