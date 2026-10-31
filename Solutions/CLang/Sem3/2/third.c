
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include "myerrors.h"

typedef struct {
    size_t file_index;
    size_t line;
    size_t column;
} Match;

typedef struct {
    Match *data;
    size_t size;
    size_t capacity;
} MatchList;

/* Длина строки без string.h */
static size_t my_strlen(const char *str)
{
    size_t len = 0;

    while (str[len] != '\0')
        ++len;

    return len;
}

/* Освобождение списка совпадений */
static void free_matches(MatchList *matches)
{
    if (matches != NULL) {
        free(matches->data);
        matches->data = NULL;
        matches->size = 0;
        matches->capacity = 0;
    }
}

/* Добавление найденного совпадения */
static ErrorsDef add_match(MatchList *matches,
                           size_t file_index,
                           size_t line,
                           size_t column)
{
    if (matches->size == matches->capacity) {
        size_t new_capacity;

        if (matches->capacity == 0) {
            new_capacity = 16;
        } else {
            if (matches->capacity > SIZE_MAX / 2)
                return MEMORY_ERROR;

            new_capacity = matches->capacity * 2;
        }

        if (new_capacity > SIZE_MAX / sizeof(Match))
            return MEMORY_ERROR;

        Match *temp = realloc(
            matches->data,
            new_capacity * sizeof(Match)
        );

        if (temp == NULL)
            return REALLOC_ERROR;

        matches->data = temp;
        matches->capacity = new_capacity;
    }

    matches->data[matches->size].file_index = file_index;
    matches->data[matches->size].line = line;
    matches->data[matches->size].column = column;
    ++matches->size;

    return STAT_OK;
}

/* Чтение всего файла в динамический буфер */
static ErrorsDef read_file(FILE *file,
                           char **result,
                           size_t *result_length)
{
    size_t capacity = 256;
    size_t length = 0;

    char *buffer = malloc(capacity);

    if (buffer == NULL)
        return MEMORY_ERROR;

    int ch;

    while ((ch = fgetc(file)) != EOF) {
        if (length >= capacity - 1) {
            if (capacity > SIZE_MAX / 2) {
                free(buffer);
                return MEMORY_ERROR;
            }

            size_t new_capacity = capacity * 2;

            char *temp = realloc(buffer, new_capacity);

            if (temp == NULL) {
                free(buffer);
                return REALLOC_ERROR;
            }

            buffer = temp;
            capacity = new_capacity;
        }

        buffer[length++] = (char)ch;
    }

    if (ferror(file)) {
        free(buffer);
        return READFILE_ERROR;
    }

    buffer[length] = '\0';

    *result = buffer;
    *result_length = length;

    return STAT_OK;
}

/* Проверка совпадения подстроки с позиции pos */
static int matches_at(const char *text,
                      size_t text_length,
                      size_t pos,
                      const char *pattern,
                      size_t pattern_length)
{
    if (pattern_length > text_length - pos)
        return 0;

    for (size_t j = 0; j < pattern_length; ++j) {
        if (text[pos + j] != pattern[j])
            return 0;
    }

    return 1;
}

/*
 * Функция с переменным числом аргументов.
 *
 * Параметры:
 * pattern     — искомая подстрока;
 * file_count  — количество файлов;
 * matches     — список результатов;
 * далее       — пути к файлам.
 */
static ErrorsDef find_all(const char *pattern,
                          size_t file_count,
                          MatchList *matches,
                          ...)
{
    if (pattern == NULL || matches == NULL)
        return INVALID_ARGUMENT;

    if (pattern[0] == '\0')
        return EMPTY_STRING;

    if (file_count == 0)
        return INVALID_ARGUMENT;

    matches->data = NULL;
    matches->size = 0;
    matches->capacity = 0;

    size_t pattern_length = my_strlen(pattern);
    ErrorsDef status = STAT_OK;

    va_list args;
    va_start(args, matches);

    for (size_t file_index = 0;
         file_index < file_count && status == STAT_OK;
         ++file_index) {

        const char *path = va_arg(args, const char *);

        if (path == NULL) {
            status = INVALID_ARGUMENT;
            break;
        }

        FILE *file = fopen(path, "rb");

        if (file == NULL) {
            status = OPENFILE_ERROR;
            break;
        }

        char *text = NULL;
        size_t text_length = 0;

        status = read_file(file, &text, &text_length);

        if (fclose(file) != 0 && status == STAT_OK)
            status = CLOSEFILE_ERROR;

        if (status != STAT_OK) {
            free(text);
            break;
        }

        size_t line = 1;
        size_t column = 1;

        for (size_t i = 0;
             i < text_length && status == STAT_OK;
             ++i) {

            if (matches_at(text, text_length, i,
                           pattern, pattern_length)) {

                status = add_match(
                    matches,
                    file_index,
                    line,
                    column
                );

                if (status != STAT_OK)
                    break;
            }

            /*
             * Координаты начала следующего символа.
             * Перенос строки учитывается как '\n'.
             */
            if (text[i] == '\n') {
                ++line;
                column = 1;
            } else {
                ++column;
            }
        }

        free(text);
    }

    va_end(args);

    if (status != STAT_OK)
        free_matches(matches);

    return status;
}

/* Вывод результатов отдельно от поиска */
static void print_results(const MatchList *matches,
                          size_t file_count,
                          const char *const paths[])
{
    for (size_t f = 0; f < file_count; ++f) {
        printf("File: %s\n", paths[f]);

        size_t found = 0;

        for (size_t i = 0; i < matches->size; ++i) {
            if (matches->data[i].file_index == f) {
                printf("  Line %zu, column %zu\n",
                       matches->data[i].line,
                       matches->data[i].column);

                ++found;
            }
        }

        if (found == 0)
            printf("  No matches found\n");
    }
}

int main(void)
{
    const char *pattern = "abc";

    const char *paths[] = {
        "file1.txt",
        "file2.txt"
    };

    const size_t file_count = sizeof(paths) / sizeof(paths[0]);

    MatchList matches = {0};

    ErrorsDef status = find_all(
        pattern,
        file_count,
        &matches,
        paths[0],
        paths[1]
    );

    if (status != STAT_OK) {
        errors(status);
        return 1;
    }

    print_results(&matches, file_count, paths);

    free_matches(&matches);

    return 0;
}
