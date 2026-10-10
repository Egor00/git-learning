#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <errno.h>

#include "mymath.h"
#include "myerrors.h"


static ErrorsDef nrt2_1(double *res, long power) {
    // возведение 2 в произвольную неотр степень (1/n < 1)
    if(power == 0) return ZERO_DIV;
    if(power < 0) return INVALID_INPUT;
    if(power == 1) {
        *res = 2.0;
        return STAT_OK;
    }
    double l = 1.0;
    double r = 2.0;
    *res = l;
    for(long i=0; i<=power;i++){
        double res1, res2;
        ErrorsDef err1 = powd(&res1, l, power);
        ErrorsDef err2 = powd(&res2, r, power);
        if(err1!=STAT_OK) return err1;
        if(err2!=STAT_OK) return err2;
        if(fabs(res1-2.0) < fabs(res2-2.0)){
            *res = l;
            r = (r-l)/2+l;
        }
        else{
            *res = r;
            l = (r-l)/2+l;
        }
    }
    return STAT_OK;
}

static ErrorsDef nrt2_2(double *res, long n) {

    *res = 1.0;
    if (n < 2l || fabs(n) > LONG_MAX) return INVALID_INPUT;
    if(n == 0) return STAT_OK;
    long long res1;
    ErrorsDef err1 = powi(&res1, 2, n);
    if (err1 != STAT_OK) return err1;
    err1 = nrt2_1(res ,res1);
    if(err1 != STAT_OK) return err1;
    return STAT_OK;
}

static ErrorsDef sqrec(double *res, long n) {
    if(n < 0 || fabs(n) > LONG_MAX) return INVALID_INPUT;
    if(n == 0) {
        *res = -0.5;
        return STAT_OK;
    }
    double x = -0.5;
    double res1;
    ErrorsDef err1;
    while(n != 0){
        n--;
        err1 = powd(&res1, x, 2);
        if (err1 != STAT_OK) return err1;
        x -= res1/2+1;
    }
    *res = x;
    return STAT_OK;
}



double deul(long n){
    double l = 2.0;
    double r = 3.0;
    double ans = l;
    for(long i = 0; i < n; i++){
        if(fabs(log(l)-1.0) - fabs(log(r)-1.0) < 0.0) {
            ans = l;
            r = (r-l)/2+l;
        } else{
            ans = r;
            l = (r-l)/2+l;
        }
    }
    return ans;
}

static ErrorsDef dpi(double *res, long n){
    double l = 3.0;
    double r = 4.0;
    *res = l;
    for(long i = 0; i <= n; i++){
        if(fabs(cos(l)+1.0) < fabs(cos(r)+1.0)) {
            *res = l;
            if((r-l)/2 - (DBL_MAX-l) > eps()) return FUNC_OVERFLOW;
            r = (r-l)/2+l;
        } else{
            *res = r;
            if((r-l)/2 - (DBL_MAX-l) > eps()) return FUNC_OVERFLOW;
            l = (r-l)/2+l;
        }
        
    }
    return STAT_OK;
}

double dln2(int n) {
    double l = 0.0;
    double r = 1.0;
    double ans = l;
    for (long i = 0; i< n; i++){
        if(fabs(exp(l)-2.0) - fabs(exp(r)-2.0) < 0.0) {
            ans = l;
            r = (r-l)/2+l;
        } else{
            ans = r;
            l = (r-l)/2+l;
        }
    }
    return ans;
}

double dsq2(long n) {
    double l = 1.0;
    double r = 2.0;
    double ans = l;
    double r1, r2;
    ErrorsDef err1, err2;
    for(int i = 0; i< n; i++){
        err1 = powd(&r1, l, 2);
        err2 = powd(&r2, r, 2);
        if (err1 != STAT_OK) return err1;
        if (err2 != STAT_OK) return err2;
        if(fabs(r1-2) - fabs(r2-2) < 0.0){
            ans = l;
            r = (r-l)/2+l;
        }
        else {
            ans = r;
            l = (r-l)/2+l;
        }
    }
    return ans;
}


