from metric import Edit, RuntimePolicy, Space


def main():
    records = ["metric", "metrics", "matrix", "tree"]
    space = Space(records, metric=Edit(), name="string_edit_representation_swap")

    query = "metricks"
    implicit_neighbors = space.neighbors(query, count=2)
    assert implicit_neighbors[0] == (1, 1)

    matrix = space.to_matrix()
    assert matrix.representation == "matrix"
    assert matrix.distance(0, 1) == 1
    assert matrix.distance(0, 2) == 2

    tree = space.to_tree()
    tree_neighbors = tree.neighbors(query, count=2)
    assert tree_neighbors == implicit_neighbors

    graph = space.to_graph(count=2)
    graph_neighbors = graph.neighbors(0)
    assert graph_neighbors[0] == (1, 1)
    assert len(graph_neighbors) == 2

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

    print("implicit nearest =", records[implicit_neighbors[0][0]], implicit_neighbors[0][1])
    print("tree nearest =", records[tree_neighbors[0][0]], tree_neighbors[0][1])
    print("graph neighbors from metric =", [records[index] for index, _distance in graph_neighbors])
    print("runtime representation =", diagnostics.representation)


if __name__ == "__main__":
    main()
