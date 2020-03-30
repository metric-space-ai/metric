from metric.distance import Euclidean, Manhatten, Chebyshev, P_norm
from metric.correlation import Entropy
import numpy
import math
from functools import partial


def my_euclidean(a, b):
    return math.sqrt(sum(((x - y) * (x - y) for x, y in zip(a, b))))


def test_entropy():
    v = numpy.float_([[5, 5], [2, 2], [3, 3], [5, 1]])
    entropy = partial(Entropy, p=3, k=2)

    assert entropy()(v) == 5.967356977156464
    assert entropy(metric=Euclidean())(v) == 5.967356977156464
    assert entropy(metric=Manhatten())(v) == 6.065860847684145
    assert entropy(metric=Chebyshev())(v) == 5.773379597323566
    assert entropy(metric=P_norm())(v) == 6.065860847684145
    assert entropy(metric=P_norm(p=10))(v) == 5.852646926226563
    assert entropy(metric=my_euclidean)(v) == 5.315860847684144  # different value because of hardcoded Euclidean (p)
