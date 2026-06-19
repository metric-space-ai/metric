from metric import (
    Space,
    coverage_representative_indices,
    coverage_representatives,
    representative_indices,
    representatives,
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
    selected = representative_indices(records, cumulative_transport_distance, k=3)
    selected_records = representatives(records, cumulative_transport_distance, k=3)
    covered = coverage_representative_indices(records, cumulative_transport_distance, radius=1.5)
    covered_records = coverage_representatives(records, cumulative_transport_distance, radius=1.5)

    assert selected == [0, 2, 4]
    assert selected_records == [records[0], records[2], records[4]]
    assert covered == [0, 2]
    assert covered_records == [records[0], records[2]]
    assert space.distance(selected[0], selected[1]) == 3.0

    print("representative histograms =", ", ".join(names[index] for index in selected))
    print("radius-cover histograms =", ", ".join(names[index] for index in covered))
    print("farthest seed distance =", space.distance(selected[0], selected[1]))


if __name__ == "__main__":
    main()
