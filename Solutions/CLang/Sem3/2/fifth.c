
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include "myerrors.h"

typedef struct {
    FILE *stream;
    char *buffer;
    size_t capacity;
    size_t length;
    int to_string;
    int error;
} Writer;

static void put_char(Writer *w, char c)
{
    if (w->error)
        return;

    if (w->to_string) {
        if (w->length + 1 >= w->capacity) {
            w->error = 1;
            return;
        }
        w->buffer[w->length] = c;
        w->length++;
        w->buffer[w->length] = '\0';
    } else {
        if (fputc((unsigned char)c, w->stream) == EOF)
            w->error = 1;
        else
            w->length++;
    }
}

static void put_text(Writer *w, const char *s)
{
    while (*s)
        put_char(w, *s++);
}

static void put_unsigned_base(Writer *w, unsigned long long value,
                              unsigned int base, int uppercase)
{
    char digits[65];
    const char *alphabet = uppercase
        ? "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        : "0123456789abcdefghijklmnopqrstuvwxyz";
    size_t n = 0;

    do {
        digits[n++] = alphabet[value % base];
        value /= base;
    } while (value != 0);

    while (n > 0)
        put_char(w, digits[--n]);
}

static void put_signed_base(Writer *w, long long value,
                            unsigned int base, int uppercase)
{
    unsigned long long magnitude;

    if (value < 0) {
        put_char(w, '-');
        magnitude = (unsigned long long)(-(value + 1)) + 1;
    } else {
        magnitude = (unsigned long long)value;
    }

    put_unsigned_base(w, magnitude, base, uppercase);
}

static void put_roman(Writer *w, int value)
{
    static const int values[] = {
        1000, 900, 500, 400, 100, 90, 50, 40,
        10, 9, 5, 4, 1
    };
    static const char *symbols[] = {
        "M", "CM", "D", "CD", "C", "XC", "L",
        "XL", "X", "IX", "V", "IV", "I"
    };

    if (value <= 0 || value > 3999) {
        w->error = 1;
        return;
    }

    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        while (value >= values[i]) {
            put_text(w, symbols[i]);
            value -= values[i];
        }
    }
}

static void put_zeckendorf(Writer *w, unsigned int value)
{
    unsigned long long fib[48];
    size_t n = 0;

    fib[n++] = 1;
    fib[n++] = 2;

    while (n < sizeof(fib) / sizeof(fib[0])) {
        unsigned long long next = fib[n - 1] + fib[n - 2];

        if (next > UINT_MAX)
            break;

        fib[n++] = next;
    }

    if (value == 0) {
        put_char(w, '1');
        return;
    }

    int started = 0;

    for (size_t i = n; i > 0; i--) {
        unsigned long long f = fib[i - 1];

        if (f <= value) {
            put_char(w, '1');
            value -= (unsigned int)f;
            started = 1;
        } else if (started) {
            put_char(w, '0');
        }
    }

    put_char(w, '1');
}

static void put_memory(Writer *w, const void *ptr, size_t size)
{
    const unsigned char *bytes = (const unsigned char *)ptr;

    for (size_t i = 0; i < size; i++) {
        if (i != 0)
            put_char(w, ' ');

        for (int bit = 7; bit >= 0; bit--)
            put_char(w, (bytes[i] & (1u << bit)) ? '1' : '0');
    }
}

static int valid_base(int base)
{
    return base >= 2 && base <= 36 ? base : 10;
}

static int digit_value(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 10;
    return -1;
}

static int parse_base(const char *s, int base, int uppercase,
                      unsigned long long *result)
{
    unsigned long long value = 0;

    if (*s == '\0')
        return 0;

    for (; *s; s++) {
        int d = digit_value(*s);

        if (d < 0 || d >= base)
            return 0;

        if (uppercase && *s >= 'a' && *s <= 'z')
            return 0;
        if (!uppercase && *s >= 'A' && *s <= 'Z')
            return 0;

        if (value > (ULLONG_MAX - (unsigned int)d) / (unsigned int)base)
            return 0;

        value = value * (unsigned int)base + (unsigned int)d;
    }

    *result = value;
    return 1;
}

