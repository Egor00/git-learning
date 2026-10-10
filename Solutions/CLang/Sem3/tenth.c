#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "myerrors.h"

#define MAX_INPUT_LENGTH 100

static ErrorsDef char_to_digit(char c, int *digit)
{
    if (digit == NULL)
        return INVALID_ARGUMENT;

    if (c >= '0' && c <= '9')
    {
        *digit = c - '0';
        return STAT_OK;
    }

    if (c >= 'A' && c <= 'Z')
    {
        *digit = c - 'A' + 10;
        return STAT_OK;
    }

    return INVALID_CHARACTER;
}

/*
    Проверка строки Stop.
*/
static int is_stop(const char *str)
{
    return str != NULL && strcmp(str, "Stop") == 0;
}


static ErrorsDef string_to_number( const char *str, int base, long long *result)
{
    size_t i;
    size_t start;
    int sign;
    int digit;
    long long value;
    long long limit;
    ErrorsDef status;

    if (str == NULL || result == NULL)
        return INVALID_ARGUMENT;

    if (base < 2 || base > 36)
        return INVALID_BASE;

    if (str[0] == '\0')
        return INVALID_INPUT;

    sign = 1;
    start = 0;

    if (str[0] == '+' || str[0] == '-')
    {
        if (str[1] == '\0')
            return INVALID_NUMBER_FORMAT;

        if (str[0] == '-')
            sign = -1;

        start = 1;
    }

    value = 0;
    limit = sign > 0 ? LLONG_MAX : LLONG_MAX;

    /*
        Для отрицательных чисел разрешаем модуль LLONG_MIN,
        который на единицу больше LLONG_MAX.
    */
    if (sign < 0)
        limit = (long long)((unsigned long long)LLONG_MAX + 1ULL);

    for (i = start; str[i] != '\0'; ++i)
    {
        status = char_to_digit(str[i], &digit);

        if (status != STAT_OK)
            return status;

        if (digit >= base)
            return INVALID_NUMBER;

        if (value > (limit - digit) / base)
            return RESULT_OVERFLOW;

        value = value * base + digit;
    }

    if (sign < 0)
    {
        if ((unsigned long long)value ==
            (unsigned long long)LLONG_MAX + 1ULL)
        {
            *result = LLONG_MIN;
        }
        else
        {
            *result = -value;
        }
    }
    else
    {
        *result = value;
    }

    return STAT_OK;
}

/*
    Удаление ведущих нулей с сохранением знака.
*/
static ErrorsDef remove_leading_zeroes( const char *str, char *result, size_t result_size)
{
    size_t start;
    size_t length;
    int negative;
    const char *digits;

    if (str == NULL || result == NULL)
        return INVALID_ARGUMENT;

    if (result_size == 0)
        return INVALID_ARRAY_SIZE;

    if (str[0] == '\0')
        return EMPTY_STRING;

    negative = 0;
    start = 0;

    if (str[0] == '-')
    {
        negative = 1;
        start = 1;
    }
    else if (str[0] == '+')
    {
        start = 1;
    }

    if (str[start] == '\0')
        return INVALID_NUMBER_FORMAT;

    digits = str + start;

    while (digits[0] == '0' && digits[1] != '\0')
        ++digits;

    /*
        Не сохраняем знак минуса для нуля.
    */
    if (negative && strcmp(digits, "0") == 0)
        negative = 0;

    length = strlen(digits);

    if (length > result_size - 1)
        return STRING_OVERFLOW;

    if (negative)
    {
        if (length + 2 > result_size)
            return STRING_OVERFLOW;

        result[0] = '-';
        memcpy(result + 1, digits, length + 1);
    }
    else
    {
        memcpy(result, digits, length + 1);
    }

    return STAT_OK;
}

/*
    Преобразование long long в систему счисления [2..36].
*/
static ErrorsDef number_to_base( long long number, int base, char *result, size_t result_size )
{
    char buffer[70];
    size_t position;
    size_t i;
    size_t output_position;
    unsigned long long value;
    unsigned long long magnitude;
    int digit;
    int negative;

    if (result == NULL)
        return INVALID_ARGUMENT;

    if (result_size == 0)
        return INVALID_ARRAY_SIZE;

    if (base < 2 || base > 36)
        return INVALID_BASE;

    position = 0;
    negative = number < 0;

    if (negative)
        magnitude = (unsigned long long)(-(number + 1)) + 1ULL;
    else
        magnitude = (unsigned long long)number;

    value = magnitude;

    if (value == 0)
    {
        if (result_size < 2)
            return STRING_OVERFLOW;

        result[0] = '0';
        result[1] = '\0';

        return STAT_OK;
    }

    while (value > 0)
    {
        if (position >= sizeof(buffer) - 1)
            return RESULT_OVERFLOW;

        digit = (int)(value % (unsigned long long)base);
        value /= (unsigned long long)base;

        if (digit < 10)
            buffer[position] = (char)('0' + digit);
        else
            buffer[position] = (char)('A' + digit - 10);

        ++position;
    }

    if (position + (negative ? 2U : 1U) > result_size)
        return STRING_OVERFLOW;

    output_position = 0;

    if (negative)
        result[output_position++] = '-';

    for (i = 0; i < position; ++i)
        result[output_position + i] = buffer[position - 1 - i];

    result[output_position + position] = '\0';

    return STAT_OK;
}

