import metric._impl.entropy as impl
from metric._impl.standards import Euclidean
from metric._common import factory_with_default


Entropy = factory_with_default(metric=Euclidean())(impl.Entropy)
