from metric.distance import RandomEMD
import numpy as np 
import math


def test__constructor():
    d1 = np.array(range(11))
    d2 = np.array(range(1, 12))
    remd = RandomEMD()

    print(remd(d1, d2))
    assert(math.isclose(remd(d1, d2), 1, rel_tol=0.01))
