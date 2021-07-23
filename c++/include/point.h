
#ifndef POINT_H
#define POINT_H

#include <iostream>
#include <string>
#include <math.h>

#include "angle.h"

struct Point{

    double x;
    double y;
    
    bool available;

    // store the adjustments
    double dx,dy;
    
    Point();
    Point(double x, double y);
    
    double sq_distance(const Point& other);
    double distance(const Point& other);
    Angle angle(Point& other);

    double xRotation(Angle& angle);
    double yRotation(Angle& angle);

    void translate(double distance, Angle& angle);
    void translate(double dx, double dy);
    
    // update position using stored dx,dy values
    void update();

    void rotate(Angle& angle);
    void rotate(Point& point, Angle& angle);

    bool operator==(const Point &p);
    Point operator-(const Point &p);

    friend std::ostream& operator<<(std::ostream &strm, const Point &p);
};

Point bisect(const Point& A, const Point& B);
Point closest(const Point& A, const Point& B, const Point& C);

#endif