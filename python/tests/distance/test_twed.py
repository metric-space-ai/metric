"""Deterministic fixture for the native TWED binding in the default wheel.

TWED (Time Warp Edit Distance) is compiled from ``python/src/distance/TWED.cpp``
(wrapping ``mtrc::TWED<double>``) into the default ``metric._impl.distance``
extension. The math runs entirely in native C++; this fixture only verifies the
metric contract over the binding.
"""

import numpy
import pytest
from metric.distance import TWED


def test_ok():
    v0 = numpy.asarray([0, 1, 1, 1, 1, 1, 2, 3], dtype=float)
    v1 = numpy.asarray([1, 1, 1, 1, 1, 2, 3, 4], dtype=float)
    v2 = numpy.asarray([2, 2, 2, 2, 2, 2, 2, 2], dtype=float)
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


def test_identity_is_zero():
    """Equal records map to distance zero (identity of indiscernibles)."""
    distance = TWED(0, 1)
    series = [0.0, 1.0, 1.0, 2.0, 3.0]
    assert distance(series, series) == 0.0


def test_symmetry_holds():
    """TWED is symmetric: d(a, b) == d(b, a)."""
    distance = TWED(0, 1)
    a = [0.0, 1.0, 1.0, 1.0, 2.0, 3.0]
    b = [0.0, 0.0, 1.0, 1.0, 1.0, 2.0, 3.0]
    assert distance(a, b) == distance(b, a)


def test_known_nonzero_distance_is_stable():
    """A single known, deterministic non-zero distance stays pinned."""
    distance = TWED(0, 1)
    a = [0.0, 1.0, 2.0, 3.0]
    b = [0.0, 1.0, 2.0, 4.0]
    assert distance(a, b) == 1.0


def test_empty_input_fails_clearly():
    """Empty sequences are outside the admitted domain and raise."""
    distance = TWED(0, 1)
    with pytest.raises(Exception):
        distance([], [])


def test_invalid_parameters_fail_clearly():
    """Non-metric parameters (elastic <= 0) are rejected by the native gate."""
    with pytest.raises(Exception):
        TWED(0, 0)
    with pytest.raises(Exception):
        TWED(-1, 1)
