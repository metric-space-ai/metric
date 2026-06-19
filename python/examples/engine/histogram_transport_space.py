from metric import Space
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
    nearest = space.neighbors(query, 2)
    assert nearest == [(1, 0.25), (3, 0.25)]

    groups = space.groups(KMedoids(groups=2))
    assert groups.algorithm == "kmedoids"
    assert groups.cluster_count == 2

    print("nearest histograms =", names[nearest[0][0]], names[nearest[1][0]])
    print("histogram groups =", groups.cluster_count)


if __name__ == "__main__":
    main()
