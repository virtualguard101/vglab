#include <stdio.h>
#include <string.h>

int bubble_sort(int arr[], int len, int *cmp, int *mov)
{
	int i, j, tmp, swapped;

	*cmp = *mov = 0;
	for (i = 0; i < len - 1; ++i) {
		swapped = 0;
		for (j = 0; j < len - 1 - i; ++j) {
			++*cmp;
			if (arr[j] > arr[j + 1]) {
				tmp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = tmp;
				*mov += 3;
				swapped = 1;
			}
		}
		if (!swapped)
			break;	/* 本趟无交换, 已经有序 */
	}
	return 0;
}

int partition(int arr[], int left, int right, int *cmp, int *mov)
{
	int pivot = arr[left];	/* 取第一个元素为枢轴 */

	++*mov;
	while (left < right) {
		/* 从右往左, 找到第一个小于枢轴的元素 */
		while (left < right) {
			++*cmp;
			if (arr[right] < pivot)
				break;
			--right;
		}
		arr[left] = arr[right];
		++*mov;

		/* 从左往右, 找到第一个大于枢轴的元素 */
		while (left < right) {
			++*cmp;
			if (arr[left] > pivot)
				break;
			++left;
		}
		arr[right] = arr[left];
		++*mov;
	}
	arr[left] = pivot;
	++*mov;
	return left;
}

int quick_sort(int arr[], int left, int right, int *cmp, int *mov)
{
	if (left < right) {
		int mid = partition(arr, left, right, cmp, mov);
		quick_sort(arr, left, mid - 1, cmp, mov);
		quick_sort(arr, mid + 1, right, cmp, mov);
	}
	return 0;
}

static int quick_sort_n(int arr[], int n, int *cmp, int *mov)
{
	*cmp = *mov = 0;
	if (n > 0)
		quick_sort(arr, 0, n - 1, cmp, mov);
	return 0;
}

typedef int (*sort_fn)(int *, int, int *, int *);

static void run(const char *name, const char *algo, sort_fn fn,
		const int *src, int n)
{
	int buf[32];
	int cmp, mov, i;

	memcpy(buf, src, (size_t) n * sizeof(int));
	fn(buf, n, &cmp, &mov);
	printf("  %-10s %-8s %6d %6d", name, algo, cmp, mov);
	printf("\n");
}

int main()
{
	const int bubble_ex[] = { 5, 2, 4, 1, 3 };
	const int quick_ex[] = { 49, 38, 65, 97, 76, 13, 27, 49 };
	const int sorted[] = { 1, 2, 3, 4, 5 };
	const int reversed[] = { 5, 4, 3, 2, 1 };
	const int dups[] = { 3, 1, 3, 2, 1 };
	const int equal[] = { 2, 2, 2, 2, 2 };
	const int *groups[] =
	    { bubble_ex, quick_ex, sorted, reversed, dups,
		equal
	};
	const char *names[] = { "bubble_ex", "quick_ex", "sorted",
		"reversed", "dups", "equal"
	};
	const int lens[] = { 5, 8, 5, 5, 5, 5 };
	int g;

	printf("%-12s %-8s %6s %6s\n", "", "", "Cmp", "Mov");
	for (g = 0; g < 6; ++g) {
		run(names[g], "bubble", bubble_sort, groups[g], lens[g]);
		run(names[g], "quick", quick_sort_n, groups[g], lens[g]);
	}
	return 0;
}
