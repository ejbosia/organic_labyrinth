
#include <iostream>
#include <chrono>
#include <fstream>
#include <vector>

#include "point.h"
#include "angle.h"
#include "config.h"
#include "maze.h"

#include <stdio.h>

#define BROWNIAN 0.05
#define SMOOTHING 0.15
#define PUSHPULL 0.004
#define K0 1.0
#define K1 5.0
#define KMIN 0.2
#define KMAX 0.6
#define D 1.0
#define MAX 20.0

#define ITERATIONS 5000

#define SAVE true
#define SAVE_ITERATION 10

int main(int argc, char** argv){

    std::cout.precision(std::numeric_limits<double>::digits10 + 2);

    std::vector<Point> points{
        Point(0,0),
        Point(0,1),
        Point(1,1),
        Point(1,0)
    };

    std::vector<Point> boundary;    

    // brownian, smoothing, push pull
    Config config = Config(
        BROWNIAN, 
        SMOOTHING,
        PUSHPULL, 
        K0,
        K1,
        KMIN,
        KMAX,
        D, 
        MAX
    );

    std::ofstream myfile;

    Maze maze(config, points);

    for(int i = 0; i < ITERATIONS; i++){

        maze.resample());
        maze.update();

        std::cout << " ITERATIONS: " <<  i << std::endl;

        if(SAVE && (i % SAVE_ITERATION == 0)){
            myfile.open ("results/" + std::to_string(i) + ".csv");

            for(int i = 0; i < maze.)

            myfile.close();  
        }
    }
    
    return 0;
}

