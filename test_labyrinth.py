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
        kmax=0.5,
    )

    l = Labyrinth(points, config)

    assert len(l.points) == 4

    l.resample()

    assert len(l.points) == 8

    