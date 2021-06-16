
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

    // auto start = chrono::high_resolution_clock::now();

    Point* start = new Point(0.0,0.0);
    Point* current = start;

    std::vector<Point> boundary;

    int xDir[4] = {1,0,-1,0};
    int yDir[4] = {0,1,0,-1};

    double x, y;

    for(int i = 0; i < 4; i++){


        for(int j = 0; j < 5; j++){

            current->next = new Point(x,y);
            current = current->next;

            x += xDir[i];
            y += yDir[i];
        }

    }

    current->next = start->next;
    start = start->next;
    // test the loop
    

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



    for(int i = 0; i < ITERATIONS; i++){

        start = resample(start, config);
        update(start, config);
        std::cout << " ITERATIONS: " <<  i << std::endl;

        if(SAVE && (i % SAVE_ITERATION == 0)){
            myfile.open ("results/" + std::to_string(i) + ".csv");

            current = start;
            do{
                myfile << current->x << "," << current->y << std::endl;
                current = current->next;
            }while(current != start);

            myfile.close();  
        }
    }
    
    return 0;
}

