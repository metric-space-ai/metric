import operator

from metric.exceptions import OptionalDependencyError

try:
    import metric._impl.mgc as impl
except ModuleNotFoundError as exc:
    if exc.name is not None and not exc.name.startswith("metric._impl"):
        # A genuinely unrelated missing dependency (e.g. a transitive import
        # inside the native binding) must surface unchanged.
        raise
    raise OptionalDependencyError(
        "native correlation binding metric._impl.mgc is unavailable; "
        "correlation is an adapter boundary until the native binding is promoted"
    ) from None

from metric.distance import Euclidean
from ._budget import budget_overrides, require_low_level_exact_budget


class _BudgetedMGC:
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
        lhs,
        rhs,
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
        lhs, rhs = require_low_level_exact_budget(
            "MGC(...)",
            lhs,
            rhs,
            distance_multiplier=2,
            memory_multiplier=2,
            **budgets,
        )
        return self._native(lhs, rhs, *args, **kwargs)

    def xcorr(
        self,
        lhs,
        rhs,
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
        lhs, rhs = require_low_level_exact_budget(
            "MGC.xcorr(...)",
            lhs,
            rhs,
            distance_multiplier=2 * _xcorr_window_count(args, kwargs),
            memory_multiplier=2,
            **budgets,
        )
        return self._native.xcorr(lhs, rhs, *args, **kwargs)


def _xcorr_window_count(args, kwargs):
    value = kwargs.get("n")
    if value is None and args:
        value = args[0]
    if value is None:
        return 1
    try:
        radius = operator.index(value)
    except TypeError:
        return 1
    if radius < 0:
        return 1
    return 2 * radius + 1


def MGC(
    metric1=Euclidean(),
    metric2=Euclidean(),
    *,
    runtime=None,
    max_memory_bytes=None,
    max_distance_evaluations=None,
    max_dense_records=None,
) -> object:
    """ Multiscale Graph Correlation factory

    :param metric1: metric for first container
    :param metric2: metric for second container
    :return: instance of MGC
    """
    return _BudgetedMGC(
        impl.create_mgc(metric1=metric1, metric2=metric2),
        runtime=runtime,
        max_memory_bytes=max_memory_bytes,
        max_distance_evaluations=max_distance_evaluations,
        max_dense_records=max_dense_records,
    )


_native_MGC_direct = impl.MGC_direct


def MGC_direct(
    lhs,
    rhs,
    *args,
    runtime=None,
    max_memory_bytes=None,
    max_distance_evaluations=None,
    max_dense_records=None,
    **kwargs,
):
    lhs, rhs = require_low_level_exact_budget(
        "MGC_direct(...)",
        lhs,
        rhs,
        runtime=runtime,
        max_memory_bytes=max_memory_bytes,
        max_distance_evaluations=max_distance_evaluations,
        max_dense_records=max_dense_records,
        distance_multiplier=2,
        memory_multiplier=2,
    )
    return _native_MGC_direct(lhs, rhs, *args, **kwargs)
