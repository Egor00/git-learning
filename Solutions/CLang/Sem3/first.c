#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "mymath.h"
#include "myerrors.h"


static ErrorsDef h(long x) {
    if (x == 0L) {
        return ZERO_DIV;
    }
    int c = 0;
    for(int i=1; i <= 100; i++){
        if(i%x == 0){
            printf("%d\n", i);
            c = 1;
            return STAT_OK;
        };
    };
    if(!c) puts("Нет таких\n");
    return STAT_OK;
}

static ErrorsDef p(long x){
    if(x <= 1L){
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

    for(long i=3; eps() <= fabs(sqrt(x)+1.0-(double)i) ; i++){
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
    if(x<0) {
        return INVALID_INPUT;
    }
    size_t n = 1;
    while (n*10000 < x) n*=10;
    while (x != 0) {
        printf("%lX ", x % n);
        x /= n;
        n /= 10000;
    }
    return STAT_OK;
    /*snprintf(out, 50, "%X", x);
    for(int i = 0; out[i]!='\0'; i++){
        printf("%c ", out[i]);
    }
    puts("");*/
}

static ErrorsDef e(long x) {
    if(x>10L || x < 1L){
        return INVALID_INPUT;
    }
    puts("Format \"Base: Powers\"");
    long long res;
    for(int j = 1; j<=10; j++){
        printf("%d: ", j);
        ErrorsDef err = powi(&res, j, 1);
        if (err!= STAT_OK) return err;
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
    if(x > LONG_MAX - 1L || (1l+x) > LONG_MAX/(x/2)) return FUNC_OVERFLOW;
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
    char *st = NULL;
    char buff[32];
    snprintf(buff, sizeof(buff),"%ld", LONG_MAX);
    if (strcmp(argv[1], buff) > 0) return report_error(INPUT_OVERFLOW);
    long n = strtol(argv[1], &st, 10);
    if(*st == '\0'){
        const char* flag = argv[2];
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