#include <sstream>
#include <numbers>
#include <future>

#include "maze.h"

Maze::Maze(const Config& config, std::vector<Point> points) :
    config(config), 
    points(points),
    boundary({}),
    normal(0, 1.0),
    distribution(0.0, std::numbers::pi * 2.0)
{}

Maze::Maze(const Config& config, std::vector<Point> points, std::vector<Point> boundary) :
    config(config),
    points(points),
    boundary(boundary),
    normal(0, 1.0),
    distribution(0.0, std::numbers::pi * 2.0)
{}

/*
Apply brownian force to every point in vector
*/
void Maze::_brownian(int index)
{
    double n = normal(generator);
    double a = distribution(generator);

    points[index].dx += this->config.B * n * cos(a);
    points[index].dy += this->config.B * n * sin(a);
}

void Maze::_smoothing(int index)
{
    int i0 = index - 1;
    int i2 = index + 1;

    // Avoid index out-of-bounds
    if (index == 0)
    {
        i0 = points.size() - 1;
    }
    else if (index == points.size() - 1)
    {
        i2 = 0;
    }

    Point* p0 = &points[i0];
    Point* p1 = &points[index];
    Point* p2 = &points[i2];
    double d0 = p1->distance(*p0);
    double d2 = p1->distance(*p2);

    p1->dx += this->config.F * ((p0->x*d2 + p2->x*d0)/(d0+d2) - p1->x);
    p1->dy += this->config.F * ((p0->y*d2 + p2->y*d0)/(d0+d2) - p1->y);
}

void Maze::_proximity_force(Point& point, const Point& p0, const Point& p1, int& counter)
{
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
void Maze::_proximity(int index, int skip){
    
    int counter = 0;

    // process other points in maze
    for(int j = 0; j < points.size(); j++){

        // skip adjacent indices
        if(abs(index-j) <= skip || abs(index-(j+1)) <= skip || abs(index-j) >= points.size()-skip || abs(index-(j+1)) >= points.size()-skip){
            continue;
        }

        _proximity_force(points[index], points[j], points[(j+1)%points.size()], counter);
    }

    // process the boundaries
    for(int j = 0; j < boundary.size(); j++){
        _proximity_force(points[index], boundary[j], boundary[(j+1)%boundary.size()], counter);
    }
        
    double force = points[index].dx * points[index].dx + points[index].dy * points[index].dy;

    if(force > config.MAX * config.MAX){
        force = sqrt(force);

        points[index].dx = points[index].dx/force * config.MAX;
        points[index].dy = points[index].dy/force * config.MAX;
    }

    // freeze the point if there are enough points close to it
    if(counter > config.freeze){
        points[index].frozen = true;
    }
}


void Maze::_calculate_force(int index)
{
    if (points[index].frozen) return;
    _proximity(index);
    _brownian(index);
    _smoothing(index);
}

/*
Apply the maze forces to each point
*/
void Maze::update(){

    std::vector<std::future<void>> tasks;
    tasks.reserve(points.size());

    for (int i = 0; i < points.size(); i++)
    {
        tasks.emplace_back(std::async(&Maze::_calculate_force, this, i));
    }

    for (auto& task : tasks)
    {
        task.wait();
    }
    
    // update the points from the forces
    for(int i = 0; i < points.size(); i++)
    {        
        tasks.at(i) = std::async([this](int index) { points[index].update(); }, i);
    }

    for (auto& task : tasks)
    {
        task.wait();
    }

    printf("NODES: %d ALIVE: %d\n", points.size(), -1);
}


void Maze::_add_or_remove(Point* p1, const Point* p2)
{
    //if (p1->frozen && p2->frozen)
    //{
    //    //p1->action = ResampleAction::NONE;
    //    return;
    //}

    double distance = p1->distance(*p2);

    if (distance > this->config.dmax) 
    {
        p1->action = ResampleAction::ADD;
        return;
    }
    if (distance < this->config.dmin) 
    {
        p1->action = ResampleAction::REMOVE;
        return;
    }

    p1->action = ResampleAction::NONE;
}


/*
Add points to the linked list of points
*/
void Maze::resample() {

    double distance{ 0 };

    Point* p1;
    Point* p2;

    std::vector<std::future<void>> tasks;
    tasks.reserve(points.size());

    p1 = &points.back();
    p2 = &points.front();

    tasks.emplace_back(std::async(&Maze::_add_or_remove, this, p1, p2));

    for (int i = 1; i < points.size(); i++)
    {
        p1 = p2;
        p2 = &points.at(i);
        tasks.emplace_back(std::async(&Maze::_add_or_remove, this, p1, p2));
    }

    for (auto& task : tasks)
    {
        task.wait();
    }

    std::vector<Point> temp;
    temp.reserve(points.size());

    for (int i = 0; i < points.size(); i++)
    {
        switch (points.at(i).action)
        {
        case ResampleAction::NONE:
            temp.emplace_back(points.at(i));
            break;
        case ResampleAction::REMOVE:
            break;
        case ResampleAction::ADD:
            temp.emplace_back(points.at(i));
            temp.emplace_back(bisect(points[i], points[(i + 1) % points.size()]));
            break;
        }
    }

    points.resize(temp.size());
    std::move(std::begin(temp), std::end(temp), std::begin(points));
}

/*
Output all of the points in a string in csv format
*/
std::string Maze::output(){
    std::stringstream output;
    output << "x,y,frozen\n";
    for(const auto& point : points){
        output << point << "\n";
    }
    return output.str();
}

