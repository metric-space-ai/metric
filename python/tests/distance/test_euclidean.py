import numpy
from metric.distance import Euclidean, Euclidean_thresholded


def test_euclidean():
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])

    assert isinstance(Euclidean, object)

    distance = Euclidean()
    assert isinstance(distance, Euclidean)
    assert callable(distance)

    assert distance(v0, v1) == 2.0
    assert distance(v1, v0) == 2.0


def test_euclidean_with_threshold():
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])

    assert isinstance(Euclidean, object)

    distance = Euclidean_thresholded(thres=1000.0, factor=3000.0)
    assert isinstance(distance, Euclidean_thresholded)
    assert callable(distance)

    assert distance(v0, v1) == 1000.0
    assert distance(v1, v0) == 1000.0
