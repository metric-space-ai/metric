"""Small metric-space operators covered by the core Python smoke path."""

from dataclasses import dataclass
import math
import operator

from metric.spaces import FiniteMetricSpace


@dataclass(frozen=True)
class GraphConstructionMetadata:
    """Metadata for a promoted graph-construction result."""

    strategy: str
    record_count: int
    edge_count: int
    directed: bool
    self_loops: bool
    exact: bool
    k: object = None
    radius: object = None
    edge_payload: str = ""
    weighting: str = ""
    symmetrization: str = ""
    normalization: str = ""
    tie_break: str = ""
    max_out_degree: object = None
    sparsification: str = ""


@dataclass(frozen=True)
class GraphConstructionResult:
    """Graph construction result with directed edge tuples and metadata."""

    edges: tuple
    metadata: GraphConstructionMetadata


@dataclass(frozen=True)
class GraphDegreeDiagnostics:
    """Degree diagnostics for a graph construction result."""

    record_count: int
    edge_count: int
    directed: bool
    degrees: tuple
    out_degrees: tuple
    in_degrees: tuple
    isolated_count: int
    max_degree: int
    average_degree: float
    degree_policy: str


@dataclass(frozen=True)
class GraphConnectivityDiagnostics:
    """Connectivity diagnostics for a graph construction result."""

    record_count: int
    edge_count: int
    directed: bool
    component_labels: tuple
    component_count: int
    isolated_count: int
    largest_component_size: int
    connected: bool
    connectivity_policy: str


@dataclass(frozen=True)
class GraphStretchDiagnostics:
    """Shortest-path stretch diagnostics for a graph construction result."""

    record_count: int
    edge_count: int
    directed: bool
    pair_count: int
    reachable_pair_count: int
    unreachable_pair_count: int
    zero_metric_pair_count: int
    max_stretch: float
    average_stretch: float
    stretch_policy: str


def pairwise_distance_matrix(records, metric):
    return FiniteMetricSpace(records, metric).pairwise_distances()


def nearest_neighbors(records, metric, query, k=1):
    return FiniteMetricSpace(records, metric).knn(query, k)


def range_neighbors(records, metric, query, radius):
    return FiniteMetricSpace(records, metric).rnn(query, radius)


def _coerce_graph_k(k):
    try:
        return operator.index(k)
    except TypeError:
        raise TypeError("k must be an integer") from None


def _validate_graph_k(records, k):
    if k < 0:
        raise ValueError("k must be non-negative")
    if k == 0:
        return

    max_neighbors = max(0, len(records) - 1)
    if k > max_neighbors:
        raise ValueError("k cannot exceed the number of non-self neighbors")


def _build_exact_knn_graph_edges(records, metric, k):
    if k == 0:
        return []

    space = FiniteMetricSpace(records, metric)
    edges = []

    for source_index in range(len(records)):
        candidates = []
        for target_index in range(len(records)):
            if source_index == target_index:
                continue
            candidates.append((
                space.distance(source_index, target_index),
                target_index,
            ))

        candidates.sort()
        for distance, target_index in candidates[:k]:
            edges.append((source_index, target_index, distance))

    return edges


def exact_knn_graph(records, metric, k):
    """Build an exact directed kNN graph result with construction metadata."""
    records = list(records)
    k = _coerce_graph_k(k)
    _validate_graph_k(records, k)
    edges = _build_exact_knn_graph_edges(records, metric, k)

    return GraphConstructionResult(
        edges=tuple(edges),
        metadata=GraphConstructionMetadata(
            strategy="exact_knn",
            record_count=len(records),
            edge_count=len(edges),
            directed=True,
            self_loops=False,
            exact=True,
            k=k,
            edge_payload="metric_distance",
            weighting="none",
            sparsification="none",
            symmetrization="none",
            normalization="none",
            tie_break="distance_then_target_index",
        ),
    )


def exact_knn_graph_edges(records, metric, k):
    """Build exact directed kNN graph edges as source, target, distance tuples."""
    return list(exact_knn_graph(records, metric, k).edges)


def _build_exact_radius_graph_edges(records, metric, radius):
    if radius < 0:
        raise ValueError("radius must be non-negative")

    space = FiniteMetricSpace(records, metric)
    edges = []

    for source_index in range(len(records)):
        for target_index in range(len(records)):
            if source_index == target_index:
                continue
            distance = space.distance(source_index, target_index)
            if distance <= radius:
                edges.append((source_index, target_index, distance))

    return edges


