#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int make_output_filename(const char *input_path, const char *flag, const char *specified_output, char **output_path)
{
    size_t len;
    const char *file_name;
    const char *slash1;
    const char *slash2;
    const char *slash;

    if (input_path == NULL || flag == NULL || output_path == NULL) {
        return 0;
    }

    *output_path = NULL;

    /* Если присутствует n, имя уже задано пользователем */
    if (flag[1] == 'n') {
        if (specified_output == NULL || specified_output[0] == '\0') {
            return 0;
        }

        len = strlen(specified_output);

        *output_path = (char *)malloc(len + 1);
        if (*output_path == NULL) {
            return 0;
        }

        strcpy(*output_path, specified_output);
        return 1;
    }

    /*
        Находим последнюю часть пути.
        Учитываем как '/', так и '\\'.
    */
    slash1 = strrchr(input_path, '/');
    slash2 = strrchr(input_path, '\\');

    if (slash1 != NULL && slash2 != NULL) {
        slash = (slash1 > slash2) ? slash1 : slash2;
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

    /*
        len содержит длину каталога вместе с последним '/' или '\\'.
        + 4 -- длина "out_".
        + длина имени файла.
        + 1 -- '\0'.
    */
    {
        size_t name_len = strlen(file_name);
        char *result = (char *)malloc(len + 4 + name_len + 1);

        if (result == NULL) {
            return 0;
        }

        if (len > 0) {
            memcpy(result, input_path, len);
        }

        memcpy(result + len, "out_", 4);
        memcpy(result + len + 4, file_name, name_len + 1);

        *output_path = result;
    }

    return 1;
}


/*
    -d
    Исключает символы арабских цифр из входного файла.
*/
int process_d(FILE *input, FILE *output)
{
    int c;

    if (input == NULL || output == NULL) {
        return 0;
    }

    while ((c = fgetc(input)) != EOF) {
        if (!isdigit((unsigned char)c)) {
            if (fputc(c, output) == EOF) {
                return 0;
            }
        }
    }

    if (ferror(input)) {
        return 0;
    }

    return 1;
}


/*
    -i
    Для каждой строки записывает количество
    символов латинского алфавита.
*/
int process_i(FILE *input, FILE *output)
{
    int c;
    int count = 0;

    if (input == NULL || output == NULL) {
        return 0;
    }

    while ((c = fgetc(input)) != EOF) {
        if (isalpha((unsigned char)c) &&
            (((unsigned char)c >= 'A' && (unsigned char)c <= 'Z') ||
             ((unsigned char)c >= 'a' && (unsigned char)c <= 'z'))) {

            count++;
        }

        if (c == '\n') {
            if (fprintf(output, "%d\n", count) < 0) {
                return 0;
            }

            count = 0;
        }
    }

    if (ferror(input)) {
        return 0;
    }

    /*
        Если последняя строка не заканчивается '\n',
        её тоже необходимо обработать.
    */
    if (count != 0) {
        if (fprintf(output, "%d\n", count) < 0) {
            return 0;
        }
    }

    return 1;
}


/*
    -s
    Для каждой строки считает символы, которые НЕ являются:
    - буквами латинского алфавита;
    - арабскими цифрами;
    - пробелом.

    Символ '\n' не считается.
*/
int process_s(FILE *input, FILE *output)
{
    int c;
    int count = 0;

    if (input == NULL || output == NULL) {
        return 0;
    }

    while ((c = fgetc(input)) != EOF) {

        if (c == '\n') {
            if (fprintf(output, "%d\n", count) < 0) {
                return 0;
            }

            count = 0;
            continue;
        }

        if (c == ' ') {
            continue;
        }

        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z')) {
            continue;
        }

        if (c >= '0' && c <= '9') {
            continue;
        }

        count++;
    }

    if (ferror(input)) {
        return 0;
    }

    /*
        Обрабатываем последнюю строку,
        если она не заканчивается '\n'.
    */
    if (count != 0) {
        if (fprintf(output, "%d\n", count) < 0) {
            return 0;
        }
    }

    return 1;
}


