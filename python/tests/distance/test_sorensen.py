from metric.distance import Sorensen
import numpy


def test_ok():
    vt0, vt1, vt2, vt3 = 0, 1, 2, 3
    distance = Sorensen()

    a = numpy.float_([vt0, vt1, vt2, vt0])
    b = numpy.float_([vt0, vt1, vt3])
    assert distance(a, b) == 0.14285714285714285
    assert distance(b, a) == 0.14285714285714285

    a = numpy.float_([vt0, vt1, vt2, vt0])
    b = numpy.float_([vt0, vt1, vt3, vt0])
    assert distance(a, b) == 0.14285714285714285
    assert distance(b, a) == 0.14285714285714285
