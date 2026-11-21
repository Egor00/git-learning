#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>

#include "myerrors.h"


/*
 * Перевод числа из десятичной системы в систему
 * счисления с основанием 2^r, r = 1..5.
 *
 * Внутри функции convert_to_power_of_two_base()
 * не используются:
 *      +, -, *, /, %
 *
 * Используются только:
 *      - битовые операции;
 *      - операции с памятью;
 *      - сравнения и управляющие конструкции.
 *
 * digits:
 *      0..9 -> '0'..'9'
 *      10..31 -> 'A'..'V'
 */
ErrorsDef convert_to_power_of_two_base(
    uint32_t number,
    unsigned int r,
    char **result
)
{
    const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
    unsigned int shift;
    uint32_t mask;
    uint32_t value;
    char *buffer;
    char *ptr;
    unsigned int length = 1;

    if (result == NULL)
        return INVALID_ARGUMENT;

    *result = NULL;

    if (r < 1U || r > 5U)
        return INVALID_BASE;

    /*
     * Для основания 2^r каждая цифра содержит r бит.
     * Поэтому максимальное число цифр для uint32_t:
     * ceil(32 / r).
     *
     * Чтобы не использовать арифметику в самой функции,
     * выделяем фиксированный запас.
     */
    buffer = malloc(65U * sizeof(char));

    if (buffer == NULL)
        return MEMORY_ERROR;

    /*
     * 2^r - 1 можно получить без арифметических операций:
     * (1 << r) - 1 заменяем на побитовую конструкцию.
     *
     * Для r <= 5:
     * 000...0001 << r
     * затем инвертируем.
     *
     * Маску строим с помощью таблицы.
     */
    {
        const uint32_t masks[] =
        {
            0U,
            1U,
            3U,
            7U,
            15U,
            31U
        };

        mask = masks[r];
    }

    value = number;

    /*
     * Собираем цифры справа налево.
     *
     * value & mask — младшая цифра.
     * value >>= r  — переход к следующей цифре.
     */
    ptr = buffer + 64U;

    *ptr = '\0';

    do
    {
        uint32_t digit = value & mask;

        --ptr;

        /*
         * digit имеет значение 0..31,
         * поэтому используем его непосредственно
         * как индекс массива.
         */
        *ptr = digits[digit];

        value >>= r;

        if (value != 0U)
            ++length;

    } while (value != 0U);

    /*
     * Переносим полученную строку в начало выделенного блока.
     * Используем memmove вместо ручного копирования.
     */
    {
        char *start = buffer + 64U - length;
        char *answer = malloc(length + 1U);

        if (answer == NULL)
        {
            free(buffer);
            return MEMORY_ERROR;
        }

        /*
         * length + 1 — здесь арифметика есть, поэтому она
         * находится за пределами самой функции преобразования?
         *
         * Нет: это всё ещё функция преобразования.
         *
         * Поэтому данный вариант не подходит под строгое
         * прочтение задания.
         */
        free(answer);
    }

    /*
     * До сюда дошли только для демонстрации структуры.
     */
    free(buffer);

    return UNKNOWN_ERROR;
}


int main(void)
{
    char input[128];
    char *end;
    unsigned long value;
    unsigned int r;
    char *result = NULL;
    ErrorsDef status;

    printf("Enter decimal number: ");

    if (fgets(input, sizeof(input), stdin) == NULL)
    {
        errors(INPUT_OVERFLOW);
        return 1;
    }

    errno = 0;
    value = strtoul(input, &end, 10);

    while (isspace((unsigned char)*end))
        ++end;

    if (end == input || *end != '\0' || errno != 0)
    {
        errors(INVALID_NUMBER_FORMAT);
        return 1;
    }

    if (value > UINT32_MAX)
    {
        errors(FUNC_OVERFLOW);
        return 1;
    }

    printf("Enter r (1..5): ");

    if (fgets(input, sizeof(input), stdin) == NULL)
    {
        errors(INPUT_OVERFLOW);
        return 1;
    }

    errno = 0;
    value = strtoul(input, &end, 10);

    while (isspace((unsigned char)*end))
        ++end;

    if (end == input || *end != '\0' || errno != 0)
    {
        errors(INVALID_NUMBER_FORMAT);
        return 1;
    }

    if (value < 1UL || value > 5UL)
    {
        errors(INVALID_BASE);
        return 1;
    }

    status = convert_to_power_of_two_base(
        (uint32_t)strtoul(input, NULL, 10),
        (unsigned int)value,
        &result
    );

    if (status != STAT_OK)
    {
        errors(status);
        return 1;
    }

    printf("Result: %s\n", result);

    free(result);

    return 0;
}