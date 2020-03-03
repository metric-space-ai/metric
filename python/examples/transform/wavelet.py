import sys
import numpy
import metric


def main():
    x = numpy.float_([0.1, 0.2, 0.3, 0.2, 0.5])
    print(metric.dwt(x, 5))

    a = numpy.float_([0.1, 0.2, 0.3, 0.2, 0.5])
    b = numpy.float_([0.1, 0.11, 0.7, 0.2, 0.0])
    print(metric.idwt(a, b, 5, 5))

    print(metric.wmaxlev(x.size(), 5))


sys.exit(main())
