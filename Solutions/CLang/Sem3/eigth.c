#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define INITIAL_SIZE 16

/*
    Коды возврата:

    0 - успех
    1 - ошибка параметров
    2 - ошибка открытия файла
    3 - ошибка памяти
    4 - ошибка чтения
    5 - ошибка записи
    6 - некорректное число
    7 - переполнение
*/

/* ============================================================
   ПОЛУЧЕНИЕ ЗНАЧЕНИЯ ЦИФРЫ
   ============================================================ */

int get_digit_value(char c, int *value)
{
    if (value == NULL) {
        return 1;
    }

    if (c >= '0' && c <= '9') {
        *value = c - '0';
        return 0;
    }

    if (c >= 'A' && c <= 'Z') {
        *value = c - 'A' + 10;
        return 0;
    }

    if (c >= 'a' && c <= 'z') {
        *value = c - 'a' + 10;
        return 0;
    }

    return 1;
}

/* ============================================================
   ПРОВЕРКА ЧИСЛА В ДАННОМ ОСНОВАНИИ
   ============================================================ */

/*
    Проверяет, является ли строка корректным представлением
    числа в системе счисления base.

    Одновременно переводит число в unsigned long long.
*/
int parse_in_base(
    const char *str,
    int base,
    unsigned long long *value
)
{
    size_t i;
    unsigned long long result = 0;

    if (str == NULL ||
        value == NULL ||
        base < 2 ||
        base > 36 ||
        str[0] == '\0') {
        return 1;
    }

    for (i = 0; str[i] != '\0'; ++i) {
        int digit;

        if (get_digit_value(str[i], &digit) != 0) {
            return 1;
        }

        /*
            Цифра должна быть меньше основания.
        */
        if (digit >= base) {
            return 1;
        }

        /*
            Проверка переполнения:
            result * base + digit <= ULLONG_MAX
        */
        if (result >
            (ULLONG_MAX - (unsigned long long)digit)
            / (unsigned long long)base) {
            return 2;
        }

        result =
            result * (unsigned long long)base
            + (unsigned long long)digit;
    }

    *value = result;

    return 0;
}

/* ============================================================
   ПОИСК МИНИМАЛЬНОГО ОСНОВАНИЯ
   ============================================================ */

int find_min_base(
    const char *str,
    int *base,
    unsigned long long *value
)
{
    int current_base;

    if (str == NULL ||
        base == NULL ||
        value == NULL ||
        str[0] == '\0') {
        return 1;
    }

    /*
        Ищем первое основание, в котором число корректно.
        Первое найденное основание и будет минимальным.
    */
    for (current_base = 2;
         current_base <= 36;
         ++current_base) {

        unsigned long long current_value;
        int status;

        status = parse_in_base(
            str,
            current_base,
            &current_value
        );

        if (status == 0) {
            *base = current_base;
            *value = current_value;

            return 0;
        }

        /*
            Если произошёл overflow, увеличение основания
            уже не сможет сделать число меньше, поэтому
            можно продолжить поиск следующего корректного
            представления только если ошибка была именно
            из-за цифры.
        */
    }

    return 6;
}

/* ============================================================
   УДАЛЕНИЕ ВЕДУЩИХ НУЛЕЙ
   ============================================================ */

int remove_leading_zeroes(
    const char *str,
    char **result
)
{
    const char *start;
    size_t length;
    char *copy;

    if (str == NULL ||
        result == NULL ||
        str[0] == '\0') {
        return 1;
    }

    start = str;

    /*
        Оставляем один ноль, если строка состоит только из нулей.
    */
    while (start[0] == '0' && start[1] != '\0') {
        ++start;
    }

    length = strlen(start);

    copy = (char *)malloc(length + 1);

    if (copy == NULL) {
        return 3;
    }

    memcpy(copy, start, length + 1);

    *result = copy;

    return 0;
}

/* ============================================================
   ЧТЕНИЕ ЛЕКСЕМЫ ИЗ ФАЙЛА
   ============================================================ */

