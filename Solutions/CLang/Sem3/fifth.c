#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <float.h>

#include "myerrors.h"

#define MAX_ITERATIONS 10000000LL
#define MAX_SIMpson_INTERVALS 1048576LL


static ErrorsDef parse_double(const char *str, double *value)
{
    char *end;
    double parsed;

    if (str == NULL || value == NULL || str[0] == '\0') {
        return INVALID_ARGUMENT;
    }

    errno = 0;
    parsed = strtod(str, &end);

    if (end == str || *end != '\0') {
        return INVALID_NUMBER_FORMAT;
    }

    if (errno == ERANGE || !isfinite(parsed)) {
        return INPUT_OVERFLOW;
    }

    *value = parsed;
    return STAT_OK;
}

/*
    Проверка epsilon.
*/
static ErrorsDef parse_epsilon(const char *str, double *epsilon)
{
    double value;
    ErrorsDef status;

    if (epsilon == NULL) {
        return INVALID_ARGUMENT;
    }

    status = parse_double(str, &value);
    if (status != STAT_OK) {
        return status;
    }

    if (value <= 0.0 || value < DBL_EPSILON) {
        return INVALID_EPSILON;
    }

    *epsilon = value;
    return STAT_OK;
}

/*
    Проверка x.
*/
static ErrorsDef parse_x(const char *str, double *x)
{
    return parse_double(str, x);
}

/*
    Ряд a: sum(x^n / n!).
*/
static ErrorsDef sum_a(double x, double epsilon, double *result)
{
    double sum = 1.0;
    double term = 1.0;
    long long n = 0;

    if (result == NULL || !isfinite(x) ||
        !isfinite(epsilon) || epsilon <= 0.0) {
        return INVALID_ARGUMENT;
    }

    while (fabs(term) >= epsilon) {
        if (n >= MAX_ITERATIONS) {
            return NO_CONVERGENCE;
        }

        ++n;
        term *= x / (double)n;

        if (!isfinite(term)) {
            return FUNC_OVERFLOW;
        }

        sum += term;

        if (!isfinite(sum)) {
            return RESULT_OVERFLOW;
        }
    }

    *result = sum;
    return STAT_OK;
}

/*
    Ряд b: sum((-1)^n * x^(2n) / (2n)!).
*/
static ErrorsDef sum_b(double x, double epsilon, double *result)
{
    double sum = 1.0;
    double term = 1.0;
    long long n = 0;

    if (result == NULL || !isfinite(x) ||
        !isfinite(epsilon) || epsilon <= 0.0) {
        return INVALID_ARGUMENT;
    }

    while (fabs(term) >= epsilon) {
        double denominator;

        if (n >= MAX_ITERATIONS) {
            return NO_CONVERGENCE;
        }

        ++n;

        denominator = (2.0 * (double)n - 1.0) *
                      (2.0 * (double)n);

        term *= -x * x / denominator;

        if (!isfinite(term)) {
            return FUNC_OVERFLOW;
        }

        sum += term;

        if (!isfinite(sum)) {
            return RESULT_OVERFLOW;
        }
    }

    *result = sum;
    return STAT_OK;
}

/*
    Ряд c:
    sum(3^(3n) * (n!)^3 * x^(2n) / (3n)!).
*/
static ErrorsDef sum_c(double x, double epsilon, double *result)
{
    double sum = 1.0;
    double term = 1.0;
    long long n = 0;

    if (result == NULL || !isfinite(x) ||
        !isfinite(epsilon) || epsilon <= 0.0) {
        return INVALID_ARGUMENT;
    }

    while (fabs(term) >= epsilon) {
        double dn;
        double numerator;
        double denominator;

        if (n >= MAX_ITERATIONS) {
            return NO_CONVERGENCE;
        }

        ++n;
        dn = (double)n;

        numerator = 27.0 * dn * dn * dn * x * x;

        denominator = (3.0 * dn - 2.0) *
                      (3.0 * dn - 1.0) *
                      (3.0 * dn);

        if (!isfinite(numerator) || !isfinite(denominator)) {
            return FUNC_OVERFLOW;
        }

        term *= numerator / denominator;

        if (!isfinite(term)) {
            return FUNC_OVERFLOW;
        }

        sum += term;

        if (!isfinite(sum)) {
            return RESULT_OVERFLOW;
        }
    }

    *result = sum;
    return STAT_OK;
}

