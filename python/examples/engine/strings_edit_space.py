"""Strings + edit-distance engine demo — adapter boundary.

The Python ``Space`` adapts string records over the native ``Edit`` binding and
exposes explicit distances and the matrix view. Neighbor search and clustering
are native-only METRIC algorithms reached through bindings.
"""

from metric import Edit, Space
from metric.exceptions import StrategyUnavailableError
from metric.strategies import KMedoids


def requires_native(label, call):
    try:
        call()
    except StrategyUnavailableError:
        print(f"{label}: requires native C++ binding")
    else:
        raise AssertionError(f"{label} should require a native binding")


def main():
    records = ["metric", "metrics", "matrix", "tree"]
    space = Space(records, Edit())

    # Adapter surface: explicit edit distances and the matrix view.
    matrix = space.to_matrix()
    assert matrix.distance(0, 1) == 1
    print("distance(metric, metrics) =", matrix.distance(0, 1))

    # Native boundary: search and clustering live in C++.
    requires_native("neighbors", lambda: space.neighbors("metricks", 2))
    requires_native("groups", lambda: space.groups(KMedoids(groups=2)))


if __name__ == "__main__":
    main()
