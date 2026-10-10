#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "mymath.h"
#include "myerrors.h"

#define INITIAL_SIZE 16


static ErrorsDef get_digit_value(char c, int *value)
{
    if (value == NULL) {
        return INVALID_ARGUMENT;
    }

    if (c >= '0' && c <= '9') {
        *value = c - '0';
        return STAT_OK;
    }

    if (c >= 'A' && c <= 'Z') {
        *value = c - 'A' + 10;
        return STAT_OK;
    }

    if (c >= 'a' && c <= 'z') {
        *value = c - 'a' + 10;
        return STAT_OK;
    }

    return INVALID_CHARACTER;
}

static ErrorsDef parse_in_base( const char *str, int base, unsigned long long *value )
{
    size_t i;
    unsigned long long result = 0;

    if (str == NULL || value == NULL) {
        return INVALID_ARGUMENT;
    }

    if (base < 2 || base > 36) {
        return INVALID_BASE;
    }

    if (str[0] == '\0') {
        return INVALID_NUMBER_FORMAT;
    }

    for (i = 0; str[i] != '\0'; ++i) {
        int digit;
        ErrorsDef status;

        status = get_digit_value(str[i], &digit);

        if (status != STAT_OK) {
            return status;
        }

        if (digit >= base) {
            return INVALID_NUMBER_FORMAT;
        }

        /*
           Проверяем, что result * base + digit
           не превышает ULLONG_MAX.
        */
        if (result >
            (ULLONG_MAX - (unsigned long long)digit) /
            (unsigned long long)base) {
            return RESULT_OVERFLOW;
        }

        result = result * (unsigned long long)base
               + (unsigned long long)digit;
    }

    *value = result;

    return STAT_OK;
}

static ErrorsDef find_min_base( const char *str, int *base, unsigned long long *value)
{
    int current_base;
    int max_digit = 0;
    size_t i;

    if (str == NULL || base == NULL || value == NULL) {
        return INVALID_ARGUMENT;
    }

    if (str[0] == '\0') {
        return INVALID_NUMBER_FORMAT;
    }

    /*
       Сначала определяем максимальную цифру.
       Минимальное возможное основание должно быть
       как минимум на единицу больше этой цифры.
    */
    for (i = 0; str[i] != '\0'; ++i) {
        int digit;
        ErrorsDef status = get_digit_value(str[i], &digit);

        if (status != STAT_OK) {
            return status;
        }

        if (digit > max_digit) {
            max_digit = digit;
        }
    }

    current_base = max_digit + 1;

    if (current_base < 2) {
        current_base = 2;
    }

    if (current_base > 36) {
        return INVALID_BASE;
    }

    /*
       Проверяем основания от минимально возможного
       до 36 включительно.
    */
    for (; current_base <= 36; ++current_base) {
        ErrorsDef status = parse_in_base(
            str,
            current_base,
            value
        );

        if (status == STAT_OK) {
            *base = current_base;
            return STAT_OK;
        }

        if (status == RESULT_OVERFLOW) {
            /*
               При увеличении основания значение числа
               с теми же цифрами не уменьшается.
               Следовательно, дальнейшие основания
               тоже приведут к переполнению.
            */
            return RESULT_OVERFLOW;
        }

        if (status != INVALID_NUMBER_FORMAT) {
            return status;
        }
    }

    return INVALID_BASE;
}

static ErrorsDef remove_leading_zeroes( const char *str, char **result )
{
    const char *start;
    size_t length;
    char *copy;

    if (str == NULL || result == NULL) {
        return INVALID_ARGUMENT;
    }

    *result = NULL;

    if (str[0] == '\0') {
        return EMPTY_STRING;
    }

    start = str;

    while (start[0] == '0' && start[1] != '\0') {
        ++start;
    }

    length = strlen(start);

    if (length == (size_t)-1) {
        return STRING_OVERFLOW;
    }

    copy = malloc(length + 1);

    if (copy == NULL) {
        return MEMORY_ERROR;
    }

    memcpy(copy, start, length + 1);
    *result = copy;

    return STAT_OK;
}

