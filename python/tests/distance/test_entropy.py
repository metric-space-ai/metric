from metric.distance import entropy
import numpy


def test_entropy():
    v = numpy.float_([[5, 5], [2, 2], [3, 3], [5, 1]])

    assert entropy(v) == 7.819791963526808
    assert entropy(v, metric='euclidean') == 7.819791963526808
    assert entropy(v, metric='manhatten') == 7.918295834054489
    assert entropy(v, metric='chebyshev') == 7.625814583693912
    assert entropy(v, metric='p-norm') == 7.918295834054489
