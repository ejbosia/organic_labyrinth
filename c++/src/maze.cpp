#include "maze.h"

Maze::Maze(Point* start){
    Maze(Config(), start);
}


Maze::Maze(const Config config, std::vector<Point> points){
    this->config = config;
    this->points = points;
    this->boundary = {};
    
    this->normal(0,1.0);
    this->distribution(0.0,M_PI*2.0);
}

Maze::Maze(const Config config, std::vector<Point> points, std::vector<Point> boundary){
    this->config = config;
    this->points = points;
    this->boundary = boundary;
    
    this->normal(0,1.0);
    this->distribution(0.0,M_PI*2.0);
}


/*
Apply brownian force to every point in vector
*/
void Maze::brownian(){

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
    if((fabs(close.x - points[i].x)+fabs(close.y - points[i].y) < this->config.R1)){
        
        // distance from closest point on line to point w/o sqrt
        double dis = close.sq_distance(point);

        // check if the squared distance is within the valid radius
        if(dis < this->config.R12){
            
            // calculate "force" using lennard jones potential
            // - note the distance used here is squared to avoid sqrt ~ R0 is squared (R02) and the exponents of LJP are halved.
            force = pow((this->config.R02 / dis),6) - pow((this->config.R02 / dis),3);

            dis = sqrt(dis);

            // TODO should "dis" be sqrt here?
            points[i].dx += this->config.A * force * (points[i].dx - close.x) / dis;
            points[i].dy += this->config.A * force * (points[i].dy - close.y) / dis;
            counter++;
        }
    }
}


/*
Apply proximity force to each point in the vector --> this looks at every other point!
*/
void Maze::proximity(int skip = 1){
    
    double dis, force;
    int counter;

    for(int i = 0; i < points.size(); i++){

        counter = 0;

        // loop through non neighbor points
        for(int j = 0; j < points.size(); j++){
            
            // skip adjacent indices
            if(abs(i-j) <= skip || abs(i-j) >= points.size()-skip){
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



    // skip the first neighbor point
    Point* current = point.next;



    int counter = 0;

    // loop until the point before the start point is found
    while(current->next != point){

        // find the closet point on the line
        close = closest(current, current->next, point);
        
        // only process if the distance "could" be within the range
        if((fabs(close.x - point->x) < this->config.R1) && (fabs(close.y - point->y) < this->config.R1)){

            // distance from closest point on line to point
            dis = close.sq_distance(*point);

            if(dis < this->config.R12){
                
                force = pow((this->config.R02 / dis),6) - pow((this->config.R02 / dis),3);

                point->dx = point->dx + this->config.A * force * (point->x - close.x) / dis;
                point->dy = point->dy + this->config.A * force * (point->y - close.y) / dis;
                counter++;
            }
        }

        current = current->next;
    }


    for(int i = 0; i < boundary.size(); i++){
        close = closest(&boundary[i], &boundary[(i+1)%boundary.size()], point);
        
        // only process if the distance "could" be within the range
        if((fabs(close.x - point->x) < this->config.R1) && (fabs(close.y - point->y) < this->config.R1)){

            // distance from closest point on line to point
            dis = close.sq_distance(*point);

            if(dis < this->config.R12){
                
                force = pow((this->config.R02 / dis),6) - pow((this->config.R02 / dis),3);

                point->dx = point->dx + this->config.A * force * (point->x - close.x) / dis;
                point->dy = point->dy + this->config.A * force * (point->y - close.y) / dis;
                counter++;
            }
        }

        current = current->next;
    }

    // if the counter is above a value, "freeze" the point
    if(counter > 300){
        point->available = false;
    }
}


/*
Apply the maze forces to each point
*/
void Maze::update(){

    brownian();
    smoothing();
    proximity();

    // apply browian and smoothing force
    // do{

    //     previous = current;

    //     current = current->next;

    //     if(current->available){
    //         proximity(current);

    //         mag = (current->dx * current->dx + current->dy * current->dy);
    //         if(mag > config.MAX * config.MAX){
                
    //             mag = sqrt(mag);
                
    //             current->dx = current->dx / mag * config.MAX;
    //             current->dy = current->dy / mag * config.MAX;
    //         }

    //         brownian(current);

    //         smoothing(previous, current, current->next);
    //     }
    // }while(current != start);



    int counter = 0;
    int alive = 0;
    // update the points from the forces
    do{

        if(current->available){
            current->x = current->x + current->dx;
            current->y = current->y + current->dy;

            current->dx = 0;
            current->dy = 0;

            alive++;
        }
        current = current->next;

        counter++;
    
    }while(current != start);

    std::cout << "NODES: " << counter << " AVAILABLE: " << alive << std::endl;

}


/*
Add points to the linked list of points
*/
Point* resample(Point* start, const Config &config){
    
    Point* current = start;
    Point* previous = start;

    double distance, x, y;

    do{
        previous = current;
        current = current->next;

        if(current->available && current->next->available){

            distance = current->distance(*(current->next));
            
            if(distance > this->config.dmax){


                x = 0.5*(current->next->x - current->x)+current->x;
                y = 0.5*(current->next->y - current->y)+current->y;

                current->next = new Point(x, y, current->next);
            }

            if(distance < this->config.dmin){
                
                Point* temp = previous->next;
                previous->next = current->next;
                delete temp;

                if(current == start){
                                    
                    return previous;
                }

                current = previous;
            }
        }

    }while(current != start);

    return start;
}


/*
Closest point on line AB to point C
*/
Point closest(Point* A, Point* B, Point* C){

    double x1 = B->x - A->x;
    double y1 = B->y - A->y;
    
    double x2 = C->x - A->x;
    double y2 = C->y - A->y;

    double dot = x1*x2 + y1*y2;

    double length = x1*x1 + y1*y1;

    if(dot < 0){
        return *A;
    }
    
    if(dot > (x1*x1+y1*y1)){
        return *B;
    }

    return Point(
        A->x + dot * x1 / length,
        A->y + dot * y1 / length
    );
}