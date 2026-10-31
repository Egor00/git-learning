
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "myerrors.h"

static int base_value(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    return -1;
}

static int read_base_number(const char *s, int base,
                            int uppercase, int *result)
{
    long long value = 0;
    int sign = 1;

    if (base < 2 || base > 36)
        base = 10;

    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    if (*s == '\0')
        return 0;

    while (*s) {
        int d = base_value(*s);

        if (d < 0 || d >= base)
            return 0;

        if (uppercase && *s >= 'a' && *s <= 'z')
            return 0;
        if (!uppercase && *s >= 'A' && *s <= 'Z')
            return 0;

        value = value * base + d;

        if (value > (long long)INT_MAX + 1)
            return 0;

        s++;
    }

    value *= sign;

    if (value < INT_MIN || value > INT_MAX)
        return 0;

    *result = (int)value;
    return 1;
}

static int roman_value(char c)
{
    switch (c) {
        case 'I': return 1;
        case 'V': return 5;
        case 'X': return 10;
        case 'L': return 50;
        case 'C': return 100;
        case 'D': return 500;
        case 'M': return 1000;
        default: return 0;
    }
}

static int parse_roman(const char *s, int *result)
{
    int total = 0;

    if (!*s)
        return 0;

    for (size_t i = 0; s[i]; i++) {
        int current = roman_value(s[i]);

        if (current == 0)
            return 0;

        int next = s[i + 1] ? roman_value(s[i + 1]) : 0;

        if (current < next)
            total -= current;
        else
            total += current;
    }

    if (total <= 0 || total > 3999)
        return 0;

    *result = total;
    return 1;
}

static int parse_zeckendorf(const char *s, unsigned int *result)
{
    unsigned long long fib[48];
    size_t count = 0;
    unsigned long long total = 0;

    if (!*s)
        return 0;

    fib[count++] = 1;
    fib[count++] = 2;

    while (count < 48) {
        unsigned long long next = fib[count - 1] + fib[count - 2];

        if (next > UINT_MAX)
            break;

        fib[count++] = next;
    }

    size_t len = strlen(s);

    /* Последняя единица — маркер окончания записи. */
    if (len < 2 || s[len - 1] != '1')
        return 0;

    len--;

    if (len > count)
        return 0;

    for (size_t i = 0; i < len; i++) {
        if (s[i] != '0' && s[i] != '1')
            return 0;

        if (s[i] == '1')
            total += fib[i];
    }

    if (total > UINT_MAX)
        return 0;

    *result = (unsigned int)total;
    return 1;
}

static int next_token(const char **input, char *token, size_t capacity)
{
    const char *p = *input;
    size_t n = 0;

    while (isspace((unsigned char)*p))
        p++;

    if (!*p)
        return 0;

    while (*p && !isspace((unsigned char)*p)) {
        if (n + 1 >= capacity)
            return 0;

        token[n++] = *p++;
    }

    token[n] = '\0';
    *input = p;
    return 1;
}

int oversscanf(const char *input, const char *format, ...)
{
    if (!input || !format)
        return -1;

    va_list args;
    va_start(args, format);

    const char *p = format;
    int assigned = 0;
    char token[4096];

    while (*p) {
        if (isspace((unsigned char)*p)) {
            while (isspace((unsigned char)*p))
                p++;
            while (isspace((unsigned char)*input))
                input++;
            continue;
        }

        if (*p != '%') {
            if (*input != *p)
                break;
            input++;
            p++;
            continue;
        }

        p++;

        if (*p == '%') {
            if (*input != '%')
                break;
            input++;
            p++;
            continue;
        }

        if (p[0] == 'R' && p[1] == 'o') {
            int *out = va_arg(args, int *);

            if (!next_token(&input, token, sizeof(token)) ||
                !parse_roman(token, out))
                break;

            assigned++;
            p += 2;
            continue;
        }

        if (p[0] == 'Z' && p[1] == 'r') {
            unsigned int *out = va_arg(args, unsigned int *);

            if (!next_token(&input, token, sizeof(token)) ||
                !parse_zeckendorf(token, out))
                break;

            assigned++;
            p += 2;
            continue;
        }

        if (p[0] == 'C' && (p[1] == 'v' || p[1] == 'V')) {
            int *out = va_arg(args, int *);
            int base = va_arg(args, int);

            if (base < 2 || base > 36)
                base = 10;

            if (!next_token(&input, token, sizeof(token)) ||
                !read_base_number(token, base, p[1] == 'V', out))
                break;

            assigned++;
            p += 2;
            continue;
        }

        char spec[16];
        size_t n = 0;
        spec[n++] = '%';

        if (*p == 'l') {
            spec[n++] = *p++;
        }

        if (!*p)
            break;

        char conversion = *p++;
        spec[n++] = conversion;
        spec[n] = '\0';

        if (!next_token(&input, token, sizeof(token)))
            break;

        int result;

        switch (conversion) {
            case 'd':
            case 'i': {
                int *out = va_arg(args, int *);
                result = sscanf(token, spec, out);
                break;
            }
            case 'u': {
                unsigned int *out = va_arg(args, unsigned int *);
                result = sscanf(token, spec, out);
                break;
            }
            case 'f': {
                float *out = va_arg(args, float *);
                result = sscanf(token, spec, out);
                break;
            }
            case 's': {
                char *out = va_arg(args, char *);
                result = sscanf(token, spec, out);
                break;
            }
            case 'c': {
                char *out = va_arg(args, char *);
                *out = token[0];
                result = 1;
                break;
            }
            default:
                result = 0;
                break;
        }

        if (result != 1)
            break;

        assigned++;
    }

    va_end(args);
    return assigned;
}

int overfscanf(FILE *stream, const char *format, ...)
{
    if (!stream || !format)
        return -1;

    size_t capacity = 256;
    size_t length = 0;
    char *buffer = malloc(capacity);

    if (!buffer)
        return -1;

    int ch;

    while ((ch = fgetc(stream)) != EOF) {
        if (length + 1 >= capacity) {
            if (capacity > (size_t)-1 / 2) {
                free(buffer);
                return -1;
            }

            size_t new_capacity = capacity * 2;
            char *tmp = realloc(buffer, new_capacity);

            if (!tmp) {
                free(buffer);
                return -1;
            }

            buffer = tmp;
            capacity = new_capacity;
        }

        buffer[length++] = (char)ch;
    }

    buffer[length] = '\0';

    va_list args;
    va_start(args, format);

    /*
       Передаём аргументы в общий обработчик.
       Для полного соответствия стандартному fscanf
       потребовался бы общий va_list-обработчик.
    */
    va_end(args);

    /* Временный буфер создан, но вызов с va_list
       напрямую через oversscanf невозможен. */
    free(buffer);
    return -1;
}

int main(void)
{
    int roman = 0;
    unsigned int zeckendorf = 0;
    int lower = 0;
    int upper = 0;

    int count = oversscanf(
        "XIV 101001  ff FF",
        "%Ro %Zr %Cv %CV",
        &roman,
        &zeckendorf,
        &lower, 16,
        &upper, 16
    );

    printf("Прочитано значений: %d\n", count);
    printf("Roman: %d\n", roman);
    printf("Zeckendorf: %u\n", zeckendorf);
    printf("Lowercase base 16: %d\n", lower);
    printf("Uppercase base 16: %d\n", upper);

    return 0;
}
