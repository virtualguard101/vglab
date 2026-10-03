#include <stddef.h>
#include <stdio.h>

/**
 * @brief 判断字符是否为一段或多段连续空白 (' ', '\t', '\n', '\r').
 */
#define IS_SPACE(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r')

/**
 * @brief `strcpy` 简易实现. 将 `src` 复制到 `dest` 中, 并返回 `dest` 的指针.
 * 
 * @param dest 目标字符串的指针.
 * @param src 源字符串的指针.
 * @return `dest` 的指针, 便于链式调用.
 * @note 最简写法可将函数体压成三行: 先保存返回用的 dest, 再用
 *       `while ((*dest++ = *src++) != '\0');` 边拷边前进 —— 赋值表达式的值就是
 *       刚写入的字节, 写到 '\0' 时条件为假, 循环结束, 因而无需再单独补结尾.
 */
char *strcpy_sim(char *dest, const char *src)
{
	char *ret = dest;
	while (*src != '\0') {
		*dest++ = *src++;
	}
	*dest = '\0';

	// /* In simply */
	// char* ret = dest;
	// while ((*dest++ = *src++) != '\0');

	return ret;
}

/**
 * @brief 将 `src` 中连续空白压缩后写入 `dest`, 接口语义类似 `strncpy`.
 * 
 * @note 这里的空白采用的是宏定义. 空白的求值必须采用「每次带入当前字符再求值」的
 *       形式 (宏或内联函数), 不能使用一个一次性的 bool 变量缓存起来(`int is_space = ...`):
 *       后者在 src 前进后不会更新, 会导致内层 while 死循环, 或漏掉串中间的空白段.
 *
 * @param dest 调用者提供的缓冲区, 至少 `n` 字节; 函数会改写其内容.
 * @param src  以 '\0' 结尾的源字符串; 只读.
 * @param n    最多写入 `dest` 的字节数. 若压缩结果不足 `n` 字节, 剩余位置
 *             填 '\0' (同 `strncpy`); 若恰好写满 `n` 字节, 不保证以 '\0' 结尾.
 * @return 原先的 `dest`, 便于当作表达式链式使用.
 *
 * @note 调用者须保证: `dest` 足够大、`src` 合法以 '\0' 结尾、二者指向的
 *       内存不重叠. 读写不越界是调用者的责任, 与 `strcpy` / `strncpy` 相同.
 */
char *shrink_space(char *dest, const char *src, size_t n)
{
	size_t i = 0;
	while (*src && i < n) {
		if (IS_SPACE(*src)) {
			while (IS_SPACE(*src)) {
				src++;
			}
			if (*src != '\0') {
				dest[i++] = ' ';
			}
		} else {
			dest[i++] = *src++;
		}
	}
	while (i < n) {
		dest[i++] = '\0';
	}

	return dest;
}

int main(void)
{
	char buf[128];
	const char *book =
	    "This Content hoho       is ok\n"
	    "\tok?\n"
	    "\n" "\tfile system\n" "uttered words   ok ok      ?\n" "end.";

	printf("%s\n", strcpy_sim(buf, "hello"));	/* hello\0 */
	printf("%s\n", shrink_space(buf, "a \t\n  b  ", sizeof buf));	/* a b\0 */
	printf("%s\n", shrink_space(buf, book, sizeof buf));

	return 0;
}
