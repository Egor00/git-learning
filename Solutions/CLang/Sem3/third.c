#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

#include "mymath.h"
#include "myerrors.h"

/*
    Преобразование строки в double.
*/
static ErrorsDef parse_double(const char *str, double *value)
{
    char *end;
    double parsed;

    if (str == NULL || value == NULL || *str == '\0') {
        return INVALID_ARGUMENT;
    }

    errno = 0;
    end = NULL;
    parsed = strtod(str, &end);

    if (end == str || end == NULL || *end != '\0') {
        return INVALID_NUMBER_FORMAT;
    }

    if (errno == ERANGE || !isfinite(parsed)) {
        return INPUT_OVERFLOW;
    }

    *value = parsed;
    return STAT_OK;
}

static ErrorsDef parse_long_long(const char *str, long long *value)
{
    char *end;
    long long parsed;

    if (str == NULL || value == NULL || *str == '\0') {
        return INVALID_ARGUMENT;
    }

    errno = 0;
    end = NULL;
    parsed = strtoll(str, &end, 10);

    if (end == str || end == NULL || *end != '\0') {
        return INVALID_NUMBER_FORMAT;
    }

    if (errno == ERANGE) {
        return INPUT_OVERFLOW;
    }

    *value = parsed;
    return STAT_OK;
}

static ErrorsDef solve_eq(double c1, double c2, double c3)
{
    double coeffs[3] = {c1, c2, c3};

    const int perms[6][3] = {
        {0, 1, 2},
        {0, 2, 1},
        {1, 0, 2},
        {1, 2, 0},
        {2, 0, 1},
        {2, 1, 0}
    };

    int i;

    if (!isfinite(c1) || !isfinite(c2) || !isfinite(c3)) {
        return INVALID_ARGUMENT;
    }

    for (i = 0; i < 6; ++i) {
        double a = coeffs[perms[i][0]];
        double b = coeffs[perms[i][1]];
        double c = coeffs[perms[i][2]];

        if (fabs(a) < eps()) {
            if (fabs(b) < eps()) {
                if (fabs(c) < eps()) {
                    puts("Бесконечное множество решений");
                } else {
                    printf("Нет решений (%.15f != 0)\n", c);
                }
            } else {
                double x = -c / b;

                if (!isfinite(x)) {
                    return FUNC_OVERFLOW;
                }

                printf("x = %.15f (линейное)\n", x);
            }

            continue;
        }

        {
            double b2 = b * b;
            double ac = a * c;
            double four_ac = 4.0 * ac;
            double discriminant;
            double denominator;

            if (!isfinite(b2) ||
                !isfinite(ac) ||
                !isfinite(four_ac)) {
                return FUNC_OVERFLOW;
            }

            discriminant = b2 - four_ac;

            if (!isfinite(discriminant)) {
                return FUNC_OVERFLOW;
            }

            denominator = 2.0 * a;

            if (!isfinite(denominator) || denominator == 0.0) {
                return FUNC_OVERFLOW;
            }

            if (discriminant > eps()) {
                double sqrt_d = sqrt(discriminant);
                double x1 = (-b + sqrt_d) / denominator;
                double x2 = (-b - sqrt_d) / denominator;

                if (!isfinite(x1) || !isfinite(x2)) {
                    return FUNC_OVERFLOW;
                }

                printf("x1 = %.15f, x2 = %.15f\n", x1, x2);
            } else if (fabs(discriminant) < eps()) {
                double x = -b / denominator;

                if (!isfinite(x)) {
                    return FUNC_OVERFLOW;
                }

                printf("x = %.15f\n", x);
            } else {
                puts("Нет действительных корней (D < 0)");
            }
        }
    }

    return STAT_OK;
}

static ErrorsDef check_divisibility(long long n1, long long n2)
{
    if (n2 == 0) {
        return ZERO_DIV;
    }

    /*
        Отдельная обработка LLONG_MIN и -1:
        эта комбинация опасна для операции остатка.
    */
    if (n1 == LLONG_MIN && n2 == -1) {
        printf("%lld кратно %lld\n", n1, n2);
    } else if (n1 % n2 == 0) {
        printf("%lld кратно %lld\n", n1, n2);
    } else {
        printf("%lld не кратно %lld\n", n1, n2);
    }

    return STAT_OK;
}

