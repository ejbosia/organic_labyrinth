'''
Labyrinth
Contains functions for creating organic labyrinths and mazes, as presented in "Organic Labyrinths and Mazes": http://www.dgp.toronto.edu/~karan/pdf/mazes.pdf

@author ejbosia
'''

import argparse

parser = argparse.ArgumentParser()
parser.add_argument("distance", help="distance between lines", type = float)
parser.add_argument("-a","--pushpull", help="push-pull multiplier", type = float)
parser.add_argument("-b", "--brownian", help="brownian mulitplier", type = float)
parser.add_argument("-f", "--fairing", help="enable output", type= float)
parser.add_argument("-m", "--metrics", help="enable metrics", action='store_true')



from shapely.geometry import Point, LinearRing, LineString

from math import sqrt, cos, sin, pi, atan2
from random import normalvariate, random

from dataclasses import dataclass

from matplotlib import pyplot
from matplotlib.animation import FuncAnimation, PillowWriter

from logging import Logger, INFO, DEBUG

from shapely_utilities import sample

from shapely_conversion import convert

from numba import jit, void, double

from numba import types
from numba.typed import Dict

import numpy as np
import cv2
import os

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
    counter = 0

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
            counter += 1
    
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
            counter += 1

    if counter == 0:
        return (0,0,0)

    dx /= counter
    dy /= counter

    # clamp the force to D
    d = sqrt(dx**2+dy**2)
    if d > config["CLAMP"]:
        dx = dx/d * config["CLAMP"]
        dy = dy/d * config["CLAMP"]

    return (dx, dy, counter)


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

        # skip frozen points
        if not points[i][2]:
            continue

        i0,i1,i2 = neighbor_indices(i, length)

        if config["B"] > 0:
            b = brownian_force(config["D"], d)
        if config["F"] > 0:
            f = smoothing_force(i0,i1,i2, points)
        if config["A"] > 0:
            a = pushpull_force(i0,i1,i2, points, boundary, config, d)
            
            
        x = config["A"]*a[0] + config["F"]*f[0] + config["B"]*b[0]
        y = config["A"]*a[1] + config["F"]*f[1] + config["B"]*b[1]

        points[i][0] +=  x
        points[i][1] +=  y

        if a[2] > config["FROZEN"]:
            points[i][2] = 0
        # points[i][2] = a[2]


@jit(nopython=True)
def _bisect(p0, p1):

    x = (p1[0]-p0[0])/2+p0[0]
    y = (p1[1]-p0[1])/2+p0[1]

    return (x,y,1)

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

        if not p0[2] and not p1[2]:
            continue
        
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
    
    D = 20

    image = cv2.imread("../images/squirtle.png", 0)

    assert not image is None

    image = image[:][::-1]

    temp = convert(image)[0].exterior

    b = temp.buffer(D)

    bls = b.exterior

    ls = b.centroid.buffer(2*D).exterior

    points = sample(ls, D)
    points = np.array([(p.x,p.y,1) for p in points])

    boundary = sample(bls,D)
    boundary = np.array([(p.x,p.y) for p in boundary])


    config = Dict.empty(
        key_type=types.unicode_type,
        value_type = types.float64
    )
    
    
    config["A"] = 0.008
    config["B"] = 0.05
    config["F"] = 0.1
    config["k0"] = 1.0
    config["k1"] = 5.0
    config["kmin"] = 0.2
    config["kmax"] = 0.5
    config["D"] = float(D)
    config["FROZEN"] = 250

    config["R0"] = config["k0"] * config["D"]
    config["R1"] = config["k1"] * config["D"]
    config["R12"] = config["R1"]**2

    config["CLAMP"] = 20.0*D
    
    print(config)

    P = 1
    d = 1
    
    fig = pyplot.figure()
    axis = pyplot.axes(xlim=(0, image.shape[1]),  ylim=(0, image.shape[0]))
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
            self.forces = None
            
        def maze_animation(self, frame_number):
            self.points = resample(self.points, self.config, self.d)

            self.forces = update(self.points, self.boundary, self.config, self.d)
            
            # connect the start and the end
            p0 = self.points[0]

            line.set_data(list(self.points[:,0])+[p0[0]], list(self.points[:,1])+[p0[1]])

            return line,


    
    maze = Maze(points, boundary, config, line, 1)

    num = 400
    pyplot.plot(maze.points[:,0], maze.points[:,1])


    for i in range(num):
        pyplot.title(str(i) + " " + str(config["B"]) + " " + str(np.max(maze.points[:,2])))
        maze.maze_animation(i)
        # pyplot.plot(maze.boundary[:,0], maze.boundary[:,1])
        pyplot.pause(0.05)
        pyplot.clf()

        pyplot.plot(maze.points[:,0], maze.points[:,1])
        # pyplot.scatter(maze.points[:,0], maze.points[:,1], c=maze.points[:,2], s=2, vmin=0, vmax=1)

        config["FROZEN"] -= 0.25
    
    pyplot.show()
    #     if config["B"] > 0:
    #         config["B"] -= 0.001
    #     else:
    #         config["B"] = 0
    # anim = FuncAnimation(fig, maze.maze_animation, frames=num, interval=20, blit=True, save_count=num)

    # writervideo = PillowWriter(fps=10)
    # anim.save('penguin.gif', writer=writervideo)
    # pyplot.close()
    

if __name__ == "__main__":

    main()