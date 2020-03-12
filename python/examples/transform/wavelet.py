import sys
import numpy
from metric.transform import dwt, idwt, wmaxlev


def main():
    x = numpy.float_([0.1, 0.2, 0.3, 0.2, 0.5])
    a, b = dwt(x, 5)
    print(list(a))
    print(list(b))

    a = numpy.float_([0.1, 0.2, 0.3, 0.2, 0.5])
    b = numpy.float_([0.1, 0.11, 0.7, 0.2, 0.0])
    print(list(idwt(a, b, 5, 5)))

    print(wmaxlev(5, 5))


sys.exit(main())
