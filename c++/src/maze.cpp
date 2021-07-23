#include "maze.h"

Maze::Maze(){
    Maze(Config(), std::vector<Point>{ Point(0,0), Point(0,1), Point(1,1), Point(1,0) });
}

Maze::Maze(const Config& config, std::vector<Point> points){
    this->config = config;
    this->points = points;
    this->boundary = {};

    this->generator();
}

Maze::Maze(const Config& config, std::vector<Point> points, std::vector<Point> boundary){
    this->config = config;
    this->points = points;
    this->boundary = boundary;

    this->generator();
}


/*
Apply brownian force to every point in vector
*/
void Maze::brownian(){

    std::normal_distribution<double> normal(0,1.0);
    std::uniform_real_distribution<double> distribution(0.0,M_PI*2.0);

    double n;   // distance magnitude
    double a;   // angle

    for(int i = 0; i < points.size(); i++){

        n = normal(generator);
        a = distribution(generator);

        points[i].dx += this->config.B * n * cos(a);
        points[i].dy += this->config.B * n * sin(a);
    }
}

/*
Apply smoothing force to every point
*/
void Maze::smoothing(){
    
    Point p0, p2;
    double d0, d2;

    // set previous distance to back-to-front distance
    p0 = points.back();
    d0 = points[0].distance(p0);

    // update points except last point
    for(int i = 0; i < points.size()-1; i++){

        p2 = points[i+1];
        d2 = points[i].distance(p2);

        std::cout << d0 << " " << d2 <<  " " << p0 << " " << points[i] << " " << points[i+1] << std::endl;

        points[i].dx += this->config.F * ((p0.x*d2 + p2.x*d0)/(d0+d2) - points[i].x);
        points[i].dy += this->config.F * ((p0.y*d2 + p2.y*d0)/(d0+d2) - points[i].y);

        // use "next" distance as the next "previous" distance
        //   ex: distance between p0 and p1 is used for updating p0 and p1
        p0 = points[i];
        d0 = d2;
    }

    // update the final point
    p2 = points.front();
    d2 = points.back().distance(p2);

    points.back().dx += this->config.F * ((p0.x*d2 + p2.x*d0)/(d0+d2) - points.back().x);
    points.back().dy += this->config.F * ((p0.y*d2 + p2.y*d0)/(d0+d2) - points.back().y);
}


/*
Calculate proximity force on one point
*/
void Maze::proximityForce(Point& point, const Point& p0, const Point& p1, int& counter){
    // get the closest point on the line
    Point close = closest(p0, p1, point);

    // check manhatten distance
    if((fabs(close.x - point.x)+fabs(close.y - point.y) < this->config.R1)){
        
        // distance from closest point on line to point w/o sqrt
        double dis = close.sq_distance(point);

        // check if the squared distance is within the valid radius
        if(dis < this->config.R12){
            
            // calculate "force" using lennard jones potential
            // - note the distance used here is squared to avoid sqrt ~ R0 is squared (R02) and the exponents of LJP are halved.
            double force = pow((this->config.R02 / dis),6) - pow((this->config.R02 / dis),3);

            dis = sqrt(dis);

            // TODO should "dis" be sqrt here?
            point.dx += this->config.A * force * (point.dx - close.x) / dis;
            point.dy += this->config.A * force * (point.dy - close.y) / dis;
            counter++;
        }
    }
}


/*
Apply proximity force to each point in the vector --> this looks at every other point!
*/
void Maze::proximity(int skip){
    
    double dis, force;
    int counter;

    for(int i = 0; i < points.size(); i++){

        // reset the counter
        counter = 0;

        // process other points in maze
        for(int j = 0; j < points.size(); j++){

            // skip adjacent indices
            if(abs(i-j) <= skip || abs(i-(j+1)) <= skip || abs(i-j) >= points.size()-skip || abs(i-(j+1)) >= points.size()-skip){
                continue;
            }

            proximityForce(points[i], points[j], points[(j+1)%points.size()], counter);
        }

        // process the boundaries
        for(int j = 0; j < boundary.size(); j++){
            proximityForce(points[i], boundary[j], boundary[(j+1)%boundary.size()], counter);
        }

        // freeze the point if there are enough points close to it
        if(counter > config.freeze){
            points[i].available = false;
        }
    }
}


/*
Apply the maze forces to each point
*/
void Maze::update(){

    brownian();
    std::cout << std::to_string(points[0].dx) << " " << std::to_string(points[0].dy) << std::endl;
    smoothing();
    std::cout << std::to_string(points[0].dx) << " " << std::to_string(points[0].dy) << std::endl;
    proximity();
    std::cout << std::to_string(points[0].dx) << " " << std::to_string(points[0].dy) << std::endl;
    std::cout << std::endl;

    int alive = 0;
    
    // update the points from the forces
    for(int i = 0; i < points.size(); i++){        
        if(points[i].available){
            points[i].update();
            alive++;
        }
    }

    std::cout << "NODES: " << points.size() << " AVAILABLE: " << alive << std::endl;
}


/*
Add points to the linked list of points
*/
void Maze::resample(){
    
    double distance, x, y;

    int p = points.size()-1;

    for(int i = 0; i < points.size(); i++){

        if(points[i].available && points[p].available){
            
            distance = points[i].distance(points[p]);
            
            // insert new point if distance is > dmax threshold
            if(distance > this->config.dmax){
                points.insert(points.begin()+i, bisect(points[i], points[p]));
                i++;    // do not reevaluate this point
            }
            // remove point if the distance between it and the previous point is too low
            else if(distance < this->config.dmin){
                points.erase(points.begin()+i);
                i--;    // reevalute this index (now a new point)
            }
        }
    }
}

/*
Output all of the points in a string
*/
std::string Maze::output(){

    std::string output = "";

    for(int i = 0; i < points.size(); i++){
        output += std::to_string(points[i].x) + "," + std::to_string(points[i].y) + "\n";
    }

    return output;

}

