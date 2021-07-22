#include "point.h"

Point::Point(){
    
}

Point::Point(double x, double y){
    this->x = x;
    this->y = y;
    this->next = NULL;

    available = true;
}

Point::Point(double x, double y, Point* next){
    this->x = x;
    this->y = y;
    this->next = next;

    available = true;
}


double Point::sq_distance(const Point &other){
    double dx = x - other.x;
    double dy = y - other.y;

    return pow(dx,2.0) + pow(dy,2.0);
}


/*
Return the distance to another point
*/
double Point::distance(const Point &other){

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



bool Point::operator==(const Point &p){

    bool x_equiv = fabs(x-p.x) < 2*std::numeric_limits<double>::epsilon();
    bool y_equiv = fabs(y-p.y) < 2*std::numeric_limits<double>::epsilon();

    return x_equiv && y_equiv;
}

Point Point::operator-(const Point &p){

    Point temp(x-p.x, y-p.y);
    return temp;
}

std::ostream& operator<<(std::ostream &strm, const Point &p){
    return strm << "(" << round(p.x*1000.0)/1000.0 << "," <<  round(p.y*1000.0)/1000.0  << ")";
}