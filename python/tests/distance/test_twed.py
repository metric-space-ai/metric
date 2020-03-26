import numpy
from metric.distance import TWED


def test_ok():
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])
    v2 = numpy.float_([2, 2, 2, 2, 2, 2, 2, 2])
    distance = TWED(0, 1)

    assert distance(v0, v1) == 7.0, "result for a close curves (series): "
    assert distance(v0, v2) == 15.0, "result for a far curves (series): "


def test_construct_default():
    distance = TWED()
    assert distance.penalty == 0
    assert distance.elastic == 1
    assert distance.is_zero_padded is False


def test_construct_override():
    distance = TWED(penalty=2, elastic=1)
    assert distance.penalty == 2
    assert distance.elastic == 1
    assert distance.is_zero_padded is False
