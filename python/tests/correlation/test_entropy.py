from metric.distance import Euclidean, Manhatten, Chebyshev, P_norm
from metric.correlation import entropy
import numpy
import math


def my_euclidean(a, b):
    return math.sqrt(sum(((x - y) * (x - y) for x, y in zip(a, b))))


def test_entropy():
    v = numpy.float_([[5, 5], [2, 2], [3, 3], [5, 1]])

    assert entropy(v) == 5.967356977156464
    assert entropy(v, metric=Euclidean()) == 5.967356977156464
    assert entropy(v, metric=Manhatten()) == 6.065860847684145
    assert entropy(v, metric=Chebyshev()) == 5.773379597323566
    assert entropy(v, metric=P_norm()) == 6.065860847684145
    assert entropy(v, metric=P_norm(p=10)) == 5.852646926226563
    assert entropy(v, metric=my_euclidean) == 5.315860847684144  # different value because of hardcoded Euclidean (p)