def exact_radius_graph(records, metric, radius):
    """Build an exact directed radius graph result with construction metadata."""
    records = list(records)
    edges = _build_exact_radius_graph_edges(records, metric, radius)

    return GraphConstructionResult(
        edges=tuple(edges),
        metadata=GraphConstructionMetadata(
            strategy="exact_radius",
            record_count=len(records),
            edge_count=len(edges),
            directed=True,
            self_loops=False,
            exact=True,
            radius=radius,
            edge_payload="metric_distance",
            weighting="none",
            sparsification="none",
            symmetrization="none",
            normalization="none",
            tie_break="source_then_target_index",
        ),
    )


def exact_radius_graph_edges(records, metric, radius):
    """Build exact directed radius graph edges as source, target, distance tuples."""
    return list(exact_radius_graph(records, metric, radius).edges)


def _merge_graph_weight(lhs, rhs, weighting):
    if weighting == "minimum_distance":
        return min(lhs, rhs)
    if weighting == "maximum_distance":
        return max(lhs, rhs)
    raise ValueError("weighting policy must be 'minimum_distance' or 'maximum_distance'")


def symmetrize_graph(graph, policy="union", weighting="minimum_distance"):
    """Symmetrize a graph construction result with a documented merge policy."""
    if policy not in {"union", "mutual"}:
        raise ValueError("symmetrization policy must be 'union' or 'mutual'")
    if weighting not in {"minimum_distance", "maximum_distance"}:
        raise ValueError("weighting policy must be 'minimum_distance' or 'maximum_distance'")

    edge_accumulators = {}
    for source_index, target_index, distance in graph.edges:
        if source_index == target_index:
            continue

        lower_index = min(source_index, target_index)
        upper_index = max(source_index, target_index)
        accumulator = edge_accumulators.setdefault((lower_index, upper_index), [None, None])
        slot = 0 if source_index == lower_index else 1
        if accumulator[slot] is None:
            accumulator[slot] = distance
        else:
            accumulator[slot] = _merge_graph_weight(accumulator[slot], distance, weighting)

    edges = []
    for (source_index, target_index), (forward_distance, reverse_distance) in sorted(edge_accumulators.items()):
        has_forward = forward_distance is not None
        has_reverse = reverse_distance is not None
        if policy == "mutual" and not (has_forward and has_reverse):
            continue

        if has_forward and has_reverse:
            distance = _merge_graph_weight(forward_distance, reverse_distance, weighting)
        elif has_forward:
            distance = forward_distance
        else:
            distance = reverse_distance
        edges.append((source_index, target_index, distance))

    return GraphConstructionResult(
        edges=tuple(edges),
        metadata=GraphConstructionMetadata(
            strategy=graph.metadata.strategy,
            record_count=graph.metadata.record_count,
            edge_count=len(edges),
            directed=False,
            self_loops=False,
            exact=graph.metadata.exact,
            k=graph.metadata.k,
            radius=graph.metadata.radius,
            max_out_degree=graph.metadata.max_out_degree,
            edge_payload=graph.metadata.edge_payload,
            weighting=weighting,
            sparsification=graph.metadata.sparsification,
            symmetrization=policy,
            normalization=graph.metadata.normalization,
            tie_break="source_index_then_target_index",
        ),
    )


def prune_graph_out_degree(graph, max_out_degree):
    """Prune a directed graph result to at most max_out_degree edges per source."""
    try:
        max_out_degree = operator.index(max_out_degree)
    except TypeError:
        raise TypeError("max_out_degree must be an integer") from None

    if max_out_degree < 0:
        raise ValueError("max_out_degree must be non-negative")
    if not graph.metadata.directed:
        raise ValueError("out-degree pruning requires a directed graph result")

    if max_out_degree == 0:
        edges = ()
    else:
        edges_by_source = {}
        for edge in graph.edges:
            source_index, _target_index, _distance = edge
            edges_by_source.setdefault(source_index, []).append(edge)

        selected_edges = []
        for source_index in sorted(edges_by_source):
            source_edges = sorted(edges_by_source[source_index], key=lambda edge: (edge[2], edge[1]))
            selected_edges.extend(source_edges[:max_out_degree])
        edges = tuple(selected_edges)

    return GraphConstructionResult(
        edges=edges,
        metadata=GraphConstructionMetadata(
            strategy=graph.metadata.strategy,
            record_count=graph.metadata.record_count,
            edge_count=len(edges),
            directed=graph.metadata.directed,
            self_loops=graph.metadata.self_loops,
            exact=graph.metadata.exact,
            k=graph.metadata.k,
            radius=graph.metadata.radius,
            max_out_degree=max_out_degree,
            edge_payload=graph.metadata.edge_payload,
            weighting=graph.metadata.weighting,
            sparsification="out_degree",
            symmetrization=graph.metadata.symmetrization,
            normalization=graph.metadata.normalization,
            tie_break="source_index_then_distance_then_target_index",
        ),
    )


