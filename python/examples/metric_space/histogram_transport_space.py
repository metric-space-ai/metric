from metric import Space, intrinsic_dimension
from metric.operators import pairwise_distance_matrix


def cumulative_transport_distance(lhs, rhs):
    """One-dimensional earth mover distance for equal-mass histograms."""
    if len(lhs) != len(rhs):
        raise ValueError("histograms must have the same number of bins")

    cumulative_delta = 0.0
    distance = 0.0
    for lhs_mass, rhs_mass in zip(lhs, rhs):
        cumulative_delta += lhs_mass - rhs_mass
        distance += abs(cumulative_delta)

    return distance


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
    nearest_id, nearest_distance = space.nearest(query)
    distances = pairwise_distance_matrix(records, cumulative_transport_distance)
    dimension = intrinsic_dimension(records, cumulative_transport_distance)

    assert names[nearest_id] == "one-step"
    assert nearest_distance == 0.25
    assert distances[0][1] == 1.0
    assert distances[0][2] == 3.0
    assert distances[3][4] == 1.0
    assert dimension > 0.0

    print("nearest histogram =", names[nearest_id], nearest_distance)
    print("distance(left-edge, right-edge) =", distances[0][2])
    print("intrinsic dimension estimate =", round(dimension, 3))


if __name__ == "__main__":
    main()
