"""Histogram transport engine demo — exact search over histograms.

The Python ``Space`` adapts histogram records over a toy transport metric and
exposes explicit distances plus native exact-scan neighbor search. Clustering
remains native-only until its binding is promoted.
"""

from metric import Space
from metric.exceptions import StrategyUnavailableError
from metric.operators import pairwise_distance_matrix
from metric.strategies import KMedoids


def cumulative_transport_distance(lhs, rhs):
    if len(lhs) != len(rhs):
        raise ValueError("histograms must have the same number of bins")

    cumulative_delta = 0.0
    distance = 0.0
    for lhs_mass, rhs_mass in zip(lhs, rhs):
        cumulative_delta += lhs_mass - rhs_mass
        distance += abs(cumulative_delta)

    return distance


def requires_native(label, call):
    try:
        call()
    except StrategyUnavailableError:
        print(f"{label}: requires native C++ binding")
    else:
        raise AssertionError(f"{label} should require a native binding")


def main():
    names = ["left-edge", "one-step", "right-edge", "split-left", "center"]
    records = [
        (1.0, 0.0, 0.0, 0.0),
        (0.0, 1.0, 0.0, 0.0),
        (0.0, 0.0, 0.0, 1.0),
        (0.5, 0.5, 0.0, 0.0),
        (0.0, 0.5, 0.5, 0.0),
    ]
    query = (0.25, 0.75, 0.0, 0.0)

    space = Space(records, cumulative_transport_distance)

    # Adapter surface: explicit pairwise distances.
    matrix = pairwise_distance_matrix(records, cumulative_transport_distance)
    print("records =", ", ".join(names))
    print("matrix rows =", len(matrix))

    neighbors = space.neighbors(query, 2)
    assert [neighbor.id for neighbor in neighbors.neighbors] == [1, 3]
    print("neighbors(query) =", [names[neighbor.id] for neighbor in neighbors.neighbors])

    # Native boundary: clustering lives in C++ but is not promoted here.
    requires_native("groups", lambda: space.groups(KMedoids(groups=2)))


if __name__ == "__main__":
    main()
