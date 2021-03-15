import metric._impl.koc as impl
from metric._impl.standards import Euclidean
from metric._impl.utils import distribution
from metric._common import factory_with_default


KOC = factory_with_default(
    metric=Euclidean(),
    distribution=distribution.Normal(-1, 1)
)(impl.KOC)
