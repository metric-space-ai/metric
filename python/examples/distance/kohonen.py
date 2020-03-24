import sys
import numpy
from metric.utils.misc import time_n_log
from metric.distance import Kohonen


def main():
    print("Kohonen Distance example have started")
    grid_w = 3
    grid_h = 2

    @time_n_log
    def simple_example():

        simple_grid = numpy.float_([
            [0, 0],
            [1, 0],
            [2, 0],

            [0, 1],
            [1, 1],
            [2, 1],
        ])

        return Kohonen(simple_grid, grid_w, grid_h)(simple_grid[0], simple_grid[5])

    @time_n_log
    def load_from_file_example(filename):
        train_dataset = numpy.genfromtxt(filename, delimiter='\t')
        return Kohonen(train_dataset, grid_w, grid_h)(train_dataset[0], train_dataset[1])

    simple_example()
    load_from_file_example('../examples/distance_examples/assets/Compound.txt')


sys.exit(main())
