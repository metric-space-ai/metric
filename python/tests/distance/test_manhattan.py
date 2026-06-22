import numpy
from metric.distance import Manhattan


def test_manhattan():
    v0 = numpy.asarray([0, 1, 1, 1, 1, 1, 2, 3], dtype=float)
    v1 = numpy.asarray([1, 1, 1, 1, 1, 2, 3, 4], dtype=float)

    assert isinstance(Manhattan, object)

    distance = Manhattan()
    assert isinstance(distance, Manhattan)
    assert callable(distance)

    assert distance(v0, v1) == 4.0
    assert distance(v1, v0) == 4.0
