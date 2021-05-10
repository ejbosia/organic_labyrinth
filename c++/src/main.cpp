
#include <iostream>
#include <chrono>
#include <fstream>
#include <vector>

#include "point.h"
#include "angle.h"
#include "config.h"

#include <stdio.h>
// #include <opencv2/core.hpp>
// #include <opencv2/imgcodecs.hpp>
// #include <opencv2/highgui.hpp>
// #include <opencv2/opencv.hpp>


#define X_MAX 100.0
#define Y_MAX 150.0

#define X_OFFSET 20.0
#define Y_OFFSET 20.0

#define LINETHICKNESS 0.5
#define ANGLE (M_PI/6)

int main(int argc, char** argv){

    std::cout.precision(std::numeric_limits<double>::digits10 + 2);

    // auto start = chrono::high_resolution_clock::now();

    // create a starting linked list of points

    Point* start = new Point(0.0,0.0);
    Point* current = start;

    int xDir[4] = {1,0,-1,0};
    int yDir[4] = {0,1,0,-1};

    double x, y;

    for(int i = 0; i < 4; i++){


        for(int j = 0; j < 5; j++){

            x += xDir[i];
            y += yDir[i];

            current->next = new Point(x,y);
            current = current->next;

            std::cout << "\t" << *current << std::endl;
        }

    }

    current->next = start;
    current = start;
    // test the loop

    do{
        
        std::cout << *current << "\t" << *current->next << "\t" << *start << std::endl;

        current = current->next;
    
    }while(current != start);




    // Config config;

    // // run 100 steps on the maze generation
    // for(int i = 0; i < 100;  i++){

    //     // resample

    //     // update


    // }
    
    // output the result as a python list
    
    return 0;
}
