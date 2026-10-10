#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <limits.h>

#include "myerrors.h"

#define EPS 1e-10

typedef double (*EquationFunction)(double);


static ErrorsDef is_convex_polygon(int count, int *result, ...)
{
    va_list args;
    double *x;
    double *y;
    int i;
    int sign = 0;

    if (result == NULL || count < 3) {
        return INVALID_ARGUMENT;
    }

    x = malloc((size_t)count * sizeof(*x));
    y = malloc((size_t)count * sizeof(*y));

    if (x == NULL || y == NULL) {
        free(x);
        free(y);
        return MEMORY_ERROR;
    }

    va_start(args, result);

    for (i = 0; i < count; ++i) {
        x[i] = va_arg(args, double);
        y[i] = va_arg(args, double);

        if (!isfinite(x[i]) || !isfinite(y[i])) {
            va_end(args);
            free(x);
            free(y);
            return INVALID_INPUT;
        }
    }

    va_end(args);

    for (i = 0; i < count; ++i) {
        int a = i;
        int b = (i + 1) % count;
        int c = (i + 2) % count;

        double cross =
            (x[b] - x[a]) * (y[c] - y[b]) -
            (y[b] - y[a]) * (x[c] - x[b]);

        if (!isfinite(cross)) {
            free(x);
            free(y);
            return FUNC_OVERFLOW;
        }

        if (fabs(cross) > EPS) {
            int current_sign = (cross > 0.0) ? 1 : -1;

            if (sign == 0) {
                sign = current_sign;
            } else if (sign != current_sign) {
                free(x);
                free(y);
                *result = 0;
                return STAT_OK;
            }
        }
    }

    free(x);
    free(y);

    /* Многоугольник из коллинеарных точек не считается выпуклым. */
    *result = (sign != 0);

    return STAT_OK;
}


static ErrorsDef evaluate_polynomial(
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
        return INVALID_ARGUMENT;
    }

    va_start(args, result);

    value = va_arg(args, double);

    if (!isfinite(value)) {
        va_end(args);
        return INVALID_INPUT;
    }

    for (i = 1; i <= degree; ++i) {
        double coefficient = va_arg(args, double);

        if (!isfinite(coefficient)) {
            va_end(args);
            return INVALID_INPUT;
        }

        value = value * x + coefficient;

        if (!isfinite(value)) {
            va_end(args);
            return FUNC_OVERFLOW;
        }
    }

    va_end(args);

    *result = value;

    return STAT_OK;
}


/* Перевод символа в значение цифры. */
static ErrorsDef digit_value(char c, int *value)
{
    if (value == NULL) {
        return INVALID_ARGUMENT;
    }

    if (c >= '0' && c <= '9') {
        *value = c - '0';
    } else if (c >= 'A' && c <= 'Z') {
        *value = c - 'A' + 10;
    } else if (c >= 'a' && c <= 'z') {
        *value = c - 'a' + 10;
    } else {
        return INVALID_CHARACTER;
    }

    return STAT_OK;
}

/*
   Перевод числа из системы счисления base
   в unsigned long long.
*/
static ErrorsDef parse_number_base(
    const char *str,
    int base,
    unsigned long long *value
)
{
    size_t i;
    unsigned long long result = 0;

    if (str == NULL || value == NULL) {
        return INVALID_ARGUMENT;
    }

    if (base < 2 || base > 36) {
        return INVALID_BASE;
    }

    if (str[0] == '\0') {
        return INVALID_NUMBER_FORMAT;
    }

    for (i = 0; str[i] != '\0'; ++i) {
        int digit;
        ErrorsDef status;

        status = digit_value(str[i], &digit);

        if (status != STAT_OK) {
            return status;
        }

        if (digit >= base) {
            return INVALID_NUMBER_FORMAT;
        }

        if (result >
            (ULLONG_MAX - (unsigned long long)digit) /
            (unsigned long long)base) {
            return RESULT_OVERFLOW;
        }

        result = result * (unsigned long long)base
               + (unsigned long long)digit;
    }

    *value = result;

    return STAT_OK;
}

/* Проверка числа Капрекара. */
static ErrorsDef is_kaprekar(
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
    int divisor_overflow = 0;

    if (result == NULL) {
        return INVALID_ARGUMENT;
    }

    if (base < 2 || base > 36) {
        return INVALID_BASE;
    }

    if (number == 0) {
        *result = 1;
        return STAT_OK;
    }

    if (number > ULLONG_MAX / number) {
        return POWER_OVERFLOW;
    }

    square = number * number;

    /*
       Для разделения квадрата используем base^digits(number).
       Если эта степень не помещается в unsigned long long,
       она больше ULLONG_MAX, а значит, и square.
    */
    temp = number;

    while (temp > 0) {
        if (divisor > ULLONG_MAX / (unsigned long long)base) {
            divisor_overflow = 1;
            break;
        }

        divisor *= (unsigned long long)base;
        temp /= (unsigned long long)base;
    }

    if (divisor_overflow) {
        left = 0;
        right = square;
    } else {
        right = square % divisor;
        left = square / divisor;
    }

    /* Проверяем сложение без риска переполнения. */
    if (left > ULLONG_MAX - right) {
        return RESULT_OVERFLOW;
    }

    *result = (left + right == number);

    return STAT_OK;
}

