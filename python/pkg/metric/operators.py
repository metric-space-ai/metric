"""Small metric-space operators covered by the core Python smoke path."""

from dataclasses import dataclass
import math
import operator
from typing import ClassVar

import numpy as np

from metric.spaces import FiniteMetricSpace
from metric.strategies import ClassicMDS, DBSCAN, DistanceProfileCorrelation, FarthestFirst, KMedoids


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


@dataclass(frozen=True)
class ClusteringResult:
    """Engine-style grouping result with assignments and cluster metadata."""

    noise_label: ClassVar[int] = -1

    assignments: tuple
    medoids: tuple
    core_records: tuple
    noise_records: tuple
    cluster_sizes: tuple
    record_count: int
    cluster_count: int
    noise_count: int
    iterations: int
    converged: bool
    algorithm: str
    representation: str


@dataclass(frozen=True)
class Outlier:
    """One outlier record ID with a deterministic isolation score."""

    record_id: int
    score: object


@dataclass(frozen=True)
class OutlierResult:
    """Engine-style outlier result with strategy and representation metadata."""

    outliers: tuple
    record_count: int
    cluster_count: int
    noise_count: int
    exact: bool
    operator_name: str
    strategy: str
    representation: str


@dataclass(frozen=True)
class RepresentativeSet:
    """Representative-selection result with coverage diagnostics."""

    representatives: tuple
    nearest_representative_distances: tuple
    record_count: int
    requested_count: int
    coverage_radius: object
    average_nearest_distance: float
    exact: bool
    strategy: str
    representation: str


@dataclass(frozen=True)
class ReductionResult:
    """Reduced metric-space result with source-record lineage."""

    space: object
    source_record_ids: tuple
    assignments: tuple
    nearest_representative_distances: tuple
    source_record_count: int
    reduced_record_count: int
    exact: bool
    operator_name: str
    strategy: str
    representation: str
    inverse_supported: bool


@dataclass(frozen=True)
class CompressionResult:
    """Compressed metric-space result with source-record lineage."""

    space: object
    source_record_ids: tuple
    assignments: tuple
    nearest_representative_distances: tuple
    source_record_count: int
    compressed_record_count: int
    compression_ratio: float
    exact: bool
    operator_name: str
    compression: str
    strategy: str
    representation: str
    lossy: bool
    inverse_supported: bool


@dataclass(frozen=True)
class MappingResult:
    """Mapped metric-space result with source-to-target lineage."""

    space: object
    source_record_ids: tuple
    source_record_count: int
    target_record_count: int
    exact: bool
    operator_name: str
    mapping: str
    strategy: str
    representation: str
    inverse_supported: bool


@dataclass(frozen=True)
class StructureDescription:
    """Exact finite-space structure diagnostics."""

    record_count: int
    pair_count: int
    zero_distance_pair_count: int
    minimum_nonzero_distance: object
    maximum_distance: object
    average_distance: float
    intrinsic_dimension: float
    has_nonzero_distances: bool
    exact: bool
    strategy: str
    representation: str


@dataclass(frozen=True)
class CorrelationResult:
    """Cross-space dependency result with explicit strategy metadata."""

    value: float
    left_record_count: int
    right_record_count: int
    pair_count: int
    exact: bool
    algorithm: str
    strategy: str
    left_representation: str
    right_representation: str


@dataclass(frozen=True)
class EmbeddingDiagnostics:
    """Quality diagnostics for a metric-space embedding."""

    raw_stress: float
    normalized_stress: float
    distance_correlation: float
    trustworthiness: float
    neighbor_k: int
    finite_coordinates: bool
    coordinate_scale: float


@dataclass(frozen=True)
class EmbeddingModel:
    """Metadata for a deterministic embedding model."""

    method: str
    dimensions: int
    source_record_ids: tuple


@dataclass(frozen=True)
class EmbeddingResult:
    """Derived coordinate view of a finite metric space."""

    coordinates: object
    embedded_space: object
    source_space: object
    model: EmbeddingModel
    source_record_ids: tuple
    source_record_count: int
    dimensions: int
    stress: float
    trustworthiness: float
    exact: bool
    operator_name: str
    strategy: str
    representation: str
    diagnostics: EmbeddingDiagnostics


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


def _coerce_positive_integer(value, name):
    try:
        value = operator.index(value)
    except TypeError:
        raise TypeError(f"{name} must be an integer") from None
    if value <= 0:
        raise ValueError(f"{name} must be positive")
    return value


