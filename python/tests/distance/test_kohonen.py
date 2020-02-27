import numpy
import metric


def test_ok():
    grid_w = 3
    grid_h = 2
    simple_grid = numpy.float_([
        [0, 0],
        [1, 0],
        [2, 0],

        [0, 1],
        [1, 1],
        [2, 1],
    ])

    distance = metric.kohonen_distance(simple_grid, grid_w, grid_h)

    print("distance: ", distance(simple_grid[0], simple_grid[5]))
