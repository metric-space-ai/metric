import numpy
from metric.distance import Kohonen


def test_distance():
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

    distance = Kohonen(simple_grid, nodes_width=grid_w, nodes_height=grid_h)

    assert distance(simple_grid[0], simple_grid[5]) > 2


def test_print_shortest_path(capsys):
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

    distance = Kohonen(simple_grid, nodes_width=grid_w, nodes_height=grid_h)
    distance.print_shortest_path(from_node=0, to_node=0)
    assert capsys.readouterr().out == "test"
