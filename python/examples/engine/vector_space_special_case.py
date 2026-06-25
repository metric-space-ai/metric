"""Vector space special case — adapter boundary demo.

Aligned vector rows over a custom Euclidean metric. The Python ``Space`` adapts
the records, exposes explicit distances and the matrix view, and reports a
runtime-policy view. Search, clustering, outlier detection, graph construction,
and classical-MDS embedding are native algorithms reached through bindings.
Space-level exact neighbor search, clustering, and outliers are promoted here;
graph construction and MDS remain unavailable until their bindings are promoted.
(Native vector mappings such as parametric diffusion coordinates are exercised by the core test suite,
which pins the native binding contract.)
"""

from math import sqrt

from metric import RuntimePolicy, Space
from metric.exceptions import StrategyUnavailableError
from metric.operators import pairwise_distance_matrix
from metric.strategies import DBSCAN, KMedoids, MDS


def euclidean(lhs, rhs):
    return sqrt(sum((left - right) ** 2 for left, right in zip(lhs, rhs)))


def requires_native(label, call):
    try:
        call()
    except StrategyUnavailableError:
        print(f"{label}: requires native C++ binding")
    else:
        raise AssertionError(f"{label} should require a native binding")


def main():
    names = [
        "alpha-0", "alpha-1", "alpha-2",
        "beta-0", "beta-1", "beta-2",
        "gamma-0", "gamma-1", "outlier",
    ]
    records = [
        (0.0, 0.0), (0.3, 0.2), (0.2, -0.1),
        (5.0, 5.0), (5.2, 4.9), (4.8, 5.1),
        (-4.0, 4.0), (-4.2, 4.1), (10.0, -5.0),
    ]

    metadata = {
        "record_kind": "aligned_vector",
        "metric_law": "metric",
        "dimensions": len(records[0]),
        "representations": ("metric_space", "matrix", "exact_tree_index", "exact_knn_graph"),
    }
    space = Space(records, metric=euclidean, name="vector_space_special_case", metadata=metadata)

    # Adapter surface: metadata, explicit distances, and the matrix view.
    assert space.metadata["record_kind"] == "aligned_vector"
    matrix = space.to_matrix()
    assert round(matrix.distance(0, 1), 12) == round(sqrt(0.13), 12)
    print("record_kind =", space.metadata["record_kind"])
    print("matrix distance(alpha-0, alpha-1) =", round(matrix.distance(0, 1), 3))
    print("matrix rows =", len(pairwise_distance_matrix(records, euclidean)))

    diagnostics = space.runtime_diagnostics(
        runtime=RuntimePolicy(exact=True, cache="lazy"),
        intent="neighbors",
    )
    assert diagnostics.representation == "metric_space"
    print("runtime policy =", diagnostics.policy_name, "via", diagnostics.representation)

    neighbors = space.neighbors((0.1, 0.1), count=3)
    assert [neighbor.id for neighbor in neighbors.neighbors] == [0, 1, 2]
    print("neighbors((0.1, 0.1)) =", [names[neighbor.id] for neighbor in neighbors.neighbors])

    groups = space.groups(strategy=KMedoids(groups=3), representation=matrix)
    assert groups.cluster_count == 3
    print("group medoids =", [names[index] for index in groups.medoids])

    outliers = space.outliers(strategy=DBSCAN(radius=0.7, min_points=2), representation=matrix)
    assert [outlier.record_id for outlier in outliers.outliers] == [8]
    print("outliers =", [names[outlier.record_id] for outlier in outliers.outliers])

    # Native boundary: indexed search / graph / MDS embedding.
    requires_native("to_tree.knn", lambda: space.to_tree().knn((0.1, 0.1), count=3))
    requires_native("to_graph", lambda: space.to_graph(count=2))
    requires_native("embed", lambda: space.embed(strategy=MDS(dimensions=2), representation=matrix))


if __name__ == "__main__":
    main()
