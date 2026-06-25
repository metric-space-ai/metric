"""Native entropy estimator adapter.

This module is not the promoted Python finite-space pipeline API. The C++
framework-level entropy path is a coordinate-space operator reached after an
explicit map/embed step. ``Entropy`` here is a thin adapter over
``metric._impl.entropy`` for full native builds and should be read
as a low-level estimator surface over coordinate-like records.
"""

from metric.exceptions import OptionalDependencyError

try:
    import metric._impl.entropy as impl
except ModuleNotFoundError as exc:
    if exc.name is not None and not exc.name.startswith("metric._impl"):
        # A genuinely unrelated missing dependency (e.g. a transitive import
        # inside the native binding) must surface unchanged.
        raise
    raise OptionalDependencyError(
        "native correlation binding metric._impl.entropy is unavailable; "
        "correlation is an adapter boundary until the native binding is promoted"
    ) from None

from metric.distance import Euclidean
from metric._common import factory_with_default


Entropy = factory_with_default(metric=Euclidean())(impl.Entropy)
