"""Representation swap — adapter boundary demo.

Explicit representations are adapters: the matrix view materializes distances
through the native ``Edit`` binding, and the tree/graph handles carry source
lineage and staleness. Indexed/graph *search* and graph *construction* are
native-only METRIC algorithms, so those operations raise until a native binding
is exposed. Space-level exact neighbor search is promoted through the native
binding. The runtime-diagnostics view is a pure policy adapter.
"""

from metric import Edit, RuntimePolicy, Space
from metric.exceptions import StrategyUnavailableError


def requires_native(label, call):
    try:
        call()
    except StrategyUnavailableError:
        print(f"{label}: requires native C++ binding")
    else:
        raise AssertionError(f"{label} should require a native binding")


def main():
    records = ["metric", "metrics", "matrix", "tree"]
    space = Space(records, metric=Edit(), name="string_edit_representation_swap")

    # Adapter surface: the materialized matrix view exposes explicit distances.
    matrix = space.to_matrix()
    assert matrix.representation == "matrix"
    assert matrix.distance(0, 1) == 1
    assert matrix.distance(0, 2) == 2
    print("matrix distance(metric, metrics) =", matrix.distance(0, 1))

    # The tree index is a stored representation; its lineage/staleness is an
    # adapter, but its search is native-only.
    tree = space.to_tree()
    assert tree.representation == "exact_tree_index"
    requires_native("tree.knn", lambda: tree.knn("metricks", count=2))

    # Adapter surface: a runtime-policy view.
    diagnostics = space.runtime_diagnostics(
        representation=matrix,
        runtime=RuntimePolicy(exact=True, cache="materialized"),
        intent="representation_swap",
    )
    assert diagnostics.representation == "matrix"
    assert diagnostics.policy_name == "exact_materialized_serial"
    print("runtime representation =", diagnostics.representation)

    neighbors = space.neighbors("metricks", count=2)
    assert [neighbor.record for neighbor in neighbors.neighbors] == ["metrics", "metric"]
    print("neighbors(metricks) =", [neighbor.record for neighbor in neighbors.neighbors])

    groups = space.groups(count=2, representation=matrix)
    assert groups.cluster_count == 2
    print("group medoids =", [records[index] for index in groups.medoids])

    # Native boundary: graph construction is not promoted here.
    requires_native("to_graph", lambda: space.to_graph(count=2))

    # Staleness bookkeeping stays in Python (no algorithm involved).
    space.touch()
    assert matrix.is_stale()
    assert tree.is_stale()
    print("representations stale after touch =", matrix.is_stale(), tree.is_stale())


if __name__ == "__main__":
    main()
