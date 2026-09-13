#include <stdio.h>
#include <string.h>

int merge(int arr[], int start, int mid, int end, int *cmp, int *mov)
{
	int n1 = mid - start + 1;	/* 左临时数组长度 */
	int n2 = end - mid;	/* 右临时数组长度 */
	int left[n1], right[n2];
	int i, j, k;

	for (i = 0; i < n1; ++i) {
		left[i] = arr[start + i];
		++*mov;
	}
	for (j = 0; j < n2; ++j) {
		right[j] = arr[mid + 1 + j];
		++*mov;
	}
	i = j = 0;
	k = start;
	/* 取两侧当前最小者写回; <= 时取左, 保证稳定 */
	while (i < n1 && j < n2) {
		++*cmp;
		if (left[i] <= right[j])
			arr[k++] = left[i++];
		else
			arr[k++] = right[j++];
		++*mov;
	}
	while (i < n1) {
		arr[k++] = left[i++];
		++*mov;
	}
	while (j < n2) {
		arr[k++] = right[j++];
		++*mov;
	}
	return 0;
}

int merge_sort_recur(int arr[], int start, int end, int *cmp, int *mov)
{
	if (start < end) {
		int mid = (start + end) / 2;
		merge_sort_recur(arr, start, mid, cmp, mov);
		merge_sort_recur(arr, mid + 1, end, cmp, mov);
		merge(arr, start, mid, end, cmp, mov);
	}
	return 0;
}

int merge_sort_iter(int arr[], int n, int *cmp, int *mov)
{
	int width, i, mid, end;

	*cmp = *mov = 0;
	for (width = 1; width < n; width *= 2) {
		for (i = 0; i + width < n; i += 2 * width) {
			mid = i + width - 1;
			end = i + 2 * width - 1;
			if (end > n - 1)
				end = n - 1;	/* 右段可能不足 width */
			merge(arr, i, mid, end, cmp, mov);
		}
	}
	return 0;
}

static int merge_sort_recur_n(int arr[], int n, int *cmp, int *mov)
{
	*cmp = *mov = 0;
	if (n > 0)
		merge_sort_recur(arr, 0, n - 1, cmp, mov);
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

int main(void)
{
	const int akaedu[] = { 5, 2, 4, 7, 1, 3, 2, 6 };
	const int leftover[] = { 5, 2, 4, 7, 1 };
	const int sorted[] = { 1, 2, 3, 4, 5 };
	const int reversed[] = { 5, 4, 3, 2, 1 };
	const int dups[] = { 3, 1, 3, 2, 1 };
	const int *groups[] = { akaedu, leftover, sorted, reversed, dups };
	const char *names[] = { "akaedu", "leftover", "sorted", "reversed",
		"dups"
	};
	const int lens[] = { 8, 5, 5, 5, 5 };
	int g;

	printf("%-12s %-8s %6s %6s\n", "", "", "Cmp", "Mov");
	for (g = 0; g < 5; ++g) {
		run(names[g], "recur", merge_sort_recur_n, groups[g],
		    lens[g]);
		run(names[g], "iter", merge_sort_iter, groups[g], lens[g]);
	}
	return 0;
}
