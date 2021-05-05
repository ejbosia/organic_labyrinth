'''
Labyrinth
Contains functions for creating organic labyrinths and mazes, as presented in "Organic Labyrinths and Mazes": http://www.dgp.toronto.edu/~karan/pdf/mazes.pdf

@author ejbosia
'''

from shapely.geometry import Point, LinearRing, LineString

from math import sqrt, cos, sin, pi, atan2
from random import normalvariate, random

from dataclasses import dataclass

from matplotlib import pyplot
from matplotlib.animation import FuncAnimation, PillowWriter

from logging import Logger, INFO, DEBUG

from shapely_utilities import sample

from numba import jit, void, double

from numba import types
from numba.typed import Dict

import numpy as np

logger = Logger(__name__)
logger.setLevel(DEBUG)


@jit(nopython=True)
def closest(A,B,C):

    e1 = np.array([B[0] - A[0], B[1] - A[1]])
    e2 = np.array([C[0] - A[0], C[1] - A[1]])

    dp = np.dot(e1, e2)

    if dp < 0:
        return A
    if dp > np.dot(e1,e1):
        return B

    len2 = e1[0] * e1[0] + e1[1] * e1[1]

    return np.array(((A[0] + (dp * e1[0]) / len2),(A[1] + (dp * e1[1]) / len2)))


'''
Calculate a brownian motion vector
'''
@jit(nopython=True)
def brownian_force(D, d):    
    d = normalvariate(0,1) * D * d
    a = random() * pi * 2    # angle between 0 and 2PI
    return (d*cos(a), d*sin(a))

@jit(nopython=True)
def neighbor_indices(i1, length):
    i0 = i1 - 1 if i1 > 0 else length-1
    i2 = i1 + 1 if i1+1 < length else 0
    
    return [i0, i1, i2]

'''
Calculate the smoothing force on a point (at index i1)
'''
@jit(nopython=True)
def smoothing_force(i0,i1,i2,points):

    p0 = points[i0]
    p1 = points[i1]
    p2 = points[i2]

    d0 = np.sqrt((p1[0]-p0[0])**2 + (p1[1]-p0[1])**2)
    d2 = np.sqrt((p1[0]-p2[0])**2 + (p1[1]-p2[1])**2)

    dx = (p0[0]*d2+p2[0]*d0)/(d0+d2) - p1[0]
    dy = (p0[1]*d2+p2[1]*d0)/(d0+d2) - p1[1]

    return (dx, dy)


'''
Calculate the push pull force on a point
    - only include points within a distance
    - use lennard jones potential to get the force
'''
@jit(nopython=True)
def pushpull_force(i0,i1,i2, points, boundaries, config, d):

    # get the point
    p1 = points[i1]

    avoid = {i0,i1,i2}

    dx = 0
    dy = 0

    # determine the valid points by comparing each linestring
    for i in range(len(points)-1):
        
        if i in avoid or i+1 in avoid or -1 in avoid:
            continue
        
        p2 = closest(points[i], points[i+1], p1)
        
        if abs(p1[0]-p2[0]) > config["R1"] or abs(p1[1]-p2[1]) > config["R1"]:
            continue
        
        d2 = (p1[0]-p2[0])**2+(p1[1]-p2[1])**2

        if d2 < config["R12"]:

            dis = np.sqrt(d2)

            r = dis/(config["D"] * d)

            E = (config["R0"]/r)**12-(config["R0"]/r)**6 
                                    
            dx += E * (p1[0]-p2[0])/dis
            dy += E * (p1[1]-p2[1])/dis
    


    # check the boundaries
    # determine the valid points by comparing each linestring
    for i in range(len(boundaries)):
        
        p2 = closest(boundaries[i-1], boundaries[i], p1)
        
        if abs(p1[0]-p2[0]) > config["R1"] or abs(p1[1]-p2[1]) > config["R1"]:
            continue
        
        d2 = (p1[0]-p2[0])**2+(p1[1]-p2[1])**2
        if d2 < config["R12"]:

            dis = np.sqrt(d2)

            r = dis/(config["D"] * d)

            E = (config["R0"]/r)**12-(config["R0"]/r)**6 
                                    
            dx += E * (p1[0]-p2[0])/dis
            dy += E * (p1[1]-p2[1])/dis


    # clamp the force to D
    d = sqrt(dx**2+dy**2)
    if d > 20:
        dx = dx/d * 20
        dy = dy/d * 20

    return (dx, dy)


