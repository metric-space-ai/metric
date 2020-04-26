from metric.distance import Euclidean, Manhatten, Chebyshev, P_norm
from metric.correlation import Entropy
import numpy
import math
from functools import partial
import pytest


def my_euclidean(a, b):
    return math.sqrt(sum(((x - y) * (x - y) for x, y in zip(a, b))))


def test_entropy():
    v = numpy.float_([[5, 5], [2, 2], [3, 3], [5, 1]])
    entropy = partial(Entropy, p=3, k=2)

    assert pytest.approx(entropy()(v), 1.014045451620507)
    assert pytest.approx(entropy(metric=Euclidean())(v), 1.014045451620507)
    assert pytest.approx(entropy(metric=Manhatten())(v), 1.582185490039048)
    assert pytest.approx(entropy(metric=Chebyshev())(v), -3.9489104772539405)
    assert pytest.approx(entropy(metric=P_norm())(v), 1.582185490039048)
    assert pytest.approx(entropy(metric=P_norm(p=10))(v), -1.6957132689132957)
    assert pytest.approx(entropy(metric=my_euclidean)(v), 1.014045451620507)


def test_estimate():
    v = numpy.float_([[5, 5], [2, 2], [3, 3], [5, 1]])
    entropy = Entropy(p=10, k=5)

    assert entropy.estimate(v) == -0.0692878817128263
    assert entropy.estimate(v, sample_size=10) == -0.0692878817128263
    assert entropy.estimate(v, threshold=0) == -0.0692878817128263
    assert entropy.estimate(v, max_iterations=2) == -0.0692878817128263
