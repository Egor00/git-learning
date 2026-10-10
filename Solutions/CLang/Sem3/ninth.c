#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <stddef.h>

#include "myerrors.h"

#define FIXED_SIZE 20

static ErrorsDef random_int(int a, int b, int *result)
{
    long long range;
    double value;

    if (result == NULL)
        return INVALID_ARGUMENT;

    if (a > b)
        return INVALID_INTERVAL;

    range = (long long)b - (long long)a + 1;

    value = (double)rand() / ((double)RAND_MAX + 1.0);

    /*
        Вычисляем случайное число в диапазоне [a, b].
        Используем long long, чтобы избежать переполнения
        при вычислении промежуточного результата.
    */
    *result = (int)((long long)a + (long long)(value * (double)range));

    return STAT_OK;
}

/*
    Заполнение массива случайными числами из [a, b].
*/
static ErrorsDef fill_array(int *array, size_t size, int a, int b)
{
    size_t i;
    ErrorsDef status;

    if (array == NULL)
        return INVALID_ARGUMENT;

    if (size == 0)
        return INVALID_ARRAY_SIZE;

    if (a > b)
        return INVALID_INTERVAL;

    for (i = 0; i < size; ++i)
    {
        status = random_int(a, b, &array[i]);

        if (status != STAT_OK)
            return status;
    }

    return STAT_OK;
}

/*
    Поиск минимума и максимума и обмен их местами за один проход.
*/
static ErrorsDef find_min_max_and_swap( int *array, size_t size, int *min_value, int *max_value)
{
    size_t i;
    size_t min_index;
    size_t max_index;
    int min;
    int max;
    int temp;

    if (array == NULL || min_value == NULL || max_value == NULL)
        return INVALID_ARGUMENT;

    if (size == 0)
        return INVALID_ARRAY_SIZE;

    min = array[0];
    max = array[0];
    min_index = 0;
    max_index = 0;

    for (i = 1; i < size; ++i)
    {
        if (array[i] < min)
        {
            min = array[i];
            min_index = i;
        }

        if (array[i] > max)
        {
            max = array[i];
            max_index = i;
        }
    }

    if (min_index != max_index)
    {
        temp = array[min_index];
        array[min_index] = array[max_index];
        array[max_index] = temp;
    }

    *min_value = min;
    *max_value = max;

    return STAT_OK;
}

/*
    Печать массива.
*/
static ErrorsDef print_array(const int *array, size_t size)
{
    size_t i;

    if (array == NULL)
        return INVALID_ARGUMENT;

    if (size == 0)
        return INVALID_ARRAY_SIZE;

    for (i = 0; i < size; ++i)
    {
        if (printf("%d", array[i]) < 0)
            return WRITEFILE_ERROR;

        if (i + 1 < size)
        {
            if (printf(" ") < 0)
                return WRITEFILE_ERROR;
        }
    }

    if (printf("\n") < 0)
        return WRITEFILE_ERROR;

    return STAT_OK;
}

/*
    Поиск элемента, ближайшего по значению к value.
    При равных расстояниях выбирается первый найденный.
*/
static ErrorsDef find_nearest( const int *array, size_t size, int value, int *nearest)
{
    size_t i;
    long long current_difference;
    long long best_difference;

    if (array == NULL || nearest == NULL)
        return INVALID_ARGUMENT;

    if (size == 0)
        return INVALID_ARRAY_SIZE;

    *nearest = array[0];

    best_difference =
        llabs((long long)array[0] - (long long)value);

    for (i = 1; i < size; ++i)
    {
        current_difference =
            llabs((long long)array[i] - (long long)value);

        if (current_difference < best_difference)
        {
            best_difference = current_difference;
            *nearest = array[i];
        }
    }

    return STAT_OK;
}

/*
    Формирование массива C:
        C[i] = A[i] + ближайший по значению элемент B.
*/
static ErrorsDef create_array_c( const int *a, size_t size_a, const int *b, size_t size_b, int **c)
{
    size_t i;
    int nearest;
    long long sum;
    int *result;
    ErrorsDef status;

    if (a == NULL || b == NULL || c == NULL)
        return INVALID_ARGUMENT;

    if (size_a == 0 || size_b == 0)
        return INVALID_ARRAY_SIZE;

    *c = NULL;

    if (size_a > (size_t)-1 / sizeof(int))
        return INVALID_ARRAY_SIZE;

    result = (int *)malloc(size_a * sizeof(int));

    if (result == NULL)
        return MEMORY_ERROR;

    for (i = 0; i < size_a; ++i)
    {
        status = find_nearest(b, size_b, a[i], &nearest);

        if (status != STAT_OK)
        {
            free(result);
            return status;
        }

        sum = (long long)a[i] + (long long)nearest;

        if (sum < INT_MIN || sum > INT_MAX)
        {
            free(result);
            return RESULT_OVERFLOW;
        }

        result[i] = (int)sum;
    }

    *c = result;

    return STAT_OK;
}