/*
    Вывод информации о числе.
*/
static ErrorsDef print_number_info( const char *name, long long number)
{
    char representation[70];
    const int bases[] = {9, 18, 27, 36};
    size_t i;
    ErrorsDef status;

    if (name == NULL)
        return INVALID_ARGUMENT;

    printf("\n%s:\n", name);
    printf("Десятичное значение: %lld\n", number);
    printf("Без ведущих нулей: %lld\n", number);

    for (i = 0; i < sizeof(bases) / sizeof(bases[0]); ++i)
    {
        status = number_to_base(
            number,
            bases[i],
            representation,
            sizeof(representation)
        );

        if (status != STAT_OK)
            return status;

        printf("Основание %d: %s\n", bases[i], representation);
    }

    return STAT_OK;
}

/*
    Разбор основания из строки.
*/
static ErrorsDef parse_base(const char *str, int *base)
{
    char *end_ptr;
    long value;

    if (str == NULL || base == NULL)
        return INVALID_ARGUMENT;

    if (*str == '\0')
        return INVALID_INPUT;

    errno = 0;
    end_ptr = NULL;
    value = strtol(str, &end_ptr, 10);

    if (str == end_ptr || *end_ptr != '\0')
        return INVALID_INPUT;

    if (errno == ERANGE || value < 2 || value > 36)
        return INVALID_BASE;

    *base = (int)value;

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
    char input[MAX_INPUT_LENGTH + 1];
    char normalized[MAX_INPUT_LENGTH + 1];

    int base;
    long long current_number;
    long long max_abs_number;
    long long sum;

    unsigned long long current_abs;
    unsigned long long max_abs;

    int has_numbers;
    size_t length;
    ErrorsDef status;

    printf("Введите основание системы счисления [2..36]: ");

    if (scanf("%100s", input) != 1)
    {
        
        return report_errors(INVALID_INPUT);
    }

    status = parse_base(input, &base);

    if (status != STAT_OK)
    {
        return report_error(status);
    }

    /*
        Очищаем остаток строки после scanf.
        Если обнаружена ошибка чтения, сообщаем о ней.
    */
    {
        int ch;

        while ((ch = getchar()) != '\n' && ch != EOF)
        {
            /* Пропускаем остаток строки. */
        }

        if (ferror(stdin))
        {
            return report_error(READFILE_ERROR);
        }
    }

    printf(
        "Введите числа в основании %d.\n"
        "Для окончания ввода введите Stop.\n",
        base
    );

    sum = 0;
    max_abs = 0;
    max_abs_number = 0;
    has_numbers = 0;

    while (1)
    {
        int ch;

        printf("> ");

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            if (ferror(stdin))
            {
                status = READFILE_ERROR;
                return report_error(status);
            }

            /* EOF завершает ввод. */
            break;
        }

        length = strlen(input);

        /*
            Если строка не поместилась в буфер, очищаем остаток
            строки и сообщаем о переполнении ввода.
        */
        if (length > 0 && input[length - 1] != '\n' &&
            !feof(stdin))
        {
            while ((ch = getchar()) != '\n' && ch != EOF)
            {
                /* Очищаем слишком длинную строку. */
            }

            errors(STRING_OVERFLOW);
            continue;
        }

        input[strcspn(input, "\r\n")] = '\0';

        if (input[0] == '\0')
            continue;

        if (is_stop(input))
            break;

        status = string_to_number(input, base, &current_number);

        if (status != STAT_OK)
        {
            errors(status);
            continue;
        }

        status = remove_leading_zeroes(
            input,
            normalized,
            sizeof(normalized)
        );

        if (status != STAT_OK)
        {
            errors(status);
            continue;
        }

        /*
            Проверка переполнения суммы до сложения.
        */
        if ((current_number > 0 &&
             sum > LLONG_MAX - current_number) ||
            (current_number < 0 &&
             sum < LLONG_MIN - current_number))
        {
            errors(RESULT_OVERFLOW);
            continue;
        }

        sum += current_number;

        if (current_number < 0)
        {
            current_abs =
                (unsigned long long)(-(current_number + 1)) + 1ULL;
        }
        else
        {
            current_abs = (unsigned long long)current_number;
        }

        if (!has_numbers || current_abs > max_abs)
        {
            max_abs = current_abs;
            max_abs_number = current_number;
        }

        has_numbers = 1;

        printf("Принято: %s = %lld\n", normalized, current_number);
    }

    if (!has_numbers)
    {
        return report_error(EMPTY_ARRAY);
    }

    status = print_number_info(
        "Максимальное по модулю число",
        max_abs_number
    );

    if (status != STAT_OK)
    {
        return report_error(status);
    }

    status = print_number_info("Сумма всех введённых чисел", sum);

    if (status != STAT_OK)
    {
        return report_error(status);
    }

    return STAT_OK;
}