import numpy
from metric.distance import Euclidean, Euclidean_thresholded, Manhatten, P_norm, Cosine


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


def test_manhatten():
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])

    assert isinstance(Manhatten, object)

    distance = Manhatten()
    assert isinstance(distance, Manhatten)
    assert callable(distance)

    assert distance(v0, v1) == 4.0
    assert distance(v1, v0) == 4.0


def test_p_norm():
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])

    assert isinstance(P_norm, object)

    distance = P_norm(p=2)
    assert isinstance(distance, P_norm)
    assert callable(distance)

    assert distance(v0, v1) == 2.0
    assert distance(v1, v0) == 2.0


def test_cosine():
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])

    assert isinstance(Cosine, object)

    distance = Cosine()
    assert isinstance(distance, Cosine)
    assert callable(distance)

    assert distance(v0, v1) == 0.970142500145332
    assert distance(v1, v0) == 0.970142500145332
