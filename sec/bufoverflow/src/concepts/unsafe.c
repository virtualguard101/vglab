/*
 * unsafe.c - 经典不安全 C 字符串/IO 函数的溢出行为(Finding Buffer Overflows)
 *
 * 论文要点: C 标准库中这些函数不做边界检查--
 *   strcpy(), strcat(), sprintf(), vsprintf(), gets(),
 *   scanf("%s"), getc/fgetc/getchar 循环
 *
 * 本文件 5 个小节, 每个小节独立函数 + main 的参数选择.
 * 触发溢出会看到(取决于构建方式与编译器的栈帧布局):
 *   - 无栈保护构建: 返回地址被 'A'(0x41) 覆盖 -> ret 到 0x41...41 -> SIGSEGV
 *   - 栈保护构建: canary 被破坏 -> *** stack smashing detected *** -> SIGABRT
 * Note:
 *   a) 是否崩溃/崩在哪, 依赖编译器如何摆放这些数组--有的 case 在带保护
 *      构建下会"侥幸存活"(溢出恰好落在同帧的另一个数组里, 没碰到 canary).
 *      溢出的后果是布局相关的/不确定的.
 *   b) canary 的确定性演示见 canary_probe.c(单缓冲,必触发 __stack_chk_fail).
 *
 * Usage: ./build/unsafe [1-5]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* gets() 太危险, C11 从将其从标准中移除, 现代 <stdio.h> 不再声明它,
 * 但 glibc 仍保留符号以兼容老二进制. 这里手动声明以演示这个历史反例.
 */
extern char *gets(char *);

/**
 * demo_strcpy() - 1. strcpy: 无界拷贝, 论文 example2.c 复刻.
 *
 * flaw: strcpy 不检查目标大小,
 * 256 字节灌进 16 字节的栈缓冲区.
 */
static void demo_strcpy(void)
{
	char buffer[16];
	char large_string[256];

	memset(large_string, 'A', sizeof(large_string) - 1);
	large_string[sizeof(large_string) - 1] = '\0';

	puts("[1] strcpy(buffer[16], large_string[256])");
	/* BUG: strcpy 不检查目标大小, 256 字节灌进 16 字节的栈缓冲区 */
	strcpy(buffer, large_string);
	printf("survived: %zu bytes copied (skeptically)\n", strlen(buffer));
}

/**
 * demo_gets() - 2. gets: 读一行且无法限制长度.
 *
 * flaw: gets 无法限制读入长度,
 * POSIX.1-2008 标记 obsolete, C11 直接删除.
 * gcc 会警告: the `gets' function is dangerous.
 */
static void demo_gets(void)
{
	char buffer[8];
	puts("[2] gets(buffer[8]) - more than 7 characters overflows (try 16 As and Enter)");
	printf("input> ");
	fflush(stdout);
	/* BUG: gets 无法限制读入长度, POSIX.1-2008 标记 obsolete, C11 直接删除 */
	gets(buffer);		/* gcc 会警告: the `gets' function is dangerous */
	printf("got: %s\n", buffer);
}

/**
 * demo_sprintf() - 3. sprintf: 无界格式化进固定缓冲区.
 *
 * flaw: 128 字节源格式化进 32 字节目标.
 */
static void demo_sprintf(void)
{
	char buffer[32];
	char user[128];
	memset(user, 'B', sizeof(user) - 1);
	user[sizeof(user) - 1] = '\0';

	puts("[3] sprintf(buffer[32], \"%s\", user[128])");
	/* BUG: 128 字节源格式化进 32 字节目标 */
	sprintf(buffer, "%s", user);
	printf("got: %s\n", buffer);
}

/**
 * demo_scanf() - 4. scanf("%s"): %s 无宽度限定, 和 gets 类似.
 *
 * 本函数演示的是正解: 用 "%15s" 限定最大 15 字符 + NUL;
 * 不带宽度的 "%s" 就是 gets 的翻版.
 */
static void demo_scanf(void)
{
	char buffer[16];
	puts("[4] scanf(\"%15s\", buffer[16]) - using %s without a width overflows; %15s is safe");
	printf("input> ");
	fflush(stdout);
	if (scanf("%15s", buffer) == 1)	/* 正解:限定最大 15 字符 + NUL */
		printf("got: %s\n", buffer);
}

/**
 * demo_getchar_loop() - 5. getc 循环: "直到分隔符"模式.
 *
 * flaw: 论文原文的循环没有 i < sizeof(buffer) 这一界,
 * 长输入直接写穿. 本函数的循环补上了这个界作为正解.
 */
static void demo_getchar_loop(void)
{
	char buffer[16];
	size_t i = 0;
	int c;

	puts("[5] while ((c = getchar()) != '\\n') buffer[i++] - unbounded loop");
	printf("input> ");
	fflush(stdout);
	/* 论文原文的循环没有 i < sizeof(buffer), 长输入直接写穿 */
	while ((c = getchar()) != '\n' && c != EOF && i < sizeof(buffer) - 1)
		buffer[i++] = (char)c;
	buffer[i] = '\0';
	printf("got: %s\n", buffer);
}

/**
 * main() - 演示入口: 按 argv[1] 分发五组不安全函数演示.
 *
 * @argc: 参数个数.
 * @argv: 参数数组, argv[1] 取 1-5 选择演示小节(默认 1).
 *
 * 触发溢出时的可观测结果见文件头注释
 * (无栈保护 -> SIGSEGV; 有栈保护 -> SIGABRT),
 * 若控制流真的被劫持, 末尾的 puts 不会到达.
 *
 * Return: 0 或 1(未知模式).
 */
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
