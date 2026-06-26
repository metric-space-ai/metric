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
from ._budget import budget_overrides, require_low_level_exact_budget


_native_entropy_factory = factory_with_default(metric=Euclidean())(impl.Entropy)


class _BudgetedEntropy:
    def __init__(
        self,
        native,
        *,
        runtime=None,
        max_memory_bytes=None,
        max_distance_evaluations=None,
        max_dense_records=None,
    ):
        self._native = native
        self._budget_defaults = (
            runtime,
            max_memory_bytes,
            max_distance_evaluations,
            max_dense_records,
        )

    def __getattr__(self, name):
        return getattr(self._native, name)

    def __call__(
        self,
        records,
        *args,
        runtime=None,
        max_memory_bytes=None,
        max_distance_evaluations=None,
        max_dense_records=None,
        **kwargs,
    ):
        budgets = budget_overrides(
            self._budget_defaults,
            runtime,
            max_memory_bytes,
            max_distance_evaluations,
            max_dense_records,
        )
        (records,) = require_low_level_exact_budget("Entropy(...)", records, **budgets)
        return self._native(records, *args, **kwargs)

    def estimate(
        self,
        records,
        *args,
        runtime=None,
        max_memory_bytes=None,
        max_distance_evaluations=None,
        max_dense_records=None,
        **kwargs,
    ):
        budgets = budget_overrides(
            self._budget_defaults,
            runtime,
            max_memory_bytes,
            max_distance_evaluations,
            max_dense_records,
        )
        (records,) = require_low_level_exact_budget("Entropy.estimate(...)", records, **budgets)
        return self._native.estimate(records, *args, **kwargs)


def Entropy(
    *args,
    runtime=None,
    max_memory_bytes=None,
    max_distance_evaluations=None,
    max_dense_records=None,
    **kwargs,
):
    return _BudgetedEntropy(
        _native_entropy_factory(*args, **kwargs),
        runtime=runtime,
        max_memory_bytes=max_memory_bytes,
        max_distance_evaluations=max_distance_evaluations,
        max_dense_records=max_dense_records,
    )