/*
    Ряд d:
    sum((-1)^n * (2n-1)!! * x^(2n) / (2n)!!).

    Область сходимости: |x| < 1.
*/
static ErrorsDef sum_d(double x, double epsilon, double *result)
{
    double sum;
    double term;
    long long n;

    if (result == NULL || !isfinite(x) ||
        !isfinite(epsilon) || epsilon <= 0.0) {
        return INVALID_ARGUMENT;
    }

    if (fabs(x) >= 1.0) {
        return INVALID_INTERVAL;
    }

    n = 1;
    term = -x * x / 2.0;
    sum = term;

    if (!isfinite(term) || !isfinite(sum)) {
        return FUNC_OVERFLOW;
    }

    while (fabs(term) >= epsilon) {
        double ratio;

        if (n >= MAX_ITERATIONS) {
            return NO_CONVERGENCE;
        }

        ratio = -x * x * (2.0 * (double)n + 1.0) /
                (2.0 * (double)n + 2.0);

        term *= ratio;

        if (!isfinite(term)) {
            return FUNC_OVERFLOW;
        }

        sum += term;

        if (!isfinite(sum)) {
            return RESULT_OVERFLOW;
        }

        ++n;
    }

    *result = sum;
    return STAT_OK;
}

/*
    Подынтегральная функция a:
    ln(1 + x) / x.
*/
static double integral_a_function(double x)
{
    if (fabs(x) < 1e-12) {
        return 1.0;
    }

    if (x <= -1.0) {
        return NAN;
    }

    return log1p(x) / x;
}

/*
    Подынтегральная функция b:
    exp(-x^2 / 2).
*/
static double integral_b_function(double x)
{
    return exp(-(x * x) / 2.0);
}

/*
    Подынтегральная функция c:
    -ln(1 - x).
*/
static double integral_c_function(double x)
{
    if (x >= 1.0) {
        return NAN;
    }

    return -log1p(-x);
}

/*
    Подынтегральная функция d:
    x^x.
*/
static double integral_d_function(double x)
{
    if (x == 0.0) {
        return 1.0;
    }

    if (x < 0.0) {
        return NAN;
    }

    return exp(x * log(x));
}

/*
    Вычисление интеграла методом Симпсона.
*/
static ErrorsDef integrate_simpson( double (*function)(double), double a, double b, double epsilon, double *result)
{
    long long n = 2;
    double previous;

    if (function == NULL || result == NULL ||
        !isfinite(a) || !isfinite(b) ||
        !isfinite(epsilon) || epsilon <= 0.0) {
        return INVALID_ARGUMENT;
    }

    if (a > b) {
        return INVALID_INTERVAL;
    }

    {
        double h = (b - a) / (double)n;
        double sum = function(a) + function(b);
        long long i;

        if (!isfinite(sum)) {
            return INVALID_FUNCTION;
        }

        for (i = 1; i < n; ++i) {
            double current_x = a + h * (double)i;
            double value = function(current_x);

            if (!isfinite(value)) {
                return INVALID_FUNCTION;
            }

            sum += (i % 2 == 0 ? 2.0 : 4.0) * value;

            if (!isfinite(sum)) {
                return RESULT_OVERFLOW;
            }
        }

        previous = sum * h / 3.0;
    }

    if (!isfinite(previous)) {
        return RESULT_OVERFLOW;
    }

    while (1) {
        double h;
        double sum;
        double current;
        long long i;

        if (n >= MAX_SIMpson_INTERVALS) {
            return NO_CONVERGENCE;
        }

        n *= 2;
        h = (b - a) / (double)n;
        sum = function(a) + function(b);

        if (!isfinite(sum)) {
            return INVALID_FUNCTION;
        }

        for (i = 1; i < n; ++i) {
            double current_x = a + h * (double)i;
            double value = function(current_x);

            if (!isfinite(value)) {
                return INVALID_FUNCTION;
            }

            sum += (i % 2 == 0 ? 2.0 : 4.0) * value;

            if (!isfinite(sum)) {
                return RESULT_OVERFLOW;
            }
        }

        current = sum * h / 3.0;

        if (!isfinite(current)) {
            return RESULT_OVERFLOW;
        }

        if (fabs(current - previous) < epsilon) {
            *result = current;
            return STAT_OK;
        }

        previous = current;
    }
}

