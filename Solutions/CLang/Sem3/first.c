#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>

#include "mymath.h"
#include "myerrors.h"


static ErrorsDef h(int x) {
    if (x == 0) {
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

static ErrorsDef p(int x){
    if(x <= 1){
        return NOT_PRIME;
    }
    if(x == 2) {
        puts("Prime");
        return STAT_OK;
    }
    if(x%2 == 0) {
        puts("Composite");
        return STAT_OK;
    }

    for(int i=3; eps() <= fabs(sqrt(x)+1.0-(double)i) ; i++){
        if(i == x) break;
        if(x % i == 0) {
            puts("Composite");
            return STAT_OK;
        }
    }
    puts("Prime");
    return STAT_OK;
}

static ErrorsDef s(int x){
    size_t n = 0;
    while (n < x) n*=10;
    while (x != 0) {
        printf("%X ", x % 10000);
        x /= 10000;
    }
    snprintf(out, 50, "%X", x);
    for(int i = 0; out[i]!='\0'; i++){
        printf("%c ", out[i]);
    }
    puts("");
}

void e(int x) {
    if(x>10){
        puts("Error. Number is too big!");
        return;
    }
    puts("Format \"Base: Powers\"");
    
    for(int j = 1; j<=10; j++){
        printf("%d: ", j);
        printf("%llu", powi(j, 1));
        if(x == 1) {
            puts("");
            continue;
        }
        for(int i = 2; i <= x; i++) printf(", %llu", powi(j, i));
        puts("");
    }
}

void a(int x) {
    printf("%d\n", (1+x)*x/2);
}


int main(int argc, char *argv[]){
    char *st = NULL;
    int32_t n = (int32_t)strtol(argv[1], &st, 10);
    if(*st == '\0'){
        char flag = argv[2][1];
        if(strcmp(flag, '-h') == 0 || strcmp(flag, '/h') == 0){
                h(n);
        }
        else if(strcmp(flag, '-p') == 0 || strcmp(flag, '/p') == 0) {
                p(n);
        }
        else if(strcmp(flag, '-s') == 0 || strcmp(flag, '/s') == 0) {
                s(n);
        }
        else if(strcmp(flag, '-e') == 0 || strcmp(flag, '/e') == 0) {
                e(n);
        }
        else if(strcmp(flag, '-a') == 0 || strcmp(flag, '/a') == 0) {
                a(n);
        }
        else if(strcmp(flag, '-f') == 0 || strcmp(flag, '/f') == 0) {
                printf("%lld\n", f(n));
        }
        else {
                puts("Format:\n ./first.out -flag(-h, -p, -s, -e, -a, -f,\n\\h, \\p, \\s, \\e, \\a, \\f) (arg)");
                
        }
    }
}