#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>

#include "mymath.h"
#include "myerrors.h"

static ErrorsDef enrt2_1(long power){
    if(power < 0) return INVALID_INPUT;
    if(power == 0) return ZERO_DIV;
    return STAT_OK;
}

static ErrorsDef esqrec(long n) {
    if(n <= -1) return INVALID_INPUT;
    return STAT_OK;
}

static ErrorsDef enrt2_2(long n) {
    if(n>INT_MAX) return FUNC_OVERFLOW;
    return STAT_OK;
}


double nrt2_1(long power) {
    // возведение 2 в произвольную неотр степень (1/n < 1)
    if(power == 1) return 2.0;
    double l = 1.0;
    double r = 2.0;
    double ans = l;
    for(int i=0; i<=power;i++){
        double res1, res2;
        ErrorsDef err1 = powd(&res1, l, power);
        ErrorsDef err2 = powd(&res2, r, power);
        if(err1!=STAT_OK) return err1;
        if(err2!=STAT_OK) return err2;
        if(fabs((fabs(res1-2.0)) - fabs(res2-2.0)) < eps()){
            ans = l;
            r = (r-l)/2+l;
        }
        else{
            ans = r;
            l = (r-l)/2+l;
        }
    }
    return ans;
}

double sqrec(long n) {

    if(n == 0) return -0.5;
    double x = -0.5;
    double res;
    ErrorsDef err;
    while(n != 0){
        n--;
        err = powd(&res, x, 2);
        if (err != STAT_OK) return err;
        x = x - res/2+1;
    }
    return x;
}

double nrt2_2(long n) {

    double ans = 1.0;
    long long res1, res2, res3;
    for(long i = 1; i <= n; i++) {
        ErrorsDef err1, err2, err3;
        err1 = powi(&res1, 2, i);
        err2 = fdoub(&res2, 2*i-3);
        err3 = fact(&res3, i);

        if (err1 != STAT_OK) return err1;
        if (err2 != STAT_OK) return err2;
        if (err3 != STAT_OK) return err3;
        if(i%2 == 0) {
            ans+=(-1.0)*res2/res1/res3;
        }
        else {
            ans+=1.0*res2/res1/res3;
        }
    }
    return ans;
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

double dpi(long n){
    double l = 3.0;
    double r = 4.0;
    double ans = l;
    for(int i = 0; i < n; i++){
        if(fabs(cos(l)+1.0) - fabs(cos(r)+1.0) < 0.0) {
            ans = l;
            r = (r-l)/2+l;
        } else{
            ans = r;
            l = (r-l)/2+l;
        }
    }
    return ans;
}

double dln2(int n) {
    double l = 0.0;
    double r = 1.0;
    double ans = l;
    for (int i = 0; i< n; i++){
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

double dsq2(int n) {
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
    long long res1, res2;
    ErrorsDef err1 = powi(&res1, 2, n);
    if (err1 != STAT_OK) return err1;
    ErrorsDef err2 = fact(&res2, n);
    if (err2 != STAT_OK) return err2;
    if (res1 > LLONG_MAX/res2) return FUNC_OVERFLOW;
    err1 = powi(&res1, res1*res2, 4);
    double d1 = res1*1.0;
    err2 = fact(&res2, 2*n);
    if (err2 != STAT_OK) return err2;
    err1= powi(&res1 ,res2, 2);
    if (err1 != STAT_OK) return err1;
    double d2 = res1*1.0;
    if(d2 - DBL_MAX/n > 0.0) return FUNC_OVERFLOW;
    d2*=n;
    double a1 = d1 / d2;
    
    double a2 = 0.0;
    for(long i = 1; i<= n; i++) {
        if(a2 - (DBL_MAX-4.0/(2*i-1)) > 0.0) return FUNC_OVERFLOW;
        if(i%2 == 0) a2+=4.0/(2*i-1)*(-1);
        else a2+=4.0/(2*i-1);
    }
    printf("Lim %.15f\n", a1);
    printf("Seq: %.15f\n", a2);
    printf("Eq: %.15f\n", dpi(n));
    return STAT_OK;
    
}

static ErrorsDef ln2(long n) {
    if(enrt2_1(n) != STAT_OK) return enrt2_1(n);
    double a1 = n*(nrt2_1(n)-1);
    double a2 = 0.0;
    for(int i = 1; i<= n; i++) {
        if(i%2 == 0) a2+=1.0/i*(-1);
        else a2+=1.0/i;
    }
    printf("Lim: %.15f\n", a1);
    printf("Seq: %.15f\n", a2);
    printf("Eq: %.15f\n", dln2(n));
    return STAT_OK;
}

static ErrorsDef sq2(long n) {
    if(esqrec(n) != STAT_OK) return esqrec(n);
    if(enrt2_2(n) != STAT_OK) return enrt2_2(n);
    printf("Lim: %.15f\n", sqrec(n));
    printf("Seq: %.15f\n", nrt2_2(n));
    printf("Eq: %.15f\n", dsq2(n));
    return STAT_OK;
}

static ErrorsDef gam(long n) {
    long double a1 = 0.0;
    long long f1 = 1;
    long long f2 = f(n);
    for(int k = 1; k <= n; k++) {
        if(f1 > LLONG_MAX/k) return FUNC_OVERFLOW;
        f1*=k;
        if(a1 > LDBL_MAX-(1.0*f2/(f(n-k)*f1))*(1.0/k)*log(f1)) return FUNC_OVERFLOW;
        if(k%2==1) a1+=(1.0*f2/(f(n-k)*f1))*(1.0/k)*(-1)*log(f1);
        else a1+=(1.0*f2/(f(n-k)*f1))*(1.0/k)*log(f1);
        printf("%Lf\n", a1);
    }
    printf("Lim: %.15Lf\n", a1);
    double res1;
    ErrorsDef err1 = powd(&res1, dpi(n), 2);
    if (err1 != STAT_OK) return err1;
    double a2 = -res1/6;
    for(int k = 2; k <= n; k++) {
        err1 = powd(&res1, sqrt(k),2);
        if (err1 != STAT_OK) return err1;
        a2+=1/res1-1.0/k;
    }
    printf("Seq: %.15f\n", a2);
    return STAT_OK;
    
}

int main(int argc, char *argv[]){
    char *st = NULL;
    char buff[32];
    snprintf(buff, sizeof(buff),"%ld", LONG_MAX);
    if (strcmp(argv[2], buff) > 0) {
        errors(INPUT_OVERFLOW);
        return INPUT_OVERFLOW;
    }
    long n = strtol(argv[2], &st, 10);
    if(*st == '\0'){
        const char* flag = argv[1];
        if(strcmp(st, "eul") == 0){
            errors(eul(n));
        }
        else if(strcmp(st, "pi") == 0){
            errors(pi(n));
        }
        else if(strcmp(st, "ln2") == 0){
            errors(ln2(n));
        }
        else if(strcmp(st, "sq2") == 0){
            errors(sq2(n));
        }
        else if(strcmp(st, "gamma") == 0){
            errors(gam(n));
        }
        else {
            puts("Format:\n ./second.out flag(eul, pi, ln2, sq2, gamma) arg");
        }
    }
    else {
        puts("Format:\n ./second.out flag(eul, pi, ln2, sq2, gamma) arg");
    }
}