/**
 * gcc -Wall -Wextra -std=c11 lang/c/types.c && ./a.out
 */

#include <stdio.h>

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

static void integer_promotion(void)
{
	unsigned char c1 = 255, c2 = 2;
	printf("unsigned char 255+2 -> %d (promoted to int, not 1)\n", c1 + c2);
}

static void signed_unsigned_cmp(void)
{
	printf("(-1 < 1u) -> %d\n", -1 < 1u);
}

static void hex_constant(void)
{
	long hex = 0xffffffff;
	printf("long hex = 0xffffffff -> %ld\n", hex);
	printf("-1L -> %ld\n\n", -1L);
}

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

static void sign_extension(void)
{
	short s = -1;
	printf("short -1 widened to int -> %#x\n", (unsigned)(int)s);
}

static void to_unsigned(void)
{
	printf("(unsigned short)-1 -> %u\n", (unsigned short)-1);
}

static void float_to_int(void)
{
	printf("(int)-3.9 -> %d\n", (int)-3.9);
}

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
