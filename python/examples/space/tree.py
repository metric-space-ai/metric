import sys
import numpy
from metric.space import Tree


def main():
    print("Simple space example have started")

    # here are some data records
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])
    v2 = numpy.float_([2, 2, 2, 1, 1, 2, 0, 0])
    v3 = numpy.float_([3, 3, 2, 2, 1, 1, 0, 0])

    # initialize the tree
    tree = Tree()

    # add data records
    tree.insert(v0)
    tree.insert(v1)
    tree.insert(v2)
    tree.insert(v3)

    # find the nearest neighbour of a data record
    v8 = numpy.float_([2, 8, 2, 1, 0, 0, 0, 0])

    nn = tree.nn(v8)
    print("nearest neighbour of v8 is v", nn)

    tree.print()

    # batch insert
    table = numpy.float_([
        [0, 1, 1, 1, 1, 1, 2, 3],
        [1, 1, 1, 1, 1, 2, 3, 4],
        [2, 2, 2, 1, 1, 2, 0, 0],
        [3, 3, 2, 2, 1, 1, 0, 0],
        [4, 3, 2, 1, 0, 0, 0, 0],
        [5, 3, 2, 1, 0, 0, 0, 0],
        [4, 6, 2, 2, 1, 1, 0, 0],
    ])

    tree2 = Tree(table)

    table2 = numpy.float_([
        [3, 7, 2, 1, 0, 0, 0, 0],
        [2, 8, 2, 1, 0, 0, 0, 0]
    ])

    tree2.insert(table2)

    tree.print()


sys.exit(main())
