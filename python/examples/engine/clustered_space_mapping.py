"""Clustered-space mapping — adapter boundary demo.

``mappings.clustered_space`` is a marshaling adapter: given a clustering
*result* (which a native clustering binding would produce) it derives a
cluster-level metric Space using only adapter-level ``space.distance`` lookups.
The clustering itself (``space.groups``) is a native-only METRIC algorithm, so
this demo constructs the clustering result explicitly and then exercises the
marshaling adapter.
"""

from metric import Edit, Space, mappings
from metric.exceptions import StrategyUnavailableError
from metric.operators import ClusteringResult
from metric.strategies import KMedoids


def main():
    records = ["metric", "metrics", "matrix", "tree", "forest"]
    space = Space(records, metric=Edit(), name="string_clustered_mapping")

    # Clustering is a native-only algorithm in the adapter-only package.
    try:
        space.groups(KMedoids(groups=2))
    except StrategyUnavailableError:
        print("groups: requires native C++ binding")
    else:
        raise AssertionError("groups should require a native binding")

    # A native clustering binding would return a result of this shape. We build
    # it explicitly here so we can demonstrate the marshaling adapter offline.
    groups = ClusteringResult(
        assignments=(0, 0, 0, 0, 1),
        medoids=(0, 4),
        core_records=(),
        noise_records=(),
        cluster_sizes=(4, 1),
        record_count=len(records),
        cluster_count=2,
        noise_count=0,
        iterations=2,
        converged=True,
        algorithm="kmedoids",
        representation="metric_space",
    )

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
