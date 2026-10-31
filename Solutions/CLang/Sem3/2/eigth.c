#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "myerrors.h"

typedef enum {
    SUM_OK = 0,
    SUM_INVALID_INPUT,
    SUM_MEMORY_ERROR
} SumStatus;

/* Преобразование символа в значение цифры. */
static int digit_value(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }

    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 10;
    }

    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 10;
    }

    return -1;
}

/* Преобразование значения цифры в символ. */
static char digit_char(int value)
{
    if (value < 10) {
        return (char)('0' + value);
    }

    return (char)('a' + value - 10);
}

/* Проверка числа, записанного в системе с заданным основанием. */
static int valid_number(const char *number, unsigned base)
{
    size_t i;

    if (number == NULL || number[0] == '\0') {
        return 0;
    }

    for (i = 0; number[i] != '\0'; ++i) {
        int digit = digit_value(number[i]);

        if (digit < 0 || (unsigned)digit >= base) {
            return 0;
        }
    }

    return 1;
}

/*
 * Сложение двух чисел в заданной системе счисления.
 * Результат выделяется динамически.
 */
static SumStatus add_numbers(
    const char *a,
    const char *b,
    unsigned base,
    char **result
)
{
    size_t len_a;
    size_t len_b;
    size_t max_len;
    size_t i;
    size_t j;
    size_t k;
    char *sum;
    unsigned carry = 0;
    size_t start;

    *result = NULL;

    if (!valid_number(a, base) || !valid_number(b, base)) {
        return SUM_INVALID_INPUT;
    }

    len_a = strlen(a);
    len_b = strlen(b);
    max_len = len_a > len_b ? len_a : len_b;

    if (max_len > SIZE_MAX - 2) {
        return SUM_MEMORY_ERROR;
    }

    sum = malloc(max_len + 2);
    if (sum == NULL) {
        return SUM_MEMORY_ERROR;
    }

    i = len_a;
    j = len_b;
    k = max_len + 1;
    sum[k] = '\0';

    while (i > 0 || j > 0 || carry > 0) {
        unsigned da = 0;
        unsigned db = 0;
        unsigned value;

        if (i > 0) {
            da = (unsigned)digit_value(a[--i]);
        }

        if (j > 0) {
            db = (unsigned)digit_value(b[--j]);
        }

        value = da + db + carry;
        sum[--k] = digit_char((int)(value % base));
        carry = value / base;
    }

    start = k;
    memmove(sum, sum + start, strlen(sum + start) + 1);

    /* Удаляем ведущие нули, оставляя один ноль для нулевого результата. */
    start = 0;
    while (sum[start] == '0' && sum[start + 1] != '\0') {
        ++start;
    }

    if (start > 0) {
        memmove(sum, sum + start, strlen(sum + start) + 1);
    }

    *result = sum;
    return SUM_OK;
}

/*
 * Аргументы:
 * base        — основание системы счисления [2..36];
 * count       — количество чисел;
 * result      — адрес указателя, куда записывается результат;
 * далее       — count строк с числами.
 *
 * Вызывающая сторона должна освободить *result через free().
 */
SumStatus sum_numbers(
    unsigned base,
    size_t count,
    char **result,
    ...
)
{
    va_list args;
    char *total = NULL;
    size_t i;
    SumStatus status = SUM_OK;

    if (result == NULL) {
        return SUM_INVALID_INPUT;
    }

    *result = NULL;

    if (base < 2 || base > 36 || count == 0) {
        return SUM_INVALID_INPUT;
    }

    va_start(args, result);

    for (i = 0; i < count; ++i) {
        const char *number = va_arg(args, const char *);

        if (!valid_number(number, base)) {
            status = SUM_INVALID_INPUT;
            break;
        }

        if (i == 0) {
            total = malloc(strlen(number) + 1);

            if (total == NULL) {
                status = SUM_MEMORY_ERROR;
                break;
            }

            strcpy(total, number);

            /* Нормализуем первый аргумент: убираем ведущие нули. */
            {
                size_t start = 0;

                while (total[start] == '0' &&
                       total[start + 1] != '\0') {
                    ++start;
                }

                if (start > 0) {
                    memmove(total, total + start,
                            strlen(total + start) + 1);
                }
            }
        } else {
            char *new_total = NULL;

            status = add_numbers(total, number, base, &new_total);

            if (status != SUM_OK) {
                break;
            }

            free(total);
            total = new_total;
        }
    }

    va_end(args);

    if (status != SUM_OK) {
        free(total);
        return status;
    }

    *result = total;
    return SUM_OK;
}

int main(void)
{
    char *result = NULL;
    SumStatus status;

    /* Суммируем числа в системе счисления с основанием 16. */
    status = sum_numbers(
        16,
        3,
        &result,
        (const char *)"FFFFFFFFFFFFFFFFFFFFFFFF",
        (const char *)"1",
        (const char *)"A"
    );

    if (status != SUM_OK) {
        fprintf(stderr, "Ошибка вычисления: %d\n", (int)status);
        return 1;
    }

    printf("Сумма: %s\n", result);

    free(result);
    return 0;
}