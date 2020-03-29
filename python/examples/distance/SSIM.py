import sys
from metric.distance import SSIM
from test_data import img1, img2


def main():
    distance = SSIM()
    print(distance(img1, img2))
    print(distance(img2, img1))


sys.exit(main())