def _coerce_non_negative_integer(value, name):
    try:
        value = operator.index(value)
    except TypeError:
        raise TypeError(f"{name} must be an integer") from None
    if value < 0:
        raise ValueError(f"{name} must be non-negative")
    return value


def _total_distance_to_all(space, candidate_index):
    return sum(
        space.distance(candidate_index, other_index)
        for other_index in range(len(space.records))
    )


def _nearest_medoid_distance(space, medoids, record_index):
    best_distance = None
    for medoid_index in medoids:
        distance = space.distance(record_index, medoid_index)
        if best_distance is None or distance < best_distance:
            best_distance = distance
    return best_distance


def _initialize_medoids(space, group_count):
    medoids = []
    best_index = 0
    best_total = None
    for candidate_index in range(len(space.records)):
        total = _total_distance_to_all(space, candidate_index)
        if best_total is None or total < best_total:
            best_index = candidate_index
            best_total = total

    medoids.append(best_index)
    selected = {best_index}

    while len(medoids) < group_count:
        next_index = None
        next_distance = None
        for candidate_index in range(len(space.records)):
            if candidate_index in selected:
                continue
            distance = _nearest_medoid_distance(space, medoids, candidate_index)
            if (
                next_distance is None
                or distance > next_distance
                or (distance == next_distance and candidate_index < next_index)
            ):
                next_index = candidate_index
                next_distance = distance

        if next_index is None:
            raise RuntimeError("failed to initialize k-medoids")

        medoids.append(next_index)
        selected.add(next_index)

    return sorted(medoids)


def _assign_to_medoids(space, medoids):
    assignments = [0] * len(space.records)
    cluster_sizes = [0] * len(medoids)

    for record_index in range(len(space.records)):
        best_cluster = 0
        best_distance = None
        for cluster_index, medoid_index in enumerate(medoids):
            distance = space.distance(record_index, medoid_index)
            if (
                best_distance is None
                or distance < best_distance
                or (distance == best_distance and medoid_index < medoids[best_cluster])
            ):
                best_cluster = cluster_index
                best_distance = distance

        assignments[record_index] = best_cluster
        cluster_sizes[best_cluster] += 1

    return assignments, cluster_sizes


def _recompute_medoids(space, assignments, cluster_sizes, current_medoids):
    medoids = []
    for cluster_index, cluster_size in enumerate(cluster_sizes):
        if cluster_size == 0:
            medoids.append(current_medoids[cluster_index])
            continue

        best_index = None
        best_total = None
        for candidate_index, assignment in enumerate(assignments):
            if assignment != cluster_index:
                continue
            total = sum(
                space.distance(candidate_index, other_index)
                for other_index, other_assignment in enumerate(assignments)
                if other_assignment == cluster_index
            )
            if best_total is None or total < best_total or (total == best_total and candidate_index < best_index):
                best_index = candidate_index
                best_total = total

        medoids.append(best_index)

    return sorted(medoids)


def _compute_cluster_medoids(space, assignments, cluster_count):
    medoids = []
    for cluster_index in range(cluster_count):
        best_index = None
        best_total = None
        for candidate_index, assignment in enumerate(assignments):
            if assignment != cluster_index:
                continue
            total = sum(
                space.distance(candidate_index, other_index)
                for other_index, other_assignment in enumerate(assignments)
                if other_assignment == cluster_index
            )
            if best_total is None or total < best_total or (total == best_total and candidate_index < best_index):
                best_index = candidate_index
                best_total = total
        if best_index is not None:
            medoids.append(best_index)
    return medoids


