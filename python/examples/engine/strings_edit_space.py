"""Strings + edit-distance engine demo — native exact search.

The Python ``Space`` adapts string records over the native ``Edit`` binding and
exposes explicit distances, the matrix view, and exact-scan neighbor search.
Clustering remains native-only until its binding is promoted.
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

    neighbors = space.neighbors("metricks", 2)
    assert [neighbor.record for neighbor in neighbors.neighbors] == ["metrics", "metric"]
    print("neighbors(metricks) =", [neighbor.record for neighbor in neighbors.neighbors])

    # Native boundary: clustering lives in C++ but is not promoted here.
    requires_native("groups", lambda: space.groups(KMedoids(groups=2)))


if __name__ == "__main__":
    main()