def graph_degree_diagnostics(graph):
    """Compute deterministic degree diagnostics for a graph construction result."""
    record_count = graph.metadata.record_count
    degrees = [0] * record_count
    out_degrees = [0] * record_count
    in_degrees = [0] * record_count

    for source_index, target_index, _distance in graph.edges:
        if (
            source_index < 0
            or target_index < 0
            or source_index >= record_count
            or target_index >= record_count
        ):
            raise ValueError("graph edge index exceeds metadata record_count")

        if graph.metadata.directed:
            out_degrees[source_index] += 1
            in_degrees[target_index] += 1
            degrees[source_index] += 1
            degrees[target_index] += 1
        else:
            degrees[source_index] += 1
            degrees[target_index] += 1

    isolated_count = sum(1 for degree in degrees if degree == 0)
    max_degree = max(degrees, default=0)
    average_degree = sum(degrees) / record_count if record_count else 0.0
    degree_policy = "directed_in_out" if graph.metadata.directed else "undirected_endpoint"

    return GraphDegreeDiagnostics(
        record_count=record_count,
        edge_count=len(graph.edges),
        directed=graph.metadata.directed,
        degrees=tuple(degrees),
        out_degrees=tuple(out_degrees),
        in_degrees=tuple(in_degrees),
        isolated_count=isolated_count,
        max_degree=max_degree,
        average_degree=average_degree,
        degree_policy=degree_policy,
    )


def graph_connectivity_diagnostics(graph):
    """Compute deterministic connectivity diagnostics for a graph construction result."""
    record_count = graph.metadata.record_count
    parents = list(range(record_count))
    has_incident_edge = [False] * record_count

    def find_root(index):
        while parents[index] != index:
            parents[index] = parents[parents[index]]
            index = parents[index]
        return index

    def union_components(lhs, rhs):
        lhs_root = find_root(lhs)
        rhs_root = find_root(rhs)
        if lhs_root == rhs_root:
            return
        if lhs_root < rhs_root:
            parents[rhs_root] = lhs_root
        else:
            parents[lhs_root] = rhs_root

    for source_index, target_index, _distance in graph.edges:
        if (
            source_index < 0
            or target_index < 0
            or source_index >= record_count
            or target_index >= record_count
        ):
            raise ValueError("graph edge index exceeds metadata record_count")

        has_incident_edge[source_index] = True
        has_incident_edge[target_index] = True
        union_components(source_index, target_index)

    root_labels = {}
    component_labels = [0] * record_count
    component_sizes = []
    isolated_count = 0
    for index in range(record_count):
        root = find_root(index)
        if root not in root_labels:
            root_labels[root] = len(root_labels)
            component_sizes.append(0)

        label = root_labels[root]
        component_labels[index] = label
        component_sizes[label] += 1
        if not has_incident_edge[index]:
            isolated_count += 1

    component_count = len(component_sizes)
    connectivity_policy = (
        "weak_undirected_reachability"
        if graph.metadata.directed
        else "undirected_reachability"
    )

    return GraphConnectivityDiagnostics(
        record_count=record_count,
        edge_count=len(graph.edges),
        directed=graph.metadata.directed,
        component_labels=tuple(component_labels),
        component_count=component_count,
        isolated_count=isolated_count,
        largest_component_size=max(component_sizes, default=0),
        connected=component_count <= 1,
        connectivity_policy=connectivity_policy,
    )


