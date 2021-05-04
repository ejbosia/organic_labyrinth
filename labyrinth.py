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
from matplotlib.animation import FuncAnimation

from logging import Logger, INFO, DEBUG

from shapely_utilities import sample

from numba import jit
import numpy as np

logger = Logger(__name__)
logger.setLevel(DEBUG)

@dataclass(frozen = True)
class Config:
    A: float
    B: float
    F: float
    k0: float
    k1: float
    kmin: float
    kmax: float

'''
Labyrinth class
This class runs the labyrinth generation code. It takes in a set of points to process, and a Config object
'''
class Labyrinth:

    '''
    Parameters:
     - list of Point objects
     - Config instance
    '''
    def __init__(self, points, D, config:Config):

        self.points = np.array(points, float)

        assert type(config) == Config
        self.config = config

        self.D = D
        self.d = 1

        self.R0 = self.config.k0 * self.D
        self.R1 = (self.config.k1 * self.D)
        self.R12 = self.R1**2

    '''
    Calculate a brownian motion vector
    '''
    def _brownian_force(self):    
        d = normalvariate(0,1) * self.D * self.d
        a = random() * pi * 2    # angle between 0 and 2PI
        return (d*cos(a), d*sin(a))

    def _neighbor_indices(self, i1):
        i0 = i1 - 1 if i1 > 0 else len(self.points)-1
        i2 = i1 + 1 if i1+1 < len(self.points) else 0
        
        return i0, i1, i2

    '''
    Calculate the smoothing force on a point (at index i1)
    '''
    def _smoothing_force(self, i1):

        i0, i1, i2 = self._neighbor_indices(i1)

        p0 = self.points[i0]
        p1 = self.points[i1]
        p2 = self.points[i2]

        d0 = p1.distance(p0)
        d2 = p1.distance(p2)

        dx = (p0.x*d2+p2.x*d0)/(d0+d2) - p1.x
        dy = (p0.y*d2+p2.y*d0)/(d0+d2) - p1.y

        return (dx, dy)
    

    def _lennard_jones(self, r):
        if r > self.R1:
            return 0

        return (self.R0/r)**12 - (self.R0/r)**6

    '''
    Calculate the push pull force on a point
     - only include points within a distance
     - use lennard jones potential to get the force
    '''
    def _pushpull_force(self, i1):
        
        valid = []
        
        # get the neighbor indices
        i0, i1, i2 = self._neighbor_indices(i1)
        
        # get the point
        p1 = self.points[i1]

        avoid = {i0,i1,i2}

        # determine the valid points by comparing each linestring
        for i in range(len(self.points)-1):
            
            if i in avoid or i+1 in avoid:
                continue
           
            ls = LineString(self.points[i:i+2])

            p2 = ls.interpolate(ls.project(p1))
            
            if abs(p1.x-p2.x) > self.R1 or abs(p1.y-p2.y) > self.R1:
                continue

            if (p1.x-p2.x)**2+(p1.y-p2.y)**2 < self.R12:
                valid.append(ls.interpolate(ls.project(p1)))
                
        
        # get the forces
        dx = 0
        dy = 0
        
        for p in valid:

            dis = p1.distance(p)
    
            if dis > 0:
                E = _lennard_jones(dis/(self.D * self.d))
                                        
                dx += E * (p1.x-p.x)/dis
                dy += E * (p1.y-p.y)/dis
        
        # clamp the force to D
        d = sqrt(dx**2+dy**2)
        if d > 20:
            dx = dx/d * 20
            dy = dy/d * 20


        return (dx, dy)


    '''
    Run an update step on the points
    '''
    def update(self):
    
        bx = by = fx = fy = ax = ay = 0
        
        # calculate the force vectors for every point and update the point
        for i,p in enumerate(self.points):
            
            if self.config.B > 0:
                bx,by = self._brownian_force()
            if self.config.F > 0:
                fx,fy = self._smoothing_force(i)
            if self.config.A > 0:
                ax,ay = self._pushpull_force(i)
                
            x = p.x + self.config.B*bx + self.config.F*fx + self.config.A*ax
            y = p.y + self.config.B*by + self.config.F*fy + self.config.A*ay
            
            self.points[i] = Point((x,y))


    def _bisect(self, p0, p1):

        x = (p1[0]-p0[0])/2+p0[0]
        y = (p1[1]-p0[1])/2+p0[1]

        return (x,y)

    ''' 
    Run a resampling of the points:
     - remove point if next closer than kmin*D
     - add a bisecting point if next is farther than kmax*D
    '''
    def resample(self):
        
        additions = []
        new_points = []

        removals = []

        dmax = (self.config.kmax * self.D)**2
        dmin = (self.config.kmin * self.D)**2

        i = 0

        # loop through every point in the list
        # - this adjusts the index to match additions and deletions
        for i in range(len(self.points)):
            
            p0 = self.points[i-1]
            p1 = self.points[i]
            
            dis = (p0[0] - p1[0])**2 + (p0[1] - p1[1])**2

            if dis > dmax:
                # add the point
                new_points.append(self._bisect(p0,p1))
                additions.append(i)
            elif dis < dmin:
                removals.append(i + len(additions))

        print(new_points, additions)

        if additions:
            self.points = np.insert(self.points, additions, new_points, axis=0)

        # remove the points
        mask = np.ones(len(self.points), np.bool)
        mask[removals] = 0
        self.points = self.points[mask]
        
        print(self.points)

    def plot(self):

        X = []
        Y = []

        for p in self.points:
            X.append(p.x)
            Y.append(p.y)

        pyplot.plot(X,Y)


def main():

    ls = Point((0,0)).buffer(5).exterior
    points = sample(ls, 1)

    points = [(p.x,p.y) for p in points]

    # config = Config(
    #     A=1,
    #     B=0.05,
    #     F=0.15,
    #     k0=0.2,
    #     k1=0.5,
    #     kmin=0.2,
    #     kmax=0.6,
    # )
    config = Config(
        A=0.01,
        B=0.01,
        F=0.05,
        k0=1,
        k1=2,
        kmin=0.2,
        kmax=0.6,
    )

    # should oscillate between 4 and 8 points

    l = Labyrinth(points, config)
    
    l.plot()

    P = 1

    for i in range(50):
        l.update()
        l.resample()
        
        if i % P == P-1:
            pyplot.clf()
            l.plot()
            pyplot.title(i)
            pyplot.pause(0.05)
    
    pyplot.show()


if __name__ == "__main__":

    main()