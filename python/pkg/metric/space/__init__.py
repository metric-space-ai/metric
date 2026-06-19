try:
    from metric._impl.space import *
except ModuleNotFoundError:
    pass

from metric.spaces import FiniteMetricSpace, MatrixSpace
from .matrix import Matrix
