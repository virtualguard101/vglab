#include <math.h>
#include <stdio.h>

typedef struct {
    double x, y;
} Complex;

double real(Complex c) {
    return c.x;
}
double imag(Complex c) {
    return c.y;
}
double magnitude(Complex c) {
    return sqrt(c.x * c.x + c.y * c.y);
}
double angle(Complex c) {
    return atan2(c.y, c.x);
}

Complex make_from_real_imag(double x, double y) {
    Complex c;
    c.x = x;
    c.y = y;
    return c;
}

Complex make_from_mag_ang(double r, double theta) {
    Complex c;
    c.x = r * cos(theta);
    c.y = r * sin(theta);
    return c;
}

Complex add(Complex a, Complex b) {
    return make_from_real_imag(real(a) + real(b), imag(a) + imag(b));
}
Complex sub(Complex a, Complex b) {
    return make_from_real_imag(real(a) - real(b), imag(a) - imag(b));
}
Complex mul(Complex a, Complex b) {
    return make_from_mag_ang(magnitude(a) * magnitude(b), angle(a) + angle(b));
}
Complex div(Complex a, Complex b) {
    return make_from_mag_ang(magnitude(a) / magnitude(b), angle(a) - angle(b));
}

void print_complex(Complex c) {
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

int main() {
    Complex c1 = make_from_real_imag(1, 2);
    Complex c2 = make_from_mag_ang(3, 4);
    print_complex(add(c1, c2));
    print_complex(sub(c1, c2));
    print_complex(mul(c1, c2));
    print_complex(div(c1, c2));

    return 0;
}
