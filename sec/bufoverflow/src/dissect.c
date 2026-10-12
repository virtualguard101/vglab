/*
 * part2_dissect.c — 溢出机理剖析：对照论文 example2.c / example3.c
 *
 * 对应论文流程：
 *   example2.c  →  strcpy 溢出 16 字节 buffer，覆盖 sfp/ret
 *   example3.c  →  覆盖 ret 前先看反汇编，精确修改 ret 跳过一条语句
 *   gdb 反汇编 → 找到 call function 之后那条指令的地址 → 算偏移
 *
 * 本文件在单个进程里完成论文的观察过程：
 *   A. dump_stack_layout()   ：打印 [局部变量 | sfp | ret] 的真实布局与内容
 *   B. ret_redirect()        ：论文 example3 的现代重演——
 *                               把函数的返回地址 +20，跳过 main 中
 *                               call 之后的赋值语句（g_x = 0/1;）。
 *
 * 重要提醒：RET_OFFSET_FROM_BUF 是**构建产物**，随编译器/版本/布局变化。
 * 遇到 g_x != 0，请用 `objdump -d build/dissect` 重算偏移（README §2 有步骤）。
 *
 * 用法：./dissect [layout|redirect]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ============ A. 栈布局观测 ============ */

static void dump_stack_layout(void)
{
    char buffer[16];
    unsigned long *after_buf;
    unsigned long sfp, ret;

    memset(buffer, 'A', sizeof(buffer));

    /* buffer 之上依次是 saved RBP (SFP)、return address。
     * 与论文 example3 用 buffer1+12 找 RET 的思路一致，
     * 只是 i386(4B 指针) 变成了 x86_64(8B 指针)。 */
    after_buf = (unsigned long *)((unsigned long)(buffer + sizeof(buffer) + 7) & ~7UL);
    sfp = *after_buf;         /* saved frame pointer (RBP) */
    ret = *(after_buf + 1);   /* return address into main  */

    printf("buffer[16]    @ %p\n", (void *)buffer);
    printf("saved RBP     @ %p = 0x%016lx\n", (void *)after_buf, sfp);
    printf("return addr   @ %p = 0x%016lx\n", (void *)(after_buf + 1), ret);

    printf("\n-- buffer 填满 'A' 后：第 17..24 字节覆盖 SFP，第 25..32 字节覆盖 RET --\n");
    printf("offset(buffer → RET slot) == %ld 字节\n",
           (long)((char *)(after_buf + 1) - buffer));
    fflush(stdout);
}

/* ============ B. 修改返回地址（论文 example3 的现代重演） ============ */

static volatile int g_x;

/* x86_64 gcc -O0 布局下，buffer1 起点到 RET 槽的距离。
 * 论文 i386 上是 12；这里实测（gcc 16 -O0）：buffer1 在 rbp-0xd，
 * RET 槽恒在 rbp+8 → 21。数字别背，重算方法：
 *   objdump -d build/dissect | sed -n '/<ret_redirect>:/,/ret/p'
 *   找 lea -0xN(%rbp)（buffer1 的地址）→ RET_OFFSET = 8 + N */
#define RET_OFFSET_FROM_BUF 21

/* 返回地址要加的步长：call 返回点 = main 里 g_x = 0;（movl 共 10 字节），
 * 再 +10 跳过 g_x = 1;（同样 10 字节），直达 printf 的取值指令。
 * 论文原稿写 +8 是笔误（其自注 "should be 10"）——差 1 字节就会落在
 * 指令中间 → SIGSEGV/SIGILL，这正是"精确覆盖"的含义。 */
#define RET_SKIP_DELTA 20

__attribute__((noinline))
static void ret_redirect(void)
{
    char buffer1[5];
    char buffer2[10];
    int *ret;

    ret = (int *)((char *)buffer1 + RET_OFFSET_FROM_BUF);
    printf("[redirect] buffer1 @ %p, patching RET slot @ %p (+%d)\n",
           (void *)buffer1, (void *)ret, RET_OFFSET_FROM_BUF);
    *ret += RET_SKIP_DELTA;   /* 跳过 call 返回后的赋值语句，见上方注释 */
}

int main(void)
{
    const char *mode = getenv("DISSECT_MODE");
    if (mode == NULL)
        mode = "layout";

    if (strcmp(mode, "layout") == 0) {
        dump_stack_layout();
        return 0;
    }

    if (strcmp(mode, "redirect") == 0) {
        ret_redirect();
        g_x = 0;
        g_x = 1;   /* 应被跳过：ret 已被改，函数 return 直达下一行 */
        printf("g_x = %d  (expect 0: 赋值 g_x=1 被跳过 → 论文 example3 的效果)\n", g_x);
        return 0;
    }

    fprintf(stderr, "usage: DISSECT_MODE=[layout|redirect] %s\n", "dissect");
    return 1;
}
