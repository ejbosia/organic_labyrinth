from labyrinth import Config, Labyrinth
from shapely.geometry import Point, LineString

def test_init():

    # create a square list of points
    points = [Point(p) for p in [(0,0),(0,1),(1,1),(1,0)]]

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
    l = Labyrinth(points, config)

    assert l.config.kmax == 1
    assert l.D == 1

def test_resample():

    points = [(0,0),(0,1),(1,1),(1,0)]
    points = [Point(p) for p in points]

    config = Config(
        A=0,
        B=0,
        F=0,
        k0=0,
        k1=0,
        kmin=0,
        kmax=0.3,
    )

    l = Labyrinth(points, config)
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

    l2 = Labyrinth(l.points, config)

    # artificially set "D" to be 1
    l2.D = 1

    assert len(l2.points) == 16

    l2.resample() # should remove the midpoints of l2 until the correct distance specs are met
    assert len(l2.points) == 4

    # check that the corners remain
    for p1,p2 in zip(points, l2.points):
        assert p1 == p2