/*
    Лексемы разделяются:
    - пробелами
    - табуляциями
    - переводами строк
    - другими whitespace-символами
*/
int read_token(
    FILE *file,
    char **token
)
{
    int c;
    size_t size = INITIAL_SIZE;
    size_t length = 0;
    char *buffer;

    if (file == NULL || token == NULL) {
        return 1;
    }

    *token = NULL;

    buffer = (char *)malloc(size);

    if (buffer == NULL) {
        return 3;
    }

    /*
        Пропускаем разделители.
    */
    do {
        c = fgetc(file);

        if (c == EOF) {
            if (ferror(file)) {
                free(buffer);
                return 4;
            }

            free(buffer);
            return 1;
        }

    } while (isspace((unsigned char)c));

    /*
        Читаем лексему.
    */
    while (c != EOF &&
           !isspace((unsigned char)c)) {

        if (length + 1 >= size) {
            size_t new_size;
            char *new_buffer;

            if (size > (size_t)-1 / 2) {
                free(buffer);
                return 3;
            }

            new_size = size * 2;

            new_buffer =
                (char *)realloc(buffer, new_size);

            if (new_buffer == NULL) {
                free(buffer);
                return 3;
            }

            buffer = new_buffer;
            size = new_size;
        }

        buffer[length] = (char)c;
        ++length;

        c = fgetc(file);
    }

    if (c == EOF && ferror(file)) {
        free(buffer);
        return 4;
    }

    buffer[length] = '\0';

    *token = buffer;

    return 0;
}

/* ============================================================
   ЗАПИСЬ РЕЗУЛЬТАТА
   ============================================================ */

int write_result(
    FILE *file,
    const char *number,
    int base,
    unsigned long long decimal_value
)
{
    if (file == NULL ||
        number == NULL ||
        base < 2 ||
        base > 36) {
        return 1;
    }

    if (fprintf(
            file,
            "%s %d %llu\n",
            number,
            base,
            decimal_value
        ) < 0) {
        return 5;
    }

    return 0;
}

/* ============================================================
   ОБРАБОТКА ФАЙЛА
   ============================================================ */

int process_file(
    const char *input_name,
    const char *output_name
)
{
    FILE *input = NULL;
    FILE *output = NULL;

    char *token = NULL;
    char *normalized = NULL;

    int status = 0;

    if (input_name == NULL ||
        output_name == NULL) {
        return 1;
    }

    input = fopen(input_name, "r");

    if (input == NULL) {
        return 2;
    }

    output = fopen(output_name, "w");

    if (output == NULL) {
        fclose(input);
        return 2;
    }

    while (1) {
        int read_status;

        /*
            Читаем очередное число.
        */
        read_status = read_token(
            input,
            &token
        );

        /*
            Конец файла.
        */
        if (read_status == 1) {
            break;
        }

        /*
            Ошибка чтения или памяти.
        */
        if (read_status != 0) {
            status = read_status;
            break;
        }

        /*
            Удаляем ведущие нули.
        */
        status = remove_leading_zeroes(
            token,
            &normalized
        );

        if (status != 0) {
            free(token);
            token = NULL;
            break;
        }

        /*
            Ищем минимальное основание.
        */
        {
            int base;
            unsigned long long decimal_value;

            status = find_min_base(
                normalized,
                &base,
                &decimal_value
            );

            if (status != 0) {
                printf(
                    "Ошибка: число \"%s\" не имеет "
                    "корректного основания от 2 до 36.\n",
                    token
                );

                free(token);
                free(normalized);

                token = NULL;
                normalized = NULL;

                status = 6;
                break;
            }

            /*
                Записываем:
                число без ведущих нулей
                минимальное основание
                десятичное значение
            */
            status = write_result(
                output,
                normalized,
                base,
                decimal_value
            );

            if (status != 0) {
                free(token);
                free(normalized);

                token = NULL;
                normalized = NULL;

                break;
            }
        }

        free(token);
        free(normalized);

        token = NULL;
        normalized = NULL;
    }

    free(token);
    free(normalized);

    /*
        Закрываем файлы.
    */
    if (fclose(input) != 0 && status == 0) {
        status = 5;
    }

    if (fclose(output) != 0 && status == 0) {
        status = 5;
    }

    return status;
}

/* ============================================================
   MAIN
   ============================================================ */

int main(int argc, char *argv[])
{
    int status;

    /*
        По условию:
        argv[1] - входной файл
        argv[2] - выходной файл

        Поэтому всего аргументов должно быть 3:
        argv[0] - имя программы
        argv[1] - input
        argv[2] - output
    */
    if (argc != 3) {
        printf(
            "Ошибка: неверное количество аргументов.\n"
        );

        printf(
            "Использование:\n"
            "  %s input.txt output.txt\n",
            argv[0]
        );

        return 1;
    }

    status = process_file(
        argv[1],
        argv[2]
    );

    if (status != 0) {
        switch (status) {
            case 2:
                printf("Ошибка открытия файла.\n");
                break;

            case 3:
                printf("Ошибка выделения памяти.\n");
                break;

            case 4:
                printf("Ошибка чтения файла.\n");
                break;

            case 5:
                printf("Ошибка записи или закрытия файла.\n");
                break;

            case 6:
                printf("Ошибка: некорректное число.\n");
                break;

            default:
                printf("Ошибка параметров.\n");
                break;
        }

        return 1;
    }

    printf("Файл успешно обработан.\n");

    return 0;
}