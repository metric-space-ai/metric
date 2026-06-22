import numpy
import pytest
from metric.distance import P_norm


def test_p_norm():
    v0 = numpy.asarray([0, 1, 1, 1, 1, 1, 2, 3], dtype=float)
    v1 = numpy.asarray([1, 1, 1, 1, 1, 2, 3, 4], dtype=float)

    assert isinstance(P_norm, object)

    distance = P_norm(p=2)
    assert isinstance(distance, P_norm)
    assert callable(distance)
    assert distance.p == 2

    assert distance(v0, v1) == 2.0
    assert distance(v1, v0) == 2.0

    with pytest.raises(ValueError):
        P_norm(p=0.5)

    with pytest.raises(AttributeError):
        distance.p = 0.5
