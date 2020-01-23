#!/usr/bin/python3
import metric
import time
import numpy

Aent = numpy.float_([
    [ 5.0 ,5.0 ],
    [ 2.0, 2.0 ],
    [ 3.0, 3.0 ],
    [ 5.0 ,1.0 ],
])


#res = metric.entropy(Aent, 3, 2.0, "chebyshev");
#print("entropy using Chebyshev: %06f" % (res));

#res = metric.entropy(Aent, 3, 2.0, "p_norm", 3);
#print("entropy using General Minkowsky, 3: %06f" % (res));

#res = metric.entropy(Aent, 3, 2.0, "p_norm", 2);
#print("entropy using General Minkowsky, 2: %06f" % (res));

res = metric.entropy(Aent, 3, 2.0, "euclidean");
print("entropy using Euclidean: %06f" % (res));

#res = metric.entropy(Aent, 3, 2.0, "p_norm", 1);
#print("entropy using General Minkowsky, 1: %06f" % (res));

res = metric.entropy(Aent, 3, 2.0, "manhatten");
print("entropy using Manhatten: %06f" % (res));

#res = metric.entropy(Aent);
#print("entropy using Default: %06f" % (res));