def kmedoids(records, metric, groups, max_iterations=100):
    """Group records with deterministic k-medoids over exact pairwise distances."""
    records = list(records)
    if not records:
        raise ValueError("cannot cluster an empty record set")

    group_count = _coerce_positive_integer(groups, "groups")
    max_iterations = _coerce_non_negative_integer(max_iterations, "max_iterations")
    if group_count > len(records):
        raise ValueError("groups cannot exceed the number of records")

    space = FiniteMetricSpace(records, metric)
    medoids = _initialize_medoids(space, group_count)
    assignments, cluster_sizes = _assign_to_medoids(space, medoids)

    iterations = 0
    converged = False
    for iteration in range(max_iterations):
        updated_medoids = _recompute_medoids(space, assignments, cluster_sizes, medoids)
        updated_assignments, updated_cluster_sizes = _assign_to_medoids(space, updated_medoids)
        if updated_medoids == medoids and updated_assignments == assignments:
            medoids = updated_medoids
            assignments = updated_assignments
            cluster_sizes = updated_cluster_sizes
            iterations = iteration + 1
            converged = True
            break

        medoids = updated_medoids
        assignments = updated_assignments
        cluster_sizes = updated_cluster_sizes
        iterations = iteration + 1

    return ClusteringResult(
        assignments=tuple(assignments),
        medoids=tuple(medoids),
        core_records=(),
        noise_records=(),
        cluster_sizes=tuple(cluster_sizes),
        record_count=len(records),
        cluster_count=group_count,
        noise_count=0,
        iterations=iterations,
        converged=converged,
        algorithm="kmedoids",
        representation="metric_space",
    )


def _validate_dbscan_parameters(radius, min_points):
    if radius < 0:
        raise ValueError("radius must be non-negative")
    return _coerce_positive_integer(min_points, "min_points")


def _dbscan_region_query(space, record_index, radius):
    return [
        candidate_index
        for candidate_index in range(len(space.records))
        if space.distance(record_index, candidate_index) <= radius
    ]


def _expand_dbscan_cluster(space, seed_neighbors, radius, min_points, cluster, visited, assigned, assignments, core_flags):
    queue = list(seed_neighbors)
    queued = [False] * len(space.records)
    for neighbor_index in seed_neighbors:
        queued[neighbor_index] = True

    cursor = 0
    while cursor < len(queue):
        candidate_index = queue[cursor]
        cursor += 1

        if not visited[candidate_index]:
            visited[candidate_index] = True
            neighbors = _dbscan_region_query(space, candidate_index, radius)
            if len(neighbors) >= min_points:
                core_flags[candidate_index] = True
                for neighbor_index in neighbors:
                    if not queued[neighbor_index]:
                        queue.append(neighbor_index)
                        queued[neighbor_index] = True

        if not assigned[candidate_index]:
            assignments[candidate_index] = cluster
            assigned[candidate_index] = True


def dbscan(records, metric, radius, min_points):
    """Group records with deterministic DBSCAN over exact pairwise distances."""
    records = list(records)
    if not records:
        raise ValueError("cannot cluster an empty record set")

    min_points = _validate_dbscan_parameters(radius, min_points)
    space = FiniteMetricSpace(records, metric)
    assignments = [ClusteringResult.noise_label] * len(records)
    assigned = [False] * len(records)
    visited = [False] * len(records)
    core_flags = [False] * len(records)

    cluster_count = 0
    for record_index in range(len(records)):
        if visited[record_index]:
            continue

        visited[record_index] = True
        neighbors = _dbscan_region_query(space, record_index, radius)
        if len(neighbors) < min_points:
            continue

        core_flags[record_index] = True
        _expand_dbscan_cluster(
            space,
            neighbors,
            radius,
            min_points,
            cluster_count,
            visited,
            assigned,
            assignments,
            core_flags,
        )
        cluster_count += 1

    cluster_sizes = [0] * cluster_count
    noise_records = []
    noise_count = 0
    for record_index in range(len(records)):
        if assigned[record_index]:
            cluster_sizes[assignments[record_index]] += 1
        else:
            assignments[record_index] = ClusteringResult.noise_label
            noise_records.append(record_index)
            noise_count += 1

    return ClusteringResult(
        assignments=tuple(assignments),
        medoids=tuple(_compute_cluster_medoids(space, assignments, cluster_count)),
        core_records=tuple(index for index, is_core in enumerate(core_flags) if is_core),
        noise_records=tuple(noise_records),
        cluster_sizes=tuple(cluster_sizes),
        record_count=len(records),
        cluster_count=cluster_count,
        noise_count=noise_count,
        iterations=1,
        converged=True,
        algorithm="dbscan",
        representation="metric_space",
    )


