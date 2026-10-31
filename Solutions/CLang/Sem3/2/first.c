
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <stdint.h>

#include "myerrors.h"


static size_t my_strlen(const char *str)
{
    size_t len = 0;

    while (str[len] != '\0')
        ++len;

    return len;
}


static int my_str_equal(const char *a, const char *b)
{
    size_t i = 0;

    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i])
            return 0;
        ++i;
    }

    return 1;
}


static ErrorsDef parse_uint(const char *str, unsigned int *result)
{
    unsigned int value = 0;
    size_t i = 0;

    if (str == NULL || result == NULL || str[0] == '\0')
        return INVALID_INPUT;

    while (str[i] != '\0') {
        unsigned char ch = (unsigned char)str[i];

        if (ch < '0' || ch > '9')
            return INVALID_NUMBER_FORMAT;

        unsigned int digit = (unsigned int)(ch - '0');

        if (value > (UINT_MAX - digit) / 10U)
            return INPUT_OVERFLOW;

        value = value * 10U + digit;
        ++i;
    }

    *result = value;
    return STAT_OK;
}


static ErrorsDef string_length(const char *str, size_t *result)
{
    if (str == NULL || result == NULL)
        return INVALID_ARGUMENT;
        

    *result = my_strlen(str);
    return STAT_OK;
}


static ErrorsDef reverse_string(const char *str, char **result)
{
    if (str == NULL || result == NULL)
        return INVALID_ARGUMENT;

    size_t len = my_strlen(str);

    if (len == SIZE_MAX)
        return STRING_OVERFLOW;

    char *res = (char*)malloc(sizeof(char)*(len + 1));

    if (res == NULL)
        return MEMORY_ERROR;

    for (size_t i = 0; i < len; ++i)
        res[i] = str[len - 1 - i];

    res[len] = '\0';
    *result = res;

    return STAT_OK;
}


static ErrorsDef uppercase_odd(const char *str, char **result)
{
    if (str == NULL || result == NULL)
        return INVALID_ARGUMENT;

    size_t len = my_strlen(str);

    if (len == SIZE_MAX)
        return STRING_OVERFLOW;

    char *res = (char*)malloc(sizeof(char)*(len + 1));

    if (res == NULL)
        return MEMORY_ERROR;

    for (size_t i = 0; i < len; ++i) {
        unsigned char ch = (unsigned char)str[i];

        if (i % 2 == 1)
            res[i] = (char)toupper(ch);
        else
            res[i] = str[i];
    }

    res[len] = '\0';
    *result = res;

    return STAT_OK;
}


static ErrorsDef reorder_string(const char *str, char **result)
{
    if (str == NULL || result == NULL)
        return INVALID_ARGUMENT;

    size_t len = my_strlen(str);

    if (len == SIZE_MAX)
        return STRING_OVERFLOW;

    char *res = (char*)malloc(sizeof(char)*(len + 1));

    if (res == NULL)
        return MEMORY_ERROR;

    size_t pos = 0;


    for (size_t i = 0; i < len; ++i) {
        if (isdigit((unsigned char)str[i]))
            res[pos++] = str[i];
    }


    for (size_t i = 0; i < len; ++i) {
        if (isalpha((unsigned char)str[i]))
            res[pos++] = str[i];
    }


    for (size_t i = 0; i < len; ++i) {
        if (!isdigit((unsigned char)str[i]) &&
            !isalpha((unsigned char)str[i]))
            res[pos++] = str[i];
    }

    res[pos] = '\0';
    *result = res;

    return STAT_OK;
}


static ErrorsDef concatenate_randomly(int argc, char *const argv[], char **result)
{
    if (argv == NULL || result == NULL || argc < 4)
        return INVALID_ARGUMENT;

    unsigned int seed;
    ErrorsDef status = parse_uint(argv[3], &seed);

    if (status != STAT_OK)
        return status;

    size_t count = (size_t)argc - 3;

    if (count > SIZE_MAX / sizeof(char *))
        return MEMORY_ERROR;

    char **strings = (char**)malloc(count * sizeof(char*));

    if (strings == NULL)
        return MEMORY_ERROR;

    strings[0] = argv[2];

    for (size_t i = 1; i < count; ++i)
        strings[i] = argv[i + 3];


    srand(seed);

    for (size_t i = count - 1; i > 0; --i) {
        size_t j = (size_t)rand() % (i + 1);

        char *temp = strings[i];
        strings[i] = strings[j];
        strings[j] = temp;
    }

    size_t total = 0;

    for (size_t i = 0; i < count; ++i) {
        size_t len = my_strlen(strings[i]);

        if (len > SIZE_MAX - total - 1) {
            free(strings);
            return STRING_OVERFLOW;
        }

        total += len;
    }

    char *res = (char*)malloc(sizeof(char)*(total + 1));

    if (res == NULL) {
        free(strings);
        return MEMORY_ERROR;
    }

    size_t pos = 0;

    for (size_t i = 0; i < count; ++i) {
        for (size_t j = 0; strings[i][j] != '\0'; ++j)
            res[pos++] = strings[i][j];
    }

    res[pos] = '\0';

    free(strings);
    *result = res;

    return STAT_OK;
}

static int report_error(ErrorsDef error) {
    if(error!=STAT_OK) {
        errors(error);
    }

    return (int)error;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        return report_error(MATCH_ARGS);
    }

    const char *flag = argv[1];
    const char *str = argv[2];

    ErrorsDef status = STAT_OK;
    char *result = NULL;

    if (my_str_equal(flag, "-l")) {
        if (argc != 3) {
            return report_error(MATCH_ARGS);
        }

        size_t len;
        status = string_length(str, &len);

        if (status == STAT_OK)
            printf("Length: %zu\n", len);

    } else if (my_str_equal(flag, "-r")) {
        if (argc != 3) {
            return report_error(MATCH_ARGS);
        }

        status = reverse_string(str, &result);

    } else if (my_str_equal(flag, "-u")) {
        if (argc != 3) {
            return report_error(MATCH_ARGS);
        }

        status = uppercase_odd(str, &result);

    } else if (my_str_equal(flag, "-n")) {
        if (argc != 3) {
            return report_error(MATCH_ARGS);
        }

        status = reorder_string(str, &result);

    } else if (my_str_equal(flag, "-c")) {
        status = concatenate_randomly(argc, argv, &result);

    } else {
        return report_error(INVALID_FLAG);
    }

    if (status != STAT_OK) {
        return report_error(status);
        free(result);
    }

    if (result != NULL) {
        printf("Result: %s\n", result);
        free(result);
    }

    return STAT_OK;
}