def graph_stretch_diagnostics(records, metric, graph):
    """Compute deterministic shortest-path stretch diagnostics for a graph result."""
    records = list(records)
    record_count = graph.metadata.record_count
    if len(records) != record_count:
        raise ValueError("graph metadata record_count must match records size")

    shortest_paths = [
        [math.inf] * record_count
        for _index in range(record_count)
    ]
    for index in range(record_count):
        shortest_paths[index][index] = 0.0

    for source_index, target_index, edge_distance in graph.edges:
        if (
            source_index < 0
            or target_index < 0
            or source_index >= record_count
            or target_index >= record_count
        ):
            raise ValueError("graph edge index exceeds metadata record_count")

        edge_distance = float(edge_distance)
        shortest_paths[source_index][target_index] = min(
            shortest_paths[source_index][target_index],
            edge_distance,
        )
        if not graph.metadata.directed:
            shortest_paths[target_index][source_index] = min(
                shortest_paths[target_index][source_index],
                edge_distance,
            )

    for through in range(record_count):
        for source_index in range(record_count):
            if math.isinf(shortest_paths[source_index][through]):
                continue
            for target_index in range(record_count):
                if math.isinf(shortest_paths[through][target_index]):
                    continue
                shortest_paths[source_index][target_index] = min(
                    shortest_paths[source_index][target_index],
                    shortest_paths[source_index][through] + shortest_paths[through][target_index],
                )

    space = FiniteMetricSpace(records, metric)
    pair_count = 0
    reachable_pair_count = 0
    unreachable_pair_count = 0
    zero_metric_pair_count = 0
    max_stretch = 0.0
    total_stretch = 0.0

    def evaluate_pair(source_index, target_index):
        nonlocal pair_count
        nonlocal reachable_pair_count
        nonlocal unreachable_pair_count
        nonlocal zero_metric_pair_count
        nonlocal max_stretch
        nonlocal total_stretch

        metric_distance = float(space.distance(source_index, target_index))
        if metric_distance == 0.0:
            zero_metric_pair_count += 1
            return

        pair_count += 1
        path_distance = shortest_paths[source_index][target_index]
        if math.isinf(path_distance):
            unreachable_pair_count += 1
            return

        stretch = path_distance / metric_distance
        reachable_pair_count += 1
        total_stretch += stretch
        max_stretch = max(max_stretch, stretch)

    if graph.metadata.directed:
        for source_index in range(record_count):
            for target_index in range(record_count):
                if source_index != target_index:
                    evaluate_pair(source_index, target_index)
    else:
        for source_index in range(record_count):
            for target_index in range(source_index + 1, record_count):
                evaluate_pair(source_index, target_index)

    stretch_policy = (
        "directed_shortest_path"
        if graph.metadata.directed
        else "undirected_shortest_path"
    )
    average_stretch = total_stretch / reachable_pair_count if reachable_pair_count else 0.0

    return GraphStretchDiagnostics(
        record_count=record_count,
        edge_count=len(graph.edges),
        directed=graph.metadata.directed,
        pair_count=pair_count,
        reachable_pair_count=reachable_pair_count,
        unreachable_pair_count=unreachable_pair_count,
        zero_metric_pair_count=zero_metric_pair_count,
        max_stretch=max_stretch,
        average_stretch=average_stretch,
        stretch_policy=stretch_policy,
    )


def representative_indices(records, metric, k, seed_index=0):
    """Select representative record IDs with deterministic farthest-first traversal."""
    records = list(records)
    try:
        k = operator.index(k)
        seed_index = operator.index(seed_index)
    except TypeError:
        raise TypeError("k and seed_index must be integers") from None

    if k < 0:
        raise ValueError("k must be non-negative")
    if k == 0:
        return []
    if not records:
        raise ValueError("cannot select representatives from an empty record set")
    if k > len(records):
        raise ValueError("k cannot exceed the number of records")
    if seed_index < 0 or seed_index >= len(records):
        raise IndexError("seed_index is outside the record set")

    space = FiniteMetricSpace(records, metric)
    selected = [seed_index]
    selected_set = {seed_index}
    nearest_selected_distances = [
        space.distance(index, seed_index)
        for index in range(len(records))
    ]

    while len(selected) < k:
        next_index = None
        next_distance = None
        for index, distance in enumerate(nearest_selected_distances):
            if index in selected_set:
                continue
            if next_distance is None or distance > next_distance:
                next_index = index
                next_distance = distance

        if next_index is None:
            raise RuntimeError("failed to select the next representative")

        selected.append(next_index)
        selected_set.add(next_index)
        for index in range(len(records)):
            nearest_selected_distances[index] = min(
                nearest_selected_distances[index],
                space.distance(index, next_index),
            )

    return selected


