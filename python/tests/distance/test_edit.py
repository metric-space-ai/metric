from metric.distance import Edit
import numpy


def test_close_strings():
    str1 = "1011001100110011001111111"
    str2 = "1000011001100110011011100"

    distance = Edit()

    assert distance(str1, str2) == 5


def test_different_strings():
    str1 = "1011001100110011001111111"
    str2 = "Absolutely different string"

    distance = Edit()

    assert distance(str1, str2) == 27


def test_array():
    a = numpy.array([-0.991021875880222, -0.768275252129114, -0.526359355330172,
                     -0.318170691552235, -0.0468979315641954, 0.227289495956795, 0.317841938040349,
                     0.463820792484335, 0.660623198153296, 0.812210713528849])

    b = numpy.array([2.36088477872717, 1.23271042331569, 0.219758852423591,
                     0.0129766138306992, 0.00923506810444738, 1.49393468371558e-07, 0.00619896971968280,
                     0.212322021636953, 0.257245700714104, 1.59223791395715])

    distance = Edit()

    assert distance(a, b) == 10
