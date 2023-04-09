#include <string>
#include <math.h>
#include <cmath>
#include <limits>
#include <numbers>

#include "angle.h"


/*
Create an angle object. This enforces angles between 0-2PI
*/
Angle::Angle(double angle){
    setAngle(angle);
}

/*
Get the angle value of the object
*/
double Angle::getAngle(){
    return angle;
}

double Angle::degrees(){
    return angle * 180 / std::numbers::pi;
}


/*
Rotate the angle, maintain 0-2PI
*/
void Angle::rotateAngle(double angle){
    setAngle(this->angle + angle);
}

/*
Set the angle, maintain 0-2PI
*/
void Angle::setAngle(double angle){
    this->angle = std::fmod(angle,(2* std::numbers::pi));
}

/*
Check if the input angle is parallel to the angle
*/
bool Angle::parallel(double a){
    return 0 == std::fmod(angle-a,(std::numbers::pi));
}

double Angle::sine(){
    return sin(angle);
}

double Angle::cosine(){
    return cos(angle);
}

double Angle::tangent(){
    return tan(angle);
}

/*
OPERATER OVERLOADS
*/

/*
Check if angles are equivalent
*/
bool Angle::operator==(const Angle& other){
    return fabs(angle - other.angle) < 2*std::numeric_limits<double>::epsilon();
}

/*
Check if angles are equivalent
*/
bool Angle::operator==(const double value){
    return fabs(angle - value) < 2*std::numeric_limits<double>::epsilon();
}


/*
Output the angle
*/
std::ostream& operator<<(std::ostream &strm, const Angle &a) {
    return strm << a.angle;
}


