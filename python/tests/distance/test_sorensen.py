try:
    from metric.distance import Sorensen
except ImportError:
    import pytest
    pytest.skip("Sorensen requires the FULL Python build", allow_module_level=True)
import numpy


def test_ok():
    vt0, vt1, vt2, vt3 = 0, 1, 2, 3
    distance = Sorensen()

    a = numpy.float64([vt0, vt1, vt2, vt0])
    b = numpy.float64([vt0, vt1, vt3])
    assert distance(a, b) == 0.14285714285714285
    assert distance(b, a) == 0.14285714285714285

    a = numpy.float64([vt0, vt1, vt2, vt0])
    b = numpy.float64([vt0, vt1, vt3, vt0])
    assert distance(a, b) == 0.14285714285714285
    assert distance(b, a) == 0.14285714285714285
