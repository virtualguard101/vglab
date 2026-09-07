#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SIZE 10000
#define MAX_VAL 10

int arr[MAX_SIZE];

void gen_random_seq(int min, int max)
{
	for (int i = 0; i < MAX_SIZE; ++i) {
		arr[i] = rand() % (max - min + 1) + min;
	}
}

int counter(int val)
{
	int count = 0;
	for (int i = 0; i < MAX_SIZE; ++i) {
		if (arr[i] == val) {
			count++;
		}
	}
	return count;
}

void print_histogram(int hist[], int n)
{
	int max = 0;
	// find the max value in the histogram
	for (int i = 0; i < n; ++i) {
		if (hist[i] > max) {
			max = hist[i];
		}
	}

	for (int h = max; h >= 1; --h) {
		for (int i = 0; i < n; ++i) {
			printf(hist[i] >= h ? "*  " : "   ");
		}
		putchar('\n');
	}

	for (int i = 0; i < n; ++i) {
		printf("%-3d", i);
	}
	putchar('\n');
}

int main()
{
	srand(time(NULL));

	int histogram[MAX_VAL] = { 0 };

	gen_random_seq(0, MAX_VAL - 1);
	for (int i = 0; i < MAX_VAL; ++i) {
		histogram[arr[i]]++;
	}

	print_histogram(histogram, MAX_VAL);

	return 0;
}
