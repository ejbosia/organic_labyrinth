#include "maze.h"


Maze::Maze(Point* start){
    Maze(Config(), start);
}


Maze::Maze(const Config config, Point& start){
    this->config = config;
    this->start = start;
    std::default_random_engine generator;
    std::normal_distribution<double> normal;
    std::uniform_real_distribution<double> distribution;
}


/*
Apply brownian force to dx,dy 
*/
void Maze::brownian(Point* point){

    double n = normal(generator);       // distance magnitude
    double a = distribution(generator); // angle

    point->dx += this->config.B * n * cos(a);
    point->dy += this->config.B * n * sin(a);

}

/*
Apply smoothing force to dx,dy 
*/
void Maze::smoothing(Point* p0, Point* p1, Point* p2){
    
    double d0 = p0->distance(*p1);
    double d2 = p2->distance(*p1);

    p1->dx += config.F * ((p0->x*d2 + p2->x*d0)/(d0+d2) - p1->x);
    p1->dy += config.F * ((p0->y*d2 + p2->y*d0)/(d0+d2) - p1->y);

}


/*
Apply proximity force to dx,dy --> this looks at every point
*/
void Maze::proximity(Point* point){
    
    // skip the first neighbor point
    Point* current = point->next;

    double dis;
    double force;

    Point close;

    int counter = 0;

    // loop until the point before the start point is found
    while(current->next != point){

        // find the closet point on the line
        close = closest(current, current->next, point);
        
        // only process if the distance "could" be within the range
        if((fabs(close.x - point->x) < config.R1) && (fabs(close.y - point->y) < config.R1)){

            // distance from closest point on line to point
            dis = close.sq_distance(*point);

            if(dis < config.R12){
                
                force = pow((config.R02 / dis),6) - pow((config.R02 / dis),3);

                point->dx = point->dx + config.A * force * (point->x - close.x) / dis;
                point->dy = point->dy + config.A * force * (point->y - close.y) / dis;
                counter++;
            }
        }

        current = current->next;
    }

    if(counter > 300){
        point->available = false;
    }
}


/*
Apply the maze forces to each point
*/
void Maze::update(Point* start, const Config &config){

    Point* current = start;
    Point* previous = start;

    double distance;
    double mag;

    std::default_random_engine generator;
    std::normal_distribution<double> normal(0,1.0);
    std::uniform_real_distribution<double> distribution(0.0,M_PI*2.0);

    // apply browian and smoothing force
    do{

        previous = current;

        current = current->next;

        if(current->available){
            proximity(current, config);

            mag = (current->dx * current->dx + current->dy * current->dy);
            if(mag > config.MAX * config.MAX){
                
                mag = sqrt(mag);
                
                current->dx = current->dx / mag * config.MAX;
                current->dy = current->dy / mag * config.MAX;
            }

            brownian(current, config, generator, normal, distribution);

            smoothing(previous, current, current->next, config);
        }
    }while(current != start);



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
            
            if(distance > config.dmax){


                x = 0.5*(current->next->x - current->x)+current->x;
                y = 0.5*(current->next->y - current->y)+current->y;

                current->next = new Point(x, y, current->next);
            }

            if(distance < config.dmin){

                previous->next = current->next;

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