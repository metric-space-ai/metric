import sys
import numpy
from metric.distance import Euclidean, Euclidean_thresholded, Manhatten, P_norm, Cosine, Chebyshev


def main():
    v0 = numpy.float_([0, 1, 1, 1, 1, 1, 2, 3])
    v1 = numpy.float_([1, 1, 1, 1, 1, 2, 3, 4])

    distance = Euclidean()
    print('Euclidian distance:', distance(v0, v1))

    distance = Euclidean_thresholded(thres=1000.0, factor=3000.0)
    print('Euclidian Threshold distance:', distance(v0, v1))

    distance = Manhatten()
    print('Manhatten distance:', distance(v0, v1))

    distance = P_norm(p=2)
    print('Minkowski (L general) Metric:', distance(v0, v1))

    distance = Cosine()
    print('Cosine distance:', distance(v0, v1))

    distance = Chebyshev()
    print('Chebyshev distance:', distance(v0, v1))


sys.exit(main())
