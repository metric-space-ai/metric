import time
from metric.distance import EMD
import test_data
import sys


def main():
    img1 = test_data.img1[:10, :10]
    img2 = test_data.img2[:10, :10]
    rows, cols = img1.shape
    cost_mat = EMD.ground_distance_matrix_of_2dgrid(cols, rows)
    max_cost = EMD.max_in_distance_matrix(cost_mat)
    distance = EMD(cost_mat, max_cost)

    start_time = time.time()
    res = distance(img1.flatten(), img2.flatten())
    end_time = time.time()
    print("Multiscale graph correlation1: %d (Time = %06fms)" % (res, (end_time - start_time)*1000))


sys.exit(main())
