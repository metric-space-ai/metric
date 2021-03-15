import numpy
from metric.distance import Chebyshev


def test_chebyshev():
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])

    assert isinstance(Chebyshev, object)

    distance = Chebyshev()
    assert isinstance(distance, Chebyshev)
    assert callable(distance)

    assert distance(v0, v1) == 1.0
    assert distance(v1, v0) == 1.0