def _coerce_grouping_strategy(strategy):
    if isinstance(strategy, KMedoids):
        return strategy
    if isinstance(strategy, DBSCAN):
        return strategy
    if hasattr(strategy, "groups"):
        return KMedoids(
            groups=strategy.groups,
            max_iterations=getattr(strategy, "max_iterations", 100),
        )
    if hasattr(strategy, "radius") and hasattr(strategy, "min_points"):
        return DBSCAN(radius=strategy.radius, min_points=strategy.min_points)
    try:
        return KMedoids(groups=operator.index(strategy))
    except TypeError:
        raise TypeError("strategy must be a KMedoids, DBSCAN, or integer group count") from None


def find_groups(records, metric, strategy):
    """Group records and return an engine-style result object."""
    strategy = _coerce_grouping_strategy(strategy)
    if isinstance(strategy, KMedoids):
        return kmedoids(records, metric, strategy.groups, strategy.max_iterations)
    if isinstance(strategy, DBSCAN):
        return dbscan(records, metric, strategy.radius, strategy.min_points)
    raise TypeError("unsupported grouping strategy")


def _coerce_outlier_strategy(strategy):
    if isinstance(strategy, DBSCAN):
        return strategy
    if hasattr(strategy, "radius") and hasattr(strategy, "min_points"):
        return DBSCAN(radius=strategy.radius, min_points=strategy.min_points)
    raise TypeError("strategy must be a DBSCAN strategy")


def _nearest_reference_distance(space, record_index, references):
    if references:
        return min(space.distance(record_index, reference_index) for reference_index in references)

    candidates = [
        candidate_index
        for candidate_index in range(len(space.records))
        if candidate_index != record_index
    ]
    if not candidates:
        return 0
    return min(space.distance(record_index, candidate_index) for candidate_index in candidates)


def find_outliers(records, metric, strategy):
    """Find unusual records and return an engine-style result object."""
    strategy = _coerce_outlier_strategy(strategy)
    records = list(records)
    groups = dbscan(records, metric, strategy.radius, strategy.min_points)
    space = FiniteMetricSpace(records, metric)
    references = [
        record_index
        for record_index, assignment in enumerate(groups.assignments)
        if assignment != ClusteringResult.noise_label
    ]

    outliers = [
        Outlier(
            record_id=record_index,
            score=_nearest_reference_distance(space, record_index, references),
        )
        for record_index in groups.noise_records
    ]
    outliers.sort(key=lambda outlier: outlier.record_id)
    outliers.sort(key=lambda outlier: outlier.score, reverse=True)

    return OutlierResult(
        outliers=tuple(outliers),
        record_count=groups.record_count,
        cluster_count=groups.cluster_count,
        noise_count=groups.noise_count,
        exact=True,
        operator_name="find_outliers",
        strategy="dbscan_noise",
        representation=groups.representation,
    )


def _coerce_denoise_strategy(strategy):
    if isinstance(strategy, DBSCAN):
        return strategy
    if hasattr(strategy, "radius") and hasattr(strategy, "min_points"):
        return DBSCAN(radius=strategy.radius, min_points=strategy.min_points)
    raise TypeError("strategy must be a DBSCAN strategy")


def denoise_space(records, metric, strategy):
    """Filter DBSCAN noise records and return a derived metric space."""
    strategy = _coerce_denoise_strategy(strategy)
    records = list(records)
    groups = dbscan(records, metric, strategy.radius, strategy.min_points)
    kept_record_ids = tuple(
        record_index
        for record_index, assignment in enumerate(groups.assignments)
        if assignment != ClusteringResult.noise_label
    )
    denoised_records = [records[index] for index in kept_record_ids]

    from metric.spaces import Space

    return MappingResult(
        space=Space(denoised_records, metric),
        source_record_ids=kept_record_ids,
        source_record_count=len(records),
        target_record_count=len(denoised_records),
        exact=True,
        operator_name="denoise",
        mapping="density_denoise",
        strategy="dbscan_noise_filter",
        representation=groups.representation,
        inverse_supported=False,
    )


def representative_indices(records, metric, k, seed_index=0):
    """Select representative record IDs with deterministic farthest-first traversal."""
    selected, _nearest_selected_distances, _records = _farthest_first_selection(records, metric, k, seed_index)
    return selected


def _farthest_first_selection(records, metric, k, seed_index=0):
    records = list(records)
    try:
        k = operator.index(k)
        seed_index = operator.index(seed_index)
    except TypeError:
        raise TypeError("k and seed_index must be integers") from None

    if k < 0:
        raise ValueError("k must be non-negative")
    if k == 0:
        return [], [], records
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

    return selected, nearest_selected_distances, records


