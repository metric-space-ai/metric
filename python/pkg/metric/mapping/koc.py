import metric._impl.koc as impl
from metric._impl.distance import Euclidean
from metric._common import factory_with_default


KOC = factory_with_default(metric=Euclidean())(impl.KOC)
