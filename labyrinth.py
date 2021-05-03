'''
Labyrinth
Contains functions for creating organic labyrinths and mazes, as presented in "Organic Labyrinths and Mazes": http://www.dgp.toronto.edu/~karan/pdf/mazes.pdf

@author ejbosia
'''

from shapely.geometry import Point, LinearRing, LineString

from math import sqrt, cos, sin, pi
from random import normalvariate, random

from dataclasses import dataclass

from matplotlib import pyplot

from logging import Logger, INFO, DEBUG

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
    def __init__(self, points, config:Config):

        self.points = points

        assert type(config) == Config
        self.config = config

        # assumes even spacing
        self.D = points[0].distance(points[1])

    '''
    Calculate a brownian motion vector
    '''
    def _brownian_force(self):    
        d = normalvariate(0,1) * self.D
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

        i0, i1, i2 = self.neighbor_indices(index)

        p0 = self.points[i0]
        p1 = self.points[i1]
        p2 = self.points[i2]

        d0 = p1.distance(p0)
        d2 = p1.distance(p2)

        dx = (p0.x*d2+p2.x*d0)/(d0+d2) - p1.x
        dy = (p0.y*d2+p2.y*d0)/(d0+d2) - p1.y

        return (dx, dy)
    
    '''
    Calculate the push pull force on a point
     - only include points within a distance
     - use lennard jones potential to get the force
    '''
    def _pushpull_force(self, i1):
        
        def _lennard_jones(r):
            if r > self.config.R1:
                return 0

            return (self.config.R0/r)**12 - (self.config.R0/r)**6

        valid = []
        
        # get the neighbor indices
        i0, i1, i2 = neighbor_indices(i1, points)
        
        # get the point
        p1 = self.points[i1]
        
        temp = self.points[i2+1:] + self.points[0:i0]

        for i in range(temp):
            
            ls = LineString(temp[i], temp[i+1])
            
            if p1.distance(ls) < R1 * delta:
                valid.append(ls.interpolate(ls.project(p1)))
                
        
        # get the forces
        dx = 0
        dy = 0
        
        for p in valid:
            
            dis = p1.distance(p)
            
            E = _lennard_jones(p.distance(p1)/(D*delta))
            
            angle = np.arctan2(p1.x-p.x, p1.y-p.y)
                    
            dx +=  np.cos(angle) * E * (p1.x-p.x)/dis
            dy +=  np.sin(angle) * E * (p1.y-p.y)/dis
        
        return (dx, dy)


    '''
    Run an update step on the points
    '''
    def update(self, dis, delta):
    
        bx,by,fx,fy,ax,ay = 0
        
        # calculate the force vectors for every point and update the point
        for i,p in enumerate(self.points):
            
            if self.config.B > 0:
                bx,by = self.brownian()
            if self.config.F > 0:
                fx,fy = self.smoothing(i)
            if self.config.A > 0:
                ax,ay = pushpull(i)
                
            self.points[i].x = p.x + B*bx + F*fx + A*ax
            self.points[i].y = p.y + B*by + F*fy + A*ay
    
    ''' 
    Run a resampling of the points:
     - remove point if next closer than kmin*D
     - add a bisecting point if next is farther than kmax*D
    '''
    def resample(self):

        new_points = []

        dmax = self.config.kmax * self.D
        dmin = self.config.kmin * self.D

        i = 0

        print(self.points)

        # loop through every point in the list
        # - this adjusts the index to match additions and deletions
        while i < len(self.points):
            
            dis = self.points[i-1].distance(self.points[i])

            if dis > dmax:
                # add the point

                bisect = LineString([self.points[i-1], self.points[i]]).interpolate(dis/2)
                self.points.insert(i, bisect)

                # skip the added point
                i += 2
            elif dis < dmin:
                # remove the point
                self.points.pop(i)
            else:
                # do not change the existing points
                i += 1



def main():

    points = [(0,0),(0,1),(1,1),(1,0)]
    points = [Point(p) for p in points]

    config = Config(
        A=0,
        B=0,
        F=0,
        k0=0,
        k1=0,
        kmin=0.6,
        kmax=0.9,
    )


    # should oscillate between 4 and 8 points

    l = Labyrinth(points, config)
    print(len(l.points))
    l.resample()
    print(len(l.points))
    l.resample()
    print(len(l.points))

if __name__ == "__main__":

    main()