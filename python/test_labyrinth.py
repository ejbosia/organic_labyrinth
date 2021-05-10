from labyrinth import Config, Labyrinth
from shapely.geometry import Point, LineString

def test_init():

    # create a square list of points
    points = [(0,0),(0,1),(1,1),(1,0)]

    # initialize config object
    config = Config(
        A=0,
        B=0,
        F=0,
        k0=0,
        k1=0,
        kmin=0,
        kmax=1,
    )

    # initialize labyrinth object
    l = Labyrinth(points, 1, config)

    assert l.config.kmax == 1
    assert l.D == 1

def test_resample():

    points = [(0,0),(0,1),(1,1),(1,0)]

    config = Config(
        A=0,
        B=0,
        F=0,
        k0=0,
        k1=0,
        kmin=0,
        kmax=0.3,
    )

    l = Labyrinth(points, 1, config)
    assert len(l.points) == 4

    l.resample() # should add mid points to each line segment
    assert len(l.points) == 8

    l.resample() # should add mid points to each line segment
    assert len(l.points) == 16

    l.resample() # should not add mid points to each line segments
    assert len(l.points) == 16

    config = Config(
        A=0,
        B=0,
        F=0,
        k0=0,
        k1=0,
        kmin=0.8,
        kmax=1.2,
    )

    l2 = Labyrinth(l.points, 1, config)

    assert len(l2.points) == 16

    l2.resample() # remove points that are closer than the min distance
    assert len(l2.points) == 0

    print(l2.points)