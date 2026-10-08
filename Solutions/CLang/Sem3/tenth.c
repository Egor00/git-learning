#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_INPUT_LENGTH 100

/*
    Коды возврата:
    0 - успех
    1 - неверные параметры
    2 - некорректное число
    3 - переполнение
    4 - ошибка памяти
*/

/*
    Возвращает числовое значение символа.

    0..9 -> 0..9
    A..Z -> 10..35
*/
int char_to_digit(char c, int *digit)
{
    if (digit == NULL)
        return 1;

    if (c >= '0' && c <= '9')
    {
        *digit = c - '0';
        return 0;
    }

    if (c >= 'A' && c <= 'Z')
    {
        *digit = c - 'A' + 10;
        return 0;
    }

    /*
        Ввод строчных букв по условию
        не требуется.
    */
    return 2;
}

/*
    Проверка, является ли строка Stop.
*/
int is_stop(const char *str)
{
    if (str == NULL)
        return 0;

    return strcmp(str, "Stop") == 0;
}

/*
    Преобразование строки числа в long long.

    Поддерживаются:
    +123
    -123
    123
    000123
    -000123

    Цифры:
    0..9
    A..Z
*/
int string_to_number(
    const char *str,
    int base,
    long long *result
)
{
    size_t i;
    size_t start;
    int sign;
    int digit;
    long long value;
    long long limit;

    if (str == NULL || result == NULL)
        return 1;

    if (base < 2 || base > 36)
        return 1;

    if (str[0] == '\0')
        return 2;

    sign = 1;
    start = 0;

    /*
        Обрабатываем знак.
    */
    if (str[0] == '+' || str[0] == '-')
    {
        if (str[1] == '\0')
            return 2;

        if (str[0] == '-')
            sign = -1;

        start = 1;
    }

    value = 0;

    /*
        Для положительного числа:
            value <= (LLONG_MAX - digit) / base

        Для отрицательного числа можно представить
        LLONG_MIN, поэтому отдельно рассчитываем
        допустимый предел.
    */
    if (sign > 0)
        limit = LLONG_MAX;
    else
        limit = LLONG_MAX + 1LL;

    for (i = start; str[i] != '\0'; ++i)
    {
        if (char_to_digit(str[i], &digit) != 0)
            return 2;

        /*
            Цифра должна быть меньше основания.
        */
        if (digit >= base)
            return 2;

        /*
            Проверка переполнения перед:
                value = value * base + digit
        */
        if (value > (limit - digit) / base)
            return 3;

        value = value * base + digit;
    }

    if (sign < 0)
    {
        /*
            Если значение равно 2^63,
            оно может быть представлено как LLONG_MIN.
        */
        if (value == LLONG_MAX + 1LL)
            *result = LLONG_MIN;
        else
            *result = -value;
    }
    else
    {
        *result = value;
    }

    return 0;
}

/*
    Удаление ведущих нулей.

    Например:
        000123 -> 123
        0000   -> 0
        -000123 -> -123
*/
int remove_leading_zeroes(
    const char *str,
    char *result,
    size_t result_size
)
{
    size_t start;
    size_t length;
    int negative;
    const char *digits;

    if (str == NULL || result == NULL || result_size == 0)
        return 1;

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

    digits = str + start;

    /*
        Пропускаем ведущие нули.
    */
    while (digits[0] == '0' && digits[1] != '\0')
        ++digits;

    length = strlen(digits);

    if (negative)
    {
        if (length + 2 > result_size)
            return 1;

        result[0] = '-';
        memcpy(result + 1, digits, length + 1);
    }
    else
    {
        if (length + 1 > result_size)
            return 1;

        memcpy(result, digits, length + 1);
    }

    return 0;
}

/*
    Преобразование числа long long в заданную систему счисления.

    Основание: [2..36].

    Для цифр 10..35 используются:
        A, B, C, ..., Z
*/
int number_to_base(
    long long number,
    int base,
    char *result,
    size_t result_size
)
{
    char buffer[70];
    size_t position;
    size_t i;
    size_t output_position;
    unsigned long long value;
    unsigned long long magnitude;
    int digit;
    int negative;

    if (result == NULL || result_size == 0)
        return 1;

    if (base < 2 || base > 36)
        return 1;

    position = 0;
    negative = number < 0;

    /*
        Получаем модуль числа без переполнения
        даже для LLONG_MIN.
    */
    if (negative)
        magnitude = (unsigned long long)(-(number + 1)) + 1ULL;
    else
        magnitude = (unsigned long long)number;

    value = magnitude;

    /*
        Отдельный случай для нуля.
    */
    if (value == 0)
    {
        if (result_size < 2)
            return 1;

        result[0] = '0';
        result[1] = '\0';

        return 0;
    }

    /*
        Получаем цифры в обратном порядке.
    */
    while (value > 0)
    {
        if (position >= sizeof(buffer) - 1)
            return 1;

        digit = (int)(value % (unsigned long long)base);
        value /= (unsigned long long)base;

        if (digit < 10)
            buffer[position] = (char)('0' + digit);
        else
            buffer[position] = (char)('A' + digit - 10);

        ++position;
    }

    /*
        Проверяем размер результирующего буфера.
    */
    if (position + (negative ? 2 : 1) > result_size)
        return 1;

    output_position = 0;

    if (negative)
    {
        result[output_position++] = '-';
    }

    /*
        Переворачиваем полученные цифры.
    */
    for (i = 0; i < position; ++i)
    {
        result[output_position + i] =
            buffer[position - 1 - i];
    }

    result[output_position + position] = '\0';

    return 0;
}