static ErrorsDef eul(long n){
    if (n < 1 || fabs(n) > LONG_MAX) return INVALID_INPUT;
    long long fac = 1ll;
    double sum = 1.0l;
    for(long i = 1l; i<=n; i++){
        if (fac > LLONG_MAX/i) return FUNC_OVERFLOW;
        fac*=i;
        if (sum - (DBL_MAX - 1.0/fac) > 0.0) return FUNC_OVERFLOW;
        sum+=1.0/fac;
    }
    double r1;
    ErrorsDef err1 = powd(&r1, 1.0+(1.0/n), n);
    if (err1!=STAT_OK) return err1;
    printf("Lim: %.15f\n", r1);
    printf("Seq: %.15f\n", sum);
    printf("Eq: %.15f\n", deul(n));
    return STAT_OK;
}

static ErrorsDef pi(long n){
    if(n < 1l || fabs(n) > LONG_MAX) return INVALID_INPUT;
    long long res1, res2;
    ErrorsDef err1 = powi(&res1, 2, n);
    if (err1 != STAT_OK) return err1;
    ErrorsDef err2 = fact(&res2, n);
    if (err2 != STAT_OK) return err2;
    if (res1 > LLONG_MAX/res2) return FUNC_OVERFLOW;
    err1 = powi(&res1, res1*res2, 4);
    double d1 = res1*1.0;
    err2 = fact(&res2, 2*n);
    
    err1= powi(&res1 ,res2, 2);
    
    double d2 = res1*1.0;
    if(d2 - DBL_MAX/n > 0.0) return FUNC_OVERFLOW;
    d2*=n;
    double a1 = d1 / d2;
    
    double a2 = 0.0;
    for(long i = 1; i<= n; i++) {
        if(fabs(a2 - (DBL_MAX-4.0/(2*i-1))) > eps()) return FUNC_OVERFLOW;
        if(i%2 == 0) a2+=4.0/(2*i-1)*(-1);
        else a2+=4.0/(2*i-1);
    }
    printf("Lim %.15f\n", a1);
    printf("Seq: %.15f\n", a2);
    double res3;
    ErrorsDef err3 = dpi(&res3, n);
    if(err3 != STAT_OK) return err3;
    printf("Eq: %.15f\n", res3);
    if (err2 != STAT_OK) return err2;
    if (err1 != STAT_OK) {
        return err1;
    }
    return STAT_OK;
    
}

static ErrorsDef ln2(long n) {
    if(n == 0) return ZERO_DIV;
    if(n <0l || fabs(n) > LONG_MAX) return INVALID_INPUT;
    double res1;
    ErrorsDef err1 = nrt2_1(&res1, n);
    if(err1 != STAT_OK) return err1;
    if((double)n - DBL_MAX/(res1-1) > eps()) return FUNC_OVERFLOW;
    double a1 = n*(res1-1);
    printf("Lim: %.15f\n", a1);
    double a2 = 0.0;
    for(long i = 1; i<= n; i++) {
        if(i%2 == 0) a2+=1.0/i*(-1);
        else a2+=1.0/i;
    }
    printf("Seq: %.15f\n", a2);
    printf("Eq: %.15f\n", dln2(n));
    return STAT_OK;
}

static ErrorsDef sq2(long n) {
    if(fabs(n) > LONG_MAX) return INVALID_INPUT;
    double res1;
    ErrorsDef err1 = sqrec(&res1, n);
    if(err1 != STAT_OK) return err1;
    printf("Lim: %.15f\n", res1);
    err1 = nrt2_2(&res1, n);
    if(err1 != STAT_OK) return err1;
    printf("Seq: %.15f\n", res1);
    printf("Eq: %.15f\n", dsq2(n));
    return STAT_OK;
}


