int select_sort(int arr[], int len)
{
	int min;
	for (int i = 0; i < len - 1; ++i) {
		/* 选取当前无序区间的最小元素, 默认第一个元素 */
		min = i;
		for (int j = i + 1; j < len; ++j) {
			if (arr[j] < arr[min]) {
				min = j;
			}
		}

		/* 将最小元素与当前无序区间的第一个元素交换 */
		if (min != i) {
			int tmp = arr[i];
			arr[i] = arr[min];
			arr[min] = tmp;
		}
	}
	return 0;
}
