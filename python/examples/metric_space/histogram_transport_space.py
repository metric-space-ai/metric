"""Histogram transport space — finite records with a transport metric.

This demo builds an explicit finite metric space over a toy 1-D transport
metric. The Python layer adapts the records and invokes the caller's metric to
expose distances, the explicit pairwise matrix, and native exact-scan search.
Structure analysis such as intrinsic dimension runs through the native binding.
"""

from metric import Space
from metric.operators import intrinsic_dimension, pairwise_distance_matrix


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

    # Adapter surface: explicit distances over a caller-provided metric.
    distances = pairwise_distance_matrix(records, cumulative_transport_distance)
    assert space.distance(0, 1) == 1.0
    assert distances[0][2] == 3.0
    assert distances[3][4] == 1.0

    print("records =", ", ".join(names))
    print("distance(left-edge, right-edge) =", distances[0][2])
    print("distance(split-left, center) =", distances[3][4])

    nearest = space.nearest(query)
    assert nearest.id == 1
    print("nearest(query) =", names[nearest.id], "distance =", nearest.distance)

    dimension = intrinsic_dimension(records, cumulative_transport_distance)
    assert dimension >= 0.0
    print("intrinsic_dimension =", round(dimension, 6))


if __name__ == "__main__":
    main()
