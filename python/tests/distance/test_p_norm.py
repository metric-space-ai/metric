import numpy
from metric.distance import P_norm


def test_p_norm():
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])

    assert isinstance(P_norm, object)

    distance = P_norm(p=2)
    assert isinstance(distance, P_norm)
    assert callable(distance)

    assert distance(v0, v1) == 2.0
    assert distance(v1, v0) == 2.0
