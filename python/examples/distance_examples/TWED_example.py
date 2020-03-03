import sys
import numpy
import metric


def main():
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])
    v2 = numpy.float_([2, 2, 2, 2, 2, 2, 2, 2])
    distance = metric.TWED(0, 1)

    print("result for a close curves (series): ", distance(v0, v1))
    print("result for a far curves (series): ", distance(v0, v2))


sys.exit(main())