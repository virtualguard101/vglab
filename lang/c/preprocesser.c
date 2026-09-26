/**
 * gcc -Wall -Wextra -std=c11 lang/c/preprocesser.c && ./a.out
 * gcc -E -std=c11 lang/c/preprocesser.c
 *
 * 8086 分支: gcc -std=c11 -DMACHINE=8086 lang/c/preprocesser.c
 */

#include <stdio.h>

/**
 * 形参和整个替换列表都加了括号. 宏名和 '(' 之间不能有空格,
 * 有空格就会变成对象式宏, 替换列表从 '(' 开始.
 */
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define VERSION 2
#if defined x || y || VERSION < 3
#define VERSION_OK 1
#else
#define VERSION_OK 0
#endif

#if 0xFFFFFFFFu + 1 == 0
#define PRE_WRAPS 1
#else
#define PRE_WRAPS 0
#endif

#define GREETING "hello, "\
	"world"

#define OBJ_LIKE (1 - 1)
#define OBJ_LIKE /* 空白多少无所谓 */ (1 - 1)

#define QUOTE(s) #s
#define VAR(n) val_ ## n
#define JOIN3(a, b, c) a ## b ## c

#define LOG2(a, b)							\
	do {								\
		printf("  %s\n", (a));					\
		printf("  %s\n", (b));					\
	} while (0)

