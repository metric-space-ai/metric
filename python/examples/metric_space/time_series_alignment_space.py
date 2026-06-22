"""Time-series alignment space — exact search over process curves.

A toy edit-distance-style alignment metric over short process curves. The
Python ``Space`` adapts the records and exposes explicit distances plus native
exact-scan nearest search and native finite-space structure diagnostics.
"""

from metric import Space
from metric.operators import intrinsic_dimension, pairwise_distance_matrix


GAP_COST = 2.0


def aligned_curve_distance(lhs, rhs):
    """Edit-distance style alignment metric for short process curves."""
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

    # Adapter surface: explicit distances over a caller-provided metric.
    distances = pairwise_distance_matrix(records, aligned_curve_distance)
    assert distances[0][1] == 2.0
    assert distances[0][2] == 6.0
    print("records =", ", ".join(names))
    print("distance(baseline, shifted) =", distances[0][1])

    nearest = space.nearest(query)
    assert nearest.id == 0
    print("nearest(query) =", names[nearest.id], "distance =", nearest.distance)

    dimension = intrinsic_dimension(records, aligned_curve_distance)
    assert dimension >= 0.0
    print("intrinsic_dimension =", round(dimension, 6))


if __name__ == "__main__":
    main()