/*
   Среди переданных строк находит числа Капрекара.
   После result_count передаются count аргументов типа const char *.
*/
static ErrorsDef find_kaprekar_numbers(
    int base,
    int count,
    int *result_count,
    ...
)
{
    va_list args;
    int i;
    int found = 0;

    if (result_count == NULL || count < 0) {
        return INVALID_ARGUMENT;
    }

    if (base < 2 || base > 36) {
        return INVALID_BASE;
    }

    va_start(args, result_count);

    for (i = 0; i < count; ++i) {
        const char *str;
        unsigned long long number;
        int kaprekar;
        ErrorsDef status;

        str = va_arg(args, const char *);

        if (str == NULL) {
            va_end(args);
            return INVALID_ARGUMENT;
        }

        status = parse_number_base(str, base, &number);

        if (status != STAT_OK) {
            va_end(args);
            return status;
        }

        status = is_kaprekar(number, base, &kaprekar);

        if (status != STAT_OK) {
            va_end(args);
            return status;
        }

        if (kaprekar) {
            ++found;
            printf("Число Капрекара: %s\n", str);
        }
    }

    va_end(args);

    *result_count = found;

    return STAT_OK;
}



static ErrorsDef geometric_mean(
    int count,
    double *result,
    ...
)
{
    va_list args;
    double logarithm_sum = 0.0;
    double calculated_result;
    int i;

    if (result == NULL || count <= 0) {
        return INVALID_ARGUMENT;
    }

    va_start(args, result);

    for (i = 0; i < count; ++i) {
        double value = va_arg(args, double);

        if (!isfinite(value)) {
            va_end(args);
            return INVALID_INPUT;
        }

        if (value <= 0.0) {
            va_end(args);
            return NEGATIVE_NUMBER;
        }

        logarithm_sum += log(value);

        if (!isfinite(logarithm_sum)) {
            va_end(args);
            return FUNC_OVERFLOW;
        }
    }

    va_end(args);

    calculated_result = exp(logarithm_sum / (double)count);

    if (!isfinite(calculated_result)) {
        return FUNC_OVERFLOW;
    }

    *result = calculated_result;

    return STAT_OK;
}


static ErrorsDef fast_power(
    double x,
    long long n,
    double *result
)
{
    double half;
    double value;

    if (result == NULL || !isfinite(x)) {
        return INVALID_ARGUMENT;
    }

    if (n == 0) {
        *result = 1.0;
        return STAT_OK;
    }

    if (x == 0.0 && n < 0) {
        return ZERO_DIV;
    }

    /*
       Обработка LLONG_MIN без непосредственного вычисления -n.
    */
    if (n == LLONG_MIN) {
        double positive_part;

        if (fast_power(x, -(n + 1), &positive_part) != STAT_OK) {
            return POWER_OVERFLOW;
        }

        value = positive_part * x;

        if (!isfinite(value)) {
            return POWER_OVERFLOW;
        }

        if (value == 0.0) {
            return ZERO_DIV;
        }

        *result = 1.0 / value;

        if (!isfinite(*result)) {
            return POWER_OVERFLOW;
        }

        return STAT_OK;
    }

    if (n < 0) {
        ErrorsDef status = fast_power(x, -n, &value);

        if (status != STAT_OK) {
            return status;
        }

        if (value == 0.0) {
            return ZERO_DIV;
        }

        *result = 1.0 / value;

        if (!isfinite(*result)) {
            return POWER_OVERFLOW;
        }

        return STAT_OK;
    }

    if (n % 2 == 0) {
        ErrorsDef status = fast_power(x, n / 2, &half);

        if (status != STAT_OK) {
            return status;
        }

        *result = half * half;
    } else {
        ErrorsDef status = fast_power(x, n - 1, &half);

        if (status != STAT_OK) {
            return status;
        }

        *result = x * half;
    }

    if (!isfinite(*result)) {
        return POWER_OVERFLOW;
    }

    return STAT_OK;
}


/* f(x) = x^2 - 2 */
static double equation_1(double x)
{
    return x * x - 2.0;
}

/* f(x) = x^3 - x - 2 */
static double equation_2(double x)
{
    return x * x * x - x - 2.0;
}

