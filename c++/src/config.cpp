#include "config.h"

Config::Config(double B, double F,double A, double k0, double k1, double kmin, double kmax,double D, double MAX){


    this->B = B;
    this->F = F;
    this->A = A;

    this->k0 = k0;
    this->k1 = k1;

    this->kmin = kmin;
    this->kmax = kmax;

    this->D = D;

    this->MAX = MAX;

    // relative values
    this->R0 = k0 * D;
    this->R1 = k1 * D;


    this->dmin = kmin * D;
    this->dmax = kmax * D;

}
