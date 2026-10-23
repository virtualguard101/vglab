.intel_syntax noprefix

.global _start

# 内存布局 (.data), 两级指针, 等价于 C:
#   char  msg[] = "Hello, world!\n";
#   char *p     = msg;     // p  存放 msg 的地址  (一级指针)
#   char **pp   = &p;      // pp 存放 p 的地址    (二级指针)
#
#   pp --> p --> msg --> "Hello, world!\n"
#   (8B)  (8B)   (14B)
#
# 访存规则 (Intel 语法):
#   lea reg, [X]  : 只计算地址 X, 不访问内存 (取地址, 类似 C 的 &)
#   mov reg, [X]  : 以 X 为地址读内存, 读出其内容 (解引用, 类似 C 的 *)
#   [rip+label]   : RIP 相对寻址, 位置无关, 是访问全局符号的常用方式

.section .data

msg:
    .ascii "Hello, world!\n"
.set msg_len, . - msg       # 汇编期常量: 当前位置 - msg 起点 = 14, 不占内存

p:
    .quad msg               # 8 字节, 内容 = msg 的地址 (链接时重定位)

pp:
    .quad p                 # 8 字节, 内容 = p 的地址

.section .text

_start:
    # 第 0 步: 取得 pp 变量自身的地址 (&pp)
    # 注意: 这里用 lea 而非 mov, 因为我们要的是 pp 这个"变量"的位置
    lea rax, [rip+pp]       # rax = &pp

    # 第 1 次解引用: 读取 pp 变量的内容, 即 p 的地址
    mov rax, [rax]          # rax = *(&pp) = pp = &p

    # 第 2 次解引用: 读取 p 变量的内容, 即 msg 的地址
    mov rsi, [rax]          # rsi = *(&p) = p = &msg
                            # 等价 C: char *buf = *pp;
                            # (若再做一次 [rsi] 才是读取字符本身, 即第 3 级)

    # write(1, buf, msg_len): rsi 此时恰好是 buf 参数
    mov rax, 1              # rax = SYS_write (系统调用号)
    mov rdi, 1              # arg0 fd = stdout
                            # arg1 rsi = buf (上面双重解引用得到)
    mov rdx, msg_len        # arg2 count = 14 (立即数, 来自 .set)
    syscall                 # 注意: syscall 会破坏 rcx/r11, 并用 rax 返回值
                            # rsi 不会被破坏, 下面可继续使用

    # 第 3 次解引用(按字节): 读 msg 的首字符 'H' (0x48 = 72)
    # movzx 零扩展: 只读 1 字节, 避免像 mov rax,[..] 那样一次读 8 字节
    movzx rdi, byte ptr [rsi]   # rdi = *buf = 'H' = 72
                                # 等价 C: exit(**pp);

    mov rax, 60             # rax = SYS_exit
    syscall                 # 退出码 = 72, 可用 echo $? 验证
