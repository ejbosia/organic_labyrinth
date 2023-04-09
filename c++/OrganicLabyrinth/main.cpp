
#include <iostream>
#include <chrono>
#include <fstream>
#include <vector>

#include "point.h"
#include "angle.h"
#include "config.h"
#include "maze.h"

#include <stdio.h>

#define ITERATIONS 251

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
    Config config = Config();

    std::ofstream myfile;

    Maze maze(config, points);

    for(int i = 0; i < ITERATIONS; i++){
        maze.resample();
        maze.update();

        std::cout << " ITERATIONS: " <<  i << std::endl;

        if(SAVE && (i % SAVE_ITERATION == 0)){
            myfile.open ("results/" + std::to_string(i) + ".csv");

            myfile << maze.output();

            myfile.close();  
        }
    }
    
    return 0;
}

