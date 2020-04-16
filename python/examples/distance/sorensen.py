import sys
from metric.distance import sorensen
import numpy


def main():
    vt0, vt1, vt2, vt3 = 0, 1, 2, 3

    a = numpy.float_([vt0, vt1, vt2, vt0])
    b = numpy.float_([vt0, vt1, vt3])
    print('Sorensen Distance result: ', sorensen(a, b))

    a = numpy.float_([vt0, vt1, vt2, vt0])
    b = numpy.float_([vt0, vt1, vt3, vt0])
    print('Sorensen Distance result: ', sorensen(a, b))


sys.exit(main())
