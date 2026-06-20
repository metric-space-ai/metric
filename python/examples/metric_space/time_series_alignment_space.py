from metric import Space, intrinsic_dimension
from metric.operators import pairwise_distance_matrix


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
    nearest = space.nearest(query)
    distances = pairwise_distance_matrix(records, aligned_curve_distance)
    dimension = intrinsic_dimension(records, aligned_curve_distance)

    assert names[nearest.id] == "baseline"
    assert nearest.distance == 1.0
    assert distances[0][1] == 2.0
    assert distances[0][2] == 6.0
    assert dimension > 0.0

    print("nearest process curve =", names[nearest.id], nearest.distance)
    print("distance(baseline, shifted) =", distances[0][1])
    print("intrinsic dimension estimate =", round(dimension, 3))


if __name__ == "__main__":
    main()
