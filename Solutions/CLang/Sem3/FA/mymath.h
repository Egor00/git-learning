#pragma once

#include <math.h>
#include <limits.h>
#include <float.h>

#include "myerrors.h"

typedef struct {
    double epsilon;
} Epsilon;

Epsilon _ = {.epsilon = 0.0000001};

void cheps(double epsi){
    _.epsilon = epsi;
}

double eps() {
    return _.epsilon;
}

int is_prime(long n) {
    if (n < 2l || n > LONG_MAX) return -1; // в целом нет смысла т.к. n уже проверяется до передачи
    if(n == 2) {
        return 1;
    }
    if(n%2 == 0) {
        return 0;
    }
    for(int i = 3; i < (int)sqrt(n)+1; i++) {
        if (n % i == 0) {
            return 0;
        }
    }
    return 1;
}

static ErrorsDef powi(long long *res, long base, long power){
    if(base > LLONG_MAX || power > LLONG_MAX) return INPUT_OVERFLOW;
    if(base == 0l && power == 0l) {
        *res = 1;
        return STAT_OK;
    }
    if(base == 1l) {
        *res = 1;
        return STAT_OK;
    }
    *res = 1;
    while(power != 0) {
        if(power % 2 == 0) {
            power /= 2;
            if(base > LLONG_MAX/base) return POWER_OVERFLOW;
            base *= base;
        }
        else{
            power-=1;
            if(*res > LLONG_MAX/base) return POWER_OVERFLOW;
            *res *= base;
        }
    }
    return STAT_OK;
}

static ErrorsDef powd(double* res, double base, long power){
    *res = 1.0;
    if (fabs(base) < eps() && power == 0) {
        *res = 1.0;
        return STAT_OK;
    }
    if(fabs(base - 1.0) < eps()) {
        *res = 1.0;
        return STAT_OK;
    }
    while (power!=0) {
        printf("%f\n", base);
        if(power%2 == 0) {
            power /= 2;
            if(base - (DBL_MAX/base) > eps()) return POWER_OVERFLOW;
            base*=base;
        }
        else {
            power -=1;
            if(*res - (DBL_MAX/base) > eps()) return POWER_OVERFLOW;
            *res*=base;
        }
        //printf("%f\n", *res);
    }
    return STAT_OK;
}

static ErrorsDef fact(long long *res, long x) {
    if(x > INT_MAX) return INPUT_OVERFLOW;
    *res = 1;
    if(!x) {
        *res = 1;
        return STAT_OK;
    }
    for(int i = 2; i <= x; i++) {
        if(*res > LLONG_MAX/i) {
            return FACTORIAL_OVERFLOW;
        }
        *res*=i;
    }
    return STAT_OK;
}

static ErrorsDef fdoub(long long *res, long x){
    if(x > INT_MAX) return INPUT_OVERFLOW;
    if(x == 0) {
        *res = 1;
        return STAT_OK;
    }
    if(x == -1) {
        *res = 1;
        return STAT_OK;
    }

    if(x%2 == 0) {
        *res = 0;
        for(int i = 2; i <= x; i+=2) {
            if(*res > LLONG_MAX/i) {
                return FACDOUB_OVERFLOW;
            }
            *res*=i;
        }
    }
    else {
        for(int i = 1; i <= x; i+=2) {
            if(*res > LLONG_MAX/i) {
                return FACDOUB_OVERFLOW;
            }
            *res*=i;
        }
    }
    return STAT_OK;
}