/*
 * vulnerable.c - 被攻击的目标程序
 *
 * 漏洞形态与论文 vulnerable.c 完全一致(真实世界对应 VVC 解码器
 * CVE-2019-17542:const char *src -> strcpy 进固定 4096 字节栈数组):
 *
 *     char buffer[128];
 *     strcpy(buffer, argv[1]);          <- 无边界检查
 *
 * 与论文时代的差别(务必理解,这决定了 payload 怎么构造):
 *   - 论文 i386:返回地址 0xbffffxxx 四个字节都非零 -> 8 字节地址
 *     原样塞进 strcpy 的字符串没问题.
 *   - x86_64:用户态虚拟地址 48 位,形如 0x00007ffd_xxxxxxxx,
 *     高 2 字节恒为 \x00\x00 -> 8 字节完整地址会被 strcpy 截断.
 *     解法(partial overwrite):只覆盖返回地址低 6 字节,
 *     strcpy 末尾自动补的 NUL 恰好落在第 7 字节--本来也是 0x00,
 *     数据完好.见 part4_exploit.c 的 build_payload().
 *
 * 编译(构建脚本已做, 勿改标志, 防御对照见 part5 与 README section4):
 *   gcc -O0 -fno-stack-protector -no-pie -z execstack
 *
 * VULN_DEBUG=1 环境变量:打印栈布局(模拟"信息泄露".
 * 真实攻击中地址靠泄露或爆破获得,见 04 阶段).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 128

/**
 * get_sp() - 读取当前栈指针 RSP.
 *
 * VULN_DEBUG 泄露模式下用于打印, 便于观察攻击端与目标
 * 各自栈地址的差距(ASLR/环境差异).
 *
 * Return: 当前 %rsp 的值.
 */
static unsigned long get_sp(void)
{
	unsigned long sp;
	__asm__ volatile ("mov %%rsp, %0":"=r" (sp));
	return sp;
}

/**
 * main() - 目标程序入口: 无边界检查地 strcpy argv[1].
 *
 * @argc: 参数个数.
 * @argv: 参数数组, argv[1] 即攻击输入(payload).
 *
 * VULN_DEBUG 非空时打印 buffer/rsp/ret 槽地址(演示"信息泄露",
 * 等价于论文里先跑 sp.c 探栈顶). 随后
 * strcpy(buffer, argv[1]) 不检查 strlen(argv[1]) vs BUF_SIZE --
 * 这就是漏洞本体: 超长输入会踩坏 SFP 与返回地址.
 *
 * Return: 0. 若返回地址已被覆盖, ret 时控制流会交给 payload,
 * 正常路径永远到不了这里的 return 之后.
 */
int main(int argc, char **argv)
{
	char buffer[BUF_SIZE];

	if (getenv("VULN_DEBUG")) {
		/* 泄露 buffer 地址(演示), 等价于论文里先跑 sp.c 探栈顶 */
		printf("[vuln] buffer    @ 0x%016lx\n", (unsigned long)buffer);
		printf("[vuln] rsp       @ 0x%016lx\n", get_sp());
		printf
		    ("[vuln] &ret slot @ 0x%016lx  (buffer+%d: 128 buf + 8 sfp)\n",
		     (unsigned long)(buffer + BUF_SIZE + sizeof(void *)),
		     BUF_SIZE + (int)sizeof(void *));
		fflush(stdout);
	}

	if (argc > 1) {
		/* BUG: 不检查 strlen(argv[1]) vs BUF_SIZE */
		strcpy(buffer, argv[1]);
	}

	printf("[vuln] argv[1] processed (%s), returning normally...\n",
	       argc > 1 ? "see you in the ret slot" : "empty");
	fflush(stdout);
	return 0;		/* <- ret 时若返回地址已被覆盖, 控制流会交给 payload */
}
