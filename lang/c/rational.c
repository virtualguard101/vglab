#include <stdio.h>
#include <stdlib.h>


typedef struct {
	int numerator;
	int denominator;
} Rational;


int gcd(int a, int b)
{
	int _a = abs(a);
	int _b = abs(b);
	return _b == 0 ? _a : gcd(_b, _a % _b);
}

int lcm(int a, int b)
{
	return a * b / gcd(a, b);
}

void print_rational(Rational r)
{
	if (r.numerator % r.denominator == 0) {
		printf("%d\n", r.numerator / r.denominator);
	} else if (r.numerator == 0) {
		printf("0\n");
	} else {
		printf("%d/%d\n", r.numerator, r.denominator);
	}
}

Rational divided(Rational r)
{
	if (r.numerator == 0) {
		return (Rational) {
		0, 1};
	} else {
		int _gcd = gcd(r.numerator, r.denominator);
		return (Rational) {
		r.numerator / _gcd, r.denominator / _gcd};
	}
}

Rational add_rational(Rational a, Rational b)
{
	if (a.denominator != b.denominator) {
		a.denominator = a.denominator * b.denominator;
		a.numerator =
		    a.numerator * b.denominator +
		    b.numerator * a.denominator;
	} else {
		a.numerator = a.numerator + b.numerator;
	}

	return divided(a);
}

Rational sub_rational(Rational a, Rational b)
{
	return add_rational(a, (Rational) {
			    -b.numerator, b.denominator}
	);
}

Rational mul_rational(Rational a, Rational b)
{
	return divided((Rational) {
		       a.numerator * b.numerator,
		       a.denominator * b.denominator}
	);
}

Rational div_rational(Rational a, Rational b)
{
	return mul_rational(a, (Rational) {
			    b.denominator, b.numerator}
	);
}

int main()
{
	Rational a = { 1, 5 };
	Rational b = { -5, 1 };

	print_rational(b);
	print_rational(add_rational(a, b));
	print_rational(sub_rational(a, b));
	print_rational(mul_rational(a, b));
	print_rational(div_rational(a, b));

	return 0;
}