static ErrorsDef gam(long n) {
    if(n<1l || fabs(n) > LONG_MAX) return INVALID_INPUT;
    long double a1 = 0.0;
    long long f1 = 1;
    long long f2;
    ErrorsDef err1 = fact(&f2, n);
    if (err1 != STAT_OK) return err1;
    for(int k = 1; k <= n; k++) {
        if(f1 > LLONG_MAX/k) return FUNC_OVERFLOW;
        f1*=k;
        long long res1;
        err1 = fact(&res1, n-k);
        if(err1 != STAT_OK) return err1;
        if(a1 > LDBL_MAX-(1.0*f2/(res1*f1))*(1.0/k)*log(f1)) return FUNC_OVERFLOW;
        if(k%2==1) a1+=(1.0*f2/(res1*f1))*(1.0/k)*(-1)*log(f1);
        else a1+=(1.0*f2/(res1*f1))*(1.0/k)*log(f1);
    }
    printf("Lim: %.15Lf\n", a1);
    if (n < 2l) return INVALID_INPUT;
    double res2 = 0.0, res3;
    ErrorsDef err2 = dpi(&res3, 20);
    if(err2!= STAT_OK) return err2;
    err1 = powd(&res2, res3, 2);
    if (err1 != STAT_OK) {
        return err1;
    }

    double a2 = -res2/6;
    printf("%f\n", a2);
    long long res4;

    for(long k = 2; k <= n; k++) {
        double sqr = sqrt((double)k);
        
        err1 = powi(&res4, (long)sqr,2);
        if(err1 != STAT_OK) return err1;
        printf("%f ", 1.0/res4);
        res2 = 1.0/res4-1.0/k;
        printf("%f ", 1.0/res4 - 1.0/k);
        if (a2 - (DBL_MAX-res2) > eps()) return FUNC_OVERFLOW;
        a2+=res2;
        printf("%f\n", a2);
    }
    printf("Seq: %.15f\n", a2);
    errno = 0;
    double t = log(n);
    if(errno) {
        puts("Error in gam because of log!");
        return UNKNOWN_ERROR;
    }
    for(long p = 2; p <= n; p++) {
        int prime = is_prime(p);
        if (prime == -1) return INVALID_INPUT;
        else if(prime == 1) {
            double tmp = 1.0 - 1.0/p;

            if(t - DBL_MAX/tmp > eps()) return FUNC_OVERFLOW;
            t*=tmp;
        }

    }
    errno = 0;
    t = -log(t);
    if(errno) {
        puts("Error in gam because of log!");
        return UNKNOWN_ERROR;
    }
    printf("Eq: %.15f\n", t);
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
    if(argc != 3) {
        puts("Format:\n ./second.out flag(eul, pi, ln2, sq2, gamma) arg");
        return report_error(MATCH_ARGS);
    }
    char *st = NULL;
    errno = 0;
    long n = strtol(argv[2], &st, 10);
    if(errno == ERANGE) return report_error(INPUT_OVERFLOW);
    if(*st == '\0'){
        const char* flag = argv[1];
        if(strcmp(flag, "eul") == 0){
            return report_error(eul(n));
        }
        else if(strcmp(flag, "pi") == 0){
            return report_error(pi(n));
        }
        else if(strcmp(flag, "ln2") == 0){
            return report_error(ln2(n));
        }
        else if(strcmp(flag, "sq2") == 0){
            return report_error(sq2(n));
        }
        else if(strcmp(flag, "gamma") == 0){
            return report_error(gam(n));
            
        }
        else {
            puts("Format:\n ./second.out flag(eul, pi, ln2, sq2, gamma) arg");
            return report_error(MATCH_ARGS);
        }
    }
    else {
        puts("Format:\n ./second.out flag(eul, pi, ln2, sq2, gamma) arg");
        return report_error(MATCH_ARGS);
    }
}