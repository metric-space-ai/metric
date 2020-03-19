from metric.distance import sorensen
import numpy


def test_ok():
    vt0, vt1, vt2, vt3 = 0, 1, 2, 3

    a = numpy.float_([vt0, vt1, vt2, vt0])
    b = numpy.float_([vt0, vt1, vt3])
    assert sorensen(a, b) == 0.14285714285714285
    assert sorensen(b, a) == 0.14285714285714285

    a = numpy.float_([vt0, vt1, vt2, vt0])
    b = numpy.float_([vt0, vt1, vt3, vt0])
    assert sorensen(a, b) == 0.14285714285714285
    assert sorensen(b, a) == 0.14285714285714285
