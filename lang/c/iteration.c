#include <stdio.h>

int multi_table(int n) {
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= i; ++j) {
            printf("%d*%d=%2d\t", j, i, j*i);
        }
        printf("\n");
    }
    return 0;
}

int diamond(int n, char c) {
    if (n % 2 == 0) {
        printf("n must be odd\n");
    }

    for (int i = 0; i <= n/2; ++i) {
        for (int j = 0; j < n/2 - i; ++j) {
            printf("\t");
        }
        for (int j = 0; j < 2*i + 1; ++j) {
            printf("%c\t", c);
        }
        printf("\n");
    }

    for (int i = n/2 - 1; i >= 0; --i) {
        for (int j = 0; j < n/2 - i; ++j) {
            printf("\t");
        }
        for (int j = 0; j < 2*i + 1; ++j) {
            printf("%c\t", c);
        }
        printf("\n");
    }
    return 0;
}

int main() {
    multi_table(9);
    diamond(5, '+');

    return 0;
}
