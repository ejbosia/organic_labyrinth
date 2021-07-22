
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

class Maze{

    private:
        Config config;
        Point* start;

    public:
        Maze(Point* start);
        Maze(const Config config, Point* start);
        void brownian(Point* point, const Config &config);
        void smoothing(Point* p0, Point* p1, Point* p2, const Config &config);
        void proximity(Point* point, const Config &config);
        void update(Point* start, const Config &config);
        void resample(Point* start, const Config &config);

};



Point closest(Point* A, Point* B, Point* C);

void pushpull(Point* target, Point* list);



#endif