def _coerce_farthest_first(strategy):
    if strategy is None:
        return FarthestFirst()
    if isinstance(strategy, FarthestFirst):
        return strategy
    if hasattr(strategy, "seed_index"):
        return FarthestFirst(seed_index=strategy.seed_index)
    raise TypeError("strategy must be a FarthestFirst strategy")


def find_representatives(records, metric, k, strategy=None):
    """Select representatives and return an engine-style result object."""
    strategy = _coerce_farthest_first(strategy)
    selected, nearest_selected_distances, records = _farthest_first_selection(
        records,
        metric,
        k,
        strategy.seed_index,
    )
    coverage_radius = max(nearest_selected_distances, default=0)
    average_nearest_distance = (
        sum(float(distance) for distance in nearest_selected_distances) / len(nearest_selected_distances)
        if nearest_selected_distances
        else 0.0
    )
    return RepresentativeSet(
        representatives=tuple(selected),
        nearest_representative_distances=tuple(nearest_selected_distances),
        record_count=len(records),
        requested_count=operator.index(k),
        coverage_radius=coverage_radius,
        average_nearest_distance=average_nearest_distance,
        exact=True,
        strategy="farthest_first",
        representation="metric_space",
    )


def _is_strategy_like(value):
    return (
        isinstance(value, (FarthestFirst, KMedoids))
        or hasattr(value, "seed_index")
        or hasattr(value, "groups")
    )


def _coerce_reduction_request(count, strategy):
    if strategy is None and _is_strategy_like(count):
        strategy = count
        count = None

    if strategy is None:
        strategy = FarthestFirst()
    elif isinstance(strategy, FarthestFirst):
        pass
    elif isinstance(strategy, KMedoids):
        pass
    elif hasattr(strategy, "seed_index"):
        strategy = FarthestFirst(seed_index=strategy.seed_index)
    elif hasattr(strategy, "groups"):
        strategy = KMedoids(
            groups=strategy.groups,
            max_iterations=getattr(strategy, "max_iterations", 100),
        )
    else:
        raise TypeError("strategy must be a FarthestFirst or KMedoids strategy")

    if count is None:
        if isinstance(strategy, KMedoids):
            count = strategy.groups
        else:
            raise TypeError("count is required for FarthestFirst reduction")
    else:
        count = _coerce_positive_integer(count, "count")

    if isinstance(strategy, KMedoids) and count != strategy.groups:
        raise ValueError("count must match strategy.groups for KMedoids reduction")

    return count, strategy


def _assign_to_representatives(space, representative_ids):
    assignments = []
    nearest_distances = []

    for record_index in range(len(space.records)):
        best_reduced_index = 0
        best_distance = None
        for reduced_index, representative_id in enumerate(representative_ids):
            distance = space.distance(record_index, representative_id)
            if (
                best_distance is None
                or distance < best_distance
                or (
                    distance == best_distance
                    and representative_id < representative_ids[best_reduced_index]
                )
            ):
                best_reduced_index = reduced_index
                best_distance = distance

        assignments.append(best_reduced_index)
        nearest_distances.append(best_distance)

    return assignments, nearest_distances


def reduce_space(records, metric, count=None, strategy=None):
    """Reduce a finite metric space to representative records with lineage."""
    count, strategy = _coerce_reduction_request(count, strategy)
    records = list(records)
    if not records:
        raise ValueError("cannot reduce an empty record set")
    if count > len(records):
        raise ValueError("count cannot exceed the number of records")

    if isinstance(strategy, FarthestFirst):
        representatives_result = find_representatives(records, metric, count, strategy=strategy)
        source_record_ids = representatives_result.representatives
        strategy_name = "farthest_first"
    elif isinstance(strategy, KMedoids):
        groups = find_groups(records, metric, strategy)
        source_record_ids = groups.medoids
        strategy_name = "kmedoids"
    else:
        raise TypeError("unsupported reduction strategy")

    source_space = FiniteMetricSpace(records, metric)
    assignments, nearest_distances = _assign_to_representatives(source_space, source_record_ids)

    from metric.spaces import Space

    reduced_records = [records[index] for index in source_record_ids]
    return ReductionResult(
        space=Space(reduced_records, metric),
        source_record_ids=tuple(source_record_ids),
        assignments=tuple(assignments),
        nearest_representative_distances=tuple(nearest_distances),
        source_record_count=len(records),
        reduced_record_count=len(reduced_records),
        exact=True,
        operator_name="reduce",
        strategy=strategy_name,
        representation="metric_space",
        inverse_supported=False,
    )