/*
    Получение случайного размера массива в диапазоне [10; 10000].
*/
static ErrorsDef random_size(size_t *size)
{
    int value;
    ErrorsDef status;

    if (size == NULL)
        return INVALID_ARGUMENT;

    status = random_int(10, 10000, &value);

    if (status != STAT_OK)
        return status;

    *size = (size_t)value;

    return STAT_OK;
}

/*
    Первая часть задания.
*/
static ErrorsDef task_part_one(int a, int b)
{
    int array[FIXED_SIZE];
    int min_value;
    int max_value;
    ErrorsDef status;

    printf("=== Часть 1 ===\n");

    status = fill_array(array, FIXED_SIZE, a, b);

    if (status != STAT_OK)
        return status;

    printf("Исходный массив:\n");

    status = print_array(array, FIXED_SIZE);

    if (status != STAT_OK)
        return status;

    status = find_min_max_and_swap(
        array,
        FIXED_SIZE,
        &min_value,
        &max_value
    );

    if (status != STAT_OK)
        return status;

    printf("Минимальный элемент: %d\n", min_value);
    printf("Максимальный элемент: %d\n", max_value);
    printf("Массив после обмена минимума и максимума:\n");

    return print_array(array, FIXED_SIZE);
}


    //Вторая часть задания.

static ErrorsDef task_part_two(void)
{
    int *a = NULL;
    int *b = NULL;
    int *c = NULL;

    size_t size_a = 0;
    size_t size_b = 0;

    ErrorsDef status;

    printf("\n=== Часть 2 ===\n");

    status = random_size(&size_a);

    if (status != STAT_OK)
        goto cleanup;

    status = random_size(&size_b);

    if (status != STAT_OK)
        goto cleanup;

    printf("Размер A: %zu\n", size_a);
    printf("Размер B: %zu\n", size_b);

    if (size_a > (size_t)-1 / sizeof(int) ||
        size_b > (size_t)-1 / sizeof(int))
    {
        status = INVALID_ARRAY_SIZE;
        goto cleanup;
    }

    a = (int *)malloc(size_a * sizeof(int));

    if (a == NULL)
    {
        status = MEMORY_ERROR;
        goto cleanup;
    }

    b = (int *)malloc(size_b * sizeof(int));

    if (b == NULL)
    {
        status = MEMORY_ERROR;
        goto cleanup;
    }

    status = fill_array(a, size_a, -1000, 1000);

    if (status != STAT_OK)
        goto cleanup;

    status = fill_array(b, size_b, -1000, 1000);

    if (status != STAT_OK)
        goto cleanup;

    status = create_array_c(a, size_a, b, size_b, &c);

    if (status != STAT_OK)
        goto cleanup;

    printf("\nПервые элементы A:\n");

    status = print_array(a, size_a < 20 ? size_a : 20);

    if (status != STAT_OK)
        goto cleanup;

    printf("Первые элементы B:\n");

    status = print_array(b, size_b < 20 ? size_b : 20);

    if (status != STAT_OK)
        goto cleanup;

    printf("Первые элементы C:\n");

    status = print_array(c, size_a < 20 ? size_a : 20);

cleanup:
    free(a);
    free(b);
    free(c);

    return status;
}


    //Разбор целого числа из аргумента командной строки.

static ErrorsDef parse_int_argument(const char *str, int *value)
{
    char *end_ptr;
    long parsed_value;

    if (str == NULL || value == NULL)
        return INVALID_ARGUMENT;

    if (*str == '\0')
        return INVALID_INPUT;

    errno = 0;
    end_ptr = NULL;
    parsed_value = strtol(str, &end_ptr, 10);

    if (str == end_ptr || *end_ptr != '\0')
        return INVALID_INPUT;

    if (errno == ERANGE || parsed_value < INT_MIN || parsed_value > INT_MAX)
        return RESULT_OVERFLOW;

    *value = (int)parsed_value;

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
    int a;
    int b;
    ErrorsDef status;

    srand((unsigned int)time(NULL));

    if (argc != 3)
    {
        fprintf(stderr, "Использование: %s <a> <b>\n", argv[0]);
        return report_error(INVALID_ARGUMENT);
    }

    status = parse_int_argument(argv[1], &a);

    if (status != STAT_OK)
    {
        
        return report_error(status);
    }

    status = parse_int_argument(argv[2], &b);

    if (status != STAT_OK)
    {

        return report_error(status);
    }

    if (a > b)
    {

        return report_error(status);
    }

    status = task_part_one(a, b);

    if (status != STAT_OK)
    {

        return report_error(status);
    }

    status = task_part_two();

    if (status != STAT_OK)
        report_error(status);

    return status;
}