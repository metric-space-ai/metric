import sys
import numpy
from metric.distance import EMD
from metric.utils.misc import time_n_log
import test_data


def main():
    rows, cols = test_data.img1.shape
    rows //= 6
    cols //= 6
    img1 = test_data.img1[:rows, :cols]
    img2 = test_data.img2[:rows, :cols]
    print(f'size {rows} x {cols}')
    cost_mat = EMD.ground_distance_matrix_of_2dgrid(cols=cols, rows=rows)
    max_cost = EMD.max_in_distance_matrix(cost_mat)
    distance = EMD(cost_mat, max_cost)

    time_n_log(distance=lambda: distance(img1.flatten(), img2.flatten()))()
    print('swap records and calculate again')
    time_n_log(distance=lambda: distance(img2.flatten(), img1.flatten()))()

    cost_mat = EMD.ground_distance_matrix_of_2dgrid(3, 3)
    max_cost = EMD.max_in_distance_matrix(cost_mat)
    distance = EMD(cost_mat, max_cost)
    vector_1 = numpy.int_([0, 0, 0, 0, 0, 0, 0, 0, 0])
    vector_2 = numpy.int_([0, 0, 0, 0, 1, 0, 0, 0, 0])
    time_n_log(distance=lambda: distance(vector_1, vector_2))()


sys.exit(main())
