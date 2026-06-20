from metric import Edit, RuntimePolicy, Space


def main():
    records = ["metric", "metrics", "matrix", "tree"]
    space = Space(records, metric=Edit(), name="string_edit_representation_swap")

    query = "metricks"
    implicit_neighbors = space.neighbors(query, count=2)
    nearest = implicit_neighbors.neighbors[0]
    assert nearest.id == 1
    assert nearest.record == "metrics"
    assert nearest.distance == 1

    matrix = space.to_matrix()
    assert matrix.representation == "matrix"
    assert matrix.distance(0, 1) == 1
    assert matrix.distance(0, 2) == 2

    tree = space.to_tree()
    tree_neighbors = space.neighbors(query, count=2, representation=tree)
    assert tree_neighbors.as_tuples() == implicit_neighbors.as_tuples()

    graph = space.to_graph(count=2)
    graph_neighbors = space.neighbors(count=2, representation=graph)
    assert graph_neighbors.rows[0][0].id == 1
    assert graph_neighbors.rows[0][0].distance == 1
    assert len(graph_neighbors.rows[0]) == 2

    materialized_groups = space.groups(count=2, representation=matrix)
    assert materialized_groups.representation == "matrix"
    assert materialized_groups.cluster_count == 2

    diagnostics = space.runtime_diagnostics(
        representation=matrix,
        runtime=RuntimePolicy(exact=True, cache="materialized"),
        intent="representation_swap",
    )
    assert diagnostics.representation == "matrix"
    assert diagnostics.policy_name == "exact_materialized_serial"

    space.touch()
    assert matrix.is_stale()
    assert tree.is_stale()
    assert graph.is_stale()

    print("implicit nearest =", nearest.record, nearest.distance)
    print("tree nearest =", tree_neighbors.neighbors[0].record, tree_neighbors.neighbors[0].distance)
    print("graph neighbors from metric =", [neighbor.record for neighbor in graph_neighbors.rows[0]])
    print("runtime representation =", diagnostics.representation)


if __name__ == "__main__":
    main()
