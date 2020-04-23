import numpy
import os
from metric.distance import SSIM


CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
img1 = numpy.loadtxt(os.path.join(CURRENT_DIR, 'fixtures/img1.csv'))
img2 = numpy.loadtxt(os.path.join(CURRENT_DIR, 'fixtures/img2.csv'))


def test_distance():
    distance = SSIM()
    assert distance(img1, img2) == 0.09074577584588275
    assert distance(img2, img1) == 0.09074577584588275


def test_config_defaults():
    distance = SSIM()

    assert distance.dynamic_range == 255
    assert distance.masking == 2


def test_config_override():
    distance = SSIM(dynamic_range=1, masking=20)
    assert distance.dynamic_range == 1
    assert distance.masking == 20
