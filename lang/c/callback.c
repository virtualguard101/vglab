#include <stdio.h>

/**
 * @brief 折半「插入点」查找的回调类型.
 *
 * 指向的函数应在有序区间 array[lo..hi] 上查找 key 应插入的下标,
 * 而不是「找到则下标、找不到则 -1」的精确查找.
 *
 * @param array 有序数组 (只读).
 * @param lo    查找区间左端 (含).
 * @param hi    查找区间右端 (含).
 * @param key   待插入关键字.
 * @return 插入下标 pos, 使得将 key 放在 pos 后 [lo..hi] 仍有序.
 *
 * @note 回调指针类型应与函数接口一致.
 */
typedef int (*bsearch_fn)(const int *array, int lo, int hi, int key);

/**
 * @brief 在有序区间 array[lo..hi] 上折半查找 key 的插入位置.
 *
 * @param array 有序数组 (只读).
 * @param lo    左端下标 (含).
 * @param hi    右端下标 (含).
 * @param key   待插入关键字.
 * @return 插入下标. 循环结束时的 lo; 相等元素插到右侧, 排序稳定.
 *
 * @note 用 `array[mid] <= key` 时 lo 右移, 故相等键不会插到已有相等元素左边.
 */
int binary_search_insert(const int *array, int lo, int hi, int key)
{
	while (lo <= hi) {
		int mid = lo + (hi - lo) / 2;

		if (array[mid] <= key)
			lo = mid + 1;
		else
			hi = mid - 1;
	}
	return lo;
}

/**
 * @brief 折半插入排序: 通过回调在有序区中定位插入点.
 *
 * @param bsearch 折半查找插入点的函数指针, 类型见 `bsearch_fn`.
 * @param array   哨兵数组: array[1..n] 为待排序数据, array[0] 作暂存, 调用者须预留.
 * @param n       元素个数 (有效下标 1..n).
 *
 * @note 排序流程固定, 「如何找插入点」交给回调, 体现回调思想:
 *       库负责外层循环与搬移, 调用者只提供查找行为.
 */
void binary_insert_sort(bsearch_fn bsearch, int *array, int n)
{
	int i, j, pos;

	for (i = 2; i <= n; i++) {
		array[0] = array[i];

		/* 回调获取插入位置 */
		pos = bsearch(array, 1, i - 1, array[0]);

		/* 执行插入并后移元素 */
		for (j = i - 1; j >= pos; j--)
			array[j + 1] = array[j];
		array[pos] = array[0];
	}
}

int main(void)
{
	/* 下标 0 不用作数据; 1..5 为待排序 */
	int a[] = { 0, 5, 2, 4, 1, 3 };
	int n = 5;
	int i;

	binary_insert_sort(binary_search_insert, a, n);
	for (i = 1; i <= n; i++)
		printf("%d%c", a[i], i == n ? '\n' : ' ');

	return 0;
}
