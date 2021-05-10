#include "maze.h"

/*
Apply brownian force to dx,dy 
*/
void brownian(Point* point, const Config &config){


    double n = normal(generator);       // distance magnitude
    double a = distribution(generator); // angle

    point->dx += n * cos(a);
    point->dy += n * sin(a);

}

/*
Apply smoothing force to dx,dy 
*/
void smoothing(Point* p0, Point* p1, Point* p2, const Config &config){
    

    double d0 = p0->distance(*p1);
    double d2 = p2->distance(*p1);

    p1->dx += (p0->x*d2 + p2->x*d0)/(d0+d2) - p1->x;
    p1->dy += (p0->y*d2 + p2->y*d0)/(d0+d2) - p1->y;

}

/*
Apply proximity force to dx,dy --> this looks at every point
*/
void proximity(Point* point, const Config &config){
    
    // skip the first point
    Point* current = point->next;

    double dx = 0;
    double dy = 0;

    double dis;
    double force;

    // loop until the point before the last is found
    while(current->next != point){

        // only process if the distance "could" be within the range
        if((current->x - point->x < config.R1) && (current->x - point->x < config.R1)){

            dis = current->distance(*point);

            if(dis < config.R1){
                
                force = pow((config.R0 / dis),12) - pow((config.R0 / dis),6);

                dx += force * (point->x - current->x) / dis;
                dy += force * (point->y - current->y) / dis;
            }
        }
    }


    double mag = sqrt(dx*dx + dy*dy);

    // clamp the force to the max
    if(mag > config.MAX){
        dx = dx/mag * config.MAX;
        dy = dy/mag * config.MAX;
    }

    point->dx += dx;
    point->dy += dy;

}


/*
Apply the maze forces to each point
*/
void update(Point* start, const Config &config){

    Point* current = start;
    Point* previous = start;

    double distance;

    do{

        previous = current;

        current = current->next;

        brownian(current, config);
        smoothing(previous, current, current->next, config);
        proximity(current, config);
    
    }while(current != start);


    // update the points from the forces
    do{
        
        current->x = current->x + current->dx;
        current->y = current->y + current->dy;

        current = current->next;
    
    }while(current != start);

}


/*
Add points to the linked list of points
*/
void resample(Point* start, const Config &config){
    
    Point* current = start;
    Point* previous = start;

    double distance;

    double x, y;

    do{

        previous = current;

        current = current->next;

        distance = current->distance(*(current->next));
        
        if(distance > config.dmax){

            x = 0.5*(current->next->x - current->x)+current->x;
            y = 0.5*(current->next->x - current->x)+current->y;

            current->next = new Point(x, y, current->next);
        }

        if(distance < config.dmin){
            previous->next = current->next;
        }

    }while(current != start);
}