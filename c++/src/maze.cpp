#include "maze.h"

/*
Apply brownian force to dx,dy 
*/
void brownian(Point* point, const Config &config,std::default_random_engine generator, std::normal_distribution<double> normal,
    std::uniform_real_distribution<double> distribution){


    double n = normal(generator);       // distance magnitude
    double a = distribution(generator); // angle

    point->dx += config.B * n * cos(a);
    point->dy += config.B * n * sin(a);

}

/*
Apply smoothing force to dx,dy 
*/
void smoothing(Point* p0, Point* p1, Point* p2, const Config &config){
    

    double d0 = p0->distance(*p1);
    double d2 = p2->distance(*p1);

    p1->dx += config.F * ((p0->x*d2 + p2->x*d0)/(d0+d2) - p1->x);
    p1->dy += config.F * ((p0->y*d2 + p2->y*d0)/(d0+d2) - p1->y);

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
        // std::cout << "\t" << current->next << "\t" << point << std::endl;

        // only process if the distance "could" be within the range
        if((current->x - point->x < config.R1) && (current->x - point->x < config.R1)){

            dis = current->distance(*point);

            if(dis < config.R1){
                
                force = pow((config.R0 / dis),12) - pow((config.R0 / dis),6);

                dx += force * (point->x - current->x) / dis;
                dy += force * (point->y - current->y) / dis;
            }
        }

        current = current->next;
    }


    double mag = sqrt(dx*dx + dy*dy);

    // clamp the force to the max
    if(mag > config.MAX){
        dx = dx/mag * config.MAX;
        dy = dy/mag * config.MAX;
    }

    point->dx += config.A * dx;
    point->dy += config.A * dy;

}


/*
Apply the maze forces to each point
*/
void update(Point* start, const Config &config){

    Point* current = start;
    Point* previous = start;

    double distance;

    std::default_random_engine generator;
    std::normal_distribution<double> normal(0,1.0);
    std::uniform_real_distribution<double> distribution(0.0,M_PI*2.0);

    do{

        previous = current;

        current = current->next;

        brownian(current, config, generator, normal, distribution);
        smoothing(previous, current, current->next, config);
        proximity(current, config);
        // std::cout << "\titer" << std::endl;

    
    }while(current != start);

    std::cout << "FORCE DONE" << std::endl;
    // update the points from the forces
    do{
        
        current->x = current->x + current->dx;
        current->y = current->y + current->dy;

        current->dx = 0;
        current->dy = 0;

        current = current->next;
    
    }while(current != start);

    std::cout << "UPDATE DONE" << std::endl;


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
            y = 0.5*(current->next->y - current->y)+current->y;

            current->next = new Point(x, y, current->next);
            std::cout << *current << "\t" << current->next << std::endl;
        }

        if(distance < config.dmin){
            previous->next = current->next;
        }

    }while(current != start);
}