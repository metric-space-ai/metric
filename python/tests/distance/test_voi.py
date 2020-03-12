from metric.distance import VOI_kl, VOI_normalized, entropy
import numpy
import pytest


def test_VOI_kl():
    v1 = numpy.float_([[5, 5], [2, 2], [3, 3], [5, 5]])
    v2 = numpy.float_([[5, 5], [2, 2], [3, 3], [1, 1]])
    distance = VOI_kl()

    assert distance(v2, v1) == pytest.approx(distance(v1, v2))
    assert distance(v1, v2) == pytest.approx(49.8916690415478)


def test_VOI_normalized():
    v1 = numpy.float_([[5, 5], [2, 2], [3, 3], [5, 5]])
    v2 = numpy.float_([[5, 5], [2, 2], [3, 3], [1, 1]])
    distance = VOI_normalized()

    assert not (distance(v2, v1) == pytest.approx(distance(v1, v2)))
    assert distance(v1, v2) == pytest.approx(0.9272535656800133)


def test_entropy():
    v = numpy.float_([[5, 5], [2, 2], [3, 3], [5, 1]])

    assert entropy(v) == 7.819791963526808
    assert entropy(v, metric='euclidean') == 7.819791963526808
    assert entropy(v, metric='manhatten') == 7.918295834054489
    assert entropy(v, metric='chebyshev') == 7.625814583693912
    assert entropy(v, metric='p-norm') == 7.918295834054489
