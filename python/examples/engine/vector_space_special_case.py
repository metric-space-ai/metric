from math import sqrt

from metric import RuntimePolicy, Space
from metric.strategies import DBSCAN, KMedoids, MDS


def euclidean(lhs, rhs):
    return sqrt(sum((left - right) ** 2 for left, right in zip(lhs, rhs)))


def main():
    names = [
        "alpha-0",
        "alpha-1",
        "alpha-2",
        "beta-0",
        "beta-1",
        "beta-2",
        "gamma-0",
        "gamma-1",
        "outlier",
    ]
    records = [
        (0.0, 0.0),
        (0.3, 0.2),
        (0.2, -0.1),
        (5.0, 5.0),
        (5.2, 4.9),
        (4.8, 5.1),
        (-4.0, 4.0),
        (-4.2, 4.1),
        (10.0, -5.0),
    ]
    query = (0.1, 0.1)

    metadata = {
        "record_kind": "aligned_vector",
        "metric_law": "metric",
        "dimensions": len(records[0]),
        "representations": ("metric_space", "matrix", "exact_tree_index", "exact_knn_graph"),
    }
    space = Space(records, metric=euclidean, name="vector_space_special_case", metadata=metadata)
    assert space.metadata["record_kind"] == "aligned_vector"
    assert space.metadata["metric_law"] == "metric"
    assert space.metadata["dimensions"] == 2

    matrix = space.to_matrix()
    assert round(matrix.distance(0, 1), 12) == round(sqrt(0.13), 12)

    implicit_neighbors = space.neighbors(query, count=3)
    assert names[implicit_neighbors[0][0]] == "alpha-0"

    tree = space.to_tree()
    tree_neighbors = tree.neighbors(query, count=3)
    assert tree_neighbors == implicit_neighbors

    graph = space.to_graph(count=2)
    graph_neighbors = graph.neighbors(0)
    assert len(graph_neighbors) == 2
    assert names[graph_neighbors[0][0]].startswith("alpha")

    groups = space.groups(strategy=KMedoids(groups=3), representation=matrix)
    assert groups.algorithm == "kmedoids"
    assert groups.cluster_count == 3
    assert groups.representation == "matrix"

    outliers = space.outliers(strategy=DBSCAN(radius=0.7, min_points=2), representation=matrix)
    assert outliers.strategy == "dbscan_noise"
    assert outliers.noise_count == 1
    assert names[outliers.outliers[0].record_id] == "outlier"

    embedding = space.embed(strategy=MDS(dimensions=2), representation=matrix)
    assert embedding.strategy == "classic_mds"
    assert embedding.dimensions == 2
    assert embedding.coordinates.shape == (len(records), 2)

    diagnostics = space.runtime_diagnostics(
        representation=matrix,
        runtime=RuntimePolicy(exact=True, cache="materialized"),
        intent="vector_space_special_case",
    )
    assert diagnostics.representation == "matrix"

    exact_diagnostics = space.runtime_diagnostics(
        runtime=RuntimePolicy(exact=True, cache="lazy"),
        intent="neighbors",
    )
    tree_diagnostics = space.runtime_diagnostics(
        representation=tree,
        runtime=RuntimePolicy(exact=True, cache="lazy"),
        intent="neighbors",
    )
    assert exact_diagnostics.representation == "metric_space"
    assert tree_diagnostics.representation == "exact_tree_index"

    print("record_kind =", space.metadata["record_kind"])
    print("metric_law =", space.metadata["metric_law"])
    print("vector dimension metadata =", space.metadata["dimensions"])
    print("available representations =", ",".join(space.metadata["representations"]))
    print("nearest vector =", names[implicit_neighbors[0][0]], round(implicit_neighbors[0][1], 3))
    print("indexed nearest =", names[tree_neighbors[0][0]], "via", tree.representation)
    print("runtime policy =", exact_diagnostics.policy_name, "via", exact_diagnostics.representation)
    print("matrix cache reuse =", groups.representation, outliers.representation)
    print("vector groups =", groups.cluster_count)
    print("vector outlier =", names[outliers.outliers[0].record_id], "score", round(outliers.outliers[0].score, 3))
    print("mds embedding dimensions =", embedding.dimensions)


if __name__ == "__main__":
    main()
