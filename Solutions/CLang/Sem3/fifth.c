#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <float.h>


/*
    Проверка epsilon.
*/
int parse_epsilon(const char *str, double *epsilon)
{
    char *end;
    double value;

    if (str == NULL || epsilon == NULL) {
        return 0;
    }

    errno = 0;
    end = NULL;

    value = strtod(str, &end);

    if (errno == ERANGE) {
        return 0;
    }

    if (end == str || *end != '\0') {
        return 0;
    }

    if (!isfinite(value) || value <= 0.0) {
        return 0;
    }

    if (value < DBL_EPSILON) {
        return 0;
    }

    *epsilon = value;

    return 1;
}


/*
    Проверка x.
*/
int parse_x(const char *str, double *x)
{
    char *end;
    double value;

    if (str == NULL || x == NULL) {
        return 0;
    }

    errno = 0;
    end = NULL;

    value = strtod(str, &end);

    if (errno == ERANGE) {
        return 0;
    }

    if (end == str || *end != '\0') {
        return 0;
    }

    if (!isfinite(value)) {
        return 0;
    }

    *x = value;

    return 1;
}


/*
    Сумма a:

        infinity
        --------
        \
        / x^n / n!
        --------
        n = 0
*/
int sum_a(double x, double epsilon, double *result)
{
    double sum = 1.0;
    double term = 1.0;
    long long n = 0;

    if (result == NULL || !isfinite(x) ||
        !isfinite(epsilon) || epsilon <= 0.0) {
        return 0;
    }

    while (fabs(term) >= epsilon) {
        n++;

        if (n > 10000000) {
            return 0;
        }

        term *= x / (double)n;

        if (!isfinite(term)) {
            return 0;
        }

        sum += term;

        if (!isfinite(sum)) {
            return 0;
        }
    }

    *result = sum;

    return 1;
}


/*
    Сумма b:

        infinity
        --------
        \
        / (-1)^n * x^(2n) / (2n)!
        --------
        n = 0
*/
int sum_b(double x, double epsilon, double *result)
{
    double sum = 1.0;
    double term = 1.0;
    long long n = 0;

    if (result == NULL || !isfinite(x) ||
        !isfinite(epsilon) || epsilon <= 0.0) {
        return 0;
    }

    while (fabs(term) >= epsilon) {
        double denominator;

        n++;

        if (n > 10000000) {
            return 0;
        }

        denominator =
            (2.0 * (double)n - 1.0) *
            (2.0 * (double)n);

        term *= -x * x / denominator;

        if (!isfinite(term)) {
            return 0;
        }

        sum += term;

        if (!isfinite(sum)) {
            return 0;
        }
    }

    *result = sum;

    return 1;
}


/*
    Сумма c:

        infinity
        --------
        \
        / 3^(3n) * (n!)^3 * x^(2n)
        / -------------------------
        /          (3n)!
        --------
        n = 0
*/
int sum_c(double x, double epsilon, double *result)
{
    double sum = 1.0;
    double term = 1.0;
    long long n = 0;

    if (result == NULL || !isfinite(x) ||
        !isfinite(epsilon) || epsilon <= 0.0) {
        return 0;
    }

    while (fabs(term) >= epsilon) {
        double numerator;
        double denominator;

        n++;

        if (n > 10000000) {
            return 0;
        }

        numerator =
            27.0 *
            (double)n *
            (double)n *
            (double)n *
            x *
            x;

        denominator =
            (3.0 * (double)n - 2.0) *
            (3.0 * (double)n - 1.0) *
            (3.0 * (double)n);

        term *= numerator / denominator;

        if (!isfinite(term)) {
            return 0;
        }

        sum += term;

        if (!isfinite(sum)) {
            return 0;
        }
    }

    *result = sum;

    return 1;
}


