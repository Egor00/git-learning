#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INITIAL_TOKEN_SIZE 16
#define MAX_PATH_SIZE 4096

/*
    Коды возврата:
    0 - успех
    1 - ошибка параметров
    2 - ошибка открытия файла
    3 - ошибка памяти
    4 - ошибка чтения
    5 - ошибка записи
*/

/* ============================================================
   ЧТЕНИЕ ЛЕКСЕМЫ
   ============================================================ */

/*
    Читает одну лексему из файла.

    Лексемами считаются последовательности символов,
    разделённые пробелами, табуляциями и переводами строк.

    Возвращает:
    0 - лексема прочитана
    1 - достигнут конец файла
    2 - ошибка памяти
    3 - ошибка чтения
*/
int read_token(FILE *file, char **token)
{
    int c;
    size_t size = INITIAL_TOKEN_SIZE;
    size_t length = 0;
    char *buffer;

    if (file == NULL || token == NULL) {
        return 3;
    }

    *token = NULL;

    buffer = (char *)malloc(size);

    if (buffer == NULL) {
        return 2;
    }

    /*
        Пропускаем разделители.
    */
    do {
        c = fgetc(file);

        if (c == EOF) {
            if (ferror(file)) {
                free(buffer);
                return 3;
            }

            free(buffer);
            return 1;
        }
    } while (isspace((unsigned char)c));

    /*
        Читаем саму лексему.
    */
    while (c != EOF && !isspace((unsigned char)c)) {
        if (length + 1 >= size) {
            size_t new_size = size * 2;
            char *new_buffer;

            if (new_size <= size) {
                free(buffer);
                return 2;
            }

            new_buffer = (char *)realloc(buffer, new_size);

            if (new_buffer == NULL) {
                free(buffer);
                return 2;
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
        return 3;
    }

    buffer[length] = '\0';

    *token = buffer;

    return 0;
}

/* ============================================================
   ЗАПИСЬ ЛЕКСЕМЫ
   ============================================================ */

int write_token(
    FILE *file,
    const char *token,
    int *first
)
{
    if (file == NULL || token == NULL || first == NULL) {
        return 1;
    }

    if (!*first) {
        if (fputc(' ', file) == EOF) {
            return 5;
        }
    }

    if (fputs(token, file) == EOF) {
        return 5;
    }

    *first = 0;

    return 0;
}

/* ============================================================
   РЕЖИМ -r
   ============================================================ */

/*
    В выходной файл записываются:
    file1_lexeme1 file2_lexeme1
    file1_lexeme2 file2_lexeme2
    ...

    Если один файл закончился раньше,
    оставшиеся лексемы второго файла дописываются.
*/
int process_r(
    const char *file1_name,
    const char *file2_name,
    const char *output_name
)
{
    FILE *file1 = NULL;
    FILE *file2 = NULL;
    FILE *output = NULL;

    char *token1 = NULL;
    char *token2 = NULL;

    int result1;
    int result2;
    int first = 1;
    int status = 0;

    if (file1_name == NULL ||
        file2_name == NULL ||
        output_name == NULL) {
        return 1;
    }

    file1 = fopen(file1_name, "r");

    if (file1 == NULL) {
        return 2;
    }

    file2 = fopen(file2_name, "r");

    if (file2 == NULL) {
        fclose(file1);
        return 2;
    }

    output = fopen(output_name, "w");

    if (output == NULL) {
        fclose(file1);
        fclose(file2);
        return 2;
    }

    while (1) {
        result1 = read_token(file1, &token1);
        result2 = read_token(file2, &token2);

        /*
            Обработка ошибок чтения / памяти.
        */
        if (result1 == 2 || result1 == 3 ||
            result2 == 2 || result2 == 3) {

            free(token1);
            free(token2);

            status = (result1 == 2 || result2 == 2) ? 3 : 4;
            break;
        }

        /*
            Оба файла закончились.
        */
        if (result1 == 1 && result2 == 1) {
            free(token1);
            free(token2);
            break;
        }

        /*
            Лексема из первого файла.
        */
        if (result1 == 0) {
            status = write_token(output, token1, &first);

            free(token1);
            token1 = NULL;

            if (status != 0) {
                free(token2);
                status = 5;
                break;
            }
        }

        /*
            Лексема из второго файла.
        */
        if (result2 == 0) {
            status = write_token(output, token2, &first);

            free(token2);
            token2 = NULL;

            if (status != 0) {
                status = 5;
                break;
            }
        }
    }

    /*
        Проверяем закрытие файлов.
    */
    if (fclose(file1) != 0 && status == 0) {
        status = 5;
    }

    if (fclose(file2) != 0 && status == 0) {
        status = 5;
    }

    if (fclose(output) != 0 && status == 0) {
        status = 5;
    }

    return status;
}

/* ============================================================
   ПЕРЕВОД ЧИСЛА В СИСТЕМУ СЧИСЛЕНИЯ
   ============================================================ */

/*
    Преобразует число value в строку с основанием base.

    Например:
    65 в системе счисления 4 -> "1001"
    65 в системе счисления 8 -> "101"
*/
int number_to_base(
    unsigned int value,
    int base,
    char **result
)
{
    char temporary[64];
    size_t length = 0;
    size_t i;
    char *output;

    if (result == NULL || (base != 4 && base != 8)) {
        return 1;
    }

    /*
        Нуля в ASCII-кодах нет для пустой строки,
        но функция должна быть универсальной.
    */
    if (value == 0) {
        output = (char *)malloc(2);

        if (output == NULL) {
            return 2;
        }

        output[0] = '0';
        output[1] = '\0';

        *result = output;
        return 0;
    }

    /*
        Получаем цифры в обратном порядке.
    */
    while (value > 0) {
        unsigned int digit = value % (unsigned int)base;

        temporary[length] =
            (char)('0' + digit);

        ++length;
        value /= (unsigned int)base;
    }

    output = (char *)malloc(length + 1);

    if (output == NULL) {
        return 2;
    }

    /*
        Переворачиваем результат.
    */
    for (i = 0; i < length; ++i) {
        output[i] = temporary[length - 1 - i];
    }

    output[length] = '\0';

    *result = output;

    return 0;
}

/* ============================================================
   ПРЕОБРАЗОВАНИЕ ЛЕКСЕМЫ В ASCII-КОДЫ
   ============================================================ */

int convert_to_ascii_codes(
    const char *token,
    int base,
    char **result
)
{
    size_t i;
    size_t length = 0;
    size_t capacity;
    char *output;

    if (token == NULL ||
        result == NULL ||
        (base != 4 && base != 8)) {
        return 1;
    }

    capacity = strlen(token) * 8 + 1;

    if (capacity < 2) {
        capacity = 2;
    }

    output = (char *)malloc(capacity);

    if (output == NULL) {
        return 2;
    }

    for (i = 0; token[i] != '\0'; ++i) {
        char *number;
        size_t number_length;
        unsigned int code;

        code = (unsigned int)(unsigned char)token[i];

        if (number_to_base(code, base, &number) != 0) {
            free(output);
            return 2;
        }

        number_length = strlen(number);

        /*
            При необходимости увеличиваем буфер.
        */
        if (length + number_length + 2 > capacity) {
            size_t new_capacity = capacity * 2;
            char *new_output;

            while (length + number_length + 2 > new_capacity) {
                if (new_capacity > (size_t)-1 / 2) {
                    free(number);
                    free(output);
                    return 2;
                }

                new_capacity *= 2;
            }

            new_output = (char *)realloc(output, new_capacity);

            if (new_output == NULL) {
                free(number);
                free(output);
                return 2;
            }

            output = new_output;
            capacity = new_capacity;
        }

        /*
            ASCII-коды символов разделяем пробелами.
        */
        if (i > 0) {
            output[length] = ' ';
            ++length;
        }

        memcpy(
            output + length,
            number,
            number_length
        );

        length += number_length;

        free(number);
    }

    output[length] = '\0';

    *result = output;

    return 0;
}

/* ============================================================
   ПРЕОБРАЗОВАНИЕ ЛАТИНСКИХ БУКВ В СТРОЧНЫЕ
   ============================================================ */

void convert_to_lowercase(char *token)
{
    size_t i;

    if (token == NULL) {
        return;
    }

    for (i = 0; token[i] != '\0'; ++i) {
        if (token[i] >= 'A' && token[i] <= 'Z') {
            token[i] = (char)(token[i] - 'A' + 'a');
        }
    }
}

/* ============================================================
   РЕЖИМ -a
   ============================================================ */

/*
    Правила:

    каждая 10-я лексема:
        1. латинские буквы -> строчные
        2. все символы -> ASCII-коды в системе 4

    каждая 2-я, но не 10-я:
        латинские буквы -> строчные

    каждая 5-я, но не 10-я:
        все символы -> ASCII-коды в системе 8

    остальные:
        без изменений
*/
int process_a(
    const char *input_name,
    const char *output_name
)
{
    FILE *input = NULL;
    FILE *output = NULL;

    char *token = NULL;
    char *converted = NULL;

    int result;
    int token_number = 0;
    int first = 1;
    int status = 0;

    if (input_name == NULL || output_name == NULL) {
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
        result = read_token(input, &token);

        /*
            Конец файла.
        */
        if (result == 1) {
            break;
        }

        /*
            Ошибка.
        */
        if (result != 0) {
            status = (result == 2) ? 3 : 4;
            break;
        }

        ++token_number;

        /*
            Каждая десятая лексема.
        */
        if (token_number % 10 == 0) {
            convert_to_lowercase(token);

            result = convert_to_ascii_codes(
                token,
                4,
                &converted
            );

            if (result != 0) {
                free(token);
                status = 3;
                break;
            }

            free(token);
            token = converted;
            converted = NULL;
        }

        /*
            Каждая вторая, но не десятая.
        */
        else if (token_number % 2 == 0) {
            convert_to_lowercase(token);
        }

        /*
            Каждая пятая, но не десятая.
        */
        else if (token_number % 5 == 0) {
            result = convert_to_ascii_codes(
                token,
                8,
                &converted
            );

            if (result != 0) {
                free(token);
                status = 3;
                break;
            }

            free(token);
            token = converted;
            converted = NULL;
        }

        /*
            Записываем результат.
        */
        result = write_token(
            output,
            token,
            &first
        );

        free(token);
        token = NULL;

        if (result != 0) {
            status = 5;
            break;
        }
    }

    free(token);
    free(converted);

    if (fclose(input) != 0 && status == 0) {
        status = 5;
    }

    if (fclose(output) != 0 && status == 0) {
        status = 5;
    }

    return status;
}

/* ============================================================
   ПРОВЕРКА ФЛАГА
   ============================================================ */

int is_flag(const char *flag, char expected)
{
    if (flag == NULL) {
        return 0;
    }

    if (strlen(flag) != 2) {
        return 0;
    }

    if (flag[0] != '-' && flag[0] != '/') {
        return 0;
    }

    if (flag[1] != expected) {
        return 0;
    }

    return 1;
}

/* ============================================================
   MAIN
   ============================================================ */

int main(int argc, char *argv[])
{
    int status;

    /*
        --------------------------------------------------------
        ФЛАГ -r
        --------------------------------------------------------

        argc = 5

        argv[1] = -r
        argv[2] = file1
        argv[3] = file2
        argv[4] = output
    */
    if (argc >= 2 && is_flag(argv[1], 'r')) {
        if (argc != 5) {
            printf(
                "Ошибка: для флага -r требуется 4 аргумента "
                "после имени программы.\n"
            );
            return 1;
        }

        status = process_r(
            argv[2],
            argv[3],
            argv[4]
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

                default:
                    printf("Неизвестная ошибка.\n");
                    break;
            }

            return 1;
        }

        printf("Режим -r выполнен успешно.\n");

        return 0;
    }

    /*
        --------------------------------------------------------
        ФЛАГ -a
        --------------------------------------------------------

        argc = 4

        argv[1] = -a
        argv[2] = input
        argv[3] = output
    */
    if (argc >= 2 && is_flag(argv[1], 'a')) {
        if (argc != 4) {
            printf(
                "Ошибка: для флага -a требуется 3 аргумента "
                "после имени программы.\n"
            );
            return 1;
        }

        status = process_a(
            argv[2],
            argv[3]
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

                default:
                    printf("Неизвестная ошибка.\n");
                    break;
            }

            return 1;
        }

        printf("Режим -a выполнен успешно.\n");

        return 0;
    }

    /*
        Неизвестный флаг или неправильное количество аргументов.
    */
    printf("Ошибка: неизвестный флаг или неверное количество аргументов.\n");
    printf("Использование:\n");
    printf("  %s -r file1 file2 output\n", argv[0]);
    printf("  %s -a input output\n", argv[0]);

    return 1;
}