/* f(x) = cos(x) - x */
static double equation_3(double x)
{
    return cos(x) - x;
}

/* Поиск корня методом дихотомии. */
static ErrorsDef bisection(
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

    if (result == NULL || function == NULL ||
        !isfinite(left) || !isfinite(right) ||
        !isfinite(epsilon) || epsilon <= 0.0 ||
        left >= right) {
        return INVALID_INTERVAL;
    }

    f_left = function(left);
    f_right = function(right);

    if (!isfinite(f_left) || !isfinite(f_right)) {
        return INVALID_FUNCTION;
    }

    if (fabs(f_left) <= epsilon) {
        *result = left;
        return STAT_OK;
    }

    if (fabs(f_right) <= epsilon) {
        *result = right;
        return STAT_OK;
    }

    /*
       Сравниваем знаки напрямую, чтобы не вычислять
       потенциально переполняющееся произведение f_left * f_right.
    */
    if ((f_left > 0.0 && f_right > 0.0) ||
        (f_left < 0.0 && f_right < 0.0)) {
        return NO_ROOT_INTERVAL;
    }

    while ((right - left) / 2.0 > epsilon) {
        double middle;
        double f_middle;

        if (iterations >= max_iterations) {
            return NO_CONVERGENCE;
        }

        middle = left + (right - left) / 2.0;
        f_middle = function(middle);

        if (!isfinite(f_middle)) {
            return INVALID_FUNCTION;
        }

        if (fabs(f_middle) <= epsilon) {
            *result = middle;
            return STAT_OK;
        }

        if ((f_left < 0.0 && f_middle > 0.0) ||
            (f_left > 0.0 && f_middle < 0.0)) {
            right = middle;
            f_right = f_middle;
        } else {
            left = middle;
            f_left = f_middle;
        }

        ++iterations;
    }

    *result = left + (right - left) / 2.0;

    return STAT_OK;
}


static int report_error(ErrorsDef error)
{
    if (error != STAT_OK) {
        errors(error);
    }

    return (int)error;
}


int main(void)
{
    ErrorsDef status;

    /* 1. Выпуклый многоугольник */
    {
        int convex;

        status = is_convex_polygon( 4, &convex, 0.0, 0.0, 4.0, 0.0, 4.0, 3.0, 0.0, 3.0);

        if (!report_error(status)) {
            return report_error(status);
        }

        printf("1. Многоугольник выпуклый: %s\n", convex ? "да" : "нет");
    }

    /* 2. Многочлен 2x^3 + 3x^2 + 4x + 5 при x = 2 */
    {
        double polynomial;

        status = evaluate_polynomial( 2.0, 3, &polynomial, 2.0, 3.0, 4.0, 5.0);

        if (!report_error(status)) {
            return report_error(status);
        }

        printf("2. Значение многочлена: %.10f\n", polynomial);
    }

    /* 3. Числа Капрекара */
    {
        int count;

        printf("\n3. Числа Капрекара:\n");

        status = find_kaprekar_numbers( 10, 7, &count, "1", "9", "10", "45", "55", "99", "297" );

        if (!report_error(status)) {
            return report_error(status);
        }

        printf("Найдено чисел Капрекара: %d\n", count);
    }

    /* 4. Среднее геометрическое */
    {
        double result;

        status = geometric_mean( 4, &result, 1.0, 2.0, 4.0, 8.0 );

        if (!report_error(status)) {
            return report_error(status);
        }

        printf("\n4. Среднее геометрическое: %.10f\n", result);
    }

    /* 5. Быстрое возведение в степень */
    {
        double result;

        status = fast_power(2.0, 10, &result);

        if (!report_error(status)) {
            return report_error(status);
        }

        printf("5. 2^10 = %.10f\n", result);

        status = fast_power(2.0, -3, &result);

        if (!report_error(status)) {
            return report_error(status);
        }

        printf("   2^(-3) = %.10f\n", result);
    }

    /* 6. Метод дихотомии */
    {
        double root;

        printf("\n6. Метод дихотомии:\n");

        status = bisection( 1.0, 2.0, 0.000001, equation_1, &root );

        if (!report_error(status)) {
            return report_error(status);
        }

        printf("   x^2 - 2 = 0: x = %.10f\n", root);

        status = bisection( 1.0, 2.0, 0.000001, equation_2, &root );

        if (!report_error(status)) {
            return report_error(status);
        }

        printf("   x^3 - x - 2 = 0: x = %.10f\n", root);

        status = bisection( 0.0, 1.0, 0.000001, equation_3, &root );

        if (!report_error(status)) {
            return report_error(status);
        }

        printf("   cos(x) - x = 0: x = %.10f\n", root);
    }

    return 0;
}