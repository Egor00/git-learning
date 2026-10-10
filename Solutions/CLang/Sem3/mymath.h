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

static ErrorsDef powi(long long *res,long base, long power){
    *res = 1;
    if(base == 0l && power == 0l) return 1ull;
    if(base == 1l) return 1ull;
    for(long i = 0; i < power; i++) {
        if(*res > LLONG_MAX/base) {
            return POWER_OVERFLOW;
        }
        *res*=base;
    }
    return STAT_OK;
}

static ErrorsDef powd(double* res, double base, long power){
    *res = 1;
    if (fabs(base) < eps() && power == 0) return 1.0;
    if(fabs(base - 1.0) <= eps()) return 1.0;
    for(long i = 0; i < power; i++) {
        if(*res - (DBL_MAX/base) > 0.0) {
            return POWER_OVERFLOW;
        }
        *res*=base;
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