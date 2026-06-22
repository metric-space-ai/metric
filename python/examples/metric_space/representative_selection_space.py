"""Representative selection over a transport space.

Representative selection (farthest-first, medoid, separated, coverage) is a
METRIC algorithm. The Python facade delegates these selectors to the native C++
binding and returns deterministic representative IDs / records while keeping the
records in their original histogram form.
"""

from metric import (
    Space,
    coverage_representative_indices,
    coverage_representatives,
    medoid,
    medoid_index,
    representative_indices,
    representatives,
    separated_representative_indices,
    separated_representatives,
)


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

    space = Space(records, cumulative_transport_distance)

    # Adapter surface: explicit distances over a caller-provided metric.
    assert space.distance(0, 2) == 3.0
    print("records =", ", ".join(names))
    print("distance(left-edge, right-edge) =", space.distance(0, 2))

    metric = cumulative_transport_distance
    selected = representative_indices(records, metric, k=3)
    assert selected == (0, 2, 4)
    assert representatives(records, metric, k=3) == tuple(records[index] for index in selected)
    assert medoid_index(records, metric) == 1
    assert medoid(records, metric) == records[1]
    assert separated_representative_indices(records, metric, 1.5) == (0, 2, 4)
    assert separated_representatives(records, metric, 1.5) == (records[0], records[2], records[4])
    assert coverage_representative_indices(records, metric, 1.5) == (0, 2)
    assert coverage_representatives(records, metric, 1.5) == (records[0], records[2])

    print("farthest-first representatives =", [names[index] for index in selected])
    print("medoid =", names[medoid_index(records, metric)])


if __name__ == "__main__":
    main()
