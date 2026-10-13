/*
 * canary_probe.c - 阶段 05: 栈保护 (Stack Canary / Stack Smashing Protector) 观测
 *
 * Canary 是编译器插入的防御: 在返回地址与局部缓冲区之间放一个随机哨兵值,
 * 函数返回前校验它是否被改动. 溢出要覆盖返回地址, 就必然先踩坏 canary,
 * 校验失败 -> __stack_chk_fail -> *** stack smashing detected *** -> abort().
 *
 * 这解释了 part1 在默认 (带保护) 构建下看到的是 SIGABRT 而非 SIGSEGV:
 * 不是没溢出,而是 canary 在 ret 之前就拦下了.
 *
 * 本文件编译两次对比 (构建脚本会做):
 *   canary_probe      -fstack-protector-all    -> 有哨兵, 溢出被 abort 拦截
 *   canary_probe_off  -fno-stack-protector     -> 无哨兵, 溢出直达 ret -> SIGSEGV
 *
 * 用法:
 *   ./canary_probe show         打印 canary 值 (每次运行都不同 -> 随机性)
 *   ./canary_probe smash 200    向 64 字节缓冲写 200 字节, 触发校验失败
 *
 * 注意: canary 防的是 "经典返回地址覆盖". 它挡不住:
 *   - 覆盖返回地址之前就被利用的局部指针/函数指针
 *   - 不经过 ret 的控制流劫持(如 GOT 改写)
 *   - 信息泄露先读出 canary 再精确重写
 * 所以 canary 是 "提高门槛", 不是 "根除漏洞", 要和 NX/ASLR 叠加.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* 读取当前函数栈帧里的 canary.
 * x86_64 下 canary 由 %fs:0x28 (TLS) 取出存入栈帧, 低字节恒为 0x00
 * (阻止 strcpy 这类以 NUL 判结束的拷贝顺手泄露/伪造 canary). */
__attribute__((noinline))
static void show_canary(void)
{
	char buffer[64];
	unsigned long canary;

	/* 直接读 %fs:0x28 - 这正是编译器 prologue 写进栈帧的那个值 */
	__asm__ volatile ("mov %%fs:0x28, %0":"=r" (canary));
	memset(buffer, 0, sizeof(buffer));

	printf("[canary] canary for this run = 0x%016lx\n", canary);
	printf
	    ("[canary] low byte = 0x%02lx (always 0x00; blocks string-copy leaks)\n",
	     canary & 0xff);
	printf
	    ("[canary] buffer @ %p - run again for a new per-process canary\n",
	     (void *)buffer);
}

/* 故意溢出: 往 64 字节缓冲写 n 字节.
 * 若本程序带 -fstack-protector 编译, 返回时校验失败 -> abort;
 * 若带 -fno-stack-protector 编译, 则一路踩到返回地址 -> SIGSEGV. */
__attribute__((noinline))
static void smash(int n)
{
	char buffer[64];

	printf("[canary] writing %d bytes to buffer[64]...\n", n);
	fflush(stdout);
	/* flaw(故意): 无边界的填充,n>64 越过缓冲,踩坏 canary 与返回地址 */
	memset(buffer, 'A', (size_t)n);
	/* 用一下 buffer,防止被优化掉 */
	printf
	    ("[canary] returning... (with protection, __stack_chk_fail stops execution after this line)\n");
	fflush(stdout);
}

int main(int argc, char **argv)
{
	if (argc >= 2 && strcmp(argv[1], "show") == 0) {
		show_canary();
		return 0;
	}
	if (argc >= 2 && strcmp(argv[1], "smash") == 0) {
		int n = (argc >= 3) ? atoi(argv[2]) : 200;
		smash(n);
		puts("[canary] smash returned successfully - build has no stack protector (canary_probe_off)");
		return 0;
	}
	fprintf(stderr, "usage: %s show | smash [n]\n", argv[0]);
	return 1;
}
