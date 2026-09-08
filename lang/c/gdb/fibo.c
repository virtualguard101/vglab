#include <stdio.h>

int fibonacci(int n)
{
	return n <= 1 ? n : fibonacci(n - 1) + fibonacci(n - 2);
}

int main()
{
	int n = 5;
	printf("The %dth Fibonacci number is %d\n", n, fibonacci(n));

	return 0;
}
