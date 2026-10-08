#include <stdlib.h>
#include <stdio.h>

#include "mymath.h"

void solve_eq(double c1, double c2, double c3) {

    double coeffs[3] = {c1, c2, c3};
    int perms[6][3] = {
        {0, 1, 2}, {0, 2, 1},
        {1, 0, 2}, {1, 2, 0},
        {2, 0, 1}, {2, 1, 0}
    };
    
    for (int i = 0; i < 6; ++i) {
        double a = coeffs[perms[i][0]];
        double b = coeffs[perms[i][1]];
        double c = coeffs[perms[i][2]];

        if (a<eps()) {
            if (b<eps()) {
                if (c<eps()) {
                    printf("Бесконечное множество решений\n");
                } else {
                    printf("Нет решений (%.15f != 0)\n", c);
                }
            } else {
                double x = -c / b;
                printf("x = %.15f (линейное)\n", x);
            }
            continue;
        }

        double discriminant = b * b - 4 * a * c;

        if (discriminant > eps()) {
            double sqrt_d = sqrt(discriminant);
            double x1 = (-b + sqrt_d) / (2 * a);
            double x2 = (-b - sqrt_d) / (2 * a);
            printf("x1 = %.15f, x2 = %.15f\n", x1, x2);
        } else if (fabs(discriminant) < eps()) {
            double x = -b / (2 * a);
            printf("x = %.15f\n", x);
        } else {
            printf("Нет действительных корней (D < 0)\n");
        }
    }
}

void check_divisibility(long long n1, long long n2) {
    if (n2 == 0) {
        puts("Деление на 0");
    }
    
    if (n1 % n2 == 0) {
        printf("%lld кратно %lld\n", n1, n2);
    } else {
        printf("%lld не кратно %lld\n", n1, n2);
    }
}

void check_triangle(double s1, double s2, double s3) {

    double sides[3] = {s1, s2, s3};
    
    // Простая сортировка пузырьком для 3 элементов
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2 - i; ++j) {
            if (sides[j] > sides[j+1]) {
                double temp = sides[j];
                sides[j] = sides[j+1];
                sides[j+1] = temp;
            }
        }
    }
    
    double a = sides[0];
    double b = sides[1];
    double c = sides[2];

    if (a <= 0 || b <= 0 || c <= 0) {
        printf("Стороны должны быть положительными числами.\n");
        return;
    }

    // Теорема Пифагора с учетом эпсилон
    double lhs = a * a + b * b;
    double rhs = c * c;

    if (fabs(lhs-rhs) < eps()) {
        puts("Стороны прямоугольного треугольника.");
        return;
    } else {
        puts("НЕ стороны прямоугольного треугольника.");
        return;
    }
}


int main(int argc, char *argv[]) {
    switch(argv[1][1]){
        case 'g' :
            cheps(strtod(argv[2], NULL));
            solve_eq(strtod(argv[3], NULL), strtod(argv[4], NULL), strtod(argv[5], NULL));
            break;
        case 'm' :
            check_divisibility(strtod(argv[2], NULL), strtod(argv[3], NULL));
            break;
        case 't' :
            cheps(strtod(argv[2], NULL));
            check_triangle(strtod(argv[3], NULL), strtod(argv[4], NULL), strtod(argv[5], NULL));
            break;
        default :
            puts("default");
            break;
    }
}