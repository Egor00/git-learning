#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "myerrors.h"

#define INITIAL_TOKEN_SIZE 16
#define MAX_PATH_SIZE 4096

static ErrorsDef read_token(FILE *file, char **token, int *eof)
{
    int c;
    size_t size = INITIAL_TOKEN_SIZE;
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
                return REALLOC_ERROR;
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

static ErrorsDef write_token( FILE *file, const char *token, int *first)
{
    if (file == NULL || token == NULL || first == NULL) {
        return INVALID_ARGUMENT;
    }

    if (!*first && fputc(' ', file) == EOF) {
        return WRITEFILE_ERROR;
    }

    if (fputs(token, file) == EOF) {
        return WRITEFILE_ERROR;
    }

    *first = 0;

    return STAT_OK;
}

static ErrorsDef process_r( const char *file1_name, const char *file2_name, const char *output_name)
{
    FILE *file1 = NULL;
    FILE *file2 = NULL;
    FILE *output = NULL;

    char *token1 = NULL;
    char *token2 = NULL;

    int eof1;
    int eof2;
    int first = 1;

    ErrorsDef status = STAT_OK;
    ErrorsDef close_status;

    if (file1_name == NULL ||
        file2_name == NULL ||
        output_name == NULL) {
        return FILENAME_ERROR;
    }

    if (file1_name[0] == '\0' ||
        file2_name[0] == '\0' ||
        output_name[0] == '\0') {
        return FILENAME_ERROR;
    }

    file1 = fopen(file1_name, "r");

    if (file1 == NULL) {
        return OPENFILE_ERROR;
    }

    file2 = fopen(file2_name, "r");

    if (file2 == NULL) {
        fclose(file1);
        return OPENFILE_ERROR;
    }

    output = fopen(output_name, "w");

    if (output == NULL) {
        fclose(file1);
        fclose(file2);
        return OPENFILE_ERROR;
    }

    while (1) {
        status = read_token(file1, &token1, &eof1);

        if (status != STAT_OK) {
            break;
        }

        status = read_token(file2, &token2, &eof2);

        if (status != STAT_OK) {
            free(token1);
            token1 = NULL;
            break;
        }

        if (eof1 && eof2) {
            free(token1);
            free(token2);
            token1 = NULL;
            token2 = NULL;
            break;
        }

        if (!eof1) {
            status = write_token(output, token1, &first);

            free(token1);
            token1 = NULL;

            if (status != STAT_OK) {
                free(token2);
                token2 = NULL;
                break;
            }
        }

        if (!eof2) {
            status = write_token(output, token2, &first);

            free(token2);
            token2 = NULL;

            if (status != STAT_OK) {
                break;
            }
        }
    }

    free(token1);
    free(token2);

    if (ferror(output) && status == STAT_OK) {
        status = WRITEFILE_ERROR;
    }

    close_status = (fclose(file1) == 0)
        ? STAT_OK : CLOSEFILE_ERROR;

    if (status == STAT_OK && close_status != STAT_OK) {
        status = close_status;
    }

    close_status = (fclose(file2) == 0)
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

static ErrorsDef number_to_base( unsigned int value, int base, char **result)
{
    char temporary[sizeof(unsigned int) * CHAR_BIT + 1];
    size_t length = 0;
    size_t i;
    char *output;

    if (result == NULL) {
        return INVALID_ARGUMENT;
    }

    *result = NULL;

    if (base != 4 && base != 8) {
        return INVALID_BASE;
    }

    if (value == 0) {
        output = malloc(2);

        if (output == NULL) {
            return MEMORY_ERROR;
        }

        output[0] = '0';
        output[1] = '\0';

        *result = output;
        return STAT_OK;
    }

    while (value > 0) {
        unsigned int digit = value % (unsigned int)base;

        if (length >= sizeof(temporary) - 1) {
            return RESULT_OVERFLOW;
        }

        temporary[length++] = (char)('0' + digit);
        value /= (unsigned int)base;
    }

    output = malloc(length + 1);

    if (output == NULL) {
        return MEMORY_ERROR;
    }

    for (i = 0; i < length; ++i) {
        output[i] = temporary[length - 1 - i];
    }

    output[length] = '\0';
    *result = output;

    return STAT_OK;
}

static ErrorsDef convert_to_ascii_codes( const char *token, int base, char **result)
{
    size_t i;
    size_t length = 0;
    size_t capacity = INITIAL_TOKEN_SIZE;
    char *output;

    if (token == NULL || result == NULL) {
        return INVALID_ARGUMENT;
    }

    *result = NULL;

    if (base != 4 && base != 8) {
        return INVALID_BASE;
    }

    output = malloc(capacity);

    if (output == NULL) {
        return MEMORY_ERROR;
    }

    output[0] = '\0';

    for (i = 0; token[i] != '\0'; ++i) {
        char *number = NULL;
        size_t number_length;
        size_t required;
        ErrorsDef status;
        unsigned int code;

        code = (unsigned int)(unsigned char)token[i];

        status = number_to_base(code, base, &number);

        if (status != STAT_OK) {
            free(output);
            return status;
        }

        number_length = strlen(number);

        if (number_length > (size_t)-1 - length - 2) {
            free(number);
            free(output);
            return STRING_OVERFLOW;
        }

        required = length + number_length + (i > 0 ? 1 : 0) + 1;

        if (required > capacity) {
            size_t new_capacity = capacity;

            while (new_capacity < required) {
                if (new_capacity > (size_t)-1 / 2) {
                    free(number);
                    free(output);
                    return STRING_OVERFLOW;
                }

                new_capacity *= 2;
            }

            {
                char *new_output = realloc(output, new_capacity);

                if (new_output == NULL) {
                    free(number);
                    free(output);
                    return REALLOC_ERROR;
                }

                output = new_output;
                capacity = new_capacity;
            }
        }

        if (i > 0) {
            output[length++] = ' ';
        }

        memcpy(output + length, number, number_length);
        length += number_length;
        output[length] = '\0';

        free(number);
    }

    *result = output;

    return STAT_OK;
}

static void convert_to_lowercase(char *token)
{
    size_t i;

    if (token == NULL) {
        return;
    }

    for (i = 0; token[i] != '\0'; ++i) {
        /*
           Изменяем только латинские ASCII-буквы.
           Остальные байты оставляем без изменений.
        */
        if (token[i] >= 'A' && token[i] <= 'Z') {
            token[i] = (char)(token[i] - 'A' + 'a');
        }
    }
}

static ErrorsDef process_a( const char *input_name, const char *output_name)
{
    FILE *input = NULL;
    FILE *output = NULL;

    char *token = NULL;
    char *converted = NULL;

    int eof = 0;
    int token_number = 0;
    int first = 1;

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
        status = read_token(input, &token, &eof);

        if (status != STAT_OK) {
            break;
        }

        if (eof) {
            break;
        }

        if (token_number == INT_MAX) {
            status = RESULT_OVERFLOW;
            break;
        }

        ++token_number;

        if (token_number % 10 == 0) {
            convert_to_lowercase(token);

            status = convert_to_ascii_codes(token, 4, &converted);

            if (status != STAT_OK) {
                break;
            }

            free(token);
            token = converted;
            converted = NULL;
        } else if (token_number % 5 == 0) {
            status = convert_to_ascii_codes(token, 8, &converted);

            if (status != STAT_OK) {
                break;
            }

            free(token);
            token = converted;
            converted = NULL;
        } else if (token_number % 2 == 0) {
            convert_to_lowercase(token);
        }

        status = write_token(output, token, &first);

        free(token);
        token = NULL;

        if (status != STAT_OK) {
            break;
        }
    }

    free(token);
    free(converted);

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

static int is_flag(const char *flag, char expected)
{
    if (flag == NULL) {
        return 0;
    }

    return (flag[0] == '-' || flag[0] == '/') &&
           flag[1] == expected &&
           flag[2] == '\0';
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

    if (argc < 2 || argv == NULL || argv[0] == NULL) {
        return report_error(MATCH_ARGS);
    }

    if (is_flag(argv[1], 'r')) {
        if (argc != 5) {
            
            printf("Использование: %s -r file1 file2 output\n", argv[0]);
            return report_error(MATCH_ARGS);;
        }

        status = process_r(argv[2], argv[3], argv[4]);

        if (status != STAT_OK) {
            
            return report_error(status);
        }

        printf("Режим -r выполнен успешно.\n");
        return 0;
    }

    if (is_flag(argv[1], 'a')) {
        if (argc != 4) {
            printf("Использование: %s -a input output\n", argv[0]);
            return report_error(status);
        }

        status = process_a(argv[2], argv[3]);

        if (status != STAT_OK) {
            return report_error(status);
        }

        printf("Режим -a выполнен успешно.\n");
        return 0;
    }

    errors(INVALID_FLAG);
    printf("Использование:\n");
    printf("  %s -r file1 file2 output\n", argv[0]);
    printf("  %s -a input output\n", argv[0]);

    return report_error(status);
}