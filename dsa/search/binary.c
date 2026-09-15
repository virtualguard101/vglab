#include <assert.h>
#include <math.h>
#include <stdio.h>


int is_sorted(int *arr, int n)
{
	for (int i = 0; i < n - 1; ++i) {
		if (arr[i] > arr[i + 1]) {
			return 0;
		}
	}
	return 1;
}

int binary_search(int *arr, int n, int key)
{
	assert(is_sorted(arr, n));

	int left = 0;
	int right = n - 1;

	while (left <= right) {
		int mid = (left + right) / 2;
		if (arr[mid] == key) {
			return mid;
		} else if (arr[mid] < key) {
			left = mid + 1;	/* 继续查找右半部分 */
		} else {
			right = mid - 1;	/* 继续查找左半部分 */
		}
	}
	return -1;
}

int binary_search_first_occ(int *arr, int n, int key)
{
	assert(is_sorted(arr, n));

	int left = 0;
	int right = n - 1;

	while (left <= right) {
		int mid = (left + right) / 2;

		if (arr[mid] >= key) {	/* 相等也往左 */
			right = mid - 1;
		} else {
			left = mid + 1;
		}
	}

	if (left < n && arr[left] == key) {	/* 是否命中 */
		return left;
	}
	return -1;
}

double binary_sqrt(double n, double acc)
{
	assert(n >= 0 && acc > 0);

	double left = 0;
	double right = n < 1 ? 1 : n;	/* n < 1 时根在 (n, 1] */
	double est;

	do {
		est = (left + right) / 2;
		if (est * est < n) {
			left = est;
		} else {
			right = est;
		}
	} while (right - left > acc && fabs(est * est - n) > acc);

	return est;
}

double binary_pow(double base, int exponent)
{
	assert(exponent >= 0);

	if (exponent == 0) {
		return 1;
	}

	double half = binary_pow(base, exponent / 2);
	if (exponent % 2 == 0) {
		return half * half;
	}
	return half * half * base;
}


int main()
{
	int arr[] = { 1, 2, 3, 3, 3, 4, 5 };
	int arr_n = sizeof(arr) / sizeof(arr[0]);
	int result = binary_search_first_occ(arr, arr_n, 3);

	printf("Found 3 at index(first occurrence): %d\n", result);
	printf("Found 3 at index(normal binary): %d\n",
	       binary_search(arr, arr_n, 3));

	printf("2^5 = %f\n", binary_pow(2, 5));
	printf("sqrt(2) with accuracy 0.000001: %f\n",
	       binary_sqrt(2, 0.000001));

	return 0;
}
