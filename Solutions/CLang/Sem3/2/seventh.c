
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <stddef.h>

#include "myerrors.h"

typedef struct {
    size_t dimension;
    double *coordinates;
} Vector;

/* Универсальный тип указателя на функцию вычисления нормы. */
typedef double (*NormFunction)(const Vector *, const void *);

typedef struct {
    size_t dimension;
    const double *data; /* Матрица хранится построчно. */
} Matrix;

typedef struct {
    size_t *indices;    /* Индексы всех максимальных векторов. */
    size_t count;
    double max_norm;
} NormResult;

typedef enum {
    OK = 0,
    INVALID_INPUT,
    MEMORY_ERROR,
    CALCULATION_ERROR
} Status;

/* Бесконечная норма: max |x[i]|. */
double norm_inf(const Vector *v, const void *context)
{
    (void)context;

    double result = 0.0;

    for (size_t i = 0; i < v->dimension; i++) {
        double value = fabs(v->coordinates[i]);

        if (value > result)
            result = value;
    }

    return result;
}

/* p-норма: (sum |x[i]|^p)^(1/p).
   context указывает на значение p типа double. */
double norm_p(const Vector *v, const void *context)
{
    if (context == NULL)
        return NAN;

    double p = *(const double *)context;

    if (!isfinite(p) || p < 1.0)
        return NAN;

    double sum = 0.0;

    for (size_t i = 0; i < v->dimension; i++)
        sum += pow(fabs(v->coordinates[i]), p);

    return pow(sum, 1.0 / p);
}

/* Матричная норма: sqrt(x^T A x).
   Матрица A передаётся через context. */
double norm_matrix(const Vector *v, const void *context)
{
    if (context == NULL)
        return NAN;

    const Matrix *matrix = (const Matrix *)context;

    if (matrix->dimension != v->dimension || matrix->data == NULL)
        return NAN;

    double sum = 0.0;

    for (size_t i = 0; i < v->dimension; i++) {
        double row_sum = 0.0;

        for (size_t j = 0; j < v->dimension; j++) {
            row_sum += matrix->data[i * v->dimension + j]
                       * v->coordinates[j];
        }

        sum += v->coordinates[i] * row_sum;
    }

    if (!isfinite(sum) || sum < 0.0)
        return NAN;

    return sqrt(sum);
}

void free_results(NormResult *results, size_t count)
{
    if (results == NULL)
        return;

    for (size_t i = 0; i < count; i++)
        free(results[i].indices);

    free(results);
}

/*
 * Аргументы после norm_count:
 *   1. vector_count указателей const Vector *
 *   2. norm_count пар:
 *        NormFunction,
 *        const void *context
 *
 * Например:
 * find_longest(n, count, 3, eps, &results,
 *              &v1, &v2, &v3,
 *              norm_inf, NULL,
 *              norm_p, &p,
 *              norm_matrix, &matrix);
 *
 * Результаты содержат индексы векторов с максимальной нормой.
 * epsilon задаёт допустимую погрешность сравнения.
 */
Status find_longest(size_t dimension,
                    size_t vector_count,
                    size_t norm_count,
                    double epsilon,
                    NormResult **out,
                    ...)
{
    if (out == NULL)
        return INVALID_INPUT;

    *out = NULL;

    if (dimension == 0 || vector_count == 0 ||
        norm_count == 0 || !isfinite(epsilon) || epsilon < 0.0)
        return INVALID_INPUT;

    if (norm_count > (size_t)-1 / sizeof(NormResult) ||
        vector_count > (size_t)-1 / sizeof(size_t))
        return MEMORY_ERROR;

    const Vector **vectors = malloc(vector_count * sizeof(*vectors));

    if (vectors == NULL)
        return MEMORY_ERROR;

    NormResult *results = calloc(norm_count, sizeof(*results));

    if (results == NULL) {
        free(vectors);
        return MEMORY_ERROR;
    }

    va_list args;
    va_start(args, out);

    for (size_t i = 0; i < vector_count; i++) {
        vectors[i] = va_arg(args, const Vector *);

        if (vectors[i] == NULL ||
            vectors[i]->coordinates == NULL ||
            vectors[i]->dimension != dimension) {
            va_end(args);
            free(vectors);
            free_results(results, norm_count);
            return INVALID_INPUT;
        }
    }

    for (size_t k = 0; k < norm_count; k++) {
        NormFunction function = va_arg(args, NormFunction);
        const void *context = va_arg(args, const void *);

        if (function == NULL) {
            va_end(args);
            free(vectors);
            free_results(results, norm_count);
            return INVALID_INPUT;
        }

        double maximum = -1.0;

        results[k].indices = malloc(vector_count * sizeof(size_t));

        if (results[k].indices == NULL) {
            va_end(args);
            free(vectors);
            free_results(results, norm_count);
            return MEMORY_ERROR;
        }

        results[k].count = 0;

        for (size_t i = 0; i < vector_count; i++) {
            double value = function(vectors[i], context);

            if (!isfinite(value) || value < 0.0) {
                va_end(args);
                free(vectors);
                free_results(results, norm_count);
                return CALCULATION_ERROR;
            }

            if (maximum < 0.0 || value > maximum + epsilon) {
                maximum = value;
                results[k].count = 0;
                results[k].indices[results[k].count++] = i;
            } else if (fabs(value - maximum) <= epsilon) {
                results[k].indices[results[k].count++] = i;
            }
        }

        results[k].max_norm = maximum;
    }

    va_end(args);
    free(vectors);

    *out = results;
    return OK;
}

int main(void)
{
    double x1[] = {1.0, 2.0, 3.0};
    double x2[] = {-3.0, 1.0, 0.0};
    double x3[] = {2.0, 2.0, 2.0};

    Vector v1 = {3, x1};
    Vector v2 = {3, x2};
    Vector v3 = {3, x3};

    /* p-норма с p = 2. */
    double p = 2.0;

    /* Положительно определённая матрица A = I. */
    double a_data[] = {
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0
    };

    Matrix matrix = {3, a_data};

    NormResult *results = NULL;

    Status status = find_longest(
        3,          /* Размерность пространства. */
        3,          /* Количество векторов. */
        3,          /* Количество норм. */
        1e-9,       /* epsilon. */
        &results,
        &v1, &v2, &v3,
        norm_inf, NULL,
        norm_p, &p,
        norm_matrix, &matrix
    );

    if (status != OK) {
        fprintf(stderr, "Ошибка: %d\n", status);
        return EXIT_FAILURE;
    }

    const char *names[] = {
        "Бесконечная норма",
        "p-норма (p = 2)",
        "Матричная норма"
    };

    for (size_t k = 0; k < 3; k++) {
        printf("%s: max = %.6f\n", names[k], results[k].max_norm);
        printf("Векторы:");

        for (size_t j = 0; j < results[k].count; j++)
            printf(" %zu", results[k].indices[j] + 1);

        putchar('\n');
    }

    free_results(results, 3);
    return EXIT_SUCCESS;
}