def representatives(records, metric, k, seed_index=0):
    """Select representative records with deterministic farthest-first traversal."""
    records = list(records)
    return [
        records[index]
        for index in representative_indices(records, metric, k, seed_index)
    ]


def medoid_index(records, metric):
    """Select the record ID with the smallest total distance to all records."""
    records = list(records)
    if not records:
        raise ValueError("cannot select a medoid from an empty record set")

    space = FiniteMetricSpace(records, metric)
    best_index = 0
    best_total_distance = None

    for candidate_index in range(len(records)):
        total_distance = sum(
            space.distance(candidate_index, other_index)
            for other_index in range(len(records))
        )
        if best_total_distance is None or total_distance < best_total_distance:
            best_index = candidate_index
            best_total_distance = total_distance

    return best_index


def medoid(records, metric):
    """Select the record with the smallest total distance to all records."""
    records = list(records)
    return records[medoid_index(records, metric)]


def separated_representative_indices(records, metric, minimum_distance):
    """Select representatives greedily separated by a minimum distance."""
    records = list(records)
    if minimum_distance < 0:
        raise ValueError("minimum_distance must be non-negative")
    if not records:
        return []

    space = FiniteMetricSpace(records, metric)
    selected = []

    for candidate_index in range(len(records)):
        if all(
            space.distance(candidate_index, selected_index) >= minimum_distance
            for selected_index in selected
        ):
            selected.append(candidate_index)

    return selected


def separated_representatives(records, metric, minimum_distance):
    """Select representative records greedily separated by a minimum distance."""
    records = list(records)
    return [
        records[index]
        for index in separated_representative_indices(records, metric, minimum_distance)
    ]


def coverage_representative_indices(records, metric, radius):
    """Select representatives that greedily cover records within a radius."""
    records = list(records)
    if radius < 0:
        raise ValueError("radius must be non-negative")
    if not records:
        return []

    space = FiniteMetricSpace(records, metric)
    selected = []
    covered = [False] * len(records)

    while not all(covered):
        seed_index = next(index for index, is_covered in enumerate(covered) if not is_covered)
        selected.append(seed_index)

        for index in range(len(records)):
            if space.distance(seed_index, index) <= radius:
                covered[index] = True

    return selected


def coverage_representatives(records, metric, radius):
    """Select representative records that greedily cover records within a radius."""
    records = list(records)
    return [
        records[index]
        for index in coverage_representative_indices(records, metric, radius)
    ]


def intrinsic_dimension(records, metric):
    """Estimate expansion dimension from finite metric-space distance growth."""
    return intrinsic_dimension_from_distances(pairwise_distance_matrix(records, metric))


def intrinsic_dimension_from_distances(distances):
    maximum_dimension = 0.0
    for row in distances:
        for radius in row:
            if radius <= 0:
                continue

            inner_count = sum(1 for distance in row if distance <= radius)
            outer_count = sum(1 for distance in row if distance <= radius * 2)
            if inner_count and outer_count >= inner_count:
                maximum_dimension = max(maximum_dimension, math.log2(outer_count / inner_count))
    return maximum_dimension


__all__ = [
    "GraphConnectivityDiagnostics",
    "GraphDegreeDiagnostics",
    "GraphStretchDiagnostics",
    "GraphConstructionMetadata",
    "GraphConstructionResult",
    "graph_connectivity_diagnostics",
    "graph_degree_diagnostics",
    "graph_stretch_diagnostics",
    "intrinsic_dimension",
    "intrinsic_dimension_from_distances",
    "coverage_representative_indices",
    "coverage_representatives",
    "exact_knn_graph",
    "exact_knn_graph_edges",
    "exact_radius_graph",
    "exact_radius_graph_edges",
    "medoid",
    "medoid_index",
    "pairwise_distance_matrix",
    "prune_graph_out_degree",
    "nearest_neighbors",
    "range_neighbors",
    "representative_indices",
    "representatives",
    "separated_representative_indices",
    "separated_representatives",
    "symmetrize_graph",
]
