.intel_syntax noprefix

.global _start

.section .text

_start:
    xor rax, rax                # SYS_read (0), xor 异或运算清零 rax 寄存器
    xor rdi, rdi                # stdin (0)
    mov rsi, rsp                # 将输入读到栈顶
    mov rdx, 128                # 读取 128 字节
    syscall

    mov rdx, rax                # 在写入之前修改 rax 前事先将读取的字节数作为写入的字节数
    mov rax, 1                  # SYS_write
    mov rdi, 1                  # stdout (1)
    mov rsi, rsp                # 从栈顶写出刚读入的数据
    # mov rdx, 128              # 写出 128 字节
    syscall

    mov rax, 60                 # SYS_exit
    xor rdi, rdi                # 退出码 0
    syscall
