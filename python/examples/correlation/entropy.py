import numpy
from metric.correlation import Entropy
from metric.distance import Euclidean, P_norm, Manhattan

aent = numpy.float_([
    [5.0, 5.0],
    [2.0, 2.0],
    [3.0, 3.0],
    [5.0, 1.0],
])

print("Entropies:")
for metric in (Euclidean, P_norm, Manhattan):
    res = Entropy(metric=metric(), p=3, k=2)(aent)
    print(f'using {metric}: {res:.5f}')

res = Entropy(p=3, k=2)(aent)
print(f'using Default: {res:.5f}')
