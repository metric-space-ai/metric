"""Core finite metric-space building blocks for the Python engine facade."""

from metric.exceptions import (
    MetricError,
    MissingMetricError,
    StaleRepresentationError,
    UnsupportedOperationError,
)
from metric.metrics import Edit, Metric
from metric.runtime import CachePolicy, RuntimeDiagnostics, RuntimePolicy, runtime_diagnostics
from metric.spaces import FiniteMetricSpace, MatrixSpace, Space


def make_space(records, metric=None, **kwargs):
    """Construct a finite metric Space from records and an explicit metric."""
    return Space(records, metric=metric, **kwargs)


__all__ = [
    "CachePolicy",
    "Edit",
    "FiniteMetricSpace",
    "MatrixSpace",
    "Metric",
    "MetricError",
    "MissingMetricError",
    "RuntimeDiagnostics",
    "RuntimePolicy",
    "Space",
    "StaleRepresentationError",
    "UnsupportedOperationError",
    "make_space",
    "runtime_diagnostics",
]