/*
    Вывод результата вычисления.
*/
static void print_result( const char *name, ErrorsDef status, double result)
{
    if (status == STAT_OK) {
        printf("%s = %.15g\n", name, result);
    } else {
        printf("%s: ", name);
        errors(status);
    }
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
    double epsilon;
    double x;

    double sum_result_a = 0.0;
    double sum_result_b = 0.0;
    double sum_result_c = 0.0;
    double sum_result_d = 0.0;

    double integral_result_a = 0.0;
    double integral_result_b = 0.0;
    double integral_result_c = 0.0;
    double integral_result_d = 0.0;

    ErrorsDef status;
    ErrorsDef sa, sb, sc, sd;
    ErrorsDef ia, ib, ic, id;

    if (argc != 3) {
        printf("Usage: %s <epsilon> <x>\n", argv[0]);
        return report_error(MATCH_ARGS);
    }

    status = parse_epsilon(argv[1], &epsilon);

    if (status != STAT_OK) {
        return report_error(status);
    }

    status = parse_x(argv[2], &x);

    if (status != STAT_OK) {
        return report_error(status);
    }

    printf("epsilon = %.15g\n", epsilon);
    printf("x = %.15g\n\n", x);

    sa = sum_a(x, epsilon, &sum_result_a);
    sb = sum_b(x, epsilon, &sum_result_b);
    sc = sum_c(x, epsilon, &sum_result_c);
    sd = sum_d(x, epsilon, &sum_result_d);

    printf("Sums:\n");

    print_result("a) sum(x^n / n!)", sa, sum_result_a);
    print_result("b) sum((-1)^n * x^(2n) / (2n)!)", sb, sum_result_b);
    print_result("c) sum(3^(3n) * (n!)^3 * x^(2n) / (3n)!)", sc, sum_result_c);
    print_result("d) sum((-1)^n * (2n-1)!! * x^(2n) / (2n)!!)", sd, sum_result_d);

    ia = integrate_simpson( integral_a_function, 0.0, 1.0, epsilon, &integral_result_a);

    ib = integrate_simpson( integral_b_function, 0.0, 1.0, epsilon, &integral_result_b);


    ic = INVALID_INTERVAL;

    id = integrate_simpson(integral_d_function, 0.0, 1.0, epsilon, &integral_result_d);

    printf("Integrals:\n");

    print_result("a) integral(ln(1+x) / x)", ia, integral_result_a);
    print_result("b) integral(exp(-x^2 / 2))",  ib, integral_result_b);
    print_result("c) integral(-ln(1-x)) on [0, 1]", ic, integral_result_c);
    print_result("d) integral(x^x)", id, integral_result_d);

    if (sa != STAT_OK || sb != STAT_OK || sc != STAT_OK || sd != STAT_OK || ia != STAT_OK || ib != STAT_OK || ic != STAT_OK || id != STAT_OK) {
        return report_error(INVALID_INTERVAL);
    }

    return 0;
}