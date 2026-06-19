"""Metric constructors exposed by the revived Python API.

The core wheel currently guarantees the edit-distance binding. Legacy standard
distance bindings are re-exported when the full extension set is available.
"""

from metric.distance import Edit

try:
    from metric.distance import Chebyshev, Euclidean, Manhattan, Manhatten, P_norm
except ImportError:
    Chebyshev = None
    Euclidean = None
    Manhattan = None
    Manhatten = None
    P_norm = None


def available():
    names = ["Edit"]
    for name, value in {
        "Euclidean": Euclidean,
        "Manhattan": Manhattan,
        "Manhatten": Manhatten,
        "Chebyshev": Chebyshev,
        "P_norm": P_norm,
    }.items():
        if value is not None:
            names.append(name)
    return tuple(names)


default = frozenset(name.lower() for name in available())

__all__ = [
    "Edit",
    "Euclidean",
    "Manhattan",
    "Manhatten",
    "Chebyshev",
    "P_norm",
    "available",
    "default",
]
