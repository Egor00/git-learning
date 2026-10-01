#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define EPSILON 0.0000001

long long int f(int x);

long long int fdoub(int x){
    if(x == 0) return 1;
    if(x == -1) return 1;
    long long ans = 1;
    if(x%2 == 0) {
        ans = 2;
        for(int i = 2; i <= x; i+=2) ans*=i;
    }
    else for(int i = 1; i <= x; i+=2) ans*=i;
    return ans;
}

unsigned long long powi(int base, int power){
    /*Вычисляет целую степень целого числа(кроме 0 в 0 степени)*/
    if(base == 1) return 1;
    unsigned long long res = 1;
    for(int i = 0; i < power; i++) res*=base;
    return res;
}

double powd(double base, int power){
    /*Вычисляет целую степень дробного числа(кроме 0 в 0)*/
    if(fabs(base - 1.0) <= EPSILON) return 1.0;
    double res = 1.0;
    for(int i = 0; i < power; i++) res*=base;
    return res;
}

double nrt2(int power) {
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

double nsq2(int32_t n) {
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


void h(int x){
    int c = 0;
    for(int i=1; i <= 100; i++){
        if(i%x == 0){
            printf("%d\n", i);
            c == 1;
        };
    };
    if(!c) puts("Нет таких\n");
}

void p(int x){
    if(x <= 1){
        puts("Neither prime nor composite");
        return;
    }
    for(int i=2; EPSILON <= fabs(sqrt(x)+1.0-(double)i) ; i++){
        if(x % i == 0) {
            puts("composite number");
            return;
        }
    }
    puts("prime number");
}

void s(int x){
    char out[50];
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
        printf("%d:", j);
        printf("%llu", powi(j, 1));
        for(int i = 1; i <= 10; i++) printf(", %llu", powi(j, i));
        puts("");
    }
}

void a(int x) {
    printf("%d\n", (1+x)*x/2);
}

long long int f(int x) {
    if(!x) {
        return 1;
    }
    long long int prod = 1;
    for(int i = 2; i <= x; i++) {
        prod*=i;
    }
    return prod;
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
    double a1 = n*(nrt2(n)-1);
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
    printf("Lim: %.15f\n", sqrec(n, 0.5));
    printf("Seq: %.15f\n", nsq2(n));
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
    printf("Lim: %.15f\n", a1);
    double a2 = -powd(dpi(n), 2)/6;
    for(int k = 2; k <= n; k++) {
        a2+=1/powd(sqrt(k),2)-1.0/k;
    }
    printf("Seq: %.15f\n", a2);
    
}

int main(int argc, char *argv[]){
    char *st;
    int32_t n = (int32_t)strtol(argv[1], &st, 10);
    if(st == NULL){
        char first = argv[2][1];
        switch(first){
            case 'h':
                h(n);
            case 'p':
                p(n);
            case 's':
                s(n);
            case 'e':
                e(n);
            case 'a':
                a(n);
            case 'f':
                printf("%lld", f(n));
            default:
                break;
        }
    } else {
        n = (int32_t)strtol(argv[2], NULL, 10);
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