import sys
from metric.distance import SSIM
import numpy


def main():
    img1 = numpy.loadtxt('fixtures/img1.csv')
    img2 = numpy.loadtxt('fixtures/img1.csv')
    distance = SSIM()
    print(distance(img1, img2))
    print(distance(img2, img1))


sys.exit(main())
