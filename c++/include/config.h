
#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <string>
#include <math.h>
#include <cmath>
#include <limits>

struct Config{

    double B, F, A;

    double k0, k1;

    double kmin, kmax;

    double dmax, dmin;
    double R0, R1;
    double D;

    double MAX;

    Config(B,F,A,k0,k1,kmin,kmax,D, MAX);

}

#endif