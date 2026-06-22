"""Process-curve engine demo — adapter boundary.

A toy alignment metric over short process curves. The Python ``Space`` adapts
the records and exposes explicit distances. Nearest search, representative
selection, and kNN-graph construction are native-only METRIC algorithms.
"""

from metric import Space
from metric.exceptions import StrategyUnavailableError
from metric.operators import exact_knn_graph, pairwise_distance_matrix


GAP_COST = 2.0


def aligned_curve_distance(lhs, rhs):
    lhs = tuple(lhs)
    rhs = tuple(rhs)

    previous = [index * GAP_COST for index in range(len(rhs) + 1)]
    for lhs_index, lhs_value in enumerate(lhs, start=1):
        current = [lhs_index * GAP_COST] + [0.0] * len(rhs)
        for rhs_index, rhs_value in enumerate(rhs, start=1):
            substitute = previous[rhs_index - 1] + min(
                abs(lhs_value - rhs_value),
                2 * GAP_COST,
            )
            delete = previous[rhs_index] + GAP_COST
            insert = current[rhs_index - 1] + GAP_COST
            current[rhs_index] = min(substitute, delete, insert)
        previous = current

    return previous[-1]


def requires_native(label, call):
    try:
        call()
    except StrategyUnavailableError:
        print(f"{label}: requires native C++ binding")
    else:
        raise AssertionError(f"{label} should require a native binding")


def main():
    names = ["baseline", "shifted", "flat", "spike"]
    records = [
        (0, 1, 1, 1, 2, 3),
        (0, 0, 1, 1, 1, 2, 3),
        (2, 2, 2, 2, 2, 2),
        (0, 1, 6, 1, 2, 3),
    ]
    query = (0, 1, 1, 1, 2, 4)

    space = Space(records, aligned_curve_distance)

    # Adapter surface: explicit distances over the alignment metric.
    matrix = pairwise_distance_matrix(records, aligned_curve_distance)
    print("records =", ", ".join(names))
    print("matrix rows =", len(matrix))

    # Native boundary: search / selection / graph construction live in C++.
    requires_native("nearest", lambda: space.nearest(query))
    requires_native("representatives", lambda: space.representatives(2))
    requires_native("exact_knn_graph", lambda: exact_knn_graph(records, aligned_curve_distance, 1))


if __name__ == "__main__":
    main()
