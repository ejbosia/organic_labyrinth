
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


void brownian(Point* point, const Config &config);
void smoothing(Point* p0, Point* p1, Point* p2, const Config &config);
void proximity(Point* point, const Config &config);

void pushpull(Point* target, Point* list);

void update(Point* start, const Config &config);

Point* resample(Point* start, const Config &config);

#endif