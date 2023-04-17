
#include <iostream>
#include <chrono>
#include <fstream>
#include <vector>

#include "point.h"
#include "angle.h"
#include "config.h"
#include "maze.h"

#include <stdio.h>

#define ITERATIONS 501

#define SAVE true
#define SAVE_ITERATION 4

#define ERR_NONE 0
#define ERR_BOUNDARY_FILE_NF     -1
#define ERR_BOUNDARY_FILE_FORMAT -2


class Timer {
public:
    Timer() : _start_time(std::chrono::high_resolution_clock::now()) {}
    ~Timer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - _start_time);
        std::cout << "Elapsed time: " << elapsed_time.count() << "ms\n";
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _start_time;
};


int read_boundary_file(const std::string& filepath, std::vector<Point>& points) {
    std::ifstream file(filepath);
    bool header = true;
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (header) {
                header = false;
                continue;
            }

            std::stringstream ss(line);
            std::string value;
            std::vector<int> row;
            while (std::getline(ss, value, ',')) {
                row.push_back(std::stoi(value));
            }
            // index 0 is the index... not valid data
            // TODO EB: fix the python script to return the correct data
            points.push_back(Point(row.at(1), row.at(2)));
        }
        file.close();
    }
    else {
        return ERR_BOUNDARY_FILE_NF;
    }
    return ERR_NONE;
}


int main(int argc, char** argv){

    std::cout.precision(std::numeric_limits<double>::digits10 + 2);

    std::vector<Point> points{
        Point(0,0),
        Point(0,1),
        Point(1,1),
        Point(1,0)
    };

    std::vector<Point> boundary;    

    if (read_boundary_file("C:\\Users\\Evan\\Documents\\GitHub\\organic_labyrinth\\c++\\x64\\Debug\\boundary.csv", boundary) != ERR_NONE) {
        return -1;
    }

    // brownian, smoothing, push pull
    Config config = Config();
    config.freeze = 160;

    std::ofstream myfile;

    Maze maze(config, points);

    {
        Timer timer{};
        for (int i = 0; i < ITERATIONS; i++) {
            maze.resample();
            maze.update();

            std::cout << " ITERATIONS: " << i << std::endl;

            if (SAVE && (i % SAVE_ITERATION == 0)) {

                myfile.open("results/" + std::format("iteration_{:04}", i) + ".csv");
                myfile << maze.output();
                myfile.close();
            }
        }
    }

    std::cin.get();
    return 0;
}

