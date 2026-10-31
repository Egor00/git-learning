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
 * Проверяет, имеет ли дробь конечное представление
 * в системе счисления с основанием base.
 *
 * Возвращает:
 *  1 — конечное;
 *  0 — бесконечное или обнаружен период;
 * -1 — ошибка вычислений.
 */
static int has_finite_representation(
    double x,
    unsigned base,
    double epsilon
)
{
    double *remainders = NULL;
    size_t count = 0;
    size_t capacity = 16;
    size_t i;
    size_t iterations = 0;
    const size_t max_iterations = 100000;

    if (!isfinite(x) || x <= 0.0 || x >= 1.0 ||
        base < 2 || base > 36 ||
        !isfinite(epsilon) || epsilon <= 0.0) {
        return -1;
    }

    remainders = malloc(capacity * sizeof(*remainders));
    if (remainders == NULL) {
        return -1;
    }

    while (x > epsilon && iterations < max_iterations) {
        double next;

        /* Если остаток уже встречался, начался период. */
        for (i = 0; i < count; ++i) {
            if (fabs(x - remainders[i]) <= epsilon) {
                free(remainders);
                return 0;
            }
        }

        if (count == capacity) {
            size_t new_capacity;
            double *tmp;

            if (capacity > SIZE_MAX / 2 / sizeof(*remainders)) {
                free(remainders);
                return -1;
            }

            new_capacity = capacity * 2;
            tmp = realloc(
                remainders,
                new_capacity * sizeof(*remainders)
            );

            if (tmp == NULL) {
                free(remainders);
                return -1;
            }

            remainders = tmp;
            capacity = new_capacity;
        }

        remainders[count++] = x;

        next = x * (double)base;

        if (!isfinite(next)) {
            free(remainders);
            return -1;
        }

        x = next - floor(next);
        ++iterations;
    }

    free(remainders);

    if (iterations == max_iterations) {
        return -1;
    }

    return 1;
}

/*
 * base — основание системы счисления;
 * epsilon — точность сравнения;
 * count — число переданных дробей;
 * result — массив результатов, выделяемый внутри функции;
 * далее идут count аргументов типа double.
 *
 * После использования вызывающий код должен вызвать free(*result).
 */
static Status check_fractions(
    unsigned base,
    double epsilon,
    size_t count,
    int **result,
    ...
)
{
    va_list args;
    int *values;
    size_t i;

    if (result == NULL || base < 2 || base > 36 ||
        !isfinite(epsilon) || epsilon <= 0.0 ||
        count == 0 || count > SIZE_MAX / sizeof(*values)) {
        return INVALID_INPUT;
    }

    *result = NULL;

    values = malloc(count * sizeof(*values));
    if (values == NULL) {
        return MEMORY_ERROR;
    }

    va_start(args, result);

    for (i = 0; i < count; ++i) {
        double x = va_arg(args, double);

        values[i] = has_finite_representation(x, base, epsilon);

        if (values[i] == -1) {
            va_end(args);
            free(values);
            return INVALID_INPUT;
        }
    }

    va_end(args);

    *result = values;
    return STATUS_OK;
}

int main(void)
{
    int *results = NULL;
    size_t i;
    const double epsilon = 1e-12;

    Status status = check_fractions(
        2, epsilon, 4, &results,
        0.5, 0.25, 0.75, 0.1
    );

    if (status != STATUS_OK) {
        fprintf(stderr, "Ошибка проверки дробей\n");
        return 1;
    }

    for (i = 0; i < 4; ++i) {
        printf(
            "Дробь %zu: %s\n",
            i + 1,
            results[i] ? "конечное представление"
                        : "бесконечное представление"
        );
    }

    free(results);
    return 0;
}