static ErrorsDef read_token( FILE *file, char **token, int *eof)
{
    int c;
    size_t size = INITIAL_SIZE;
    size_t length = 0;
    char *buffer;

    if (file == NULL || token == NULL || eof == NULL) {
        return INVALID_ARGUMENT;
    }

    *token = NULL;
    *eof = 0;

    buffer = malloc(size);

    if (buffer == NULL) {
        return MEMORY_ERROR;
    }

    /* Пропускаем разделители. */
    do {
        c = fgetc(file);

        if (c == EOF) {
            if (ferror(file)) {
                free(buffer);
                return READFILE_ERROR;
            }

            free(buffer);
            *eof = 1;
            return STAT_OK;
        }
    } while (isspace((unsigned char)c));

    /* Читаем лексему. */
    while (c != EOF && !isspace((unsigned char)c)) {
        if (length >= size - 1) {
            size_t new_size;
            char *new_buffer;

            if (size > (size_t)-1 / 2) {
                free(buffer);
                return STRING_OVERFLOW;
            }

            new_size = size * 2;
            new_buffer = realloc(buffer, new_size);

            if (new_buffer == NULL) {
                free(buffer);
                return REALLOC_ERROR;
            }

            buffer = new_buffer;
            size = new_size;
        }

        buffer[length++] = (char)c;
        c = fgetc(file);
    }

    if (c == EOF && ferror(file)) {
        free(buffer);
        return READFILE_ERROR;
    }

    buffer[length] = '\0';
    *token = buffer;

    return STAT_OK;
}

static ErrorsDef write_result( FILE *file, const char *number, int base, unsigned long long decimal_value)
{
    if (file == NULL || number == NULL) {
        return INVALID_ARGUMENT;
    }

    if (base < 2 || base > 36) {
        return INVALID_BASE;
    }

    if (fprintf(
            file,
            "%s %d %llu\n",
            number,
            base,
            decimal_value
        ) < 0) {
        return WRITEFILE_ERROR;
    }

    return STAT_OK;
}

static ErrorsDef process_file( const char *input_name, const char *output_name)
{
    FILE *input = NULL;
    FILE *output = NULL;

    char *token = NULL;
    char *normalized = NULL;

    int eof = 0;
    ErrorsDef status = STAT_OK;
    ErrorsDef close_status;

    if (input_name == NULL || output_name == NULL ||
        input_name[0] == '\0' || output_name[0] == '\0') {
        return FILENAME_ERROR;
    }

    input = fopen(input_name, "r");

    if (input == NULL) {
        return OPENFILE_ERROR;
    }

    output = fopen(output_name, "w");

    if (output == NULL) {
        fclose(input);
        return OPENFILE_ERROR;
    }

    while (1) {
        int base;
        unsigned long long decimal_value;

        status = read_token(input, &token, &eof);

        if (status != STAT_OK) {
            break;
        }

        if (eof) {
            break;
        }

        status = remove_leading_zeroes(token, &normalized);

        if (status != STAT_OK) {
            break;
        }

        status = find_min_base(
            normalized,
            &base,
            &decimal_value
        );

        if (status != STAT_OK) {
            fprintf(
                stderr,
                "Ошибка обработки числа \"%s\".\n",
                token
            );
            break;
        }

        status = write_result(
            output,
            normalized,
            base,
            decimal_value
        );

        if (status != STAT_OK) {
            break;
        }

        free(token);
        free(normalized);

        token = NULL;
        normalized = NULL;
    }

    free(token);
    free(normalized);

    if (ferror(output) && status == STAT_OK) {
        status = WRITEFILE_ERROR;
    }

    close_status = (fclose(input) == 0)
        ? STAT_OK : CLOSEFILE_ERROR;

    if (status == STAT_OK && close_status != STAT_OK) {
        status = close_status;
    }

    close_status = (fclose(output) == 0)
        ? STAT_OK : CLOSEFILE_ERROR;

    if (status == STAT_OK && close_status != STAT_OK) {
        status = close_status;
    }

    return status;
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
    ErrorsDef status;

    if (argc != 3) {

        printf( "Использование:\n  %s input.txt output.txt\n", argv[0] );

        return report_error(MATCH_ARGS);
    }

    status = process_file(argv[1], argv[2]);

    if (status != STAT_OK) {
        
        return report_error(status);
    }

    printf("Файл успешно обработан.\n");

    return 0;
}