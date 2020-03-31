import numpy
from metric.distance import Manhatten


def test_manhatten():
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])

    assert isinstance(Manhatten, object)

    distance = Manhatten()
    assert isinstance(distance, Manhatten)
    assert callable(distance)

    assert distance(v0, v1) == 4.0
    assert distance(v1, v0) == 4.0
