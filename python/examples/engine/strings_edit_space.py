from metric import Edit, Space
from metric.strategies import KMedoids


def main():
    records = ["metric", "metrics", "matrix", "tree"]
    space = Space(records, Edit())

    matrix = space.to_matrix()
    assert matrix.distance(0, 1) == 1

    neighbors = space.neighbors("metricks", 2)
    assert neighbors[0] == (1, 1)

    groups = space.groups(KMedoids(groups=2))
    assert groups.algorithm == "kmedoids"
    assert groups.cluster_count == 2

    print("nearest string =", records[neighbors[0][0]], neighbors[0][1])
    print("string groups =", groups.cluster_count)


if __name__ == "__main__":
    main()
