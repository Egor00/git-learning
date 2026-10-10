#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "myerrors.h"

static int is_valid_flag(const char *flag)
{
    if (flag == NULL) {
        return 0;
    }

    return strcmp(flag, "-d") == 0 ||
           strcmp(flag, "/d") == 0 ||
           strcmp(flag, "-i") == 0 ||
           strcmp(flag, "/i") == 0 ||
           strcmp(flag, "-s") == 0 ||
           strcmp(flag, "/s") == 0 ||
           strcmp(flag, "-a") == 0 ||
           strcmp(flag, "/a") == 0 ||
           strcmp(flag, "-nd") == 0 ||
           strcmp(flag, "/nd") == 0 ||
           strcmp(flag, "-ni") == 0 ||
           strcmp(flag, "/ni") == 0 ||
           strcmp(flag, "-ns") == 0 ||
           strcmp(flag, "/ns") == 0 ||
           strcmp(flag, "-na") == 0 ||
           strcmp(flag, "/na") == 0;
}

static char get_operation(const char *flag)
{
    size_t len;

    if (flag == NULL) {
        return '\0';
    }

    len = strlen(flag);

    if (len == 2) {
        return flag[1];
    }

    if (len == 3 && flag[1] == 'n') {
        return flag[2];
    }

    return '\0';
}

static int has_n(const char *flag)
{
    if (flag == NULL) {
        return 0;
    }

    return flag[1] == 'n';
}

static ErrorsDef make_output_filename(const char *input_path,  const char *flag, const char *specified_output, char **output_path)
{
    size_t len;
    size_t name_len;
    const char *file_name;
    const char *slash1;
    const char *slash2;
    const char *slash;
    char *result;

    if (input_path == NULL || flag == NULL || output_path == NULL ||
        input_path[0] == '\0') {
        return FILENAME_ERROR;
    }

    *output_path = NULL;

    if (has_n(flag)) {
        if (specified_output == NULL || specified_output[0] == '\0') {
            return FILENAME_ERROR;
        }

        if (strcmp(input_path, specified_output) == 0) {
            return FILENAME_ERROR;
        }

        len = strlen(specified_output);

        if (len == (size_t)-1) {
            return MEMORY_ERROR;
        }

        result = (char *)malloc(len + 1);
        if (result == NULL) {
            return MEMORY_ERROR;
        }

        memcpy(result, specified_output, len + 1);
        *output_path = result;

        return STAT_OK;
    }

    slash1 = strrchr(input_path, '/');
    slash2 = strrchr(input_path, '\\');

    if (slash1 != NULL && slash2 != NULL) {
        slash = slash1 > slash2 ? slash1 : slash2;
    } else if (slash1 != NULL) {
        slash = slash1;
    } else {
        slash = slash2;
    }

    if (slash != NULL) {
        file_name = slash + 1;
        len = (size_t)(file_name - input_path);
    } else {
        file_name = input_path;
        len = 0;
    }

    if (file_name[0] == '\0') {
        return FILENAME_ERROR;
    }

    name_len = strlen(file_name);

    if (len > (size_t)-1 - 5 ||
        name_len > (size_t)-1 - len - 5) {
        return MEMORY_ERROR;
    }

    result = (char *)malloc(len + 4 + name_len + 1);
    if (result == NULL) {
        return MEMORY_ERROR;
    }

    if (len > 0) {
        memcpy(result, input_path, len);
    }

    memcpy(result + len, "out_", 4);
    memcpy(result + len + 4, file_name, name_len + 1);

    if (strcmp(input_path, result) == 0) {
        free(result);
        return FILENAME_ERROR;
    }

    *output_path = result;
    return STAT_OK;
}

static ErrorsDef process_d(FILE *input, FILE *output)
{
    int c;

    if (input == NULL || output == NULL) {
        return INVALID_ARGUMENT;
    }

    while ((c = fgetc(input)) != EOF) {
        if (!isdigit((unsigned char)c)) {
            if (fputc(c, output) == EOF) {
                return WRITEFILE_ERROR;
            }
        }
    }

    if (ferror(input)) {
        return READFILE_ERROR;
    }

    return STAT_OK;
}

