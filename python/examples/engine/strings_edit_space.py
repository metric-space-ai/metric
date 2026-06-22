"""Strings + edit-distance engine demo — native exact search.

The Python ``Space`` adapts string records over the native ``Edit`` binding and
exposes explicit distances, the matrix view, and exact-scan neighbor search.
Clustering runs through the native grouping binding.
"""

from metric import Edit, Space
from metric.strategies import KMedoids


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

    groups = space.groups(KMedoids(groups=2))
    assert groups.cluster_count == 2
    print("group medoids =", [records[index] for index in groups.medoids])


if __name__ == "__main__":
    main()
