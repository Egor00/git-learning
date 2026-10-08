#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#define EPS 1e-10
#define MAX_INPUT 256

/*
    Коды возврата:
    0 - успех
    1 - некорректные параметры
    2 - ошибка вычисления
    3 - ошибка выделения памяти
*/

/* ============================================================
   1. ПРОВЕРКА ВЫПУКЛОСТИ МНОГОУГОЛЬНИКА
   ============================================================ */

/*
    count - количество вершин.
    После count передаются пары:
    x1, y1, x2, y2, ..., xn, yn
*/
int is_convex_polygon(int count, int *result, ...)
{
    va_list args;
    double *x;
    double *y;
    int i;
    int sign = 0;

    if (result == NULL || count < 3) {
        return 1;
    }

    x = (double *)malloc((size_t)count * sizeof(double));
    y = (double *)malloc((size_t)count * sizeof(double));

    if (x == NULL || y == NULL) {
        free(x);
        free(y);
        return 3;
    }

    va_start(args, result);

    for (i = 0; i < count; ++i) {
        x[i] = va_arg(args, double);
        y[i] = va_arg(args, double);
    }

    va_end(args);

    for (i = 0; i < count; ++i) {
        int a = i;
        int b = (i + 1) % count;
        int c = (i + 2) % count;

        double cross =
            (x[b] - x[a]) * (y[c] - y[b]) -
            (y[b] - y[a]) * (x[c] - x[b]);

        if (fabs(cross) > EPS) {
            int current_sign = (cross > 0.0) ? 1 : -1;

            if (sign == 0) {
                sign = current_sign;
            } else if (sign != current_sign) {
                free(x);
                free(y);

                *result = 0;
                return 0;
            }
        }
    }

    free(x);
    free(y);

    /*
        Если все точки лежат на одной прямой,
        это не является выпуклым многоугольником.
    */
    if (sign == 0) {
        *result = 0;
    } else {
        *result = 1;
    }

    return 0;
}

/* ============================================================
   2. ВЫЧИСЛЕНИЕ МНОГОЧЛЕНА
   ============================================================ */

/*
    degree - степень многочлена.

    После degree передаются коэффициенты:
    a_n, a_(n-1), ..., a_1, a_0

    Например:
    2*x^2 + 3*x + 5

    evaluate_polynomial(2.0, 2, &result, 2.0, 3.0, 5.0);
*/
int evaluate_polynomial(
    double x,
    int degree,
    double *result,
    ...
)
{
    va_list args;
    double value;
    int i;

    if (result == NULL || degree < 0 || !isfinite(x)) {
        return 1;
    }

    va_start(args, result);

    value = va_arg(args, double);

    for (i = 1; i <= degree; ++i) {
        double coefficient = va_arg(args, double);
        value = value * x + coefficient;
    }

    va_end(args);

    if (!isfinite(value)) {
        return 2;
    }

    *result = value;

    return 0;
}

/* ============================================================
   3. ЧИСЛА КАПРЕКАРА
   ============================================================ */

/*
    Перевод одного символа в значение цифры.
*/
int digit_value(char c, int *value)
{
    if (value == NULL) {
        return 1;
    }

    if (c >= '0' && c <= '9') {
        *value = c - '0';
    } else if (c >= 'A' && c <= 'Z') {
        *value = c - 'A' + 10;
    } else if (c >= 'a' && c <= 'z') {
        *value = c - 'a' + 10;
    } else {
        return 1;
    }

    return 0;
}

/*
    Проверяет строковое представление неотрицательного числа
    в системе счисления base и переводит его в unsigned long long.
*/
int parse_number_base(
    const char *str,
    int base,
    unsigned long long *value
)
{
    size_t i;
    unsigned long long result = 0;

    if (str == NULL || value == NULL || base < 2 || base > 36) {
        return 1;
    }

    if (str[0] == '\0') {
        return 1;
    }

    for (i = 0; str[i] != '\0'; ++i) {
        int digit;

        if (digit_value(str[i], &digit) != 0) {
            return 1;
        }

        if (digit >= base) {
            return 1;
        }

        if (result > (ULLONG_MAX - (unsigned long long)digit)
                    / (unsigned long long)base) {
            return 2;
        }

        result = result * (unsigned long long)base
               + (unsigned long long)digit;
    }

    *value = result;

    return 0;
}

/*
    Проверка числа Капрекара.

    Например, в десятичной системе:
    9^2 = 81 -> 8 + 1 = 9
    45^2 = 2025 -> 20 + 25 = 45
*/
int is_kaprekar(
    unsigned long long number,
    int base,
    int *result
)
{
    unsigned long long square;
    unsigned long long divisor = 1;
    unsigned long long right;
    unsigned long long left;
    unsigned long long temp;

    if (result == NULL || base < 2 || base > 36) {
        return 1;
    }

    if (number == 0) {
        *result = 1;
        return 0;
    }

    if (number > ULLONG_MAX / number) {
        return 2;
    }

    square = number * number;

    /*
        Определяем количество цифр числа number
        в системе счисления base.
    */
    temp = number;

    while (temp >= (unsigned long long)base) {
        temp /= (unsigned long long)base;

        if (divisor > ULLONG_MAX / (unsigned long long)base) {
            return 2;
        }

        divisor *= (unsigned long long)base;
    }

    right = square % divisor;
    left = square / divisor;

    if (left + right == number) {
        *result = 1;
    } else {
        *result = 0;
    }

    return 0;
}