def compress_space(records, metric, count=None, strategy=None):
    """Compress a finite metric space by retaining representative records."""
    reduction = reduce_space(records, metric, count, strategy=strategy)
    compression_ratio = reduction.reduced_record_count / reduction.source_record_count

    return CompressionResult(
        space=reduction.space,
        source_record_ids=reduction.source_record_ids,
        assignments=reduction.assignments,
        nearest_representative_distances=reduction.nearest_representative_distances,
        source_record_count=reduction.source_record_count,
        compressed_record_count=reduction.reduced_record_count,
        compression_ratio=compression_ratio,
        exact=reduction.exact,
        operator_name="compress",
        compression="representatives",
        strategy=reduction.strategy,
        representation=reduction.representation,
        lossy=True,
        inverse_supported=False,
    )


def map_space(records, transform, metric):
    """Map records through a deterministic transform and return a new Space."""
    if not callable(transform):
        raise TypeError("transform must be callable")
    if not callable(metric):
        raise TypeError("metric must be callable")

    records = list(records)
    mapped_records = [transform(record) for record in records]

    from metric.spaces import Space

    return MappingResult(
        space=Space(mapped_records, metric),
        source_record_ids=tuple(range(len(records))),
        source_record_count=len(records),
        target_record_count=len(mapped_records),
        exact=True,
        operator_name="map",
        mapping="deterministic_transform",
        strategy="deterministic_transform",
        representation="metric_space",
        inverse_supported=False,
    )


def _coerce_embedding_strategy(dimensions, strategy):
    if strategy is None:
        return ClassicMDS(dimensions=dimensions)
    if isinstance(strategy, ClassicMDS):
        return strategy
    if hasattr(strategy, "dimensions"):
        return ClassicMDS(dimensions=strategy.dimensions)
    raise TypeError("strategy must be a ClassicMDS strategy")


def _coerce_embedding_dimensions(dimensions):
    try:
        dimensions = operator.index(dimensions)
    except TypeError:
        raise TypeError("dimensions must be an integer") from None
    if dimensions <= 0:
        raise ValueError("dimensions must be positive")
    return dimensions


def _embedding_distance(lhs, rhs):
    return float(np.linalg.norm(np.asarray(lhs, dtype=float) - np.asarray(rhs, dtype=float)))


def _embedded_distance_matrix(coordinates):
    record_count = coordinates.shape[0]
    distances = [[0.0] * record_count for _index in range(record_count)]
    for lhs_index in range(record_count):
        for rhs_index in range(lhs_index + 1, record_count):
            distance = _embedding_distance(coordinates[lhs_index], coordinates[rhs_index])
            distances[lhs_index][rhs_index] = distance
            distances[rhs_index][lhs_index] = distance
    return distances


def _local_neighbor_overlap(source_distances, embedded_distances):
    record_count = len(source_distances)
    if record_count <= 1:
        return 1.0, 0

    neighbor_k = min(2, record_count - 1)
    overlap_sum = 0
    for record_index in range(record_count):
        source_neighbors = sorted(
            (
                (source_distances[record_index][candidate_index], candidate_index)
                for candidate_index in range(record_count)
                if candidate_index != record_index
            )
        )[:neighbor_k]
        embedded_neighbors = sorted(
            (
                (embedded_distances[record_index][candidate_index], candidate_index)
                for candidate_index in range(record_count)
                if candidate_index != record_index
            )
        )[:neighbor_k]
        overlap_sum += len(
            {candidate_index for _distance, candidate_index in source_neighbors}
            & {candidate_index for _distance, candidate_index in embedded_neighbors}
        )

    return overlap_sum / (record_count * neighbor_k), neighbor_k


def _embedding_distance_correlation(source_profile, embedded_profile, raw_stress):
    if len(source_profile) < 2:
        return 1.0 if raw_stress <= 1e-12 else 0.0
    try:
        return _pearson_correlation(source_profile, embedded_profile)
    except ValueError:
        return 1.0 if raw_stress <= 1e-12 else 0.0


