import sys
import numpy
import metric


def main():
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])

    distance = metric.Euclidian()
    print('Euclidian distance:', distance(v0, v1))

    distance = metric.Euclidian_thresholded(thres_=1000.0, factor_=3000.0)
    print('Euclidian Threshold distance:', distance(v0, v1))

    distance = metric.Manhatten()
    print('Manhatten distance:', distance(v0, v1))

    distance = metric.P_norm(p_=2)
    print('Minkowski (L general) Metric:', distance(v0, v1))

    distance = metric.Cosine()
    print('Cosine distance:', distance(v0, v1))


sys.exit(main())