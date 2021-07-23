
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
        std::normal_distribution<double> normal;
        std::uniform_real_distribution<double> distribution;

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

        string output();
};

#endif