/*
    Сумма d:

        infinity
        --------
        \
        / (-1)^n * (2n-1)!! * x^(2n)
        / ---------------------------
        /           (2n)!!
        --------
        n = 1
*/
int sum_d(double x, double epsilon, double *result)
{
    double sum;
    double term;
    long long n;

    if (result == NULL || !isfinite(x) ||
        !isfinite(epsilon) || epsilon <= 0.0) {
        return 0;
    }

    /*
        Ряд сходится при |x| < 1.
    */
    if (fabs(x) >= 1.0) {
        return 0;
    }

    n = 1;

    term = -x * x / 2.0;
    sum = term;

    if (!isfinite(term) || !isfinite(sum)) {
        return 0;
    }

    while (fabs(term) >= epsilon) {
        double ratio;

        if (n >= 10000000) {
            return 0;
        }

        ratio =
            -x * x *
            (2.0 * (double)n + 1.0) /
            (2.0 * (double)n + 2.0);

        term *= ratio;

        if (!isfinite(term)) {
            return 0;
        }

        sum += term;

        if (!isfinite(sum)) {
            return 0;
        }

        n++;
    }

    *result = sum;

    return 1;
}


/*
    Подынтегральная функция a:

        ln(1 + x) / x
*/
double integral_a_function(double x)
{
    if (fabs(x) < 1e-12) {
        return 1.0;
    }

    return log1p(x) / x;
}


/*
    Подынтегральная функция b:

        exp(-x^2 / 2)
*/
double integral_b_function(double x)
{
    return exp(-(x * x) / 2.0);
}


/*
    Подынтегральная функция c:

        ln(1 / (1 - x))
*/
double integral_c_function(double x)
{
    return -log1p(-x);
}


/*
    Подынтегральная функция d:

        x^x
*/
double integral_d_function(double x)
{
    if (x == 0.0) {
        return 1.0;
    }

    return exp(x * log(x));
}


/*
    Численное вычисление интеграла
    методом Симпсона.
*/
int integrate_simpson(double (*function)(double),
                      double a,
                      double b,
                      double epsilon,
                      double *result)
{
    long long n = 2;
    double previous;
    double current;

    if (function == NULL ||
        result == NULL ||
        !isfinite(a) ||
        !isfinite(b) ||
        !isfinite(epsilon) ||
        epsilon <= 0.0 ||
        a > b) {

        return 0;
    }

    /*
        Первое приближение.
    */
    {
        double h = (b - a) / (double)n;
        double sum = function(a) + function(b);
        long long i;

        if (!isfinite(sum)) {
            return 0;
        }

        for (i = 1; i < n; i++) {
            double current_x = a + h * (double)i;
            double value = function(current_x);

            if (!isfinite(value)) {
                return 0;
            }

            if (i % 2 == 0) {
                sum += 2.0 * value;
            } else {
                sum += 4.0 * value;
            }
        }

        previous = sum * h / 3.0;
    }

    if (!isfinite(previous)) {
        return 0;
    }

    /*
        Увеличиваем количество разбиений,
        пока результат не станет достаточно точным.
    */
    while (1) {
        double h;
        double sum;
        long long i;

        if (n > 1048576) {
            return 0;
        }

        n *= 2;

        h = (b - a) / (double)n;

        sum = function(a) + function(b);

        if (!isfinite(sum)) {
            return 0;
        }

        for (i = 1; i < n; i++) {
            double current_x = a + h * (double)i;
            double value = function(current_x);

            if (!isfinite(value)) {
                return 0;
            }

            if (i % 2 == 0) {
                sum += 2.0 * value;
            } else {
                sum += 4.0 * value;
            }
        }

        current = sum * h / 3.0;

        if (!isfinite(current)) {
            return 0;
        }

        if (fabs(current - previous) < epsilon) {
            *result = current;
            return 1;
        }

        previous = current;
    }
}


/*
    Вывод результата.
*/
void print_result(const char *name,
                  int success,
                  double result)
{
    if (success) {
        printf("%s = %.15g\n", name, result);
    } else {
        printf("%s: error during calculation\n", name);
    }
}


