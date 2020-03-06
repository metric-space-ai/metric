from metric.space import Matrix
import numpy


def test_2d():
    data = numpy.float_([
        [0, 0, 0, 0, 0],
        [1.74120000000000, 4.07812000000000, -0.0927036000000, 41.7888000000000, 41.7888000000000],
        [7.75309000000000, 16.2466000000000, 3.03956000000000, 186.074000000000, 186.074000000000],
        [2.85493000000000, 3.25380000000000, 2.50559000000000, 68.5184000000000, 68.5184000000000],
        [5.81414000000000, 8.14015000000000, 3.22950000000000, 139.539000000000, 139.539000000000],
        [2.57927000000000, 2.63399000000000, 2.46802000000000, 61.9026000000000, 61.9026000000000]
    ])

    matrix = Matrix(data)

    assert len(matrix) == 6
    assert matrix(1, 3) == 37.9158821105957


def test_create():
    matrix = Matrix()
    assert len(matrix) == 0

    matrix = Matrix(numpy.float_([1.0]))
    assert len(matrix) == 1

    matrix = Matrix(numpy.float_([1], [2], [3]))
    assert len(matrix) == 3


def test_insert():
    matrix = Matrix()
    id1 = matrix.insert(numpy.float_([1.0]))
    assert len(matrix) == 1
    assert id1 == 0

    ids = list(matrix.insert(numpy.float_([1, 2, 3, 4])))
    assert len(matrix) == 5
    assert ids == [1, 2, 3, 4]
