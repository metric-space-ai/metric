import numpy
from metric.distance import entropy

Aent = numpy.float_([
    [5.0, 5.0],
    [2.0, 2.0],
    [3.0, 3.0],
    [5.0, 1.0],
])


res = entropy(Aent, 3, 2.0, metric="chebyshev")
print("entropy using Chebyshev: ", res)

res = entropy(Aent, 3, 2.0, "p-norm")
print("entropy using General Minkowsky: ", res)

res = entropy(Aent, 3, 2.0, metric="euclidean")
print("entropy using Euclidean: ", res)

res = entropy(Aent, 3, 2.0, metric="manhatten")
print("entropy using Manhatten: ", res)

res = entropy(Aent)
print("entropy using Default: ", res)