/*
    Среди переданных строк находит числа Капрекара.

    count - количество строк.
    После count передаются const char *.
*/
int find_kaprekar_numbers(
    int base,
    int count,
    int *result_count,
    ...
)
{
    va_list args;
    int i;
    int found = 0;

    if (result_count == NULL ||
        count < 0 ||
        base < 2 ||
        base > 36) {
        return 1;
    }

    va_start(args, result_count);

    for (i = 0; i < count; ++i) {
        const char *str;
        unsigned long long number;
        int kaprekar;

        str = va_arg(args, const char *);

        if (str == NULL) {
            va_end(args);
            return 1;
        }

        if (parse_number_base(str, base, &number) != 0) {
            va_end(args);
            return 1;
        }

        if (is_kaprekar(number, base, &kaprekar) != 0) {
            va_end(args);
            return 2;
        }

        if (kaprekar) {
            ++found;
            printf("Число Капрекара: %s\n", str);
        }
    }

    va_end(args);

    *result_count = found;

    return 0;
}

/* ============================================================
   4. СРЕДНЕЕ ГЕОМЕТРИЧЕСКОЕ
   ============================================================ */

/*
    count - количество чисел.
    После count передаются double.

    Например:
    geometric_mean(4, &result, 1.0, 2.0, 4.0, 8.0);
*/
int geometric_mean(
    int count,
    double *result,
    ...
)
{
    va_list args;
    double logarithm_sum = 0.0;
    int i;

    if (result == NULL || count <= 0) {
        return 1;
    }

    va_start(args, result);

    for (i = 0; i < count; ++i) {
        double value = va_arg(args, double);

        /*
            Среднее геометрическое вещественных чисел
            в данном варианте определяем для положительных чисел.
        */
        if (!isfinite(value) || value <= 0.0) {
            va_end(args);
            return 1;
        }

        logarithm_sum += log(value);

        if (!isfinite(logarithm_sum)) {
            va_end(args);
            return 2;
        }
    }

    va_end(args);

    *result = exp(logarithm_sum / (double)count);

    if (!isfinite(*result)) {
        return 2;
    }

    return 0;
}

/* ============================================================
   5. БЫСТРОЕ РЕКУРСИВНОЕ ВОЗВЕДЕНИЕ В СТЕПЕНЬ
   ============================================================ */

/*
    Рекурсивное быстрое возведение в степень.

    x^n:
        n = 0 -> 1
        n > 0 -> квадрат половины степени
        n < 0 -> 1 / x^(-n)
*/
int fast_power(
    double x,
    long long n,
    double *result
)
{
    double half;
    double value;

    if (result == NULL || !isfinite(x)) {
        return 1;
    }

    if (n == 0) {
        *result = 1.0;
        return 0;
    }

    if (x == 0.0 && n < 0) {
        return 1;
    }

    /*
        Специальный случай минимального значения long long,
        чтобы не выполнять -n напрямую.
    */
    if (n == LLONG_MIN) {
        double positive_part;

        if (fast_power(x, -(n + 1), &positive_part) != 0) {
            return 2;
        }

        value = positive_part * x;

        if (value == 0.0) {
            return 1;
        }

        *result = 1.0 / value;
        return 0;
    }

    if (n < 0) {
        if (fast_power(x, -n, &value) != 0) {
            return 2;
        }

        if (value == 0.0) {
            return 1;
        }

        *result = 1.0 / value;
        return 0;
    }

    if (n % 2 == 0) {
        if (fast_power(x, n / 2, &half) != 0) {
            return 2;
        }

        *result = half * half;
    } else {
        if (fast_power(x, n - 1, &half) != 0) {
            return 2;
        }

        *result = x * half;
    }

    if (!isfinite(*result)) {
        return 2;
    }

    return 0;
}

/* ============================================================
   6. МЕТОД ДИХОТОМИИ
   ============================================================ */

typedef double (*EquationFunction)(double);

/*
    f(x) = x^2 - 2
*/
double equation_1(double x)
{
    return x * x - 2.0;
}

/*
    f(x) = x^3 - x - 2
*/
double equation_2(double x)
{
    return x * x * x - x - 2.0;
}

/*
    f(x) = cos(x) - x
*/
double equation_3(double x)
{
    return cos(x) - x;
}

