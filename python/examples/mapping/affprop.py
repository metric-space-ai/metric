import sys
import numpy
from metric.mapping import affprop
from metric.space import Matrix


def main():
    data = numpy.float_([
        [0, 0, 0, 0, 0],
        [1.74120000000000, 4.07812000000000, -0.0927036000000, 41.7888000000000, 41.7888000000000],
        [7.75309000000000, 16.2466000000000, 3.03956000000000, 186.074000000000, 186.074000000000],
        [2.85493000000000, 3.25380000000000, 2.50559000000000, 68.5184000000000, 68.5184000000000],
        [5.81414000000000, 8.14015000000000, 3.22950000000000, 139.539000000000, 139.539000000000],
        [2.57927000000000, 2.63399000000000, 2.46802000000000, 61.9026000000000, 61.9026000000000]
    ])
    assignments, exemplars, counts = affprop(Matrix(data))
    print('assignments', list(assignments))
    print('exemplars', list(exemplars))
    print('counts', list(counts))


sys.exit(main())


