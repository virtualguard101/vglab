.intel_syntax noprefix

.global _start

.section .data

.file_path:
    .ascii "flag.txt\0"

.section .text

_start:
    mov rax, 2                   # SYS_open
    lea rdi, [rip + .file_path]  # 文件路径字符串的地址
    xor esi, esi                 # flags = 0, 只读打开
    syscall                      # rax = 新文件描述符

    sub rsp, 4096                # 在栈上预留 4096 字节缓冲区
    mov rdi, rax                 # read 的第一个参数: open 返回的 fd
    xor eax, eax                 # SYS_read
    mov rsi, rsp                 # read 的第二个参数: 缓冲区地址
    mov edx, 4096                # read 的第三个参数: 最多读取 4096 字节
    syscall                      # rax = 实际读取的字节数

    mov rdx, rax                 # write 的长度 = 实际读取的字节数
    mov eax, 1                   # SYS_write
    mov edi, 1                   # stdout
    mov rsi, rsp                 # 要写出的缓冲区地址
    syscall

    mov eax, 60                  # SYS_exit
    mov edi, 42                  # 退出码 42
    syscall