static int format_core(Writer *w, const char *format, va_list args)
{
    const char *p = format;

    while (*p && !w->error) {
        if (*p != '%') {
            put_char(w, *p++);
            continue;
        }

        p++;

        if (*p == '%') {
            put_char(w, '%');
            p++;
            continue;
        }

        if (p[0] == 'R' && p[1] == 'o') {
            int value = va_arg(args, int);
            put_roman(w, value);
            p += 2;
            continue;
        }

        if (p[0] == 'Z' && p[1] == 'r') {
            unsigned int value = va_arg(args, unsigned int);
            put_zeckendorf(w, value);
            p += 2;
            continue;
        }

        if (p[0] == 'C' && (p[1] == 'v' || p[1] == 'V')) {
            int value = va_arg(args, int);
            int base = valid_base(va_arg(args, int));
            put_signed_base(w, value, (unsigned int)base, p[1] == 'V');
            p += 2;
            continue;
        }

        if (p[0] == 't' && p[1] == 'o') {
            const char *s = va_arg(args, const char *);
            int base = valid_base(va_arg(args, int));
            unsigned long long value;

            if (!parse_base(s, base, 0, &value) ||
                value > (unsigned long long)LLONG_MAX) {
                w->error = 1;
            } else {
                put_signed_base(w, (long long)value, 10, 0);
            }

            p += 2;
            continue;
        }

        if (p[0] == 'T' && p[1] == 'O') {
            const char *s = va_arg(args, const char *);
            int base = valid_base(va_arg(args, int));
            unsigned long long value;

            if (!parse_base(s, base, 1, &value) ||
                value > (unsigned long long)LLONG_MAX) {
                w->error = 1;
            } else {
                put_signed_base(w, (long long)value, 10, 0);
            }

            p += 2;
            continue;
        }

        if (p[0] == 'm' &&
            (p[1] == 'i' || p[1] == 'u' ||
             p[1] == 'd' || p[1] == 'f')) {
            if (p[1] == 'i') {
                int value = va_arg(args, int);
                put_memory(w, &value, sizeof(value));
            } else if (p[1] == 'u') {
                unsigned int value = va_arg(args, unsigned int);
                put_memory(w, &value, sizeof(value));
            } else if (p[1] == 'd') {
                double value = va_arg(args, double);
                put_memory(w, &value, sizeof(value));
            } else {
                double value = (double)va_arg(args, double);
                float fvalue = (float)value;
                put_memory(w, &fvalue, sizeof(fvalue));
            }

            p += 2;
            continue;
        }

        /* Стандартный спецификатор: собираем его строку. */
        char spec[64];
        size_t n = 0;
        spec[n++] = '%';

        while (*p && strchr("diuoxXfFeEgGaAcsp", *p) == NULL) {
            if (n + 2 >= sizeof(spec)) {
                w->error = 1;
                break;
            }
            spec[n++] = *p++;
        }

        if (w->error || !*p) {
            w->error = 1;
            break;
        }

        char conversion = *p++;
        spec[n++] = conversion;
        spec[n] = '\0';

        char temp[4096];
        int written = -1;

        switch (conversion) {
            case 'd':
            case 'i':
                written = snprintf(temp, sizeof(temp), spec,
                                   va_arg(args, int));
                break;
            case 'u':
            case 'o':
            case 'x':
            case 'X':
                written = snprintf(temp, sizeof(temp), spec,
                                   va_arg(args, unsigned int));
                break;
            case 'f':
            case 'F':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
            case 'a':
            case 'A':
                written = snprintf(temp, sizeof(temp), spec,
                                   va_arg(args, double));
                break;
            case 'c':
                written = snprintf(temp, sizeof(temp), spec,
                                   va_arg(args, int));
                break;
            case 's':
                written = snprintf(temp, sizeof(temp), spec,
                                   va_arg(args, const char *));
                break;
            case 'p':
                written = snprintf(temp, sizeof(temp), spec,
                                   va_arg(args, void *));
                break;
            default:
                w->error = 1;
                break;
        }

        if (written < 0 || (size_t)written >= sizeof(temp)) {
            w->error = 1;
        } else {
            put_text(w, temp);
        }
    }

    if (w->to_string && w->capacity > 0)
        w->buffer[w->length] = '\0';

    return w->error ? -1 : (int)w->length;
}

int overfprintf(FILE *stream, const char *format, ...)
{
    if (!stream || !format)
        return -1;

    Writer w = {stream, NULL, 0, 0, 0, 0};

    va_list args;
    va_start(args, format);
    int result = format_core(&w, format, args);
    va_end(args);

    return result;
}

int oversprintf(char *buffer, size_t capacity, const char *format, ...)
{
    if (!buffer || capacity == 0 || !format)
        return -1;

    buffer[0] = '\0';

    Writer w = {NULL, buffer, capacity, 0, 1, 0};

    va_list args;
    va_start(args, format);
    int result = format_core(&w, format, args);
    va_end(args);

    return result;
}

int main(void)
{
    char buffer[1024];

    if (overfprintf(stdout, "Roman: %Ro\n", 1987) < 0)
        return 1;

    if (overfprintf(stdout, "Zeckendorf: %Zr\n", 100u) < 0)
        return 1;

    if (overfprintf(stdout, "Base 16: %Cv\n", 255, 16) < 0)
        return 1;

    if (overfprintf(stdout, "Base 16 uppercase: %CV\n", 255, 16) < 0)
        return 1;

    if (overfprintf(stdout, "Convert to decimal: %to\n", "ff", 16) < 0)
        return 1;

    if (overfprintf(stdout, "Memory: %mi\n", 42) < 0)
        return 1;

    if (oversprintf(buffer, sizeof(buffer),
                    "Number: %d, string: %s, Roman: %Ro",
                    42, "hello", 14) < 0)
        return 1;

    puts(buffer);
    return 0;
}
