#include <stdio.h>


int partition(int *arr, int left, int right)
{
	int pivot = arr[left];
	while (left < right) {
		while (left < right && arr[right] >= pivot) {
			--right;
		}
		arr[left] = arr[right];

		while (left < right && arr[left] <= pivot) {
			++left;
		}
		arr[right] = arr[left];
	}
	arr[left] = pivot;
	return left;
}

/* 从start到end之间找出第k小的元素 */
int order_statistics(int *arr, int start, int end, int k)
{
	/* 用partition函数把序列分成两半，中间的pivot元素是序列中的第i个 */
	int pivot = partition(arr, start, end);
	if (k == pivot) {
		return arr[pivot];
	} else if (k < pivot) {
		/* 从左半部分找出第k-i小的元素并返回 */
		return order_statistics(arr, start, pivot - 1, k);
	} else {
		/* 从右半部分找出第k-i小的元素并返回 */
		return order_statistics(arr, pivot + 1, end, k - pivot);
	}
}

int main()
{
	int arr[] = { 3, 2, 1, 5, 6, 4 };
	int n = sizeof(arr) / sizeof(arr[0]);
	int k = 0;
	int result = order_statistics(arr, 0, n - 1, k);
	printf("The %dth smallest element is %d\n", k + 1, result);
	return 0;
}
