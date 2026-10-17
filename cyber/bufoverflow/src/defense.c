/*
 * defense.c - 同一漏洞的安全版本(源码层防御)
 *
 * vulnerable.c 的漏洞是 strcpy(buffer, argv[1]) 无边界检查.
 * 本文件把同一个功能(把 argv[1] 拷进固定缓冲区)用三种安全写法重写,
 * 并保留一个"看起来安全其实仍错"的反面教材,讲清楚边界防御的坑.
 *
 * 防御分层(从源码到系统, 层层独立):
 *   源码层(本文件): 边界检查 / 安全 API
 *   编译器: -fstack-protector(canary, 见 canary_probe.c)
 *   链接器: -z noexecstack(NX, exploit 的 shellcode 在这里会段错误而非执行)
 *   内核  : ASLR(栈地址随机化, exploit的固定猜解几乎必然失败)
 *   语言  : 用 Rust/Go 等内存安全语言从根上消除这类 bug
 *
 * 用法: ./defense [1-4] "待拷贝字符串"
 *   1  strncpy + 显式补 NUL(经典, 但 strncpy 语义有坑)
 *   2  snprintf(推荐永远 NUL 结尾, 返回值可检测截断)
 *   3  运行时长度检查 + 拒绝服务(宁可不处理也不溢出)
 *   4  反面教材: strncpy(buffer, src, strlen(src)) -- 长度取错, 照样溢出
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 128

/**
 * safe_strncpy() - 模式 1: strncpy + 手动补结束符(经典, 但语义有坑).
 *
 * @src: 待拷贝字符串.
 *
 * strncpy 的两个坑:
 *   a) 源 >= n 时不写结束符 -> 必须自己补 buffer[n-1]='\0'
 *   b) 源 < n 时用 NUL 填满剩余(性能, 非安全问题)
 * 记住 n 永远是"目标容量", 不是源长度(对比 still_vulnerable() 的错误).
 */
static void safe_strncpy(const char *src)
{
	char buffer[BUF_SIZE];

	strncpy(buffer, src, sizeof(buffer) - 1);
	buffer[sizeof(buffer) - 1] = '\0';
	printf("[1] strncpy: copied %zu bytes into a %d-byte buffer (%s)\n",
	       strlen(buffer), BUF_SIZE,
	       strlen(src) >=
	       BUF_SIZE ? "truncated; return address safe" : "complete");
}

/**
 * safe_snprintf() - 模式 2: snprintf(推荐的安全写法).
 *
 * @src: 待拷贝字符串.
 *
 * snprintf 总是写 NUL 结尾; 返回"本应写入的长度",
 * 据此可判断是否发生截断, 比 strncpy 更易用/更难出错.
 */
static void safe_snprintf(const char *src)
{
	char buffer[BUF_SIZE];

	int want = snprintf(buffer, sizeof(buffer), "%s", src);
	if (want < 0) {
		perror("snprintf");
		return;
	}
	if ((size_t)want >= sizeof(buffer))
		printf
		    ("[2] snprintf: input %d bytes > capacity %d; safely truncated\n",
		     want, BUF_SIZE);
	else
		printf("[2] snprintf: wrote all %d bytes\n", want);
}

/**
 * safe_reject() - 模式 3: 先检查长度, 拒绝超长输入.
 *
 * @src: 待拷贝字符串.
 *
 * 最保守策略: 超长就拒绝处理, 不做任何"尽力而为"的截断.
 * 真实服务里这通常对应"返回 400 / 丢弃报文", 而不是崩溃.
 */
static void safe_reject(const char *src)
{
	char buffer[BUF_SIZE];

	if (strlen(src) >= sizeof(buffer)) {
		fprintf(stderr,
			"[3] rejected: input %zu bytes >= capacity %d; not processed\n",
			strlen(src), BUF_SIZE);
		return;
	}
	memcpy(buffer, src, strlen(src) + 1);	/* 已验证长度,memcpy 安全 */
	printf("[3] length check passed; processed %zu bytes\n",
	       strlen(buffer));
}

/**
 * still_vulnerable() - 模式 4: 反面教材, n 取成了源长度.
 *
 * @src: 待拷贝字符串.
 *
 * flaw: 第三参数取 strlen(src) 而非 sizeof(buffer),
 * src 超长时 strncpy 会写满 strlen(src) 字节 -> 溢出,
 * 和直接 strcpy 没有区别. 编译器不会报错, canary 才拦得住.
 */
static void still_vulnerable(const char *src)
{
	char buffer[BUF_SIZE];

	puts("[4] flawed example: strncpy(buffer, src, strlen(src)) - still overflows");
	puts("    lesson: the bound must be the destination capacity, never the source length.");
	fflush(stdout);
	strncpy(buffer, src, strlen(src));
	buffer[sizeof(buffer) - 1] = '\0';
	printf("    copied %zu bytes\n", strlen(buffer));
}

/**
 * main() - 防御对照入口: 按 argv[1] 分发四种拷贝写法.
 *
 * @argc: 参数个数.
 * @argv: 参数数组, 用法 "./defense [1-4] \"待拷贝字符串\"":
 *          1 strncpy + 显式补 NUL(经典, 但 strncpy 语义有坑)
 *          2 snprintf(建议永远 NUL 结尾, 返回值可检测截断)
 *          3 运行时长度检查 + 拒绝服务(宁可不处理也不溢出)
 *          4 反面教材: strncpy(buffer, src, strlen(src)) -- 长度取错, 照样溢出
 *
 * 无论哪个模式, 只要返回地址未被破坏, 末尾都会正常打印
 * "returned normally"(对照 vulnerable.c 的溢出崩溃).
 *
 * Return: 0 或 1(用法/未知模式).
 */
int main(int argc, char **argv)
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s [1-4] <string>\n", argv[0]);
		fprintf(stderr,
			"  1 strncpy  2 snprintf(recommended)  3 length check+reject  4 flawed example\n");
		return 1;
	}

	const char *src = argv[2];
	switch (atoi(argv[1])) {
	case 1:
		safe_strncpy(src);
		break;
	case 2:
		safe_snprintf(src);
		break;
	case 3:
		safe_reject(src);
		break;
	case 4:
		still_vulnerable(src);
		break;
	default:
		fprintf(stderr, "unknown mode\n");
		return 1;
	}
	puts("[defense] returned normally - return address was not corrupted.");
	return 0;
}
