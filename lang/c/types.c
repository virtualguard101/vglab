/**
 * gcc -Wall -Wextra -std=c11 lang/c/types.c && ./a.out
 */

#include <stdio.h>

/**
 * @brief 打印各基本类型的字节数, 以及不写 signed/unsigned 的 char 是哪种符号性.
 */
static void show_widths(void)
{
	printf
	    ("sizeof  char=%zu short=%zu int=%zu long=%zu ll=%zu ptr=%zu\n",
	     sizeof(char), sizeof(short), sizeof(int), sizeof(long),
	     sizeof(long long), sizeof(void *));
	printf("sizeof  float=%zu double=%zu long double=%zu\n",
	       sizeof(float), sizeof(double), sizeof(long double));
	printf("plain char is %s\n\n", (char)-1 < 0 ? "signed" : "unsigned");
}

/**
 * @brief 整数提升: unsigned char 相加前先变成 int, 255+2 得到 257, 不是模 256 的 1.
 */
static void integer_promotion(void)
{
	unsigned char c1 = 255, c2 = 2;
	printf("unsigned char 255+2 -> %d (promoted to int, not 1)\n", c1 + c2);
}

/**
 * @brief 有符号与无符号比较: -1 先转成 unsigned, 变成很大的正数, 所以 -1 < 1u 为假.
 */
static void signed_unsigned_cmp(void)
{
	printf("(-1 < 1u) -> %d\n", -1 < 1u);
}

/**
 * @brief 无后缀十六进制常量装不进 int 时类型是 unsigned int.
 *        赋给 long 后是 4294967295, 不是 -1. -1L 才是有符号的 -1.
 */
static void hex_constant(void)
{
	long hex = 0xffffffff;
	printf("long hex = 0xffffffff -> %ld\n", hex);
	printf("-1L -> %ld\n\n", -1L);
}

/**
 * @brief 运算的类型在赋值之前就定了. 两个 int 先乘会溢出, long long 接不住.
 *        后缀要写在乘法的操作数上.
 */
static void multiply_before_widen(void)
{
	/* 两个 int 先乘会溢出. 后缀要写在乘法之前. */
	long long bad = 1234567890 * 1234567890;
	long long good = 1234567890LL * 1234567890;
	printf
	    ("int multiply then widen (1234567890 * 1234567890) -> %lld\n",
	     bad);
	printf
	    ("LL suffix before multiply (1234567890LL * 1234567890) -> %lld\n\n",
	     good);
}

/**
 * @brief short 变宽到 int 时按符号扩展, -1 的高位补 1.
 */
static void sign_extension(void)
{
	short s = -1;
	printf("short -1 widened to int -> %#x\n", (unsigned)(int)s);
}

/**
 * @brief 负数转无符号按模 2^N 解释, (unsigned short)-1 是 65535.
 */
static void to_unsigned(void)
{
	printf("(unsigned short)-1 -> %u\n", (unsigned short)-1);
}

/**
 * @brief 浮点转整数向 0 截断, 小数部分丢掉. (int)-3.9 是 -3.
 */
static void float_to_int(void)
{
	printf("(int)-3.9 -> %d\n", (int)-3.9);
}

/**
 * @brief 24 位尾数装不下 20000001, 转成 float 再打印会少 1.
 */
static void int_to_float(void)
{
	printf("(float)20000001 -> %.0f\n", (float)20000001);
}

int main(void)
{
	show_widths();
	integer_promotion();
	signed_unsigned_cmp();
	hex_constant();
	multiply_before_widen();
	sign_extension();
	to_unsigned();
	float_to_int();
	int_to_float();

	return 0;
}
