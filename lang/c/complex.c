#include <math.h>
#include <stdio.h>
#include <stdlib.h>


enum coordinate_type { RECTANGULAR = 1, POLAR };
typedef struct {
	enum coordinate_type t;
	double a, b;
} Complex;

Complex make_from_real_imag(double x, double y)
{
	Complex c;
	c.a = x;
	c.b = y;
	c.t = RECTANGULAR;
	return c;
}

Complex make_from_mag_ang(double r, double theta)
{
	Complex c;
	c.a = r;
	c.b = theta;
	c.t = POLAR;
	return c;
}


double real(Complex c)
{
	if (c.t == RECTANGULAR) {
		return c.a;
	} else if (c.t == POLAR) {
		return c.a * cos(c.b);
	} else {
		printf("Invalid complex coordinate type\n");
		exit(EXIT_FAILURE);
	}
}

double imag(Complex c)
{
	if (c.t == RECTANGULAR) {
		return c.b;
	} else if (c.t == POLAR) {
		return c.a * sin(c.b);
	} else {
		printf("Invalid complex coordinate type\n");
		exit(EXIT_FAILURE);
	}
}

double magnitude(Complex c)
{
	if (c.t == RECTANGULAR) {
		return sqrt(c.a * c.a + c.b * c.b);
	} else if (c.t == POLAR) {
		return c.a;
	} else {
		printf("Invalid complex coordinate type\n");
		exit(EXIT_FAILURE);
	}
}

double angle(Complex c)
{
	if (c.t == RECTANGULAR) {
		return atan2(c.b, c.a);
	} else if (c.t == POLAR) {
		return c.b;
	} else {
		printf("Invalid complex coordinate type\n");
		exit(EXIT_FAILURE);
	}
}


Complex add_complex(Complex a, Complex b)
{
	return make_from_real_imag(real(a) + real(b), imag(a) + imag(b));
}

Complex sub_complex(Complex a, Complex b)
{
	return make_from_real_imag(real(a) - real(b), imag(a) - imag(b));
}

Complex mul_complex(Complex a, Complex b)
{
	return make_from_mag_ang(magnitude(a) * magnitude(b),
				 angle(a) + angle(b));
}

Complex div_complex(Complex a, Complex b)
{
	return make_from_mag_ang(magnitude(a) / magnitude(b),
				 angle(a) - angle(b));
}

void print_complex(Complex c)
{
	if (real(c) == 0) {
		printf("%fi\n", imag(c));
	} else {
		if (imag(c) == 0) {
			printf("%f\n", real(c));
		} else if (imag(c) > 0) {
			printf("%f+%fi\n", real(c), imag(c));
		} else {
			printf("%f%fi\n", real(c), imag(c));
		}
	}
}

int main()
{
	Complex c1 = make_from_real_imag(sqrt(3), 1);
	Complex c2 = make_from_mag_ang(2, M_PI / 6);
	print_complex(add_complex(c1, c2));
	print_complex(sub_complex(c1, c2));
	print_complex(mul_complex(c1, c2));
	print_complex(div_complex(c1, c2));

	return 0;
}
