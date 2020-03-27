import numpy
from metric.distance import VOI_kl, VOI_normalized

v1 = numpy.float_([
    [5.0, 5.0],
    [2.0, 2.0],
    [3.0, 3.0],
    [5.0, 5.0],
])

v2 = numpy.float_([
    [5.0, 5.0],
    [2.0, 2.0],
    [3.0, 3.0],
    [1.0, 1.0],
])

result = VOI_kl()(v1, v2)
print(f'KL VOI: {result:.5f}')

result = VOI_normalized()(v1, v2)
print(f'VOI normalized: {result:.5f}')