int main(int argc, char *argv[])
{
    double epsilon;
    double x;

    double sum_result_a;
    double sum_result_b;
    double sum_result_c;
    double sum_result_d;

    double integral_result_a;
    double integral_result_b;
    double integral_result_c;
    double integral_result_d;

    int success_sum_a;
    int success_sum_b;
    int success_sum_c;
    int success_sum_d;

    int success_integral_a;
    int success_integral_b;
    int success_integral_c;
    int success_integral_d;


    /*
        Проверяем количество аргументов.

        argv[0] - имя программы
        argv[1] - epsilon
        argv[2] - x
    */
    if (argc != 3) {
        fprintf(stderr,
                "Usage: %s <epsilon> <x>\n",
                argv[0]);

        return 1;
    }


    /*
        Проверяем epsilon.
    */
    if (!parse_epsilon(argv[1], &epsilon)) {
        fprintf(stderr,
                "Error: epsilon must be a positive finite number.\n");

        return 1;
    }


    /*
        Проверяем x.
    */
    if (!parse_x(argv[2], &x)) {
        fprintf(stderr,
                "Error: x must be a finite real number.\n");

        return 1;
    }


    printf("epsilon = %.15g\n", epsilon);
    printf("x = %.15g\n\n", x);


    /*
        ==========================
        СУММЫ
        ==========================
    */

    success_sum_a = sum_a(
        x,
        epsilon,
        &sum_result_a
    );

    success_sum_b = sum_b(
        x,
        epsilon,
        &sum_result_b
    );

    success_sum_c = sum_c(
        x,
        epsilon,
        &sum_result_c
    );

    success_sum_d = sum_d(
        x,
        epsilon,
        &sum_result_d
    );


    printf("Sums:\n");

    print_result(
        "a) sum(x^n / n!)",
        success_sum_a,
        sum_result_a
    );

    print_result(
        "b) sum((-1)^n * x^(2n) / (2n)!)",
        success_sum_b,
        sum_result_b
    );

    print_result(
        "c) sum(3^(3n) * (n!)^3 * x^(2n) / (3n)!)",
        success_sum_c,
        sum_result_c
    );

    print_result(
        "d) sum((-1)^n * (2n-1)!! * x^(2n) / (2n)!!)",
        success_sum_d,
        sum_result_d
    );


    /*
        ==========================
        ИНТЕГРАЛЫ
        ==========================
    */

    success_integral_a = integrate_simpson(
        integral_a_function,
        0.0,
        1.0,
        epsilon,
        &integral_result_a
    );

    success_integral_b = integrate_simpson(
        integral_b_function,
        0.0,
        1.0,
        epsilon,
        &integral_result_b
    );

    success_integral_c = integrate_simpson(
        integral_c_function,
        0.0,
        1.0,
        epsilon,
        &integral_result_c
    );

    success_integral_d = integrate_simpson(
        integral_d_function,
        0.0,
        1.0,
        epsilon,
        &integral_result_d
    );


    printf("\nIntegrals:\n");

    print_result(
        "a) integral(ln(1+x) / x)",
        success_integral_a,
        integral_result_a
    );

    print_result(
        "b) integral(exp(-x^2 / 2))",
        success_integral_b,
        integral_result_b
    );

    print_result(
        "c) integral(ln(1 / (1-x)))",
        success_integral_c,
        integral_result_c
    );

    print_result(
        "d) integral(x^x)",
        success_integral_d,
        integral_result_d
    );


    /*
        Если вычисление хотя бы одного значения
        завершилось ошибкой, возвращаем ненулевой код.
    */
    if (!success_sum_a ||
        !success_sum_b ||
        !success_sum_c ||
        !success_sum_d ||
        !success_integral_a ||
        !success_integral_b ||
        !success_integral_c ||
        !success_integral_d) {

        return 1;
    }


    return 0;
}