static ErrorsDef check_triangle(double s1, double s2, double s3)
{
    double sides[3] = {s1, s2, s3};
    double a, b, c;
    double lhs, rhs;
    int i, j;

    if (!isfinite(s1) || !isfinite(s2) || !isfinite(s3)) {
        return INVALID_ARGUMENT;
    }

    /* Сортировка сторон по возрастанию. */
    for (i = 0; i < 2; ++i) {
        for (j = 0; j < 2 - i; ++j) {
            if (sides[j] > sides[j + 1]) {
                double temp = sides[j];
                sides[j] = sides[j + 1];
                sides[j + 1] = temp;
            }
        }
    }

    a = sides[0];
    b = sides[1];
    c = sides[2];

    if (a <= 0.0 || b <= 0.0 || c <= 0.0) {
        return NEG_SIDES;
    }

    lhs = a * a + b * b;
    rhs = c * c;

    if (!isfinite(lhs) || !isfinite(rhs)) {
        return FUNC_OVERFLOW;
    }

    if (fabs(lhs - rhs) < eps()) {
        puts("Стороны прямоугольного треугольника.");
    } else {
        puts("НЕ стороны прямоугольного треугольника.");
    }

    return STAT_OK;
}


static int report_error(ErrorsDef error)
{
    if (error != STAT_OK) {
        errors(error);
    }

    return (int)error;
}

int main(int argc, char *argv[])
{
    ErrorsDef status;

    double epsilon;
    double x1, x2, x3;

    long long n1, n2;

    if (argc < 2 || argv == NULL || argv[1] == NULL) {
        puts("Format: ./third.out flag(-g, -m, -t, /g, /m, /t) args");
        return report_error(MATCH_ARGS);
    }

    if (strcmp(argv[1], "-g") == 0 ||
        strcmp(argv[1], "/g") == 0) {

        if (argc != 6) {
            return report_error(MATCH_ARGS);
        }

        status = parse_double(argv[2], &epsilon);
        if (status != STAT_OK) {
            return report_error(status);
        }

        if (epsilon <= 0.0 || epsilon < DBL_EPSILON) {
            return report_error(INVALID_EPSILON);
        }

        cheps(epsilon);

        status = parse_double(argv[3], &x1);
        if (status != STAT_OK) {
            return report_error(status);
        }

        status = parse_double(argv[4], &x2);
        if (status != STAT_OK) {
            return report_error(status);
        }

        status = parse_double(argv[5], &x3);
        if (status != STAT_OK) {
            return report_error(status);
        }

        return report_error(solve_eq(x1, x2, x3));
    }

    /*
        Флаг -m или /m:
        проверка делимости.
    */
    if (strcmp(argv[1], "-m") == 0 ||
        strcmp(argv[1], "/m") == 0) {

        if (argc != 4) {
            return report_error(MATCH_ARGS);
        }

        status = parse_long_long(argv[2], &n1);
        if (status != STAT_OK) {
            return report_error(status);
        }

        status = parse_long_long(argv[3], &n2);
        if (status != STAT_OK) {
            return report_error(status);
        }

        return report_error(check_divisibility(n1, n2));
    }

    /*
        Флаг -t или /t:
        проверка прямоугольного треугольника.
    */
    if (strcmp(argv[1], "-t") == 0 ||
        strcmp(argv[1], "/t") == 0) {

        if (argc != 6) {
            return report_error(MATCH_ARGS);
        }

        status = parse_double(argv[2], &epsilon);
        if (status != STAT_OK) {
            return report_error(status);
        }

        if (epsilon <= 0.0 || epsilon < DBL_EPSILON) {
            return report_error(INVALID_EPSILON);
        }

        cheps(epsilon);

        status = parse_double(argv[3], &x1);
        if (status != STAT_OK) {
            return report_error(status);
        }

        status = parse_double(argv[4], &x2);
        if (status != STAT_OK) {
            return report_error(status);
        }

        status = parse_double(argv[5], &x3);
        if (status != STAT_OK) {
            return report_error(status);
        }

        return report_error(check_triangle(x1, x2, x3));
    }

    puts("Format: ./third.out flag(-g, -m, -t, /g, /m, /t) args");
    return report_error(INVALID_FLAG);
}
