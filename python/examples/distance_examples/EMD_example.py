#!/usr/bin/python3
import time
import numpy
import metric
from test_data import img1,img2
import sys


im1_R = int(img1.shape[0]/6);
im1_C = int(img1[0].shape[0]/6);
cost_mat = metric.EMD_details.ground_distance_matrix_of_2dgrid(im1_C, im1_R);
maxCost = metric.EMD_details.max_in_distance_matrix(cost_mat);
distance = metric.EMD(cost_mat, maxCost);

Av = numpy.int_([])
Bv = numpy.int_([])
for i in range(0, im1_R):
    for j in range(0, im1_C):
        Av = numpy.append(Av, img1[i][j])
        Bv = numpy.append(Bv, img2[i][j])

start_time = time.time()
res = distance(Av,Bv)
end_time = time.time()
print("Multiscale graph correlation1: %d (Time = %06fms)" % (res, (end_time - start_time)*1000) );