def embed_space(records, metric, dimensions=2, strategy=None):
    """Embed a finite metric space into deterministic Euclidean coordinates."""
    if not callable(metric):
        raise TypeError("metric must be callable")

    strategy = _coerce_embedding_strategy(dimensions, strategy)
    dimensions = _coerce_embedding_dimensions(strategy.dimensions)
    records = list(records)
    source_distances = np.asarray(pairwise_distance_matrix(records, metric), dtype=float)

    if np.any(source_distances < 0):
        raise ValueError("distance matrix contains negative distances")

    record_count = len(records)
    if record_count == 0:
        coordinates = np.zeros((0, dimensions), dtype=float)
    else:
        squared_distances = source_distances ** 2
        centering = np.eye(record_count) - np.full((record_count, record_count), 1.0 / record_count)
        gram = -0.5 * centering @ squared_distances @ centering
        gram = (gram + gram.T) * 0.5
        eigenvalues, eigenvectors = np.linalg.eigh(gram)
        order = np.argsort(eigenvalues)[::-1]

        coordinates = np.zeros((record_count, dimensions), dtype=float)
        for coordinate_index, eigen_index in enumerate(order[:dimensions]):
            eigenvalue = max(float(eigenvalues[eigen_index]), 0.0)
            if eigenvalue <= 0:
                continue
            vector = eigenvectors[:, eigen_index]
            anchor_index = int(np.argmax(np.abs(vector)))
            if vector[anchor_index] < 0:
                vector = -vector
            coordinates[:, coordinate_index] = vector * math.sqrt(eigenvalue)

    embedded_records = [tuple(float(value) for value in row) for row in coordinates.tolist()]
    embedded_distances = _embedded_distance_matrix(coordinates)
    source_profile = _upper_triangle(source_distances.tolist())
    embedded_profile = _upper_triangle(embedded_distances)
    raw_stress = math.sqrt(
        sum(
            (source_distance - embedded_distance) ** 2
            for source_distance, embedded_distance in zip(source_profile, embedded_profile)
        )
    )
    source_norm = math.sqrt(sum(distance ** 2 for distance in source_profile))
    normalized_stress = raw_stress / source_norm if source_norm else 0.0
    trustworthiness, neighbor_k = _local_neighbor_overlap(source_distances.tolist(), embedded_distances)
    distance_correlation = _embedding_distance_correlation(source_profile, embedded_profile, raw_stress)
    coordinate_scale = float(np.max(np.abs(coordinates))) if coordinates.size else 0.0

    from metric.spaces import Space

    diagnostics = EmbeddingDiagnostics(
        raw_stress=raw_stress,
        normalized_stress=normalized_stress,
        distance_correlation=distance_correlation,
        trustworthiness=trustworthiness,
        neighbor_k=neighbor_k,
        finite_coordinates=bool(np.isfinite(coordinates).all()),
        coordinate_scale=coordinate_scale,
    )

    return EmbeddingResult(
        coordinates=coordinates,
        embedded_space=Space(embedded_records, _embedding_distance),
        source_space=Space(records, metric),
        model=EmbeddingModel(
            method="classic_mds",
            dimensions=dimensions,
            source_record_ids=tuple(range(record_count)),
        ),
        source_record_ids=tuple(range(record_count)),
        source_record_count=record_count,
        dimensions=dimensions,
        stress=normalized_stress,
        trustworthiness=trustworthiness,
        exact=True,
        operator_name="embed",
        strategy="classic_mds",
        representation="metric_space",
        diagnostics=diagnostics,
    )


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


def _upper_triangle(distances):
    values = []
    for row_index, row in enumerate(distances):
        if len(row) != len(distances):
            raise ValueError("distance matrix must be square")
        for column_index in range(row_index + 1, len(row)):
            values.append(float(row[column_index]))
    return values


def _pearson_correlation(lhs, rhs):
    if len(lhs) != len(rhs):
        raise ValueError("distance profiles must have equal length")
    if not lhs:
        raise ValueError("distance profiles must contain at least one pair")

    lhs_mean = sum(lhs) / len(lhs)
    rhs_mean = sum(rhs) / len(rhs)
    numerator = sum((left - lhs_mean) * (right - rhs_mean) for left, right in zip(lhs, rhs))
    lhs_scale = sum((left - lhs_mean) ** 2 for left in lhs) ** 0.5
    rhs_scale = sum((right - rhs_mean) ** 2 for right in rhs) ** 0.5
    if lhs_scale == 0 or rhs_scale == 0:
        raise ValueError("distance profiles must have non-zero variance")
    return numerator / (lhs_scale * rhs_scale)


