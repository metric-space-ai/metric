from metric import Space
from metric.operators import exact_knn_graph


GAP_COST = 2.0


def aligned_curve_distance(lhs, rhs):
    lhs = tuple(lhs)
    rhs = tuple(rhs)

    previous = [index * GAP_COST for index in range(len(rhs) + 1)]
    for lhs_index, lhs_value in enumerate(lhs, start=1):
        current = [lhs_index * GAP_COST] + [0.0] * len(rhs)
        for rhs_index, rhs_value in enumerate(rhs, start=1):
            substitute = previous[rhs_index - 1] + min(
                abs(lhs_value - rhs_value),
                2 * GAP_COST,
            )
            delete = previous[rhs_index] + GAP_COST
            insert = current[rhs_index - 1] + GAP_COST
            current[rhs_index] = min(substitute, delete, insert)
        previous = current

    return previous[-1]


def main():
    names = ["baseline", "shifted", "flat", "spike"]
    records = [
        (0, 1, 1, 1, 2, 3),
        (0, 0, 1, 1, 1, 2, 3),
        (2, 2, 2, 2, 2, 2),
        (0, 1, 6, 1, 2, 3),
    ]
    query = (0, 1, 1, 1, 2, 4)

    space = Space(records, aligned_curve_distance)
    nearest_id, nearest_distance = space.nearest(query)
    assert names[nearest_id] == "baseline"
    assert nearest_distance == 1.0

    representatives = space.representatives(2)
    assert representatives.strategy == "farthest_first"
    assert representatives.record_count == len(records)

    graph = exact_knn_graph(records, aligned_curve_distance, 1)
    assert graph.metadata.strategy == "exact_knn"
    assert graph.metadata.record_count == len(records)

    print("nearest process curve =", names[nearest_id], nearest_distance)
    print("process curve graph edges =", graph.metadata.edge_count)


if __name__ == "__main__":
    main()
