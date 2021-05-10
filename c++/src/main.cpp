
#include <iostream>
#include <chrono>
#include <fstream>
#include <vector>

#include "point.h"
#include "angle.h"
#include "config.h"
#include "maze.h"

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

            current->next = new Point(x,y);
            current = current->next;

            x += xDir[i];
            y += yDir[i];
        }

    }

    current->next = start->next;
    start = start->next;
    // test the loop

    Config config = Config(0.05, 0.1, 0.006, 1.0, 5.0, 0.2, 0.6, 1.0, 20.0*1.0);

    for(int i = 0; i < 5; i++){
        resample(start, config);
        update(start, config);
    }


    std::ofstream myfile;
    myfile.open ("example.txt");
    

    current = start;
    myfile << "x = [" << std::endl;
    do{
        myfile << *current << "," << std::endl;
        current = current->next;
    }while(current != start);
    myfile << "]" << std::endl;

    myfile.close();
    
    return 0;
}

