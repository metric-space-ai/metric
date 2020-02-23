import numpy
import metric


def test_euclidian():
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])

    assert isinstance(metric.Euclidian, object)

    distance = metric.Euclidian()
    assert isinstance(distance, metric.Euclidian)
    assert callable(distance)

    assert distance(v0, v1) == 2.0
    assert distance(v1, v0) == 2.0


def test_euclidian_with_threshold():
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])

    assert isinstance(metric.Euclidian, object)

    distance = metric.Euclidian_thresholded(thres_=1000.0, factor_=3000.0)
    assert isinstance(distance, metric.Euclidian_thresholded)
    assert callable(distance)

    assert distance(v0, v1) == 1000.0
    assert distance(v1, v0) == 1000.0


def test_manhatten():
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])

    assert isinstance(metric.Manhatten, object)

    distance = metric.Manhatten()
    assert isinstance(distance, metric.Manhatten)
    assert callable(distance)

    assert distance(v0, v1) == 4.0
    assert distance(v1, v0) == 4.0


def test_p_norm():
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])

    assert isinstance(metric.P_norm, object)

    distance = metric.P_norm(p_=2)
    assert isinstance(distance, metric.P_norm)
    assert callable(distance)

    assert distance(v0, v1) == 2.0
    assert distance(v1, v0) == 2.0


def test_cosine():
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])

    assert isinstance(metric.Cosine, object)

    distance = metric.Cosine()
    assert isinstance(distance, metric.Cosine)
    assert callable(distance)

    assert distance(v0, v1) == 0.970142500145332
    assert distance(v1, v0) == 0.970142500145332
