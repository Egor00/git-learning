#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <math.h>
#include <stdint.h>

#include "myerrors.h"

typedef enum {
    STATUS_OK = 0,
    INVALID_INPUT,
    MEMORY_ERROR
} Status;

/*
 * Вычисляет коэффициенты многочлена
 *
 * f(x) = f[0] + f[1]*x + ... + f[n]*x^n
 *
 * в представлении
 *
 * f(x) = g[0] + g[1]*(x-a) + ... + g[n]*(x-a)^n.
 *
 * Аргументы:
 * epsilon — допустимая погрешность;
 * a       — точка сдвига;
 * result  — адрес указателя на результирующий массив;
 * n       — степень многочлена;
 * далее   — n+1 коэффициентов типа double.
 *
 * Результирующий массив освобождает вызывающий код.
 */
Status reexpand_polynomial(
    double epsilon,
    double a,
    double **result,
    size_t n,
    ...
)
{
    double *g;
    va_list args;
    size_t i;
    size_t j;

    if (result == NULL) {
        return INVALID_INPUT;
    }

    *result = NULL;

    if (!isfinite(epsilon) || epsilon <= 0.0 ||
        !isfinite(a) || n == SIZE_MAX ||
        n + 1 > SIZE_MAX / sizeof(*g)) {
        return INVALID_INPUT;
    }

    g = calloc(n + 1, sizeof(*g));
    if (g == NULL) {
        return MEMORY_ERROR;
    }

    va_start(args, n);

    for (i = 0; i <= n; ++i) {
        g[i] = va_arg(args, double);

        if (!isfinite(g[i])) {
            va_end(args);
            free(g);
            return INVALID_INPUT;
        }
    }

    va_end(args);

    /*
     * Последовательно применяем схему Горнера.
     * При каждом шаге коэффициенты умножаются
     * на (x-a) и к свободному члену прибавляется
     * очередной коэффициент исходного многочлена.
     */
    for (i = n; i > 0; --i) {
        for (j = n; j >= i; --j) {
            g[j] = g[j - 1] + a * g[j];

            if (!isfinite(g[j])) {
                free(g);
                return INVALID_INPUT;
            }

            if (fabs(g[j]) < epsilon) {
                g[j] = 0.0;
            }
        }

        g[i - 1] = g[i - 1] + 0.0;
    }

    /*
     * Корректный способ переразложения:
     * строим коэффициенты от старшей степени
     * к младшей, выполняя умножение на (y+a).
     */
    {
        double *f = malloc((n + 1) * sizeof(*f));

        if (f == NULL) {
            free(g);
            return MEMORY_ERROR;
        }

        for (i = 0; i <= n; ++i) {
            f[i] = g[i];
        }

        for (i = 0; i <= n; ++i) {
            g[i] = 0.0;
        }

        g[0] = f[n];

        for (i = n; i > 0; --i) {
            for (j = n - i + 1; j > 0; --j) {
                g[j] = g[j - 1] + a * g[j];
            }

            g[0] = a * g[0] + f[i - 1];
        }

        free(f);
    }

    for (i = 0; i <= n; ++i) {
        if (fabs(g[i]) < epsilon) {
            g[i] = 0.0;
        }
    }

    *result = g;
    return STATUS_OK;
}

int main(void)
{
    double *coefficients = NULL;
    size_t i;
    const size_t degree = 2;

    /*
     * f(x) = 1 + 2x + 3x^2
     * a = 1
     *
     * Ожидается:
     * f(x) = 6 + 8(x-1) + 3(x-1)^2
     */
    Status status = reexpand_polynomial(
        1e-10,
        1.0,
        &coefficients,
        degree,
        1.0,
        2.0,
        3.0
    );

    if (status != STATUS_OK) {
        fprintf(stderr, "Ошибка вычисления\n");
        return 1;
    }

    for (i = 0; i <= degree; ++i) {
        printf("g[%zu] = %.10f\n", i, coefficients[i]);
    }

    free(coefficients);
    return 0;
}