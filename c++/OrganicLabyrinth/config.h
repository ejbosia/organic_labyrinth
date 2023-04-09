
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
    double R02, R12;    // squared values of R0 and R1
    double D;

    double MAX;

    int freeze;

    Config();

    Config(double B, double F,double A, double k0, double k1, double kmin, double kmax,double D, double MAX, int freeze = INT32_MAX);

};

#endif