/*
    Поиск корня на [left, right] методом дихотомии.
*/
int bisection(
    double left,
    double right,
    double epsilon,
    EquationFunction function,
    double *result
)
{
    double f_left;
    double f_right;
    int iterations = 0;
    const int max_iterations = 1000000;

    if (result == NULL ||
        function == NULL ||
        !isfinite(left) ||
        !isfinite(right) ||
        !isfinite(epsilon) ||
        epsilon <= 0.0 ||
        left >= right) {
        return 1;
    }

    f_left = function(left);
    f_right = function(right);

    if (!isfinite(f_left) || !isfinite(f_right)) {
        return 2;
    }

    /*
        Если один из концов уже является корнем.
    */
    if (fabs(f_left) <= epsilon) {
        *result = left;
        return 0;
    }

    if (fabs(f_right) <= epsilon) {
        *result = right;
        return 0;
    }

    /*
        Для метода дихотомии функция должна менять знак
        на концах интервала.
    */
    if (f_left * f_right > 0.0) {
        return 1;
    }

    while ((right - left) / 2.0 > epsilon) {
        double middle;
        double f_middle;

        if (iterations >= max_iterations) {
            return 2;
        }

        middle = left + (right - left) / 2.0;
        f_middle = function(middle);

        if (!isfinite(f_middle)) {
            return 2;
        }

        if (fabs(f_middle) <= epsilon) {
            *result = middle;
            return 0;
        }

        if (f_left * f_middle < 0.0) {
            right = middle;
            f_right = f_middle;
        } else {
            left = middle;
            f_left = f_middle;
        }

        ++iterations;
    }

    *result = left + (right - left) / 2.0;

    return 0;
}

/* ============================================================
   MAIN
   ============================================================ */

int main(void)
{
    int status;

    /* --------------------------------------------------------
       1. Выпуклый многоугольник
       -------------------------------------------------------- */

    {
        int convex;

        status = is_convex_polygon(
            4,
            &convex,
            0.0, 0.0,
            4.0, 0.0,
            4.0, 3.0,
            0.0, 3.0
        );

        if (status != 0) {
            printf("Ошибка проверки многоугольника.\n");
            return 1;
        }

        printf("1. Многоугольник выпуклый: %s\n",
               convex ? "да" : "нет");
    }

    /* --------------------------------------------------------
       2. Многочлен
       2x^3 + 3x^2 + 4x + 5
       при x = 2
       -------------------------------------------------------- */

    {
        double polynomial;

        status = evaluate_polynomial(
            2.0,
            3,
            &polynomial,
            2.0,
            3.0,
            4.0,
            5.0
        );

        if (status != 0) {
            printf("Ошибка вычисления многочлена.\n");
            return 1;
        }

        printf("2. Значение многочлена: %.10f\n", polynomial);
    }

    /* --------------------------------------------------------
       3. Числа Капрекара
       -------------------------------------------------------- */

    {
        int count;

        printf("\n3. Числа Капрекара:\n");

        status = find_kaprekar_numbers(
            10,
            7,
            &count,
            "1",
            "9",
            "10",
            "45",
            "55",
            "99",
            "297"
        );

        if (status != 0) {
            printf("Ошибка поиска чисел Капрекара.\n");
            return 1;
        }

        printf("Найдено чисел Капрекара: %d\n", count);
    }

    /* --------------------------------------------------------
       4. Среднее геометрическое
       -------------------------------------------------------- */

    {
        double result;

        status = geometric_mean(
            4,
            &result,
            1.0,
            2.0,
            4.0,
            8.0
        );

        if (status != 0) {
            printf("Ошибка вычисления среднего геометрического.\n");
            return 1;
        }

        printf("\n4. Среднее геометрическое: %.10f\n", result);
    }

    /* --------------------------------------------------------
       5. Быстрое возведение в степень
       -------------------------------------------------------- */

    {
        double result;

        status = fast_power(2.0, 10, &result);

        if (status != 0) {
            printf("Ошибка возведения в степень.\n");
            return 1;
        }

        printf("5. 2^10 = %.10f\n", result);

        status = fast_power(2.0, -3, &result);

        if (status != 0) {
            printf("Ошибка возведения в отрицательную степень.\n");
            return 1;
        }

        printf("   2^(-3) = %.10f\n", result);
    }

    /* --------------------------------------------------------
       6. Метод дихотомии
       -------------------------------------------------------- */

    {
        double root;

        printf("\n6. Метод дихотомии:\n");

        /*
            x^2 - 2 = 0
            Корень находится на [1; 2].
        */
        status = bisection(
            1.0,
            2.0,
            0.000001,
            equation_1,
            &root
        );

        if (status != 0) {
            printf("Ошибка поиска корня equation_1.\n");
            return 1;
        }

        printf("   x^2 - 2 = 0: x = %.10f\n", root);

        /*
            x^3 - x - 2 = 0
            Корень находится на [1; 2].
        */
        status = bisection(
            1.0,
            2.0,
            0.000001,
            equation_2,
            &root
        );

        if (status != 0) {
            printf("Ошибка поиска корня equation_2.\n");
            return 1;
        }

        printf("   x^3 - x - 2 = 0: x = %.10f\n", root);

        /*
            cos(x) - x = 0
            Корень находится на [0; 1].
        */
        status = bisection(
            0.0,
            1.0,
            0.000001,
            equation_3,
            &root
        );

        if (status != 0) {
            printf("Ошибка поиска корня equation_3.\n");
            return 1;
        }

        printf("   cos(x) - x = 0: x = %.10f\n", root);
    }

    return 0;
}