/*
    -a
    Символы, отличные от цифр, заменяются их ASCII-кодами
    в шестнадцатеричной системе счисления.

    Например:

        abc!12

    превращается в:

        6162632112

    Здесь:
        a -> 61
        b -> 62
        c -> 63
        ! -> 21

    Цифры остаются без изменений.
*/
int process_a(FILE *input, FILE *output)
{
    int c;

    if (input == NULL || output == NULL) {
        return 0;
    }

    while ((c = fgetc(input)) != EOF) {

        if (c >= '0' && c <= '9') {
            if (fputc(c, output) == EOF) {
                return 0;
            }
        } else {
            if (fprintf(output, "%02X", (unsigned char)c) < 0) {
                return 0;
            }
        }
    }

    if (ferror(input)) {
        return 0;
    }

    return 1;
}


/*
    Проверка флага.
*/
int is_valid_flag(const char *flag)
{
    if (flag == NULL) {
        return 0;
    }

    if (strcmp(flag, "-d") == 0 ||
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
        strcmp(flag, "/na") == 0) {

        return 1;
    }

    return 0;
}


/*
    Возвращает последнюю букву флага:
        -d  -> d
        -nd -> d
        -i  -> i
        -ni -> i
        и т.д.
*/
char get_operation(const char *flag)
{
    size_t len;

    if (flag == NULL) {
        return '\0';
    }

    len = strlen(flag);

    if (len == 2) {
        return flag[1];
    }

    if (len == 3) {
        return flag[2];
    }

    return '\0';
}


/*
    Проверяет наличие символа n во флаге.
*/
int has_n(const char *flag)
{
    if (flag == NULL) {
        return 0;
    }

    return flag[1] == 'n';
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
    int success = 0;

    /*
        Возможные варианты:

        ./program -d input.txt
        ./program -i input.txt
        ./program -s input.txt
        ./program -a input.txt

        ./program -nd input.txt output.txt
        ./program -ni input.txt output.txt
        ./program -ns input.txt output.txt
        ./program -na input.txt output.txt
    */

    /*if (argc < 3 || argc > 4) {
        fprintf(stderr,
                "Usage:\n"
                "  %s -d|-i|-s|-a input_file\n"
                "  %s -nd|-ni|-ns|-na input_file output_file\n",
                argv[0], argv[0]);

        return 1;
    }*/

    flag = argv[1];
    input_path = argv[2];

    if (!is_valid_flag(flag)) {
        fprintf(stderr, "Error: invalid flag '%s'\n", flag);
        return 1;
    }

    /*
        Проверяем количество аргументов
        в зависимости от наличия n.
    */
    if (has_n(flag)) {
        if (argc != 4) {
            fprintf(stderr,
                    "Error: this flag requires an output file.\n");
            return 1;
        }

        specified_output = argv[3];
    } else {
        if (argc != 3) {
            fprintf(stderr,
                    "Error: this flag does not accept an output file.\n");
            return 1;
        }
    }

    operation = get_operation(flag);

    /*
        Формируем имя выходного файла.
    */
    if (!make_output_filename(input_path,
                              flag,
                              specified_output,
                              &output_path)) {

        fprintf(stderr,
                "Error: cannot allocate memory or create output path.\n");

        return 1;
    }

    /*
        Открываем входной файл.
    */
    input = fopen(input_path, "r");

    if (input == NULL) {
        fprintf(stderr,
                "Error: cannot open input file '%s'.\n",
                input_path);

        free(output_path);
        return 1;
    }

    /*
        Открываем выходной файл.
    */
    output = fopen(output_path, "w");

    if (output == NULL) {
        fprintf(stderr,
                "Error: cannot open output file '%s'.\n",
                output_path);

        fclose(input);
        free(output_path);

        return 1;
    }

    /*
        Выполняем выбранную операцию.
    */
    switch (operation) {
        case 'd':
            success = process_d(input, output);
            break;

        case 'i':
            success = process_i(input, output);
            break;

        case 's':
            success = process_s(input, output);
            break;

        case 'a':
            success = process_a(input, output);
            break;

        default:
            success = 0;
            break;
    }

    /*
        Закрываем файлы.
    */
    if (fclose(input) != 0) {
        success = 0;
    }

    if (fclose(output) != 0) {
        success = 0;
    }

    if (!success) {
        fprintf(stderr,
                "Error: processing file failed.\n");

        free(output_path);
        return 1;
    }

    printf("Result saved to: %s\n", output_path);

    free(output_path);

    return 0;
}