'''
Run an update step on the points
'''
@jit(nopython=True)
def update(points, boundary, config, d):

    bx = 0
    by = 0
    fx = 0
    fy = 0
    ax = 0
    ay = 0
    
    length = len(points)

    # calculate the force vectors for every point and update the point
    for i in range(len(points)):
        
        i0,i1,i2 = neighbor_indices(i, length)

        if config["B"] > 0:
            b = brownian_force(config["D"], d)
        if config["F"] > 0:
            f = smoothing_force(i0,i1,i2, points)
        if config["A"] > 0:
            a = pushpull_force(i0,i1,i2, points, boundary, config, d)
            
        points[i][0] += config["B"]*b[0] + config["F"]*f[0] + config["A"]*a[0]
        points[i][1] += config["B"]*b[1] + config["F"]*f[1] + config["A"]*a[1]


@jit(nopython=True)
def _bisect(p0, p1):

    x = (p1[0]-p0[0])/2+p0[0]
    y = (p1[1]-p0[1])/2+p0[1]

    return (x,y)

''' 
Run a resampling of the points:
    - remove point if next closer than kmin*D
    - add a bisecting point if next is farther than kmax*D
'''
def resample(points, config, d):
    
    additions = []
    new_points = []

    removals = []

    dmax = (config["kmax"] * config["D"])**2
    dmin = (config["kmin"] * config["D"])**2

    i = 0

    # loop through every point in the list
    # - this adjusts the index to match additions and deletions
    for i in range(len(points)):
        
        p0 = points[i-1]
        p1 = points[i]
        
        dis = (p0[0] - p1[0])**2 + (p0[1] - p1[1])**2

        if dis > dmax:
            # add the point
            new_points.append(_bisect(p0,p1))
            additions.append(i)
        elif dis < dmin:
            removals.append(i + len(additions))

    if additions:
        points = np.insert(points, additions, new_points, axis=0)

    # remove the points
    mask = np.ones(len(points), np.bool)
    mask[removals] = 0
    points = points[mask]

    return points




def main():
    
    ls = LinearRing([(-2,-2), (-2,2), (2,2), (2,-2)])
    

    bls = LinearRing([(0,-10), (-10,5), (10,5)])

    boundary = sample(bls,1)
    boundary = np.array([(p.x,p.y) for p in boundary])

    points = sample(ls, 1)

    points = np.array([(p.x,p.y) for p in points])

    config = Dict.empty(
        key_type=types.unicode_type,
        value_type = types.float64
    )
    
    
    config["A"] = 0.006
    config["B"] = 0.05
    config["F"] = 0.1
    config["k0"] = 1.0 
    config["k1"] = 10.0
    config["kmin"] = 0.2
    config["kmax"] = 0.6
    config["D"] = 1.0

    config["R0"] = config["k0"] * config["D"]
    config["R1"] = config["k1"] * config["D"]
    config["R12"] = config["R1"]**2
    
    P = 1
    d = 1
    
    fig = pyplot.figure()
    axis = pyplot.axes(xlim=(-20, 20),  ylim=(-20, 20))
    pyplot.gca().set_axis_off()
    line, = axis.plot([], [], lw=3)


    '''
    This class is used just to clean up the code
    '''
    class Maze:
        
        def __init__(self, points, boundary, config, line, d):
            self.points = points
            self.boundary = boundary
            self.config = config
            self.line = line
            self.d = d
            
        def maze_animation(self, frame_number):
            
            update(self.points, self.boundary, self.config, self.d)
            self.points = resample(self.points, self.config, self.d)
            
            # connect the start and the end
            p0 = self.points[0]

            line.set_data(list(self.points[:,0])+[p0[0]], list(self.points[:,1])+[p0[1]])

            return line,


    
    maze = Maze(points, boundary, config, line, 1)

    num = 200

    for i in range(num):
        pyplot.clf()
        maze.maze_animation(i)
        pyplot.plot(maze.boundary[:,0], maze.boundary[:,1])
        pyplot.plot(maze.points[:,0], maze.points[:,1])
        pyplot.pause(0.05)
    
    # anim = FuncAnimation(fig, maze.maze_animation, frames=num, interval=20, blit=True, save_count=num)

    # writervideo = PillowWriter(fps=10)
    # anim.save('test2.gif', writer=writervideo)
    # pyplot.close()
    

if __name__ == "__main__":

    main()