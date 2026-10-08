#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "mymath.h"


double nrt2_1(int power) {
    if(power == 1) return 2.0;
    double l = 1.0;
    double r = 2.0;
    double ans = l;
    for(int i=0; i<=power;i++){
        if((fabs(powd(l, power)-2.0)) - fabs(powd(r, power)-2.0) < 0.0){
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

double sqrec(int32_t n, double x) {
    if(n <= 0) return x;
    n--;
    sqrec(n, x-powd(x, 2)/2+1);
}

double nrt2_2(int32_t n) {
    double ans = 1.0;
    for(int i = 1; i <= n; i++) {
        if(i%2 == 0) {
            ans+=(-1.0)*fdoub(2*i-3)/(powi(2, i)*f(i));
        }
        else {
            ans+=1.0*fdoub(2*i-3)/(powi(2, i)*f(i));
        }
    }
    return ans;
}

double deul(int n){
    double l = 2.0;
    double r = 3.0;
    double ans = l;
    for(int i = 0; i < n; i++){
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

double dpi(int n){
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
    for(int i = 0; i< n; i++){
        if(fabs(powd(l, 2)-2) - fabs(powd(r, 2)-2) < 0.0){
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


void eul(int n){
    int fac = 1;
    double sum = 1;
    for(int i = 1; i<=n; i++){
        fac*=i;
        sum+=1.0/fac;
    }
    printf("Lim: %.15f\n", powd(1.0+(1.0/n), n));
    printf("Seq: %.15f\n", sum);
    printf("Eq: %.15f\n", deul(n));

}

void pi(int n){
    double d1 = powi((powi(2, n)*f(n)), 4)*1.0;
    double d2 = n*powi(f(2*n), 2)*1.0;
    double a1 = d1 / d2;
    if(n>6) {
        puts("Слишком большое число для первого способа\n, не вмещается в longlong:(");
    }
    
    double a2 = 0.0;
    for(int i = 1; i<= n; i++) {
        if(i%2 == 0) a2+=4.0/(2*i-1)*(-1);
        else a2+=4.0/(2*i-1);
    }
    printf("Lim %.15f\n", a1);
    printf("Seq: %.15f\n", a2);
    printf("Eq: %.15f\n", dpi(n));
    
}

void ln2(int n) {
    double a1 = n*(nrt2_1(n)-1);
    double a2 = 0.0;
    for(int i = 1; i<= n; i++) {
        if(i%2 == 0) a2+=1.0/i*(-1);
        else a2+=1.0/i;
    }
    printf("Lim: %.15f\n", a1);
    printf("Seq: %.15f\n", a2);
    printf("Eq: %.15f\n", dln2(n));
}

void sq2(int n) {
    printf("Lim: %.15f\n", sqrec(n, -0.5));
    printf("Seq: %.15f\n", nrt2_2(n));
    printf("Eq: %.15f\n", dsq2(n));
}

void gam(int n) {
    long double a1 = 0.0;
    long long f1 = 1;
    long long f2 = f(n);
    for(int k = 1; k <= n; k++) {
        f1*=k;
        if(k%2==1) a1+=(1.0*f2/(f(n-k)*f1))*(1.0/k)*(-1)*log(f1);
        else a1+=(1.0*f2/(f(n-k)*f1))*(1.0/k)*log(f1);
        printf("%Lf\n", a1);
    }
    printf("Lim: %.15Lf\n", a1);
    double a2 = -powd(dpi(n), 2)/6;
    for(int k = 2; k <= n; k++) {
        a2+=1/powd(sqrt(k),2)-1.0/k;
    }
    printf("Seq: %.15f\n", a2);
    
}

int main(int argc, char *argv[]){
    char *st;
    strtol(argv[1], &st, 10);
    if(*st == '\0'){
        int32_t n = (int32_t)strtol(argv[2], NULL, 10);
        if(strcmp(st, "eul") == 0){
            eul(n);
        }
        else if(strcmp(st, "pi") == 0){
            pi(n);
        }
        else if(strcmp(st, "ln2") == 0){
            ln2(n);
        }
        else if(strcmp(st, "sq2") == 0){
            sq2(n);
        }
        else if(strcmp(st, "gamma") == 0){
            gam(n);
        }
    }
}