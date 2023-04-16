#include <sstream>
#include <numbers>

#include "maze.h"

Maze::Maze(const Config& config, std::vector<Point> points){
    this->config = config;
    this->points = points;
    this->boundary = {};
}

Maze::Maze(const Config& config, std::vector<Point> points, std::vector<Point> boundary){
    this->config = config;
    this->points = points;
    this->boundary = boundary;
}

/*
Apply brownian force to every point in vector
*/
void Maze::_brownian(){

    std::normal_distribution<double> normal(0,1.0);
    std::uniform_real_distribution<double> distribution(0.0,std::numbers::pi*2.0);

    double n;   // distance magnitude
    double a;   // angle

    for(int i = 0; i < points.size(); i++){

        if(points[i].frozen){
            continue;
        }

        n = normal(generator);
        a = distribution(generator);

        points[i].dx += this->config.B * n * cos(a);
        points[i].dy += this->config.B * n * sin(a);
    }
}

/*
Apply smoothing force to every point
*/
void Maze::_smoothing(){
    
    Point p0, p2;
    double d0, d2;

    // set previous distance to back-to-front distance
    p0 = points.back();
    d0 = points.front().distance(p0);

    // update points except last point
    for(int i = 0; i < points.size()-1; i++){

        if(points[i].frozen){
            continue;
        }

        p2 = points[i+1];
        d2 = points[i].distance(p2);

        points[i].dx += this->config.F * ((p0.x*d2 + p2.x*d0)/(d0+d2) - points[i].x);
        points[i].dy += this->config.F * ((p0.y*d2 + p2.y*d0)/(d0+d2) - points[i].y);

        // use "next" distance as the next "previous" distance
        //   ex: distance between p0 and p1 is used for updating p0 and p1
        p0 = points[i];
        d0 = d2;
    }

    // update the final point
    p2 = points.front();

    if(points.back().frozen){
        return;
    }

    d2 = points.back().distance(p2);

    points.back().dx += this->config.F * ((p0.x*d2 + p2.x*d0)/(d0+d2) - points.back().x);
    points.back().dy += this->config.F * ((p0.y*d2 + p2.y*d0)/(d0+d2) - points.back().y);
}


/*
Calculate proximity force on one point
*/
void Maze::_proximityForce(Point& point, const Point& p0, const Point& p1, int& counter){

    Point close = closest(p0, p1, point);

    if ((fabs(close.x - point.x) + fabs(close.y - point.y) > this->config.R1)) {
        return;
    }
        
    // distance from closest point on line to point w/o sqrt
    double dis = close.sq_distance(point);

    // check if the squared distance is within the valid radius
    if(dis < this->config.R12){
            
        // calculate "force" using lennard jones potential
        // - note the distance used here is squared to avoid sqrt ~ R0 is squared (R02) and the exponents of LJP are halved.
        double force = pow((this->config.R02 / dis),6) - pow((this->config.R02 / dis),3);

        dis = sqrt(dis);

        // TODO should "dis" be sqrt here?
        point.dx += this->config.A * force * (point.x - close.x) / dis;
        point.dy += this->config.A * force * (point.y - close.y) / dis;
        counter++;
    }
}


/*
Apply proximity force to each point in the vector --> this looks at every other point!
*/
void Maze::_proximity(int skip){
    
    int counter;

    for(int i = 0; i < points.size(); i++){

        if(points[i].frozen){
            continue;
        }

        // reset the counter
        counter = 0;

        // process other points in maze
        for(int j = 0; j < points.size(); j++){

            // skip adjacent indices
            if(abs(i-j) <= skip || abs(i-(j+1)) <= skip || abs(i-j) >= points.size()-skip || abs(i-(j+1)) >= points.size()-skip){
                continue;
            }

            _proximityForce(points[i], points[j], points[(j+1)%points.size()], counter);
        }

        // process the boundaries
        for(int j = 0; j < boundary.size(); j++){
            _proximityForce(points[i], boundary[j], boundary[(j+1)%boundary.size()], counter);
        }
        
        double force = points[i].dx * points[i].dx + points[i].dy * points[i].dy;

        if(force > config.MAX * config.MAX){
            force = sqrt(force);

            points[i].dx = points[i].dx/force * config.MAX;
            points[i].dy = points[i].dy/force * config.MAX;
        }

        // freeze the point if there are enough points close to it
        if(counter > config.freeze){
            points[i].frozen = true;
        }
    }
}


/*
Apply the maze forces to each point
*/
void Maze::update(){

    _proximity();
    _brownian();
    _smoothing();

    int alive = 0;
    
    // update the points from the forces
    for(int i = 0; i < points.size(); i++){        
        if(points[i].frozen){
            continue;
        }
        points[i].update();
        alive++;
    }

    std::cout << "NODES: " << points.size() << " AVAILABLE: " << alive << std::endl;
}


/*
Add points to the linked list of points
*/
void Maze::resample(){
    
    double distance{0};
    Point* p1;
    Point* p2;

    for(int i = 0; i < points.size(); i++){
        if (i == 0) 
        {
            p1 = &points.back();
            p2 = &points.front();
        }
        else
        {
            p1 = &points.at(i - 1);
            p2 = &points.at(i);
        }

        if (p1->frozen && p2->frozen) 
        {
            continue;
        }

        distance = p1->distance(*p2);
            
        // insert new point if distance is > dmax threshold
        if(distance > this->config.dmax){
            points.insert(points.begin() + i, bisect(*p1, *p2));
            i++;    // do not reevaluate this point
        }
        // remove point if the distance between it and the previous point is too low
        else if(distance < this->config.dmin){
            points.erase(points.begin()+i);
            i--;    // reevalute this index (now a new point)
        }
    }
}

/*
Output all of the points in a string in csv format
*/
std::string Maze::output(){
    std::stringstream output;
    output << "x,y\n";
    for(const auto& point : points){
        output << point << "\n";
    }
    return output.str();
}

