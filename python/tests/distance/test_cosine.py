import numpy
from metric.distance import Cosine


def test_cosine():
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])

    assert isinstance(Cosine, object)

    distance = Cosine()
    assert isinstance(distance, Cosine)
    assert callable(distance)

    assert distance(v0, v1) == 0.970142500145332
    assert distance(v1, v0) == 0.970142500145332