def compare_spaces(
    left_records,
    left_metric,
    right_records,
    right_metric,
    strategy=None,
    *,
    left_representation="records",
    right_representation="records",
):
    """Compare aligned finite metric spaces through pairwise distance profiles."""
    if strategy is None:
        strategy = DistanceProfileCorrelation()
    if not isinstance(strategy, DistanceProfileCorrelation):
        raise TypeError("strategy must be a DistanceProfileCorrelation instance")
    if strategy.method != "pearson":
        raise ValueError("DistanceProfileCorrelation currently supports method='pearson'")

    left_records = list(left_records)
    right_records = list(right_records)
    if len(left_records) != len(right_records):
        raise ValueError("space comparison requires aligned record sets of equal size")

    left_profile = _upper_triangle(pairwise_distance_matrix(left_records, left_metric))
    right_profile = _upper_triangle(pairwise_distance_matrix(right_records, right_metric))

    return CorrelationResult(
        value=_pearson_correlation(left_profile, right_profile),
        left_record_count=len(left_records),
        right_record_count=len(right_records),
        pair_count=len(left_profile),
        exact=True,
        algorithm="distance_profile_correlation",
        strategy="distance_profile_correlation",
        left_representation=left_representation,
        right_representation=right_representation,
    )


def correlate_spaces(left_records, left_metric, right_records, right_metric, strategy=None):
    """Alias for compare_spaces for correlation-oriented workflows."""
    return compare_spaces(left_records, left_metric, right_records, right_metric, strategy)


def intrinsic_dimension(records, metric):
    """Estimate expansion dimension from finite metric-space distance growth."""
    return intrinsic_dimension_from_distances(pairwise_distance_matrix(records, metric))


def describe_structure(records, metric):
    """Describe exact finite-space structure with all-pairs diagnostics."""
    records = list(records)
    distances = pairwise_distance_matrix(records, metric)
    pair_count = 0
    zero_distance_pair_count = 0
    minimum_nonzero_distance = 0
    maximum_distance = 0
    distance_sum = 0.0
    has_nonzero_distances = False

    for lhs_index in range(len(records)):
        for rhs_index in range(lhs_index + 1, len(records)):
            distance = distances[lhs_index][rhs_index]
            pair_count += 1
            distance_sum += float(distance)
            if distance <= 0:
                zero_distance_pair_count += 1
            elif not has_nonzero_distances or distance < minimum_nonzero_distance:
                minimum_nonzero_distance = distance
                has_nonzero_distances = True
            if pair_count == 1 or maximum_distance < distance:
                maximum_distance = distance

    return StructureDescription(
        record_count=len(records),
        pair_count=pair_count,
        zero_distance_pair_count=zero_distance_pair_count,
        minimum_nonzero_distance=minimum_nonzero_distance,
        maximum_distance=maximum_distance,
        average_distance=distance_sum / pair_count if pair_count else 0.0,
        intrinsic_dimension=intrinsic_dimension_from_distances(distances),
        has_nonzero_distances=has_nonzero_distances,
        exact=True,
        strategy="exact_all_pairs",
        representation="metric_space",
    )


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
    "ClusteringResult",
    "CompressionResult",
    "CorrelationResult",
    "EmbeddingDiagnostics",
    "EmbeddingModel",
    "EmbeddingResult",
    "GraphConnectivityDiagnostics",
    "GraphDegreeDiagnostics",
    "GraphStretchDiagnostics",
    "GraphConstructionMetadata",
    "GraphConstructionResult",
    "MappingResult",
    "Outlier",
    "OutlierResult",
    "RepresentativeSet",
    "ReductionResult",
    "StructureDescription",
    "compare_spaces",
    "correlate_spaces",
    "compress_space",
    "dbscan",
    "denoise_space",
    "describe_structure",
    "embed_space",
    "find_groups",
    "find_outliers",
    "find_representatives",
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
    "kmedoids",
    "map_space",
    "medoid",
    "medoid_index",
    "pairwise_distance_matrix",
    "prune_graph_out_degree",
    "nearest_neighbors",
    "range_neighbors",
    "reduce_space",
    "representative_indices",
    "representatives",
    "separated_representative_indices",
    "separated_representatives",
    "symmetrize_graph",
]
