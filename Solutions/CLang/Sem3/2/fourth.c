
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include "myerrors.h"

typedef struct {
    char *filename;
    size_t size;
    unsigned long long importance;
} File;

typedef struct {
    File *files;
    size_t count;
    size_t total_size;
    unsigned long long total_importance;
} BackupResult;

/* Чтение токена произвольной длины из stdin */
static ErrorsDef read_token(char **result)
{
    size_t capacity = 32;
    size_t length = 0;

    char *str = malloc(capacity);
    if (str == NULL)
        return MEMORY_ERROR;

    int ch;

    do {
        ch = getchar();
    } while (ch == ' ' || ch == '\t' ||
             ch == '\n' || ch == '\r' ||
             ch == '\f' || ch == '\v');

    if (ch == EOF) {
        free(str);
        return INVALID_INPUT;
    }

    while (ch != EOF &&
           ch != ' ' && ch != '\t' &&
           ch != '\n' && ch != '\r' &&
           ch != '\f' && ch != '\v') {

        if (length >= capacity - 1) {
            if (capacity > SIZE_MAX / 2) {
                free(str);
                return MEMORY_ERROR;
            }

            size_t new_capacity = capacity * 2;
            char *temp = realloc(str, new_capacity);

            if (temp == NULL) {
                free(str);
                return REALLOC_ERROR;
            }

            str = temp;
            capacity = new_capacity;
        }

        str[length++] = (char)ch;
        ch = getchar();
    }

    if (ch == EOF && ferror(stdin)) {
        free(str);
        return READFILE_ERROR;
    }

    str[length] = '\0';
    *result = str;

    return STAT_OK;
}

/* Преобразование токена в size_t */
static ErrorsDef parse_size(const char *str, size_t *result)
{
    size_t value = 0;

    if (str == NULL || result == NULL || str[0] == '\0')
        return INVALID_NUMBER_FORMAT;

    for (size_t i = 0; str[i] != '\0'; ++i) {
        if (str[i] < '0' || str[i] > '9')
            return INVALID_NUMBER_FORMAT;

        size_t digit = (size_t)(str[i] - '0');

        if (value > (SIZE_MAX - digit) / 10)
            return INPUT_OVERFLOW;

        value = value * 10 + digit;
    }

    *result = value;
    return STAT_OK;
}

/* Преобразование токена в unsigned long long */
static ErrorsDef parse_importance(
    const char *str,
    unsigned long long *result)
{
    unsigned long long value = 0;

    if (str == NULL || result == NULL || str[0] == '\0')
        return INVALID_NUMBER_FORMAT;

    for (size_t i = 0; str[i] != '\0'; ++i) {
        if (str[i] < '0' || str[i] > '9')
            return INVALID_NUMBER_FORMAT;

        unsigned long long digit =
            (unsigned long long)(str[i] - '0');

        if (value > (ULLONG_MAX - digit) / 10ULL)
            return INPUT_OVERFLOW;

        value = value * 10ULL + digit;
    }

    *result = value;
    return STAT_OK;
}

static int same_string(const char *a, const char *b)
{
    size_t i = 0;

    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i])
            return 0;
        ++i;
    }

    return a[i] == b[i];
}

static void free_files(File *files, size_t count)
{
    if (files != NULL) {
        for (size_t i = 0; i < count; ++i)
            free(files[i].filename);

        free(files);
    }
}

static void free_result(BackupResult *result)
{
    if (result != NULL) {
        free_files(result->files, result->count);
        result->files = NULL;
        result->count = 0;
        result->total_size = 0;
        result->total_importance = 0;
    }
}

/*
 * Решение задачи о рюкзаке 0/1.
 * dp[i][w] — максимальная важность среди первых i файлов
 * при вместимости не более w.
 */
