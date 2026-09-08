#include <stdio.h>
#include <stdlib.h>

int gcd(int a, int b)
{
	int _a = abs(a);
	int _b = abs(b);
	return _b == 0 ? _a : gcd(_b, _a % _b);
}

int fibonacci(int n)
{
	return n <= 1 ? n : fibonacci(n - 1) + fibonacci(n - 2);
}

int lame(int a, int b)
{
	int min = a < b ? a : b;
	if (b == 0)
		return min >= fibonacci(0);
	return min >= fibonacci(lame(b, a % b));
}

int main()
{
	int a, b;
	printf("Enter two numbers: ");
	scanf("%d %d", &a, &b);
	printf("The GCD of %d and %d is %d\n", a, b, gcd(a, b));
	printf("Lame's Law: %d\n", lame(a, b));
	printf("The Fibonacci of %d is %d\n", a, fibonacci(a));

	return 0;
}