#define report(test, ...)						\
	((test) ? printf("  ok: " #test "\n")				\
		: printf("  " __VA_ARGS__))

/* 可变参数为空时吃掉前面的逗号, 这是 gcc 扩展. C23 用 __VA_OPT__. */
#define DEBUGP(format, ...) printf(format, ## __VA_ARGS__)

#define VAL_(x) #x
#define STR_(x) VAL_(x)

#define check(test)							\
	((test) ? (void)0						\
		: (void)printf("%s:%s  %s failed\n",			\
			       __FILE__, STR_(__LINE__), #test))

static int val_1 = 11;
static int val_2 = 22;
static int ab = 5;

static int vals[] = { 9, 3, 5, 2, 1, 0, 8, 7, 6, 4 };

static int rec_calls;

static int nsub_z;
static int alt[27];

static int once[] = { 42 };

static int max_fn(int a, int b)
{
	return a > b ? a : b;
}

static inline int max_inline(int a, int b)
{
	return a > b ? a : b;
}

/**
 * @brief 宏版每次比较失败都会再递归一次. 最大值在 vals[0], 所以每层两次.
 */
static int max_macro(int n)
{
	rec_calls++;
	return n == 0 ? vals[0] : MAX(vals[n], max_macro(n - 1));
}

/**
 * @brief 真正的函数每个实参只求值一次, 从 n 递归到 0 共 n+1 次.
 */
static int max_fn_rec(int n)
{
	rec_calls++;
	return n == 0 ? vals[0] : max_fn(vals[n], max_fn_rec(n - 1));
}

/**
 * @brief 函数必须写在同名宏之前, 否则定义那一行自己会被展开.
 *        f(3) 只展开一层, 参数是 2*3. 再展开会变成 2*(2*3).
 */
static int f(int v)
{
	return v;
}

/**
 * @brief 续行和相邻字符串. 展开后是 "hello, world".
 */
static void string_join(void)
{
	printf("greeting -> %s\n\n", GREETING);
}

/**
 * @brief 重复定义必须 token 相同. 改定义要先 #undef.
 *        宏没有块作用域, 写在函数里也生效到 #undef 或文件结束.
 */
static void object_like(void)
{
#define X 3
#define INSIDE 7
	int y = 99;

	printf("OBJ_LIKE -> %d\n", OBJ_LIKE);
	printf("X -> %d\n", X);
#undef X
#define X 2
	printf("X after undef -> %d\n", X);
	printf("macro inside function -> %d\n", INSIDE);
	printf("variable y=%d is not the #if name\n\n", y);
#undef X
#undef INSIDE
}

/**
 * @brief 括号, 副作用, 重复计算, 以及 do-while 包住的多条语句.
 */
static void function_like(void)
{
	unsigned i = 0x05;
	unsigned j = 0x20;
	int a, b, m;

	printf("MAX(i & 0x0f, j & 0x0f) -> %u\n", MAX(i & 0x0f, j & 0x0f));
	/*
	 * 省掉形参括号后的写法. > 比 & 优先级高, 不加括号会警告, 这里故意留下这个表达式.
	 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"
	printf("no inner parens          -> %u\n",
	       (i & 0x0f > j & 0x0f ? i & 0x0f : j & 0x0f));
#pragma GCC diagnostic pop
	printf("MAX(3, 1) + 1 -> %d\n", MAX(3, 1) + 1);
	printf("no outer parens -> %d\n", 3 > 1 ? 3 : 1 + 1);

	a = 1;
	b = 2;
	m = MAX(++a, ++b);
	printf("macro  ++ -> %d, a=%d b=%d\n", m, a, b);
	a = 1;
	b = 2;
	m = max_fn(++a, ++b);
	printf("fn     ++ -> %d, a=%d b=%d\n", m, a, b);
	a = 1;
	b = 2;
	m = max_inline(++a, ++b);
	printf("inline ++ -> %d, a=%d b=%d\n", m, a, b);

	rec_calls = 0;
	m = max_macro(9);
	printf("macro recursion    -> %d, calls=%d\n", m, rec_calls);
	rec_calls = 0;
	m = max_fn_rec(9);
	printf("function recursion -> %d, calls=%d\n", m, rec_calls);

	if (m > 0)
		LOG2("in if", "still in if");
	else
		printf("  else\n");
	printf("\n");
}

/**
 * @brief # 把实参收成字符串, 连续空白收成一个空格. ## 把 token 粘起来.
 */
static void stringify_and_paste(void)
{
	printf("quote  -> [%s]\n", QUOTE(hello world));
	printf("escape -> [%s]\n", QUOTE(say "hi"));
	printf("paste  -> %d %d\n", VAR(1), VAR(2));
	printf("empty  -> %d\n\n", JOIN3(a, b,));
}

/**
 * @brief __VA_ARGS__ 接住 '...'. DEBUGP 没有额外参数时不能留下逗号.
 */
static void variadic(void)
{
	int x = 1;
	int y = 2;

	report(x > y, "x is %d but y is %d\n", x, y);
	report(y > x, "x is %d but y is %d\n", x, y);
	DEBUGP("  info no. %d\n", 1);
	DEBUGP("  info\n");
	printf("\n");
}

/*
 * 放在使用点前面. x 若提前定义, 后面的变量名 x 会被换成 2.
 * once 展开成 once[0] 后不再展开, 所以不会变成 once[0][0].
 */
#define x 2
#define f(a) f(x * (a))
#define once once[0]
#define sh(x) printf("n" #x "=%d, or %d\n", n ## x, alt[x])
#define sub_z 26

/**
 * @brief # 和 ## 的实参不先展开. 其它实参先展开. 宏不展开自己.
 */
static void expansion_order(void)
{
	nsub_z = 7;
	alt[26] = 99;
	sh(sub_z);
	printf("f(3) -> %d (argument was 6, not 12)\n", f(3));
	printf("once -> %d (once[0], not once[0][0])\n\n", once);
}

#undef f
#undef x
#undef once
#undef sh
#undef sub_z

/**
 * @brief #if 看不见变量和 enum. 没定义的名字是 0.
 *        预处理里的无符号加法按 uintmax_t 的宽度算.
 */
static void conditional(void)
{
#if MACHINE == 68000
	const char *name = "68000";
#elif MACHINE == 8086
	const char *name = "8086";
#else
	const char *name = "default";
#endif
#if 0
	not valid C;
#endif
	printf("MACHINE -> %s\n", name);
	printf("VERSION_OK -> %d\n", VERSION_OK);
	printf("preprocessor 0xFFFFFFFFu + 1 == 0 -> %d\n", PRE_WRAPS);
	printf("runtime       0xFFFFFFFFu + 1 == 0 -> %d\n\n",
	       0xFFFFFFFFu + 1 == 0);
}

/**
 * @brief __FILE__ / __LINE__ 随位置变. 字符串化行号要先套一层宏.
 *        __func__ 不是宏. check 失败时打印, 不调用 abort.
 */
static void predefined(void)
{
	printf("func -> %s\n", __func__);
	printf("stdc -> %ld\n", (long)__STDC_VERSION__);
	printf("date -> %s %s\n", __DATE__, __TIME__);
	printf("VAL_ -> %s\n", VAL_(__LINE__));
	printf("STR_ -> %s\n", STR_(__LINE__));
	check(2 > 3);
	check(2 < 3);
}

int main(void)
{
	string_join();
	object_like();
	function_like();
	stringify_and_paste();
	variadic();
	expansion_order();
	conditional();
	predefined();
	return 0;
}
