from metric import Edit, Space, mappings
from metric.strategies import KMedoids


def main():
    records = ["metric", "metrics", "matrix", "tree", "forest"]
    space = Space(records, metric=Edit(), name="string_clustered_mapping")

    groups = space.groups(KMedoids(groups=2))
    assert groups.algorithm == "kmedoids"
    assert groups.assignments == (0, 0, 0, 0, 1)
    assert groups.medoids == (0, 4)

    mapping = mappings.make_clustered_space_mapping(groups)
    model = mappings.fit(mapping, space)
    clustered = mappings.transform(model, space)
    direct = mappings.clustered_space(space, groups)

    assert clustered.mapping == "clustered_space"
    assert clustered.strategy == "kmedoids"
    assert clustered.source_record_count == len(records)
    assert clustered.target_record_count == groups.cluster_count
    assert clustered.source_records == ((0, 1, 2, 3), (4,))
    assert clustered.representative_records == groups.medoids
    assert direct.source_records == clustered.source_records

    cluster_space = clustered.space
    assert len(cluster_space) == 2
    assert cluster_space.records[0] == mappings.ClusterRecord(
        label=0,
        representative=0,
        members=(0, 1, 2, 3),
    )
    assert cluster_space.records[1].representative == 4
    assert cluster_space.distance(0, 1) == space.distance(groups.medoids[0], groups.medoids[1])

    print("cluster representatives =", [records[index] for index in clustered.representative_records])
    print("cluster member counts =", [len(members) for members in clustered.source_records])
    print("cluster distance =", cluster_space.distance(0, 1))


if __name__ == "__main__":
    main()
