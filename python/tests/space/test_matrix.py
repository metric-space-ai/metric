from metric.space import Matrix
import numpy


def test_create_empty():
    matrix = Matrix()
    assert len(matrix) == 0


def test_create_1d():
    data = numpy.float_([1.0, 2.0])
    matrix = Matrix(data)
    assert len(matrix) == 1


def test_create_2d():
    matrix = Matrix(numpy.float_([[1], [2], [3]]))
    assert len(matrix) == 3


def test_insert_1d():
    matrix = Matrix()
    id1 = matrix.insert(numpy.float_([1.0, 2.0]))
    assert len(matrix) == 1
    assert id1 == 0

    id2 = matrix.insert(numpy.float_([2.0, 3.0]))
    assert len(matrix) == 2
    assert id2 == 1


def test_insert_2d():
    matrix = Matrix()

    ids = list(matrix.insert(numpy.float_([[1], [2], [3], [4]])))
    assert len(matrix) == 4
    assert ids == [0, 1, 2, 3]

    ids = list(matrix.insert(numpy.float_([[1], [2], [3], [4]])))
    assert len(matrix) == 8
    assert ids == [4, 5, 6, 7]
