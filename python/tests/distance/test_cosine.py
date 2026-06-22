import numpy
from metric.distance import Cosine


def test_cosine():
    v0 = numpy.asarray([0, 1, 1, 1, 1, 1, 2, 3], dtype=float)
    v1 = numpy.asarray([1, 1, 1, 1, 1, 2, 3, 4], dtype=float)

    assert isinstance(Cosine, object)

    distance = Cosine()
    assert isinstance(distance, Cosine)
    assert callable(distance)

    assert distance(v0, v1) == 0.07797913037736921
    assert distance(v1, v0) == 0.07797913037736921
