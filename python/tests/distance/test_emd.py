from metric.distance import EMD
import numpy


def test__ground_distance_matrix_of_2dgrid():
    cols = 2
    rows = 2
    cost_mat = EMD.ground_distance_matrix_of_2dgrid(cols=cols, rows=rows)
    expected_matrix = numpy.array([[0, 1000, 1000, 1000],
                                   [1000, 0, 1000, 1000],
                                   [1000, 1000, 0, 1000],
                                   [1000, 1000, 1000, 0]])
    assert numpy.all(numpy.array(cost_mat) == expected_matrix)


def test__max_in_distance_matrix():
    cost_matrix = numpy.array([[1, 2, 3, 4],
                               [2, -30, 10, 1]])
    max_cost = EMD.max_in_distance_matrix(cost_matrix)

    assert max_cost == 10


def test__distance__symmetry():
    cost_matrix = EMD.ground_distance_matrix_of_2dgrid(3, 3)
    max_cost = EMD.max_in_distance_matrix(cost_matrix)
    vector_1 = numpy.array([0, 0, 0, 0, 0, 0, 0, 0, 0])
    vector_2 = numpy.array([0, 0, 0, 0, 1, 0, 0, 0, 0])
    distance = EMD(cost_matrix, max_cost)

    assert distance(vector_1, vector_2) == 1000
    assert distance(vector_1, vector_2) == 1000
