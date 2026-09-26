/**
 * gcc -Wall -Wextra -std=c11 lang/c/bitop.c && ./a.out
 *
 */

#include <limits.h>
#include <stdio.h>

static int side_calls;

/**
 * @brief 带副作用的下标函数. 每调用一次 side_calls 加 1, 返回 0, 用来数求值次数.
 */
static int bump(void)
{
	side_calls++;
	return 0;
}

/**
 * @brief 统计 x 的二进制里有多少个 1.
 */
static int countbit(unsigned int x)
{
	int n = 0;

	while (x) {
		x &= x - 1;	/* 每次消掉最低位的 1 */
		n++;
	}
	return n;
}

/**
 * @brief 循环右移: 移出的低位补回高位.
 */
static unsigned int rotate_right(unsigned int x, unsigned int n)
{
	unsigned int w = sizeof x * CHAR_BIT;

	n %= w;			/* n 先对宽度取模, 0 直接返回, 避免 << 宽度 */
	if (n == 0)
		return x;
	return (x >> n) | (x << (w - n));
}

/**
 * @brief 把各位异或到一起. 结果为 1 表示 x 里 1 的个数是奇数.
 */
static int parity(unsigned int x)
{
	int p = 0;

	while (x) {
		p ^= (int)(x & 1u);
		x >>= 1;
	}
	return p;
}

/**
 * @brief 用异或不借助临时变量交换. a 和 b 必须指向两个不同的对象, 否则结果被清成 0.
 */
static void xor_swap(unsigned int *a, unsigned int *b)
{
	*a = *a ^ *b;
	*b = *b ^ *a;
	*a = *a ^ *b;
}

/**
 * @brief 与, 或, 异或, 取反. ~ 之前先做整数提升, unsigned char 取反不是 8 位结果.
 */
static void bitwise_logic(void)
{
	unsigned char c = 0xfc;

	printf("~ of unsigned char 0xfc -> %#x (promoted, not 0x03)\n",
	       (unsigned)~c);
	printf("0xf0 & 0x3c -> %#x\n", 0xf0u & 0x3cu);
	printf("0xf0 | 0x3c -> %#x\n", 0xf0u | 0x3cu);
	printf("0xf0 ^ 0x3c -> %#x\n\n", 0xf0u ^ 0x3cu);
}

/**
 * @brief 无符号移位相当于乘除 2 的幂. + 比 << 紧.
 *        同一个位型, 常量按 unsigned 右移补 0, 赋给 int 之后 gcc 补符号位.
 */
static void shifts(void)
{
	unsigned int u = 3u;
	int s = 0xcffffff3;

	printf("unsigned 3 << 1 -> %u, 3 >> 1 -> %u\n", u << 1, u >> 1);
	printf("unsigned overflow: ~0u << 4 -> %#x\n", ~0u << 4);
	/* 1u << 2 + 1 解析成 1u << (2 + 1). + 比 << 紧. */
	printf("1u << (2 + 1) -> %u, (1u << 2) + 1 -> %u\n",
	       1u << (2 + 1), (1u << 2) + 1);
	printf("const 0xcffffff3 >> 2 -> %#x (unsigned, fill 0)\n",
	       0xcffffff3 >> 2);
	printf("int    0xcffffff3 >> 2 -> %#x (gcc: fill sign bit)\n\n",
	       (unsigned)(s >> 2));
}

/**
 * @brief 掩码: 取出, 清 0, 置 1, 翻转, 以及低 n 位全 1 的写法. 接着演示计数和循环右移.
 */
static void masks(void)
{
	unsigned int a = 0x12345678;
	unsigned int mask = 0x0000ff00;
	unsigned int low8 = ~(~0u << 8);
	unsigned int flipped;

	printf("extract 8..15 -> %#x\n", (a & mask) >> 8);
	printf("clear   8..15 -> %#x\n", a & ~mask);
	printf("set     8..15 -> %#x\n", a | mask);
	flipped = a ^ (1u << 6);
	printf("flip bit 6    -> %#x\n", flipped);
	printf("low 8 bits mask -> %#x\n", low8);
	printf("countbit(0x12345678) -> %d\n", countbit(a));
	/* 右移 16 位得到 0xbeefdead. 教材写成 0xefdeadbe, 那是右移 8 位. */
	printf("rotate_right(0xdeadbeef, 16) -> %#x\n",
	       rotate_right(0xdeadbeef, 16));
	printf("rotate_right(0xdeadbeef, 8)  -> %#x\n",
	       rotate_right(0xdeadbeef, 8));
	printf("rotate_right(0xdeadbeef, 0)  -> %#x\n\n",
	       rotate_right(0xdeadbeef, 0));
}

