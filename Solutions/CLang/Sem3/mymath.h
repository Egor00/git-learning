#include <math.h>



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


unsigned long long powi(int base, int power){
    /*Вычисляет целую степень целого числа(кроме 0 в 0 степени)*/
    if(base == 1) return 1;
    unsigned long long res = 1;
    for(int i = 0; i < power; i++) res*=base;
    return res;
}

double powd(double base, int power){
    /*Вычисляет целую степень дробного числа(кроме 0 в 0)*/
    if(fabs(base - 1.0) <= eps()) return 1.0;
    double res = 1.0;
    for(int i = 0; i < power; i++) res*=base;
    return res;
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