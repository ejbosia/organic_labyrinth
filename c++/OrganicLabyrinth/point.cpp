#include "point.h"


Point::Point() : x(0.), y(0.), frozen(false), dx(0.), dy(0.){}
Point::Point(double x, double y) : x(x), y(y), frozen(false), dx(0.), dy(0.) {}


double Point::sq_distance(const Point &other) const{
    double dx = x - other.x;
    double dy = y - other.y;

    return pow(dx,2.0) + pow(dy,2.0);
}

/*
Return the distance to another point
*/
double Point::distance(const Point &other) const{
    return sqrt(sq_distance(other));
}

Angle Point::angle(Point& other){
    double dx = other.x - x;
    double dy = other.y - y;

    return Angle(atan2(dy, dx));
}


double Point::xRotation(Angle& angle){

    Angle a(-angle.getAngle());

    double c = a.cosine();
    double s = a.sine();
    return x * c - y * s;
}


double Point::yRotation(Angle& angle){

    Angle a(-angle.getAngle());

    double c = a.cosine();
    double s = a.sine();

    return x * s + y * c;
}

/*
Rotate the point about the origin
*/
void Point::rotate(Angle& angle){
    
    // get the coordinates of this point
    double c = angle.cosine();
    double s = angle.sine();


    double x_temp = x * c - y * s;
    y = x * s + y * c;
    x = x_temp;
}

/*
Rotate the point about another point
*/
void Point::rotate(Point& point, Angle& angle){

    // translate this point so the input point is (0,0)
    x -= point.x;
    y -= point.y;

    rotate(angle);
    // translate this point back so the input point is itself
    x += point.x;
    y += point.y;
}

/*
Translate the point a dx and dy amount
*/
void Point::translate(double dx, double dy){
    x += dx;
    y += dy;
}

/*
Translate the point a distance in the direction of the input angle
*/
void Point::translate(double distance, Angle& angle){
    double s = angle.sine();
    double c = angle.cosine();

    translate(distance * c, distance * s);
}

/*
Translate the point using the stored dx, dy values
*/
void Point::update(){
    
    x += dx;
    y += dy;
    
    dx = dy = 0;
}

bool Point::operator==(const Point &p){

    bool x_equiv = fabs(x-p.x) < 2*std::numeric_limits<double>::epsilon();
    bool y_equiv = fabs(y-p.y) < 2*std::numeric_limits<double>::epsilon();

    return x_equiv && y_equiv;
}

 Point Point::operator+(const Point &p){
     return Point(x+p.x, y+p.y);
 }

 Point Point::operator-(const Point &p){
     return Point(x-p.x, y-p.y);
 }

std::ostream& operator<<(std::ostream &strm, const Point &p){
    return strm << round(p.x*1000.0)/1000.0 << "," <<  round(p.y*1000.0)/1000.0 << "," << p.frozen;
}

/*

Non-class point functions

*/

/*
Return the midpoint of two points
*/
Point bisect(const Point& A, const Point& B){
    return Point(0.5*(B.x - A.x)+A.x,
                 0.5*(B.y - A.y)+A.y);
}



/*
Closest point on finite line "AB" to point "C"
*/
Point closest(const Point& A, const Point& B, const Point& C){

    double x1 = B.x - A.x;
    double y1 = B.y - A.y;
    
    double x2 = C.x - A.x;
    double y2 = C.y - A.y;

    double dot = x1*x2 + y1*y2;

    double length = x1*x1 + y1*y1;

    if(dot < 0){
        return A;
    }
    
    if(dot > (x1*x1+y1*y1)){
        return B;
    }

    return Point(A.x + dot * x1 / length,
                 A.y + dot * y1 / length);
}