/**
 * @brief 异或: 奇偶校验, 两个不同对象的交换, 以及指向同一对象时会被清 0.
 */
static void xor_props(void)
{
	unsigned int a = 0xaaaa;
	unsigned int b = 0x5555;
	unsigned int same = 0xabcd;
	unsigned int *alias = &same;
	unsigned int bits = 0xb;	/* 1011, three 1s: odd */

	printf("parity(0xb) -> %d (1 means odd)\n", parity(bits));
	xor_swap(&a, &b);
	printf("xor swap distinct -> %#x %#x\n", a, b);
	xor_swap(&same, alias);
	printf("xor swap alias    -> %#x (cleared)\n\n", same);
}

/**
 * @brief == 比 & 紧. 不加括号时 flags & 1u == 0 解析成 flags & (1u == 0).
 */
static void precedence(void)
{
	unsigned int flags = 0;

	/* flags & 1u == 0 解析成 flags & (1u == 0), 不是 (flags & 1u) == 0. */
	printf("(flags & 1u) == 0 -> %d\n", (flags & 1u) == 0);
	printf("flags & (1u == 0) -> %u\n\n", flags & (1u == 0));
}

/**
 * @brief 复合赋值对左操作数只求值一次. 展开成 a = a + 1 时 bump() 会调用两次.
 */
static void compound_assign(void)
{
	int a[1] = { 10 };

	side_calls = 0;
	a[bump()] += 1;
	printf("a[bump()] += 1: calls=%d a[0]=%d\n", side_calls, a[0]);

	a[0] = 10;
	side_calls = 0;
	a[bump()] = a[bump()] + 1;
	printf("expanded assign: calls=%d a[0]=%d\n\n", side_calls, a[0]);
}

/**
 * @brief ?: 只求值被选中的分支. 两个算术分支会做寻常算术转换, 1 和 1.0 的结果是 double.
 */
static void conditional_op(void)
{
	int a = 2;
	int b = 5;

	side_calls = 0;
	printf("max(%d, %d) -> %d\n", a, b, a > b ? a : b);
	printf("only chosen branch runs, calls=%d, value=%d\n",
	       side_calls, 1 ? 7 : bump());
	printf("branch type 1 ? 1 : 1.0 -> %.1f\n\n", 1 ? 1 : 1.0);
}

/**
 * @brief 逗号运算符从左到右求值, 值等于最右边. 左边的副作用留下.
 *        参数列表里的逗号是分隔符, 要再用逗号运算符必须加括号.
 */
static void comma_op(void)
{
	int t = 0;
	int i, j, n = 4;
	int dropped = 0;
	int value = (dropped = 1, dropped = 2, 3 + 4);

	printf("comma value -> %d, dropped=%d (left effects kept)\n",
	       value, dropped);
	printf("second arg (t = 3, t + 2) -> %d\n", (t = 3, t + 2));
	printf("for i, j:\n");
	for (i = 0, j = n; i < j; i++, j--)
		printf("  i=%d j=%d\n", i, j);
	printf("\n");
}

/**
 * @brief sizeof 一般不求值, 操作数是变长数组类型时才会求值.
 *        typedef 是给已有类型起名字, array_t 等价于 char[10].
 */
static void sizeof_and_typedef(void)
{
	int a[12];
	int n = 0;
	int vla_n = 4;
	typedef char array_t[10];
	array_t bytes;

	size_t vla_bytes;

	printf("sizeof a / sizeof a[0] -> %zu\n", sizeof a / sizeof a[0]);
	(void)sizeof n++;
	printf("sizeof does not eval, n -> %d\n", n);
	vla_bytes = sizeof(int[vla_n++]);
	printf("sizeof VLA type -> %zu, vla_n -> %d\n", vla_bytes, vla_n);
	printf("typedef array_t[10] width -> %zu\n", sizeof bytes);
}

int main(void)
{
	bitwise_logic();
	shifts();
	masks();
	xor_props();
	precedence();
	compound_assign();
	conditional_op();
	comma_op();
	sizeof_and_typedef();
	return 0;
}