static ErrorsDef process_i(FILE *input, FILE *output)
{
    int c;
    size_t count = 0;

    if (input == NULL || output == NULL) {
        return INVALID_ARGUMENT;
    }

    while ((c = fgetc(input)) != EOF) {
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z')) {
            if (count == (size_t)-1) {
                return RESULT_OVERFLOW;
            }
            count++;
        }

        if (c == '\n') 
        {
            if (fprintf(output, "%lu\n", (unsigned long)count) < 0) {
                return WRITEFILE_ERROR;
            }

            count = 0;
        }
    }

    if (ferror(input)) {
        return READFILE_ERROR;
    }

    if (count != 0) {
        if (fprintf(output, "%lu\n",
                    (unsigned long)count) < 0) {
            return WRITEFILE_ERROR;
        }
    }

    return STAT_OK;
}

static ErrorsDef process_s(FILE *input, FILE *output)
{
    int c;
    size_t count = 0;

    if (input == NULL || output == NULL) {
        return INVALID_ARGUMENT;
    }

    while ((c = fgetc(input)) != EOF) {
        if (c == '\n') {
            if (fprintf(output, "%lu\n",
                        (unsigned long)count) < 0) {
                return WRITEFILE_ERROR;
            }

            count = 0;
            continue;
        }

        if (c == ' ' ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9')) {
            continue;
        }

        if (count == (size_t)-1) {
            return RESULT_OVERFLOW;
        }

        count++;
    }

    if (ferror(input)) {
        return READFILE_ERROR;
    }

    if (count != 0) {
        if (fprintf(output, "%lu\n",
                    (unsigned long)count) < 0) {
            return WRITEFILE_ERROR;
        }
    }

    return STAT_OK;
}

static ErrorsDef process_a(FILE *input, FILE *output)
{
    int c;

    if (input == NULL || output == NULL) {
        return INVALID_ARGUMENT;
    }

    while ((c = fgetc(input)) != EOF) {
        if (c >= '0' && c <= '9') {
            if (fputc(c, output) == EOF) {
                return WRITEFILE_ERROR;
            }
        } else {
            if (fprintf(output, "%02X",
                        (unsigned int)(unsigned char)c) < 0) {
                return WRITEFILE_ERROR;
            }
        }
    }

    if (ferror(input)) {
        return READFILE_ERROR;
    }

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
    const char *flag;
    const char *input_path;
    const char *specified_output = NULL;

    char *output_path = NULL;

    FILE *input = NULL;
    FILE *output = NULL;

    char operation;
    ErrorsDef status = STAT_OK;

    if (argc < 3 || argv == NULL ||
        argv[1] == NULL || argv[2] == NULL) {
        return report_error(MATCH_ARGS);
    }

    flag = argv[1];
    input_path = argv[2];

    if (!is_valid_flag(flag)) {
        return report_error(INVALID_FLAG);
    }

    if (has_n(flag)) {
        if (argc != 4 || argv[3] == NULL) {
            return report_error(MATCH_ARGS);
        }

        specified_output = argv[3];
    } else if (argc != 3) {
        return report_error(MATCH_ARGS);
    }

    if (input_path[0] == '\0') {
        return report_error(FILENAME_ERROR);
    }

    operation = get_operation(flag);

    if (operation == '\0') {
        return report_error(INVALID_FLAG);
    }

    status = make_output_filename(
        input_path, flag, specified_output, &output_path);

    if (status != STAT_OK) {
        return report_error(status);
    }

    /*
        Запрещаем открывать один и тот же путь для чтения и записи.
        Это предотвращает случайное уничтожение исходного файла.
    */
    if (strcmp(input_path, output_path) == 0) {
        free(output_path);
        return report_error(FILENAME_ERROR);
    }

    input = fopen(input_path, "r");

    if (input == NULL) {
        free(output_path);
        return report_error(OPENFILE_ERROR);
    }

    output = fopen(output_path, "w");

    if (output == NULL) {
        if (fclose(input) != 0) {
            free(output_path);
            return report_error(CLOSEFILE_ERROR);
        }

        free(output_path);
        return report_error(OPENFILE_ERROR);
    }

    switch (operation) {
        case 'd':
            status = process_d(input, output);
            break;

        case 'i':
            status = process_i(input, output);
            break;

        case 's':
            status = process_s(input, output);
            break;

        case 'a':
            status = process_a(input, output);
            break;

        default:
            status = INVALID_FLAG;
            break;
    }

    if (fclose(input) != 0 && status == STAT_OK) {
        status = CLOSEFILE_ERROR;
    }

    if (fclose(output) != 0 && status == STAT_OK) {
        status = CLOSEFILE_ERROR;
    }

    if (status != STAT_OK) {
        free(output_path);
        return report_error(status);
    }

    printf("Result saved to: %s\n", output_path);

    free(output_path);
    return 0;
}
