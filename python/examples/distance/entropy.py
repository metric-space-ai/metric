import numpy
from metric.distance import entropy

aent = numpy.float_([
    [5.0, 5.0],
    [2.0, 2.0],
    [3.0, 3.0],
    [5.0, 1.0],
])

print("Entropies:")
for metric in ('chebyshev', 'p-norm', 'euclidean', 'manhatten'):
    res = entropy(aent, 3, 2.0, metric=metric)
    print(f'using {metric}: {res:.5f}')

res = entropy(aent)
print(f'using Default: {res:.5f}')
