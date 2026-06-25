"""Research-only finite metric-space diagnostics.

Nothing in this module is part of the promoted API surface. Functions here are
small, deterministic prototypes for roadmap candidates that still need stronger
fixtures, cost models, and promotion review.
"""

from dataclasses import dataclass
import math


def _pair_count(record_count):
    return 0 if record_count < 2 else record_count * (record_count - 1) // 2


_PROMOTION_PREFLIGHT_RULE = (
    "research-only exact finite-space prototype; promoted routes must preflight "
    "record count, pair count, and distance-evaluation budget before dense work"
)


def _cost_model(
    *,
    distance_evaluations,
    pair_count=0,
    dense_matrix_materialized=False,
    policy="exact_metric_calls",
    exact=True,
    notes=(),
):
    return {
        "policy": policy,
        "exact": bool(exact),
        "pair_count": int(pair_count),
        "distance_evaluations": int(distance_evaluations),
        "dense_matrix_materialized": bool(dense_matrix_materialized),
        "preflight_rule": _PROMOTION_PREFLIGHT_RULE,
        "notes": tuple(notes),
    }


def _sampled_pair_ordinals(pair_count, sample_count):
    if pair_count == 0:
        return ()
    if sample_count is None or sample_count >= pair_count:
        return tuple(range(pair_count))
    if sample_count <= 0:
        raise ValueError("sample_count must be positive")
    return tuple((index * pair_count) // sample_count for index in range(sample_count))


def _pair_for_ordinal(record_count, ordinal):
    row = 0
    row_start = 0
    while row + 1 < record_count:
        row_pairs = record_count - row - 1
        if ordinal < row_start + row_pairs:
            return row, row + 1 + (ordinal - row_start)
        row_start += row_pairs
        row += 1
    raise IndexError("pair ordinal is outside the finite space")


def _finite_distance(value):
    numeric = float(value)
    if not math.isfinite(numeric):
        raise ValueError("finite metric-space diagnostics require finite distances")
    if numeric < 0:
        raise ValueError("finite metric-space diagnostics require non-negative distances")
    return numeric


def _pair_distances(records, metric, sample_count=None):
    record_list = list(records)
    pair_total = _pair_count(len(record_list))
    ordinals = _sampled_pair_ordinals(pair_total, sample_count)
    values = []
    for ordinal in ordinals:
        lhs, rhs = _pair_for_ordinal(len(record_list), ordinal)
        values.append(_finite_distance(metric(record_list[lhs], record_list[rhs])))
    return record_list, pair_total, values


def _quantile(sorted_values, probability):
    if not sorted_values:
        return 0.0
    if len(sorted_values) == 1:
        return float(sorted_values[0])
    clamped = min(1.0, max(0.0, float(probability)))
    position = clamped * (len(sorted_values) - 1)
    lower = math.floor(position)
    upper = math.ceil(position)
    if lower == upper:
        return float(sorted_values[lower])
    fraction = position - lower
    return float(sorted_values[lower]) * (1.0 - fraction) + float(sorted_values[upper]) * fraction


def _histogram(sorted_values, bucket_count, value_range=None):
    if bucket_count < 0:
        raise ValueError("bucket_count must be non-negative")
    if bucket_count == 0 or not sorted_values:
        return (), ()
    if value_range is None:
        minimum = float(sorted_values[0])
        maximum = float(sorted_values[-1])
    else:
        minimum, maximum = (float(value_range[0]), float(value_range[1]))
    if maximum < minimum:
        raise ValueError("histogram range maximum must be >= minimum")
    span = maximum - minimum
    edges = tuple(minimum + span * index / bucket_count for index in range(bucket_count + 1))
    counts = [0] * bucket_count
    for value in sorted_values:
        if span == 0:
            bucket = 0
        else:
            bucket = int(((float(value) - minimum) / span) * bucket_count)
            if bucket < 0:
                bucket = 0
            elif bucket >= bucket_count:
                bucket = bucket_count - 1
        counts[bucket] += 1
    return edges, tuple(counts)


@dataclass(frozen=True)
class DistanceDistributionSketch:
    """Research-only sketch of unordered finite-space pair distances."""

    record_count: int
    pair_count: int
    evaluated_pair_count: int
    sample_count: int
    exact: bool
    minimum: float
    maximum: float
    mean: float
    median: float
    quantile_probabilities: tuple
    quantile_values: tuple
    histogram_edges: tuple
    histogram_counts: tuple
    operator_name: str = "sketch"
    strategy: str = "distance_distribution_sketch"
    metric_status: str = "diagnostic_not_a_metric"
    validity: str = (
        "research-only distance-distribution sketch over unordered pairwise "
        "finite metric distances; no derived metric space is produced"
    )

    @property
    def source_record_count(self):
        return self.record_count

    @property
    def target_record_count(self):
        return 0

    @property
    def source_indices(self):
        return tuple(range(self.record_count))

    @property
    def cost_model(self):
        policy = "exact_all_pairs" if self.exact else "deterministic_pair_ordinal_sample"
        return _cost_model(
            distance_evaluations=self.evaluated_pair_count,
            pair_count=self.pair_count,
            dense_matrix_materialized=False,
            policy=policy,
            exact=self.exact,
            notes=("distance distribution uses unordered source-record pairs",),
        )

    def to_dict(self):
        return {
            "source_record_count": self.source_record_count,
            "target_record_count": self.target_record_count,
            "source_indices": self.source_indices,
            "target_indices": (),
            "record_count": self.record_count,
            "pair_count": self.pair_count,
            "evaluated_pair_count": self.evaluated_pair_count,
            "sample_count": self.sample_count,
            "exact": self.exact,
            "minimum": self.minimum,
            "maximum": self.maximum,
            "mean": self.mean,
            "median": self.median,
            "quantile_probabilities": self.quantile_probabilities,
            "quantile_values": self.quantile_values,
            "histogram_edges": self.histogram_edges,
            "histogram_counts": self.histogram_counts,
            "cost_model": self.cost_model,
            "operator_name": self.operator_name,
            "strategy": self.strategy,
            "metric_status": self.metric_status,
            "validity": self.validity,
        }


@dataclass(frozen=True)
class DistanceDistributionDrift:
    """Research-only drift diagnostic between two finite-space distance sketches."""

    source: DistanceDistributionSketch
    target: DistanceDistributionSketch
    mean_absolute_drift: float
    median_absolute_drift: float
    maximum_quantile_absolute_drift: float
    histogram_l1_drift: float
    operator_name: str = "compare"
    strategy: str = "distance_distribution_drift"
    metric_status: str = "diagnostic_not_a_metric"
    validity: str = (
        "research-only diagnostic comparing distance-distribution sketches; "
        "it reports observable drift and does not construct a metric"
    )

    @property
    def source_record_count(self):
        return self.source.record_count

    @property
    def target_record_count(self):
        return self.target.record_count

    @property
    def cost_model(self):
        source_evaluations = self.source.evaluated_pair_count
        target_evaluations = self.target.evaluated_pair_count
        return _cost_model(
            distance_evaluations=2 * (source_evaluations + target_evaluations),
            pair_count=self.source.pair_count + self.target.pair_count,
            dense_matrix_materialized=False,
            policy="distance_distribution_drift_two_pass",
            exact=self.source.exact and self.target.exact,
            notes=(
                "current prototype computes raw pair ranges once for shared histogram range "
                "and once for each sketch",
            ),
        )

    def to_dict(self):
        return {
            "source_record_count": self.source_record_count,
            "target_record_count": self.target_record_count,
            "source": self.source.to_dict(),
            "target": self.target.to_dict(),
            "cost_model": self.cost_model,
            "mean_absolute_drift": self.mean_absolute_drift,
            "median_absolute_drift": self.median_absolute_drift,
            "maximum_quantile_absolute_drift": self.maximum_quantile_absolute_drift,
            "histogram_l1_drift": self.histogram_l1_drift,
            "operator_name": self.operator_name,
            "strategy": self.strategy,
            "metric_status": self.metric_status,
            "validity": self.validity,
        }


@dataclass(frozen=True)
class KNNRecallRow:
    """Per-query research diagnostic for source-vs-sketch neighbor recall."""

    query_index: int
    source_neighbor_indices: tuple
    sketch_neighbor_indices: tuple
    recall: float

    def to_dict(self):
        return {
            "query_index": self.query_index,
            "source_neighbor_indices": self.source_neighbor_indices,
            "sketch_neighbor_indices": self.sketch_neighbor_indices,
            "recall": self.recall,
        }


@dataclass(frozen=True)
class KNNRecallSketch:
    """Research-only kNN behavior diagnostic for a finite-space subset/sketch."""

    source_record_count: int
    sketch_record_count: int
    query_count: int
    neighbor_count: int
    sketch_indices: tuple
    average_recall: float
    minimum_recall: float
    rows: tuple
    operator_name: str = "sketch"
    strategy: str = "knn_recall_sketch"
    metric_status: str = "diagnostic_not_a_metric"
    validity: str = (
        "research-only kNN-recall diagnostic comparing source nearest-neighbor "
        "behavior with a source-record subset; no derived metric is produced"
    )

    @property
    def target_record_count(self):
        return self.sketch_record_count

    @property
    def source_indices(self):
        return tuple(range(self.source_record_count))

    @property
    def representative_indices(self):
        return self.sketch_indices

    @property
    def cost_model(self):
        evaluations = 0
        for row in self.rows:
            evaluations += max(0, self.source_record_count - 1)
            evaluations += self.sketch_record_count - (1 if row.query_index in self.sketch_indices else 0)
        return _cost_model(
            distance_evaluations=evaluations,
            pair_count=_pair_count(self.source_record_count),
            dense_matrix_materialized=False,
            policy="per_query_exact_scan",
            exact=True,
            notes=("source and sketch nearest-neighbor sets are scanned directly from metric calls",),
        )

    def to_dict(self):
        return {
            "source_record_count": self.source_record_count,
            "target_record_count": self.target_record_count,
            "source_indices": self.source_indices,
            "target_indices": self.sketch_indices,
            "sketch_record_count": self.sketch_record_count,
            "sketch_indices": self.sketch_indices,
            "representative_indices": self.representative_indices,
            "query_count": self.query_count,
            "neighbor_count": self.neighbor_count,
            "average_recall": self.average_recall,
            "minimum_recall": self.minimum_recall,
            "rows": tuple(row.to_dict() for row in self.rows),
            "cost_model": self.cost_model,
            "operator_name": self.operator_name,
            "strategy": self.strategy,
            "metric_status": self.metric_status,
            "validity": self.validity,
        }


@dataclass(frozen=True)
class MetricGraphEdge:
    """Weighted edge between source records in a research-only metric graph."""

    lhs_index: int
    rhs_index: int
    distance: float

    def to_dict(self):
        return {
            "lhs_index": self.lhs_index,
            "rhs_index": self.rhs_index,
            "distance": self.distance,
        }


@dataclass(frozen=True)
class MetricGraphSpanner:
    """Research-only graph spanner over source records with metric edge weights."""

    record_count: int
    pair_count: int
    edge_count: int
    stretch_bound: float
    maximum_stretch: float
    average_stretch: float
    disconnected_pair_count: int
    edges: tuple
    operator_name: str = "sketch"
    strategy: str = "greedy_metric_graph_spanner"
    metric_status: str = "derived_graph_metric"
    validity: str = (
        "research-only metric graph spanner over source records; graph edges "
        "inherit source metric distances and shortest-path stretch is diagnostic"
    )

    @property
    def source_record_count(self):
        return self.record_count

    @property
    def target_record_count(self):
        return self.record_count

    @property
    def source_indices(self):
        return tuple(range(self.record_count))

    @property
    def representative_indices(self):
        return self.source_indices

    @property
    def cost_model(self):
        return _cost_model(
            distance_evaluations=self.pair_count,
            pair_count=self.pair_count,
            dense_matrix_materialized=False,
            policy="exact_all_pairs_then_greedy_spanner",
            exact=True,
            notes=("shortest-path checks do not invoke the source metric",),
        )

    def to_dict(self):
        return {
            "source_record_count": self.source_record_count,
            "target_record_count": self.target_record_count,
            "source_indices": self.source_indices,
            "target_indices": self.representative_indices,
            "representative_indices": self.representative_indices,
            "record_count": self.record_count,
            "pair_count": self.pair_count,
            "edge_count": self.edge_count,
            "stretch_bound": self.stretch_bound,
            "maximum_stretch": self.maximum_stretch,
            "average_stretch": self.average_stretch,
            "disconnected_pair_count": self.disconnected_pair_count,
            "edges": tuple(edge.to_dict() for edge in self.edges),
            "cost_model": self.cost_model,
            "operator_name": self.operator_name,
            "strategy": self.strategy,
            "metric_status": self.metric_status,
            "validity": self.validity,
        }


@dataclass(frozen=True)
class HierarchicalMetricNetLevel:
    """One radius level in a research-only nested metric net."""

    radius: float
    representative_indices: tuple
    parent_indices: tuple
    assignment_indices: tuple
    assignment_positions: tuple
    multiplicities: tuple
    coverage_radius: float
    average_assignment_distance: float

    def to_dict(self):
        return {
            "radius": self.radius,
            "representative_indices": self.representative_indices,
            "parent_indices": self.parent_indices,
            "assignment_indices": self.assignment_indices,
            "assignment_positions": self.assignment_positions,
            "multiplicities": self.multiplicities,
            "coverage_radius": self.coverage_radius,
            "average_assignment_distance": self.average_assignment_distance,
        }


@dataclass(frozen=True)
class HierarchicalMetricNet:
    """Research-only nested radius cover over source records."""

    source_record_count: int
    target_record_count: int
    level_count: int
    requested_radii: tuple
    radii: tuple
    representative_indices: tuple
    levels: tuple
    operator_name: str = "represent"
    strategy: str = "hierarchical_metric_net"
    metric_status: str = "source_metric_subset"
    validity: str = (
        "research-only nested metric net over finite source records; levels are "
        "source-record subsets with assignment and parent lineage, and no "
        "coordinates or centroids are assumed"
    )

    def to_dict(self):
        representative_work = sum(len(level.representative_indices) for level in self.levels)
        return {
            "source_record_count": self.source_record_count,
            "target_record_count": self.target_record_count,
            "source_indices": tuple(range(self.source_record_count)),
            "level_count": self.level_count,
            "requested_radii": self.requested_radii,
            "radii": self.radii,
            "representative_indices": self.representative_indices,
            "levels": tuple(level.to_dict() for level in self.levels),
            "cost_model": _cost_model(
                distance_evaluations=self.source_record_count * max(1, representative_work),
                pair_count=_pair_count(self.source_record_count),
                dense_matrix_materialized=False,
                policy="greedy_nested_radius_cover_estimate",
                exact=True,
                notes=("result-based upper-bound estimate for greedy cover and assignment scans",),
            ),
            "operator_name": self.operator_name,
            "strategy": self.strategy,
            "metric_status": self.metric_status,
            "validity": self.validity,
        }


@dataclass(frozen=True)
class AdaptiveDensityEqualization:
    """Research-only local-volume-prioritized thinning result."""

    source_record_count: int
    target_record_count: int
    radius: float
    representative_indices: tuple
    assignment_indices: tuple
    assignment_positions: tuple
    multiplicities: tuple
    local_volume_counts: tuple
    selection_priority: tuple
    coverage_radius: float
    average_assignment_distance: float
    source_average_local_volume_count: float
    target_average_local_volume_count: float
    local_volume_count_drift: float
    operator_name: str = "equalize"
    strategy: str = "adaptive_local_volume_equalization"
    metric_status: str = "source_metric_subset"
    validity: str = (
        "research-only density-equalizing thinning over a finite metric space; "
        "source records with lower local volume are selected first, then a "
        "maximal radius-separated subset is built without coordinates"
    )

    def to_dict(self):
        assignment_work = self.source_record_count * max(1, self.target_record_count)
        local_volume_work = self.source_record_count * self.source_record_count
        target_volume_work = self.target_record_count * self.target_record_count
        return {
            "source_record_count": self.source_record_count,
            "target_record_count": self.target_record_count,
            "source_indices": tuple(range(self.source_record_count)),
            "radius": self.radius,
            "representative_indices": self.representative_indices,
            "assignment_indices": self.assignment_indices,
            "assignment_positions": self.assignment_positions,
            "multiplicities": self.multiplicities,
            "local_volume_counts": self.local_volume_counts,
            "selection_priority": self.selection_priority,
            "coverage_radius": self.coverage_radius,
            "average_assignment_distance": self.average_assignment_distance,
            "source_average_local_volume_count": self.source_average_local_volume_count,
            "target_average_local_volume_count": self.target_average_local_volume_count,
            "local_volume_count_drift": self.local_volume_count_drift,
            "cost_model": _cost_model(
                distance_evaluations=local_volume_work + assignment_work + target_volume_work,
                pair_count=_pair_count(self.source_record_count),
                dense_matrix_materialized=False,
                policy="local_volume_then_sparse_first_radius_net",
                exact=True,
                notes=("selection comparisons are bounded by the same metric-call scale",),
            ),
            "operator_name": self.operator_name,
            "strategy": self.strategy,
            "metric_status": self.metric_status,
            "validity": self.validity,
        }


@dataclass(frozen=True)
class MetricMeasureDrift:
    """Research-only weighted finite metric-measure drift diagnostic."""

    source_record_count: int
    target_record_count: int
    source_weights: tuple
    target_weights: tuple
    source_average_pair_distance: float
    target_average_pair_distance: float
    average_pair_distance_drift: float
    source_diameter: float
    target_diameter: float
    diameter_drift: float
    source_weight_entropy: float
    target_weight_entropy: float
    weight_entropy_drift: float
    operator_name: str = "compare"
    strategy: str = "weighted_metric_measure_drift"
    metric_status: str = "diagnostic_not_a_metric"
    validity: str = (
        "research-only finite metric-measure diagnostic comparing normalized "
        "weights and weighted pairwise distances; it is not a full "
        "Gromov-Wasserstein solver"
    )

    def to_dict(self):
        source_pair_count = _pair_count(self.source_record_count)
        target_pair_count = _pair_count(self.target_record_count)
        return {
            "source_record_count": self.source_record_count,
            "target_record_count": self.target_record_count,
            "source_indices": tuple(range(self.source_record_count)),
            "target_indices": tuple(range(self.target_record_count)),
            "source_weights": self.source_weights,
            "target_weights": self.target_weights,
            "source_average_pair_distance": self.source_average_pair_distance,
            "target_average_pair_distance": self.target_average_pair_distance,
            "average_pair_distance_drift": self.average_pair_distance_drift,
            "source_diameter": self.source_diameter,
            "target_diameter": self.target_diameter,
            "diameter_drift": self.diameter_drift,
            "source_weight_entropy": self.source_weight_entropy,
            "target_weight_entropy": self.target_weight_entropy,
            "weight_entropy_drift": self.weight_entropy_drift,
            "cost_model": _cost_model(
                distance_evaluations=(
                    self.source_record_count * self.source_record_count
                    + self.target_record_count * self.target_record_count
                ),
                pair_count=source_pair_count + target_pair_count,
                dense_matrix_materialized=False,
                policy="weighted_ordered_pair_observables",
                exact=True,
                notes=("ordered pairs include the diagonal so weights define a probability product measure",),
            ),
            "operator_name": self.operator_name,
            "strategy": self.strategy,
            "metric_status": self.metric_status,
            "validity": self.validity,
        }


@dataclass(frozen=True)
class DensityHierarchyQuotientLevel:
    """One radius-component quotient level over source records."""

    radius: float
    groups: tuple
    representative_indices: tuple
    parent_positions: tuple
    assignment_indices: tuple
    assignment_positions: tuple
    multiplicities: tuple
    within_group_radii: tuple
    average_within_group_distances: tuple
    maximum_within_group_radius: float
    average_within_group_distance: float

    def to_dict(self):
        return {
            "radius": self.radius,
            "groups": self.groups,
            "representative_indices": self.representative_indices,
            "parent_positions": self.parent_positions,
            "assignment_indices": self.assignment_indices,
            "assignment_positions": self.assignment_positions,
            "multiplicities": self.multiplicities,
            "within_group_radii": self.within_group_radii,
            "average_within_group_distances": self.average_within_group_distances,
            "maximum_within_group_radius": self.maximum_within_group_radius,
            "average_within_group_distance": self.average_within_group_distance,
        }


@dataclass(frozen=True)
class DensityHierarchyQuotient:
    """Research-only hierarchy of radius-component medoid quotients."""

    source_record_count: int
    target_record_count: int
    level_count: int
    requested_radii: tuple
    radii: tuple
    representative_indices: tuple
    levels: tuple
    operator_name: str = "quotient"
    strategy: str = "density_hierarchy_quotient"
    metric_status: str = "source_medoid_quotient"
    validity: str = (
        "research-only quotient hierarchy over finite metric radius components; "
        "quotient records are source medoids with explicit group lineage and "
        "distances between representatives inherit the source metric"
    )

    def to_dict(self):
        source_pairs = _pair_count(self.source_record_count)
        return {
            "source_record_count": self.source_record_count,
            "target_record_count": self.target_record_count,
            "source_indices": tuple(range(self.source_record_count)),
            "level_count": self.level_count,
            "requested_radii": self.requested_radii,
            "radii": self.radii,
            "representative_indices": self.representative_indices,
            "levels": tuple(level.to_dict() for level in self.levels),
            "cost_model": _cost_model(
                distance_evaluations=self.source_record_count * (self.source_record_count + 1) // 2,
                pair_count=source_pairs,
                dense_matrix_materialized=True,
                policy="dense_distance_matrix_then_radius_components",
                exact=True,
                notes=("prototype materializes a finite distance matrix for quotient hierarchy construction",),
            ),
            "operator_name": self.operator_name,
            "strategy": self.strategy,
            "metric_status": self.metric_status,
            "validity": self.validity,
        }


def _validate_indices(indices, record_count, name):
    normalized = []
    seen = set()
    for index in indices:
        if isinstance(index, bool):
            raise TypeError(f"{name} indices must be integers")
        try:
            normalized_index = int(index.__index__())
        except AttributeError:
            try:
                normalized_index = int(index)
            except (TypeError, ValueError):
                raise TypeError(f"{name} indices must be integers") from None
        if normalized_index < 0 or normalized_index >= record_count:
            raise IndexError(f"{name} index is outside the source record set")
        if normalized_index in seen:
            raise ValueError(f"{name} indices must be unique")
        seen.add(normalized_index)
        normalized.append(normalized_index)
    return tuple(normalized)


def _validate_radius(radius):
    numeric = float(radius)
    if not math.isfinite(numeric):
        raise ValueError("radius must be finite")
    if numeric < 0:
        raise ValueError("radius must be non-negative")
    return numeric


def _normalized_weights(records, weights, name):
    record_count = len(records)
    if weights is None:
        if record_count == 0:
            return ()
        return tuple(1.0 / record_count for _ in range(record_count))
    raw = tuple(float(weight) for weight in weights)
    if len(raw) != record_count:
        raise ValueError(f"{name} weights must match record count")
    for weight in raw:
        if not math.isfinite(weight):
            raise ValueError(f"{name} weights must be finite")
        if weight < 0:
            raise ValueError(f"{name} weights must be non-negative")
    total = sum(raw)
    if record_count == 0:
        if total != 0:
            raise ValueError(f"{name} weights for an empty space must sum to zero")
        return ()
    if total <= 0:
        raise ValueError(f"{name} weights must have positive total mass")
    return tuple(weight / total for weight in raw)


def _validate_radii(radii):
    normalized = []
    for radius in radii:
        numeric = float(radius)
        if not math.isfinite(numeric):
            raise ValueError("radii must be finite")
        if numeric < 0:
            raise ValueError("radii must be non-negative")
        normalized.append(numeric)
    return tuple(normalized)


def _nearest_indices(records, metric, query_index, candidate_indices, count):
    ranked = []
    query = records[query_index]
    for candidate_index in candidate_indices:
        if candidate_index == query_index:
            continue
        ranked.append((_finite_distance(metric(query, records[candidate_index])), candidate_index))
    ranked.sort(key=lambda item: (item[0], item[1]))
    return tuple(index for _, index in ranked[:count])


def _shortest_path(adjacency, source, target):
    if source == target:
        return 0.0
    distances = {source: 0.0}
    visited = set()
    while True:
        current = None
        current_distance = math.inf
        for node, distance in distances.items():
            if node not in visited and distance < current_distance:
                current = node
                current_distance = distance
        if current is None:
            return math.inf
        if current == target:
            return current_distance
        visited.add(current)
        for neighbor, weight in adjacency[current]:
            candidate = current_distance + weight
            if candidate < distances.get(neighbor, math.inf):
                distances[neighbor] = candidate


def _add_edge(adjacency, lhs, rhs, distance):
    adjacency[lhs].append((rhs, distance))
    adjacency[rhs].append((lhs, distance))


def _cover_flags(records, metric, representatives, radius):
    covered = [False] * len(records)
    for representative_index in representatives:
        representative = records[representative_index]
        for record_index, record in enumerate(records):
            if not covered[record_index] and _finite_distance(metric(record, representative)) <= radius:
                covered[record_index] = True
    return covered


def _nearest_representative(records, metric, source_index, representative_indices):
    source = records[source_index]
    best = None
    for position, representative_index in enumerate(representative_indices):
        distance = _finite_distance(metric(source, records[representative_index]))
        candidate = (distance, representative_index, position)
        if best is None or candidate < best:
            best = candidate
    return best


def _nearest_parent(records, metric, representative_index, parent_indices):
    best = None
    representative = records[representative_index]
    for parent_index in parent_indices:
        distance = _finite_distance(metric(representative, records[parent_index]))
        candidate = (distance, parent_index)
        if best is None or candidate < best:
            best = candidate
    return None if best is None else best[1]


def _local_volume_counts(records, metric, radius):
    counts = []
    for record in records:
        count = 0
        for other in records:
            if _finite_distance(metric(record, other)) <= radius:
                count += 1
        counts.append(count)
    return tuple(counts)


def _weighted_pair_observables(records, metric, weights):
    average = 0.0
    diameter = 0.0
    for lhs_index, lhs in enumerate(records):
        for rhs_index, rhs in enumerate(records):
            distance = _finite_distance(metric(lhs, rhs))
            average += weights[lhs_index] * weights[rhs_index] * distance
            if distance > diameter:
                diameter = distance
    return average, diameter


def _weight_entropy(weights):
    entropy = 0.0
    for weight in weights:
        if weight > 0:
            entropy -= weight * math.log(weight)
    return entropy


def _finite_distance_matrix(records, metric):
    matrix = [[0.0 for _ in records] for _ in records]
    for lhs_index, lhs in enumerate(records):
        for rhs_index in range(lhs_index, len(records)):
            distance = _finite_distance(metric(lhs, records[rhs_index]))
            matrix[lhs_index][rhs_index] = distance
            matrix[rhs_index][lhs_index] = distance
    return matrix


def _radius_components_from_matrix(distance_matrix, radius):
    record_count = len(distance_matrix)
    visited = [False] * record_count
    groups = []
    for start in range(record_count):
        if visited[start]:
            continue
        visited[start] = True
        stack = [start]
        group = []
        while stack:
            current = stack.pop()
            group.append(current)
            for candidate in range(record_count):
                if not visited[candidate] and distance_matrix[current][candidate] <= radius:
                    visited[candidate] = True
                    stack.append(candidate)
        groups.append(tuple(sorted(group)))
    groups.sort(key=lambda group: group[0])
    return tuple(groups)


def _component_medoid(distance_matrix, group):
    best = None
    for candidate in group:
        total = sum(distance_matrix[candidate][member] for member in group)
        ranked = (total, candidate)
        if best is None or ranked < best:
            best = ranked
    return best[1]


def _quotient_parent_positions(groups, parent_groups):
    if not parent_groups:
        return tuple(None for _ in groups)
    parent_sets = [set(group) for group in parent_groups]
    parents = []
    for group in groups:
        group_set = set(group)
        parent_position = None
        for position, parent_set in enumerate(parent_sets):
            if group_set.issubset(parent_set):
                parent_position = position
                break
        parents.append(parent_position)
    return tuple(parents)


def _density_quotient_level(distance_matrix, radius, parent_groups):
    groups = _radius_components_from_matrix(distance_matrix, radius)
    representative_indices = tuple(_component_medoid(distance_matrix, group) for group in groups)
    assignment_indices = [None] * len(distance_matrix)
    assignment_positions = [None] * len(distance_matrix)
    within_group_radii = []
    average_within_group_distances = []
    total_assignment_distance = 0.0
    for position, group in enumerate(groups):
        representative_index = representative_indices[position]
        distances = tuple(distance_matrix[representative_index][member] for member in group)
        within_group_radii.append(max(distances, default=0.0))
        average_distance = 0.0 if not distances else sum(distances) / len(distances)
        average_within_group_distances.append(average_distance)
        total_assignment_distance += sum(distances)
        for member in group:
            assignment_indices[member] = representative_index
            assignment_positions[member] = position
    average_within_group_distance = (
        0.0 if not distance_matrix else total_assignment_distance / len(distance_matrix)
    )
    return DensityHierarchyQuotientLevel(
        radius=radius,
        groups=groups,
        representative_indices=representative_indices,
        parent_positions=_quotient_parent_positions(groups, parent_groups),
        assignment_indices=tuple(assignment_indices),
        assignment_positions=tuple(assignment_positions),
        multiplicities=tuple(len(group) for group in groups),
        within_group_radii=tuple(within_group_radii),
        average_within_group_distances=tuple(average_within_group_distances),
        maximum_within_group_radius=max(within_group_radii, default=0.0),
        average_within_group_distance=average_within_group_distance,
    )


def _assignment_summary(records, metric, representative_indices):
    assignment_indices = []
    assignment_positions = []
    assignment_distances = []
    multiplicities = [0] * len(representative_indices)
    for source_index in range(len(records)):
        distance, representative_index, position = _nearest_representative(
            records, metric, source_index, representative_indices
        )
        assignment_indices.append(representative_index)
        assignment_positions.append(position)
        assignment_distances.append(distance)
        multiplicities[position] += 1
    coverage_radius = max(assignment_distances, default=0.0)
    average_assignment_distance = (
        0.0 if not assignment_distances else sum(assignment_distances) / len(assignment_distances)
    )
    return (
        tuple(assignment_indices),
        tuple(assignment_positions),
        tuple(multiplicities),
        coverage_radius,
        average_assignment_distance,
    )


def hierarchical_metric_net(records, metric, radii):
    """Build a deterministic nested radius net from finite metric distances."""

    record_list = list(records)
    requested_radii = _validate_radii(radii)
    normalized_radii = tuple(sorted(requested_radii, reverse=True))
    previous_representatives = ()
    levels = []

    for radius in normalized_radii:
        representatives = list(previous_representatives)
        covered = _cover_flags(record_list, metric, representatives, radius)
        for record_index in range(len(record_list)):
            if covered[record_index]:
                continue
            representatives.append(record_index)
            representative = record_list[record_index]
            for candidate_index, candidate in enumerate(record_list):
                if not covered[candidate_index] and _finite_distance(metric(candidate, representative)) <= radius:
                    covered[candidate_index] = True

        (
            assignment_indices,
            assignment_positions,
            multiplicities,
            coverage_radius,
            average_assignment_distance,
        ) = _assignment_summary(record_list, metric, representatives)

        parent_indices = tuple(
            _nearest_parent(record_list, metric, representative_index, previous_representatives)
            for representative_index in representatives
        )
        level = HierarchicalMetricNetLevel(
            radius=radius,
            representative_indices=tuple(representatives),
            parent_indices=parent_indices,
            assignment_indices=tuple(assignment_indices),
            assignment_positions=tuple(assignment_positions),
            multiplicities=tuple(multiplicities),
            coverage_radius=coverage_radius,
            average_assignment_distance=average_assignment_distance,
        )
        levels.append(level)
        previous_representatives = tuple(representatives)

    representative_indices = () if not levels else levels[-1].representative_indices
    return HierarchicalMetricNet(
        source_record_count=len(record_list),
        target_record_count=len(representative_indices),
        level_count=len(levels),
        requested_radii=requested_radii,
        radii=normalized_radii,
        representative_indices=representative_indices,
        levels=tuple(levels),
    )


def adaptive_density_equalization(records, metric, radius):
    """Thin a finite metric space by sparse-first local-volume priority."""

    radius = _validate_radius(radius)
    record_list = list(records)
    local_volume_counts = _local_volume_counts(record_list, metric, radius)
    priority = tuple(
        index for index, _ in sorted(enumerate(local_volume_counts), key=lambda item: (item[1], item[0]))
    )

    representatives = []
    for candidate_index in priority:
        candidate = record_list[candidate_index]
        separated = True
        for representative_index in representatives:
            if _finite_distance(metric(candidate, record_list[representative_index])) <= radius:
                separated = False
                break
        if separated:
            representatives.append(candidate_index)

    if record_list:
        (
            assignment_indices,
            assignment_positions,
            multiplicities,
            coverage_radius,
            average_assignment_distance,
        ) = _assignment_summary(record_list, metric, representatives)
    else:
        assignment_indices = ()
        assignment_positions = ()
        multiplicities = ()
        coverage_radius = 0.0
        average_assignment_distance = 0.0

    target_counts = _local_volume_counts([record_list[index] for index in representatives], metric, radius)
    source_average = 0.0 if not local_volume_counts else sum(local_volume_counts) / len(local_volume_counts)
    target_average = 0.0 if not target_counts else sum(target_counts) / len(target_counts)
    return AdaptiveDensityEqualization(
        source_record_count=len(record_list),
        target_record_count=len(representatives),
        radius=radius,
        representative_indices=tuple(representatives),
        assignment_indices=assignment_indices,
        assignment_positions=assignment_positions,
        multiplicities=multiplicities,
        local_volume_counts=local_volume_counts,
        selection_priority=priority,
        coverage_radius=coverage_radius,
        average_assignment_distance=average_assignment_distance,
        source_average_local_volume_count=source_average,
        target_average_local_volume_count=target_average,
        local_volume_count_drift=target_average - source_average,
    )


def metric_measure_drift(
    source_records,
    target_records,
    metric,
    *,
    target_metric=None,
    source_weights=None,
    target_weights=None,
):
    """Compare finite metric-measure spaces by weighted pair-distance observables."""

    source_list = list(source_records)
    target_list = list(target_records)
    target_metric = metric if target_metric is None else target_metric
    source_weights = _normalized_weights(source_list, source_weights, "source")
    target_weights = _normalized_weights(target_list, target_weights, "target")
    source_average, source_diameter = _weighted_pair_observables(source_list, metric, source_weights)
    target_average, target_diameter = _weighted_pair_observables(target_list, target_metric, target_weights)
    source_entropy = _weight_entropy(source_weights)
    target_entropy = _weight_entropy(target_weights)
    return MetricMeasureDrift(
        source_record_count=len(source_list),
        target_record_count=len(target_list),
        source_weights=source_weights,
        target_weights=target_weights,
        source_average_pair_distance=source_average,
        target_average_pair_distance=target_average,
        average_pair_distance_drift=target_average - source_average,
        source_diameter=source_diameter,
        target_diameter=target_diameter,
        diameter_drift=target_diameter - source_diameter,
        source_weight_entropy=source_entropy,
        target_weight_entropy=target_entropy,
        weight_entropy_drift=target_entropy - source_entropy,
    )


def density_hierarchy_quotient(records, metric, radii):
    """Build a research-only hierarchy of radius-component medoid quotients."""

    record_list = list(records)
    requested_radii = _validate_radii(radii)
    normalized_radii = tuple(sorted(requested_radii, reverse=True))
    distance_matrix = _finite_distance_matrix(record_list, metric)
    parent_groups = ()
    levels = []
    for radius in normalized_radii:
        level = _density_quotient_level(distance_matrix, radius, parent_groups)
        levels.append(level)
        parent_groups = level.groups
    representative_indices = () if not levels else levels[-1].representative_indices
    return DensityHierarchyQuotient(
        source_record_count=len(record_list),
        target_record_count=len(representative_indices),
        level_count=len(levels),
        requested_radii=requested_radii,
        radii=normalized_radii,
        representative_indices=representative_indices,
        levels=tuple(levels),
    )


def metric_graph_spanner(records, metric, *, stretch=2.0):
    """Build a deterministic greedy graph spanner over finite metric distances."""

    stretch_bound = float(stretch)
    if not math.isfinite(stretch_bound) or stretch_bound < 1.0:
        raise ValueError("stretch must be finite and >= 1.0")

    record_list = list(records)
    record_count = len(record_list)
    pair_distances = []
    for lhs in range(record_count):
        for rhs in range(lhs + 1, record_count):
            distance = _finite_distance(metric(record_list[lhs], record_list[rhs]))
            pair_distances.append((distance, lhs, rhs))
    pair_distances.sort(key=lambda item: (item[0], item[1], item[2]))

    adjacency = {index: [] for index in range(record_count)}
    edges = []
    for distance, lhs, rhs in pair_distances:
        current = _shortest_path(adjacency, lhs, rhs)
        threshold = stretch_bound * distance
        if current > threshold:
            _add_edge(adjacency, lhs, rhs, distance)
            edges.append(MetricGraphEdge(lhs, rhs, distance))

    stretch_values = []
    disconnected = 0
    for distance, lhs, rhs in pair_distances:
        shortest = _shortest_path(adjacency, lhs, rhs)
        if math.isinf(shortest):
            disconnected += 1
            continue
        if distance == 0.0:
            stretch_value = 1.0 if shortest == 0.0 else math.inf
        else:
            stretch_value = shortest / distance
        stretch_values.append(stretch_value)

    finite_stretches = [value for value in stretch_values if math.isfinite(value)]
    maximum_stretch = max(finite_stretches, default=0.0)
    average_stretch = 0.0 if not finite_stretches else sum(finite_stretches) / len(finite_stretches)
    return MetricGraphSpanner(
        record_count=record_count,
        pair_count=len(pair_distances),
        edge_count=len(edges),
        stretch_bound=stretch_bound,
        maximum_stretch=maximum_stretch,
        average_stretch=average_stretch,
        disconnected_pair_count=disconnected,
        edges=tuple(edges),
    )


def knn_recall_sketch(records, metric, sketch_indices, *, count=1, query_indices=None):
    """Compare source kNN behavior with kNN behavior inside a source-record subset."""

    if isinstance(count, bool):
        raise TypeError("count must be an integer")
    try:
        neighbor_count = int(count.__index__())
    except AttributeError:
        try:
            neighbor_count = int(count)
        except (TypeError, ValueError):
            raise TypeError("count must be an integer") from None
    if neighbor_count <= 0:
        raise ValueError("count must be positive")

    record_list = list(records)
    source_indices = tuple(range(len(record_list)))
    sketch_indices = _validate_indices(sketch_indices, len(record_list), "sketch")
    if query_indices is None:
        query_indices = source_indices
    else:
        query_indices = _validate_indices(query_indices, len(record_list), "query")

    rows = []
    for query_index in query_indices:
        source_neighbors = _nearest_indices(record_list, metric, query_index, source_indices, neighbor_count)
        sketch_neighbors = _nearest_indices(record_list, metric, query_index, sketch_indices, neighbor_count)
        denominator = len(source_neighbors)
        if denominator == 0:
            recall = 1.0
        else:
            recall = len(set(source_neighbors).intersection(sketch_neighbors)) / denominator
        rows.append(
            KNNRecallRow(
                query_index=query_index,
                source_neighbor_indices=source_neighbors,
                sketch_neighbor_indices=sketch_neighbors,
                recall=recall,
            )
        )
    average_recall = 1.0 if not rows else sum(row.recall for row in rows) / len(rows)
    minimum_recall = 1.0 if not rows else min(row.recall for row in rows)
    return KNNRecallSketch(
        source_record_count=len(record_list),
        sketch_record_count=len(sketch_indices),
        query_count=len(query_indices),
        neighbor_count=neighbor_count,
        sketch_indices=sketch_indices,
        average_recall=average_recall,
        minimum_recall=minimum_recall,
        rows=tuple(rows),
    )


def distance_distribution_sketch(
    records,
    metric,
    *,
    quantile_probabilities=(0.0, 0.25, 0.5, 0.75, 1.0),
    bucket_count=10,
    sample_count=None,
    histogram_range=None,
):
    """Sketch unordered pairwise distances of a finite metric space."""

    record_list, pair_total, values = _pair_distances(records, metric, sample_count)
    sorted_values = sorted(values)
    evaluated = len(sorted_values)
    exact = evaluated == pair_total
    if evaluated == 0:
        quantiles = tuple(0.0 for _ in quantile_probabilities)
        edges, counts = _histogram(sorted_values, bucket_count, histogram_range)
        return DistanceDistributionSketch(
            record_count=len(record_list),
            pair_count=pair_total,
            evaluated_pair_count=0,
            sample_count=0,
            exact=exact,
            minimum=0.0,
            maximum=0.0,
            mean=0.0,
            median=0.0,
            quantile_probabilities=tuple(quantile_probabilities),
            quantile_values=quantiles,
            histogram_edges=edges,
            histogram_counts=counts,
        )
    quantiles = tuple(_quantile(sorted_values, probability) for probability in quantile_probabilities)
    edges, counts = _histogram(sorted_values, bucket_count, histogram_range)
    return DistanceDistributionSketch(
        record_count=len(record_list),
        pair_count=pair_total,
        evaluated_pair_count=evaluated,
        sample_count=evaluated,
        exact=exact,
        minimum=float(sorted_values[0]),
        maximum=float(sorted_values[-1]),
        mean=sum(sorted_values) / evaluated,
        median=_quantile(sorted_values, 0.5),
        quantile_probabilities=tuple(quantile_probabilities),
        quantile_values=quantiles,
        histogram_edges=edges,
        histogram_counts=counts,
    )


def distance_distribution_drift(
    source_records,
    target_records,
    metric,
    *,
    target_metric=None,
    quantile_probabilities=(0.0, 0.25, 0.5, 0.75, 1.0),
    bucket_count=10,
    sample_count=None,
):
    """Compare source and target spaces by pairwise distance-distribution drift."""

    target_metric = metric if target_metric is None else target_metric
    source_values = _pair_distances(source_records, metric, sample_count)[2]
    target_values = _pair_distances(target_records, target_metric, sample_count)[2]
    combined = source_values + target_values
    histogram_range = None if not combined else (min(combined), max(combined))
    source = distance_distribution_sketch(
        source_records,
        metric,
        quantile_probabilities=quantile_probabilities,
        bucket_count=bucket_count,
        sample_count=sample_count,
        histogram_range=histogram_range,
    )
    target = distance_distribution_sketch(
        target_records,
        target_metric,
        quantile_probabilities=quantile_probabilities,
        bucket_count=bucket_count,
        sample_count=sample_count,
        histogram_range=histogram_range,
    )
    quantile_drifts = [
        abs(float(source_value) - float(target_value))
        for source_value, target_value in zip(source.quantile_values, target.quantile_values)
    ]
    source_hist_total = sum(source.histogram_counts)
    target_hist_total = sum(target.histogram_counts)
    histogram_l1 = 0.0
    for source_count, target_count in zip(source.histogram_counts, target.histogram_counts):
        source_mass = 0.0 if source_hist_total == 0 else source_count / source_hist_total
        target_mass = 0.0 if target_hist_total == 0 else target_count / target_hist_total
        histogram_l1 += abs(source_mass - target_mass)
    return DistanceDistributionDrift(
        source=source,
        target=target,
        mean_absolute_drift=abs(source.mean - target.mean),
        median_absolute_drift=abs(source.median - target.median),
        maximum_quantile_absolute_drift=max(quantile_drifts, default=0.0),
        histogram_l1_drift=histogram_l1,
    )


__all__ = [
    "AdaptiveDensityEqualization",
    "DensityHierarchyQuotient",
    "DensityHierarchyQuotientLevel",
    "DistanceDistributionDrift",
    "DistanceDistributionSketch",
    "HierarchicalMetricNet",
    "HierarchicalMetricNetLevel",
    "KNNRecallRow",
    "KNNRecallSketch",
    "MetricGraphEdge",
    "MetricGraphSpanner",
    "MetricMeasureDrift",
    "adaptive_density_equalization",
    "density_hierarchy_quotient",
    "distance_distribution_drift",
    "distance_distribution_sketch",
    "hierarchical_metric_net",
    "knn_recall_sketch",
    "metric_graph_spanner",
    "metric_measure_drift",
]
