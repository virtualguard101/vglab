/*
 * part1_unsafe.c - 经典不安全 C 字符串/IO 函数的溢出行为(对照论文 "Finding Buffer Overflows" 一节)
 *
 * 论文要点:C 标准库中这些函数不做边界检查--
 *   strcpy(), strcat(), sprintf(), vsprintf(), gets(),
 *   scanf("%s"), getc/fgetc/getchar 循环
 *
 * 本文件 5 个小节,每个小节独立函数 + main 的参数选择.
 * 触发溢出会看到(取决于构建方式与编译器的栈帧布局):
 *   - 无栈保护构建:返回地址被 'A'(0x41) 覆盖 -> ret 到 0x41...41 -> SIGSEGV
 *   - 栈保护构建:canary 被破坏 -> *** stack smashing detected *** -> SIGABRT
 * 两点提醒:
 *   a) 是否崩溃/崩在哪,依赖编译器如何摆放这些数组--有的 case 在带保护
 *      构建下会"侥幸存活"(溢出恰好落在同帧的另一个数组里,没碰到 canary).
 *      这本身是教学点:溢出的后果是布局相关的/不确定的.
 *   b) canary 的确定性演示见 canary_probe.c(单缓冲,必触发 __stack_chk_fail).
 *
 * 用法:./unsafe [1-5]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* gets() 太危险,C11 从标准移除/现代 <stdio.h> 不再声明它,
 * 但 glibc 仍保留符号以兼容老二进制.手动声明以演示这个历史反例.
 * 这件事本身就是教学点:危险 API 会被语言标准"除名". */
extern char *gets(char *);

/* ---------- 1. strcpy:无界拷贝,论文 example2.c 的原样复现 ---------- */
static void demo_strcpy(void)
{
	char buffer[16];
	char large_string[256];

	memset(large_string, 'A', sizeof(large_string) - 1);
	large_string[sizeof(large_string) - 1] = '\0';

	puts("[1] strcpy(buffer[16], large_string[256]) - Aleph One example2.c");
	/* flaw: strcpy 不检查目标大小,256 字节灌进 16 字节的栈缓冲区 */
	strcpy(buffer, large_string);
	printf("survived: %zu bytes copied?\n", strlen(buffer));
}

/* ---------- 2. gets:读一行且无法限制长度,C11 已移除 ---------- */
static void demo_gets(void)
{
	char buffer[8];
	puts("[2] gets(buffer[8]) - more than 7 characters overflows (try 16 As and Enter)");
	printf("input> ");
	fflush(stdout);
	/* flaw: gets 无法限制读入长度,POSIX.1-2008 标记 obsolete,C11 直接删除 */
	gets(buffer);		/* gcc 会警告: the `gets' function is dangerous */
	printf("got: %s\n", buffer);
}

/* ---------- 3. sprintf:无界格式化进固定缓冲区 ---------- */
static void demo_sprintf(void)
{
	char buffer[32];
	char user[128];
	memset(user, 'B', sizeof(user) - 1);
	user[sizeof(user) - 1] = '\0';

	puts("[3] sprintf(buffer[32], \"%s\", user[128])");
	/* flaw: 128 字节源格式化进 32 字节目标 */
	sprintf(buffer, "%s", user);
	printf("got: %s\n", buffer);
}

/* ---------- 4. scanf("%s"):%s 无宽度限定 == gets ---------- */
static void demo_scanf(void)
{
	char buffer[16];
	puts("[4] scanf(\"%15s\", buffer[16]) - using %s without a width overflows; %15s is safe");
	printf("input> ");
	fflush(stdout);
	if (scanf("%15s", buffer) == 1)	/* 正解:限定最大 15 字符 + NUL */
		printf("got: %s\n", buffer);
}

/* ---------- 5. getc 循环:论文点名的"直到分隔符"模式 ---------- */
static void demo_getchar_loop(void)
{
	char buffer[16];
	size_t i = 0;
	int c;

	puts("[5] while ((c = getchar()) != '\\n') buffer[i++] - unbounded loop");
	printf("input> ");
	fflush(stdout);
	/* flaw: 论文原文的循环没有 i < sizeof(buffer),长输入直接写穿 */
	while ((c = getchar()) != '\n' && c != EOF && i < sizeof(buffer) - 1)
		buffer[i++] = (char)c;
	buffer[i] = '\0';
	printf("got: %s\n", buffer);
}

int main(int argc, char **argv)
{
	int which = (argc > 1) ? atoi(argv[1]) : 1;

	switch (which) {
	case 1:
		demo_strcpy();
		break;
	case 2:
		demo_gets();
		break;
	case 3:
		demo_sprintf();
		break;
	case 4:
		demo_scanf();
		break;
	case 5:
		demo_getchar_loop();
		break;
	default:
		fprintf(stderr, "usage: %s [1-5]\n", argv[0]);
		return 1;
	}
	puts("(normally unreachable if overflow smashed the return address)");
	return 0;
}
