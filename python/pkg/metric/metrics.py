"""Metric constructors exposed by the revived Python API.

The core wheel currently guarantees the edit-distance binding. Legacy standard
distance bindings and their compatibility aliases are re-exported when the full
extension set is available.
"""

from typing import Protocol, runtime_checkable

from metric.distance import Edit


@runtime_checkable
class Metric(Protocol):
    """Protocol for Python metric callables accepted by Space."""

    def __call__(self, lhs: object, rhs: object):
        ...


import metric.distance as _distance


def _optional_metric(name):
    return getattr(_distance, name, None)


Chebyshev = _optional_metric("Chebyshev")
Euclidean = _optional_metric("Euclidean")
Euclidean_thresholded = _optional_metric("Euclidean_thresholded")
Manhattan = _optional_metric("Manhattan")
Minkowski = _optional_metric("Minkowski")
P_norm = _optional_metric("P_norm")
ThresholdedEuclidean = _optional_metric("ThresholdedEuclidean")


def available():
    names = ["Edit"]
    for name, value in {
        "Euclidean": Euclidean,
        "Euclidean_thresholded": Euclidean_thresholded,
        "Manhattan": Manhattan,
        "Minkowski": Minkowski,
        "Chebyshev": Chebyshev,
        "P_norm": P_norm,
        "ThresholdedEuclidean": ThresholdedEuclidean,
    }.items():
        if value is not None:
            names.append(name)
    return tuple(names)


default = frozenset(name.lower() for name in available())

__all__ = [
    "Edit",
    "Euclidean",
    "Euclidean_thresholded",
    "Metric",
    "Manhattan",
    "Minkowski",
    "Chebyshev",
    "P_norm",
    "ThresholdedEuclidean",
    "available",
    "default",
]
