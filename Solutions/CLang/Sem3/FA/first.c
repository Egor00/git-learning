#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "myerrors.h"
#include "mymath.h"



static ErrorsDef h(long x) {
    if (x == 0L) {
        return ZERO_DIV;
    }
    int c = 0;
    for(int i=1; i <= 100; i++){
        if(i%x == 0){
            printf("%d\n", i);
            c = 1;
        };
    };
    if(!c) puts("Нет таких");
    return STAT_OK;
}

static ErrorsDef p(long x){
    if(x <= 1L || x > LONG_MAX){
        return INVALID_INPUT;
    }
    if(x == 2L) {
        puts("Prime");
        return STAT_OK;
    }
    if(x%2L == 0L) {
        puts("Composite");
        return STAT_OK;
    }

    for(long i=3; eps() <= fabs(sqrt(x)+1.0-i) ; i++){
        if(i == x) break;
        if(x % i == 0) {
            puts("Composite");
            return STAT_OK;
        }
    }
    puts("Prime");
    return STAT_OK;
}

static ErrorsDef s(long x){
    if (x > LONG_MAX || x < 0) return INVALID_INPUT;
    char s[32];
    int err = snprintf(s, sizeof(s), "%lX", x);
    if(err < 0) return INVALID_INPUT;
    else if(err >= sizeof(s)) return FUNC_OVERFLOW;
    for(int i = 0; s[i] != '\0'; i++) {
        printf("%c ", s[i]);
    }
    puts("");
    return STAT_OK;
}

static ErrorsDef e(long x) {

    if(x>10 || x < 1){
        return INVALID_INPUT;
    }
    puts("Format \"Base: Powers\"");
    long long res;
    for(int j = 1; j<=10; j++){
        printf("%d: ", j);
        ErrorsDef err = powi(&res, j, 1l);
        if (err!= STAT_OK) {
            printf("%d", err);
            return err;
        }
        printf("%lld", res);
        if(x == 1) {
            puts("");
            continue;
        }
        for(long i = 2; i <= x; i++) {
            err = powi(&res, j, i);
            if (err!= STAT_OK) return err;
            printf(", %lld", res);
        }
        puts("");
    }
    return STAT_OK;
}

static ErrorsDef a(long x) {
    if(x < 1l) return INVALID_INPUT;
    if(x > LONG_MAX - 1L || (1l+x) > LONG_MAX/x) return FUNC_OVERFLOW;
    printf("%ld\n", (1l+x)*x/2);
    return STAT_OK;
}

unsigned long long int f(long x) {
    if(x < 0) return INVALID_INPUT;
    if(!x) {
        printf("%llu\n", 1ull);
        return STAT_OK;
    }
    unsigned long long int prod = 1ull;
    for(long i = 2l; i <= x; i++) {
        if(prod>ULLONG_MAX/i) return FUNC_OVERFLOW;
        prod*=i;
    }
    printf("%llu\n", prod);
    return STAT_OK;
}


static int report_error(ErrorsDef error)
{
    if (error != STAT_OK) {
        errors(error);
    }

    return (int)error;
}

int main(int argc, char *argv[]){
    if(argc != 3) return report_error(INVALID_ARGUMENT);
    char *st = NULL;
    errno = 0;
    long n = strtol(argv[2], &st, 10);
    if(errno == ERANGE) return report_error(INPUT_OVERFLOW);
    if(*st == '\0'){
        const char* flag = argv[1];
        if(strcmp(flag, "-h") == 0 || strcmp(flag, "/h") == 0){
            return report_error(h(n));
        }
        else if(strcmp(flag, "-p") == 0 || strcmp(flag, "/p") == 0) {
            return report_error(p(n));
        }
        else if(strcmp(flag, "-s") == 0 || strcmp(flag, "/s") == 0) {
            return report_error(s(n));
        }
        else if(strcmp(flag, "-e") == 0 || strcmp(flag, "/e") == 0) {
            return report_error(e(n));
        }
        else if(strcmp(flag, "-a") == 0 || strcmp(flag, "/a") == 0) {
            return report_error(a(n));
        }
        else if(strcmp(flag, "-f") == 0 || strcmp(flag, "/f") == 0) {
            return report_error(f(n));
        }
        else {
            puts("Format:\n ./first.out (arg) -flag(-h, -p, -s, -e, -a, -f,\n/h, /p, /s, /e, /a, /f)");
            return report_error(MATCH_ARGS);
        }
    }
    else {
        puts("Format:\n ./first.out (arg) -flag(-h, -p, -s, -e, -a, -f,\n/h, /p, /s, /e, /a, /f)");
        return report_error(MATCH_ARGS);
    }
}