static ErrorsDef select_files(
    const File *files,
    size_t n,
    size_t capacity,
    BackupResult *result)
{
    if (files == NULL || result == NULL || n == 0)
        return INVALID_ARGUMENT;

    result->files = NULL;
    result->count = 0;
    result->total_size = 0;
    result->total_importance = 0;

    if (capacity == SIZE_MAX)
        return MEMORY_ERROR;

    size_t rows = n + 1;
    size_t cols = capacity + 1;

    if (rows > SIZE_MAX / cols)
        return MEMORY_ERROR;

    size_t cells = rows * cols;

    if (cells > SIZE_MAX / sizeof(unsigned long long))
        return MEMORY_ERROR;

    unsigned long long *dp =
        calloc(cells, sizeof(unsigned long long));

    if (dp == NULL)
        return MEMORY_ERROR;

    /*
     * Индекс элемента dp[i][w]:
     * i * cols + w
     */
    for (size_t i = 1; i <= n; ++i) {
        for (size_t w = 0; w <= capacity; ++w) {
            unsigned long long without =
                dp[(i - 1) * cols + w];

            unsigned long long best = without;

            if (files[i - 1].size <= w) {
                unsigned long long previous =
                    dp[(i - 1) * cols + w - files[i - 1].size];

                if (previous >
                    ULLONG_MAX - files[i - 1].importance) {
                    free(dp);
                    return FUNC_OVERFLOW;
                }

                unsigned long long with =
                    previous + files[i - 1].importance;

                if (with > best)
                    best = with;
            }

            dp[i * cols + w] = best;
        }
    }

    /* Восстанавливаем выбранные файлы */
    size_t selected_count = 0;
    size_t w = capacity;

    for (size_t i = n; i > 0; --i) {
        if (dp[i * cols + w] !=
            dp[(i - 1) * cols + w]) {
            ++selected_count;
            w -= files[i - 1].size;
        }
    }

    if (selected_count > SIZE_MAX / sizeof(File)) {
        free(dp);
        return MEMORY_ERROR;
    }

    File *selected = NULL;

    if (selected_count > 0) {
        selected = malloc(selected_count * sizeof(File));

        if (selected == NULL) {
            free(dp);
            return MEMORY_ERROR;
        }
    }

    /*
     * Восстановление с конца, поэтому записываем
     * элементы в обратные позиции массива.
     */
    w = capacity;
    size_t pos = selected_count;

    for (size_t i = n; i > 0; --i) {
        if (dp[i * cols + w] !=
            dp[(i - 1) * cols + w]) {
            --pos;

            selected[pos] = files[i - 1];
            w -= files[i - 1].size;
        }
    }

    result->files = selected;
    result->count = selected_count;
    result->total_importance = dp[n * cols + capacity];

    for (size_t i = 0; i < selected_count; ++i)
        result->total_size += selected[i].size;

    free(dp);
    return STAT_OK;
}

int main(void)
{
    char *token = NULL;
    size_t n = 0;
    size_t capacity = 0;

    ErrorsDef status = read_token(&token);

    if (status != STAT_OK) {
        errors(status);
        return 1;
    }

    status = parse_size(token, &n);
    free(token);
    token = NULL;

    if (status != STAT_OK || n == 0) {
        errors(status != STAT_OK ? status : INVALID_ARRAY_SIZE);
        return 1;
    }

    status = read_token(&token);

    if (status != STAT_OK) {
        errors(status);
        return 1;
    }

    status = parse_size(token, &capacity);
    free(token);
    token = NULL;

    if (status != STAT_OK) {
        errors(status);
        return 1;
    }

    if (n > SIZE_MAX / sizeof(File)) {
        errors(MEMORY_ERROR);
        return 1;
    }

    File *files = calloc(n, sizeof(File));

    if (files == NULL) {
        errors(MEMORY_ERROR);
        return 1;
    }

    size_t loaded = 0;

    for (size_t i = 0; i < n; ++i) {
        status = read_token(&files[i].filename);

        if (status != STAT_OK)
            break;

        status = read_token(&token);

        if (status != STAT_OK)
            break;

        status = parse_size(token, &files[i].size);
        free(token);
        token = NULL;

        if (status != STAT_OK || files[i].size == 0) {
            if (status == STAT_OK)
                status = INVALID_INPUT;
            break;
        }

        status = read_token(&token);

        if (status != STAT_OK)
            break;

        status = parse_importance(token, &files[i].importance);
        free(token);
        token = NULL;

        if (status != STAT_OK)
            break;

        /* Имена файлов должны быть уникальными */
        for (size_t j = 0; j < i; ++j) {
            if (same_string(files[i].filename,
                            files[j].filename)) {
                status = INVALID_INPUT;
                break;
            }
        }

        if (status != STAT_OK)
            break;

        ++loaded;
    }

    free(token);

    if (status != STAT_OK) {
        free_files(files, n);
        errors(status);
        return 1;
    }

    /*
     * loaded == n: все строки успешно считаны.
     * Дополнительные токены во входе запрещены.
     */
    int ch;

    do {
        ch = getchar();
    } while (ch == ' ' || ch == '\t' ||
             ch == '\n' || ch == '\r' ||
             ch == '\f' || ch == '\v');

    if (ch != EOF || ferror(stdin)) {
        free_files(files, n);
        errors(INVALID_INPUT);
        return 1;
    }

    BackupResult result = {0};

    status = select_files(files, n, capacity, &result);

    if (status == STAT_OK) {
        printf("TOTAL_SIZE %zu\n", result.total_size);
        printf("TOTAL_IMPORTANCE %llu\n", result.total_importance);
        printf("FILES %zu\n", result.count);

        for (size_t i = 0; i < result.count; ++i)
            printf("%s\n", result.files[i].filename);
    } else {
        errors(status);
    }

    free_result(&result);
    free_files(files, n);

    return status == STAT_OK ? 0 : 1;
}
