#include "config.h"

Config::Config(){

    this->B = 0.05;
    this->F = 0.15;
    this->A = 0.006;

    this->k0 = 1.0;
    this->k1 = 5.0;

    this->kmin = 0.2;
    this->kmax = 0.6;

    this->D = 1.0;

    this->MAX = 20.0 * A;

    // relative values
    this->R0 = k0 * D;
    this->R1 = k1 * D;

    this->R02 = R0 * R0;
    this->R12 = R1 * R1;

    this->dmin = kmin * D;
    this->dmax = kmax * D;

    this->freeze = 300;
}




Config::Config(double B, double F,double A, double k0, double k1, double kmin, double kmax,double D, double MAX, int freeze){


    this->B = B;
    this->F = F;
    this->A = A;

    this->k0 = k0;
    this->k1 = k1;

    this->kmin = kmin;
    this->kmax = kmax;

    this->D = D;

    this->MAX = MAX * A;

    // relative values
    this->R0 = k0 * D;
    this->R1 = k1 * D;

    this->R02 = R0 * R0;
    this->R12 = R1 * R1;

    this->dmin = kmin * D;
    this->dmax = kmax * D;

    this->freeze = freeze;
}
