#include <stdio.h>
#include <string.h>


int insert_sort(int arr[], int len, int *cmp, int *mov)
{
	int key, i, j;

	*cmp = *mov = 0;
	for (j = 1; j < len; ++j) {
		key = arr[j];	/* 待插入的元素 */
		++*mov;
		i = j - 1;	/* 已排序的最后一个元素 */
		while (i >= 0) {
			++*cmp;
			if (arr[i] <= key)
				break;
			arr[i + 1] = arr[i];	/* 后移比待插入元素大的元素 */
			++*mov;
			--i;	/* 继续向前比较 */
		}
		arr[i + 1] = key;	/* 插入待插入元素 */
		++*mov;
	}
	return 0;
}

/* arr[0] 为哨兵, 有效数据在 arr[1..len-1] */
int insert_sort_with_sentinel(int arr[], int len, int *cmp, int *mov)
{
	int i, j;

	*cmp = *mov = 0;
	for (j = 2; j < len; ++j) {
		++*cmp;
		if (arr[j] < arr[j - 1]) {	/* 小于前驱才需要插入 */
			arr[0] = arr[j];	/* 将待插入元素暂存到哨兵位置 */
			++*mov;
			for (i = j - 1; arr[0] < arr[i]; --i) {
				++*cmp;
				arr[i + 1] = arr[i];	/* 记录后移 */
				++*mov;
			}
			++*cmp;	/* 循环因 arr[0] >= arr[i] 结束 */
			arr[i + 1] = arr[0];	/* 插入到正确位置 */
			++*mov;
		}
	}
	return 0;
}

int insert_sort_binary(int arr[], int len, int *cmp, int *mov)
{
	int i, j, low, high, mid;

	*cmp = *mov = 0;
	for (j = 2; j < len; ++j) {
		arr[0] = arr[j];	/* 哨兵 */
		++*mov;
		low = 1;	/* 有序模块的第一个元素下标 */
		high = j - 1;	/* 有序模块的最后一个元素下标 */

		/* 折半查找插入位置 */
		while (low <= high) {
			mid = (low + high) / 2;
			++*cmp;
			if (arr[mid] > arr[0])	/* 插入位置应在 mid 左边 */
				high = mid - 1;
			else	/* 插入位置应在 mid 右边 */
				low = mid + 1;
		}

		for (i = j - 1; i >= high + 1; --i) {
			arr[i + 1] = arr[i];	/* 后移带插入位置后的所有元素 */
			++*mov;
		}
		arr[high + 1] = arr[0];
		++*mov;
	}
	return 0;
}

typedef int (*sort_fn)(int *, int, int *, int *);

static void run(const char *name, const char *algo, sort_fn fn,
		const int *src, int n, int with_slot0)
{
	int buf[32];
	int cmp, mov;

	if (with_slot0) {
		buf[0] = 0;
		memcpy(buf + 1, src, (size_t) n * sizeof(int));
		fn(buf, n + 1, &cmp, &mov);
	} else {
		memcpy(buf, src, (size_t) n * sizeof(int));
		fn(buf, n, &cmp, &mov);
	}
	printf("  %-10s %-10s %6d %6d\n", name, algo, cmp, mov);
}

int main()
{
	const int akaedu[] = { 10, 5, 2, 4, 7 };
	const int sorted[] = { 1, 2, 3, 4, 5 };
	const int reversed[] = { 5, 4, 3, 2, 1 };
	const int dups[] = { 3, 1, 3, 2, 1 };
	const int *groups[] = { akaedu, sorted, reversed, dups };
	const char *names[] = { "akaedu", "sorted", "reversed", "dups" };
	int g;

	printf("%-12s %-10s %6s %6s\n", "", "", "Cmp", "Mov");
	for (g = 0; g < 4; ++g) {
		run(names[g], "plain", insert_sort, groups[g], 5, 0);
		run(names[g], "sentinel", insert_sort_with_sentinel,
		    groups[g], 5, 1);
		run(names[g], "binary", insert_sort_binary, groups[g], 5,
		    1);
	}
	return 0;
}
