import sys
import numpy
from metric.mapping import kmeans


def main():
    data = numpy.float_([
           [0, 0, 0, 0, 0],
           [1.74120000000000, 4.07812000000000, -0.0927036000000, 41.7888000000000, 41.7888000000000],
           [7.75309000000000, 16.2466000000000, 3.03956000000000, 186.074000000000, 186.074000000000],
           [2.85493000000000, 3.25380000000000, 2.50559000000000, 68.5184000000000, 68.5184000000000],
           [5.81414000000000, 8.14015000000000, 3.22950000000000, 139.539000000000, 139.539000000000],
           [2.57927000000000, 2.63399000000000, 2.46802000000000, 61.9026000000000, 61.9026000000000]
    ])

    assignments, means, counts = kmeans(data, 4)  # clusters the data in 4 groups.

    print("assignments", list(assignments))
    print("means", list(map(list, means)))
    print("counts", list(counts))
    # TODO


sys.exit(main())