/*
    Вывод информации о числе.
*/
int print_number_info(
    const char *name,
    long long number
)
{
    char representation[70];
    int bases[] = {9, 18, 27, 36};
    size_t i;

    if (name == NULL)
        return 1;

    printf("\n%s:\n", name);

    printf("Десятичное значение: %lld\n", number);

    printf("Без ведущих нулей: %lld\n", number);

    for (i = 0; i < sizeof(bases) / sizeof(bases[0]); ++i)
    {
        if (number_to_base(
                number,
                bases[i],
                representation,
                sizeof(representation)) != 0)
        {
            fprintf(
                stderr,
                "Ошибка преобразования числа в основание %d.\n",
                bases[i]
            );

            return 1;
        }

        printf(
            "Основание %d: %s\n",
            bases[i],
            representation
        );
    }

    return 0;
}

int main(void)
{
    char input[MAX_INPUT_LENGTH + 1];
    char normalized[MAX_INPUT_LENGTH + 1];

    int base;
    int scan_result;
    int conversion_result;

    long long current_number;
    long long max_abs_number;
    long long sum;

    unsigned long long current_abs;
    unsigned long long max_abs;

    int has_numbers;

    /*
        Ввод основания.
    */
    printf("Введите основание системы счисления [2..36]: ");

    scan_result = scanf("%d", &base);

    if (scan_result != 1)
    {
        fprintf(stderr, "Ошибка: основание должно быть целым числом.\n");
        return 1;
    }

    if (base < 2 || base > 36)
    {
        fprintf(
            stderr,
            "Ошибка: основание должно находиться в диапазоне [2..36].\n"
        );

        return 1;
    }

    /*
        Удаляем остаток строки после scanf.
    */
    while (getchar() != '\n')
    {
        /*
            Пустой цикл.
        */
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
        printf("> ");

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            /*
                EOF рассматриваем как окончание ввода.
            */
            break;
        }

        /*
            Убираем символ перевода строки.
        */
        input[strcspn(input, "\r\n")] = '\0';

        /*
            Пропускаем пустые строки.
        */
        if (input[0] == '\0')
            continue;

        /*
            Проверяем Stop.
        */
        if (is_stop(input))
            break;

        /*
            Убираем ведущие нули.
        */
        if (remove_leading_zeroes(
                input,
                normalized,
                sizeof(normalized)) != 0)
        {
            fprintf(stderr, "Ошибка обработки строки.\n");
            continue;
        }

        /*
            Преобразуем число из введённого основания
            в long long.
        */
        conversion_result = string_to_number(
            input,
            base,
            &current_number
        );

        if (conversion_result == 2)
        {
            fprintf(
                stderr,
                "Ошибка: строка \"%s\" не является "
                "корректным числом в основании %d.\n",
                input,
                base
            );

            continue;
        }

        if (conversion_result == 3)
        {
            fprintf(
                stderr,
                "Ошибка: число \"%s\" вызывает переполнение.\n",
                input
            );

            continue;
        }

        if (conversion_result != 0)
        {
            fprintf(stderr, "Ошибка обработки числа.\n");
            continue;
        }

        /*
            Проверяем переполнение суммы.
        */
        if ((current_number > 0 &&
             sum > LLONG_MAX - current_number) ||
            (current_number < 0 &&
             sum < LLONG_MIN - current_number))
        {
            fprintf(
                stderr,
                "Ошибка: сумма чисел вызывает переполнение.\n"
            );

            /*
                Текущее число не добавляем.
            */
            continue;
        }

        sum += current_number;

        /*
            Вычисляем абсолютное значение безопасно
            даже для LLONG_MIN.
        */
        if (current_number < 0)
        {
            current_abs =
                (unsigned long long)(-(current_number + 1)) + 1ULL;
        }
        else
        {
            current_abs = (unsigned long long)current_number;
        }

        /*
            Максимум по модулю.
        */
        if (!has_numbers || current_abs > max_abs)
        {
            max_abs = current_abs;
            max_abs_number = current_number;
        }

        has_numbers = 1;

        printf(
            "Принято: %s = %lld\n",
            normalized,
            current_number
        );
    }

    /*
        Проверяем, было ли введено хотя бы одно число.
    */
    if (!has_numbers)
    {
        fprintf(stderr, "Не введено ни одного числа.\n");
        return 1;
    }

    /*
        Выводим максимальное по модулю число.
    */
    if (print_number_info(
            "Максимальное по модулю число",
            max_abs_number) != 0)
    {
        return 1;
    }

    /*
        Выводим сумму.
    */
    if (print_number_info(
            "Сумма всех введённых чисел",
            sum) != 0)
    {
        return 1;
    }

    return 0;
}