
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
        std::vector<Point> points;
        std::vector<Point> boundary;
        std::default_random_engine generator;

        void brownian();
        void smoothing();
        void proximity(int skip=1);
        void proximityForce(Point& point, const Point& p0, const Point& p1, int& counter);

    public:
        Maze();
        Maze(const Config& config, std::vector<Point> points);
        Maze(const Config& config, std::vector<Point> points, std::vector<Point> boundary);

        void update();
        void resample();

        std::string output();
};

#endif