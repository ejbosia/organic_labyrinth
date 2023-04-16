#pragma once

#include <ostream>


class Angle{

    private:
        double angle;
        

    public:
        Angle();
        Angle(double angle);

        // mutators and accessors
        double getAngle();
        double degrees();
        void rotateAngle(double angle);
        void setAngle(double angle);
        bool parallel(double angle);

        // trig functions
        double sine();
        double cosine();
        double tangent();

        // operator overloads
        bool operator==(const Angle& other);
        bool operator==(const double value);

        friend std::ostream& operator<<(std::ostream &strm, const Angle &a);

};