#include <stdio.h>
#include <stdlib.h>
#include <string.h>


enum { CHUNK = 1 << 20 };	/* 1 << 20 == 2^20 字节 (1 MiB) */

size_t mem_consume(void)
{
	size_t total = 0;

	for (;;) {
		void *p = malloc(CHUNK);
		if (p == NULL)
			return total;
		/**
		 * Linux 中通常会开启 overcommit:
		 * https://www.ibm.com/docs/es/linux-on-systems?topic=considerations-memory-overcommitment
		 * 只 malloc 不写页时虚拟地址很容易一直成功.
		 * 写入整块, 才会真正提交物理页 / 触发失败或 OOM.
		 */
		memset(p, 0, CHUNK);
		total += CHUNK;
		if ((total / CHUNK) % 64 == 0)
			printf("allocated %zu MiB\n", total >> 20);
		/* 故意不 free: 指针泄漏, 块一直占着 */
	}
}

int main(void)
{
	size_t n = mem_consume();
	printf("malloc failed after %zu bytes (%zu MiB)\n", n, n >> 20);
	return 0;
}
