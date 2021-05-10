
#ifndef MAZE_H
#define MAZE_H

#include <iostream>
#include <string>
#include <math.h>
#include <cmath>
#include <limits>
#include <random>

#include "point.h"
#include "config.h"

std::default_random_engine generator;
std::normal_distribution<double> normal(0,1.0);
std::uniform_real_distribution<double> distribution(0.0,M_PI*2.0);


void brownian(Point* point, const Config &config);
void smoothing(Point* p0, Point* p1, Point* p2, const Config &config);
void proximity(Point* point, const Config &config);

void pushpull(Point* target, Point* list);

void update(Point* start, const Config &config);

void resample(Point* start, const Config &config);

#endif