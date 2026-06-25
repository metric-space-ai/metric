"""Metric-space operator surface for the revived Python API.

This module is an adapter boundary, not an algorithm library. The result
dataclasses marshal native results into stable public objects, and a few
helpers invoke the caller's own metric/transform callable. Exact-scan pair,
neighbor, representative-selection, reduction, compression, intrinsic-dimension,
structure-description, grouping, singular-record scoring, density filtering, and aligned
distance-profile-correlation loops are exposed through the native C++ binding;
higher METRIC algorithms (graph construction, embedding, MGC, non-aligned
correlation) remain native-only facades until their binding is exposed. METRIC's
production numerics live in native C++.
"""

from dataclasses import dataclass
import math
import operator
from typing import ClassVar

from metric.exceptions import (
    IncompatibleSpaceError,
    MetricComputationError,
    OptionalDependencyError,
    StrategyUnavailableError,
    UnsupportedOperationError,
)


_RECALL_CALIBRATION_MIN_BUDGET = 4
_RECALL_CALIBRATION_BUDGET_FRACTION = 4
_RECALL_CALIBRATION_MAX_HOLDOUT_RECORDS = 16


def _numpy():
    try:
        import numpy as np
    except ModuleNotFoundError as exc:
        raise OptionalDependencyError(
            "This conversion requires numpy. Install numpy or use to_dict()/plain Python results."
        ) from exc
    return np


def _native_metric_module():
    try:
        from metric._impl import metric as native_metric
    except ModuleNotFoundError as exc:
        raise StrategyUnavailableError(
            "native C++ operator bindings are unavailable because metric._impl.metric could not be imported"
        ) from exc
    return native_metric


def native_metric_module_or_none():
    """Return the native ``metric._impl.metric`` module, or ``None`` if absent.

    Adapter probe: this swallows the missing-extension import error so callers
    (capability introspection) can inspect which native bindings exist without
    ever surfacing a raw ``ModuleNotFoundError``. It performs no computation.
    """
    try:
        from metric._impl import metric as native_metric
    except (ImportError, ModuleNotFoundError):
        return None
    return native_metric


def native_binding_available(name):
    """Report whether the native module exposes the binding ``name``.

    Adapter probe: it imports the native module defensively and checks
    ``hasattr``. Returns ``False`` when the native extension is missing or the
    specific binding has not been promoted. No native call is made.
    """
    native = native_metric_module_or_none()
    return native is not None and hasattr(native, name)


def _normalize_neighbor_count(k=None, count=None):
    if k is not None and count is not None and k != count:
        raise ValueError("use either k or count, not conflicting values")
    requested = count if count is not None else k
    if requested is None:
        requested = 1
    if isinstance(requested, bool):
        raise TypeError("neighbor count must be an integer")
    try:
        requested = operator.index(requested)
    except TypeError:
        raise TypeError("neighbor count must be an integer") from None
    if requested < 0:
        raise ValueError("neighbor count must be non-negative")
    return requested


def _normalize_optional_distance_budget(max_distance_evaluations):
    if max_distance_evaluations is None:
        return None
    if isinstance(max_distance_evaluations, bool):
        raise TypeError("max_distance_evaluations must be an integer")
    try:
        max_distance_evaluations = operator.index(max_distance_evaluations)
    except TypeError:
        raise TypeError("max_distance_evaluations must be an integer") from None
    if max_distance_evaluations < 0:
        raise ValueError("max_distance_evaluations must be non-negative")
    return max_distance_evaluations


def _sample_indices(record_count, sample_count):
    sample_count = min(record_count, sample_count)
    if sample_count <= 0:
        return []
    if sample_count >= record_count:
        return list(range(record_count))
    if sample_count == 1:
        return [0]

    last = record_count - 1
    indices = []
    seen = set()
    for offset in range(sample_count):
        index = round(offset * last / (sample_count - 1))
        if index not in seen:
            indices.append(index)
            seen.add(index)

    index = 0
    while len(indices) < sample_count and index < record_count:
        if index not in seen:
            indices.append(index)
            seen.add(index)
        index += 1
    return sorted(indices)


def _sampled_neighbor_budget_split(record_count, distance_budget):
    candidate_budget = min(record_count, distance_budget)
    if candidate_budget <= 0:
        return 0, 0
    if record_count <= candidate_budget or candidate_budget < _RECALL_CALIBRATION_MIN_BUDGET:
        return candidate_budget, 0

    holdout_count = min(
        record_count - 1,
        max(
            1,
            min(
                _RECALL_CALIBRATION_MAX_HOLDOUT_RECORDS,
                candidate_budget // _RECALL_CALIBRATION_BUDGET_FRACTION,
            ),
        ),
    )
    sample_count = candidate_budget - holdout_count
    if sample_count <= 0:
        return candidate_budget, 0
    return sample_count, holdout_count


def _sampled_neighbor_holdout_indices(record_count, sampled_indices, holdout_count):
    if holdout_count <= 0:
        return []
    sampled_index_set = set(sampled_indices)
    candidates = [
        record_index
        for record_index in range(record_count)
        if record_index not in sampled_index_set
    ]
    return [
        candidates[offset]
        for offset in _sample_indices(len(candidates), holdout_count)
    ]


def _unmeasured_recall_diagnostics(reason, *, recall_distance_evaluations=0):
    return {
        "recall_measured": False,
        "recall": None,
        "standard_error": 0.0,
        "confidence_radius_95": 0.0,
        "recall_sample_query_count": 0,
        "recall_calibration_record_count": 0,
        "recall_relevant_count": 0,
        "recall_hit_count": 0,
        "recall_distance_evaluations": recall_distance_evaluations,
        "recall_measurement_reason": reason,
    }


def _recall_confidence_interval(hit_count, relevant_count):
    if relevant_count <= 0:
        return 0.0, 0.0
    recall = hit_count / relevant_count
    variance = recall * (1.0 - recall) / relevant_count
    standard_error = math.sqrt(max(0.0, variance))
    return standard_error, min(1.0, 1.96 * standard_error)


def _sampled_neighbor_recall_diagnostics(
    records,
    metric,
    queries,
    sampled_distances_by_query,
    *,
    sampled_indices,
    holdout_indices,
    remaining_distance_budget,
    count=None,
    radius=None,
    validate_distance=None,
):
    if not queries:
        return _unmeasured_recall_diagnostics("no query was available for recall calibration")
    if not sampled_indices:
        return _unmeasured_recall_diagnostics("no sampled candidates were available for recall calibration")
    if not holdout_indices or remaining_distance_budget <= 0:
        return _unmeasured_recall_diagnostics("no distance budget remained for recall calibration")
    if radius is None and (count is None or count <= 0):
        return _unmeasured_recall_diagnostics("no requested neighbors were available for recall calibration")

    measured_queries = 0
    total_relevant = 0
    total_hits = 0
    total_calibration_records = 0
    recall_distance_evaluations = 0

    for query, sampled_distances in zip(queries, sampled_distances_by_query):
        if recall_distance_evaluations >= remaining_distance_budget:
            break

        window_distances = dict(sampled_distances)
        measured_holdout_count = 0
        for record_index in holdout_indices:
            if recall_distance_evaluations >= remaining_distance_budget:
                break
            distance = metric(query, records[record_index])
            recall_distance_evaluations += 1
            measured_holdout_count += 1
            if validate_distance is not None:
                validate_distance(distance, "query", record_index)
            window_distances[record_index] = distance

        if measured_holdout_count <= 0:
            continue

        if radius is None:
            exact_pairs = sorted(
                window_distances.items(),
                key=lambda item: (item[1], item[0]),
            )[:count]
            sampled_pairs = sorted(
                sampled_distances.items(),
                key=lambda item: (item[1], item[0]),
            )[:count]
        else:
            exact_pairs = [
                (record_index, distance)
                for record_index, distance in window_distances.items()
                if distance <= radius
            ]
            sampled_pairs = [
                (record_index, distance)
                for record_index, distance in sampled_distances.items()
                if distance <= radius
            ]

        exact_ids = {record_index for record_index, _distance in exact_pairs}
        if not exact_ids:
            continue
        sampled_ids = {record_index for record_index, _distance in sampled_pairs}

        measured_queries += 1
        total_relevant += len(exact_ids)
        total_hits += len(exact_ids & sampled_ids)
        total_calibration_records += len(window_distances)

    if measured_queries <= 0 or total_relevant <= 0:
        reason = (
            "calibration window contained no true radius neighbors"
            if radius is not None
            else "calibration window contained no comparable nearest neighbors"
        )
        return _unmeasured_recall_diagnostics(
            reason,
            recall_distance_evaluations=recall_distance_evaluations,
        )

    recall = total_hits / total_relevant
    standard_error, confidence_radius_95 = _recall_confidence_interval(
        total_hits,
        total_relevant,
    )
    return {
        "recall_measured": True,
        "recall": recall,
        "standard_error": standard_error,
        "confidence_radius_95": confidence_radius_95,
        "recall_sample_query_count": measured_queries,
        "recall_calibration_record_count": total_calibration_records,
        "recall_relevant_count": total_relevant,
        "recall_hit_count": total_hits,
        "recall_distance_evaluations": recall_distance_evaluations,
        "recall_measurement_reason": (
            "measured over sampled candidates plus holdout records within the distance budget"
        ),
    }


def _refuse_over_budget_neighbor_helper(operation, record_count, budget):
    raise MetricComputationError(
        f"{operation} refused exact neighbor search before running metric calls: "
        f"records={record_count}, estimated_distance_evaluations={record_count}, "
        f"distance_evaluation_budget={budget}. "
        "Suggested fallback: pass exact=False to use a bounded sampled scan, "
        "reduce the query batch, or raise max_distance_evaluations."
    )


def _sampled_neighbor_scan(records, metric, query, *, count=None, radius=None, indices):
    sampled_distances = {}
    pairs = []
    for record_index in indices:
        distance = metric(query, records[record_index])
        sampled_distances[record_index] = distance
        if radius is None or distance <= radius:
            pairs.append((record_index, distance))
    pairs.sort(key=lambda item: (item[1], item[0]))
    if count is not None:
        pairs = pairs[:count]
    return pairs, sampled_distances


def _sampled_neighbor_pairs(records, metric, query, *, count=None, radius=None, sample_count):
    pairs, _sampled_distances = _sampled_neighbor_scan(
        records,
        metric,
        query,
        count=count,
        radius=radius,
        indices=_sample_indices(len(records), sample_count),
    )
    return pairs


def _sampled_neighbor_diagnostics(record_count, sample_count, *, budget, estimated_distance_evaluations):
    candidate_count = min(record_count, sample_count)
    return {
        "diagnostic": "search_approximation",
        "bounded": True,
        "candidate_policy": "regular_sample",
        "candidate_count": candidate_count,
        "candidate_universe": record_count,
        "candidate_fraction": 1.0 if record_count == 0 else candidate_count / record_count,
        "distance_evaluations": candidate_count,
        "sampled_distance_evaluations": candidate_count,
        "sampled_record_count": candidate_count,
        "estimated_distance_evaluations": estimated_distance_evaluations,
        "distance_evaluation_budget": budget,
        "recall_measured": False,
        "recall": None,
        "standard_error": 0.0,
        "confidence_radius_95": 0.0,
        "recall_sample_query_count": 0,
        "recall_calibration_record_count": 0,
        "recall_relevant_count": 0,
        "recall_hit_count": 0,
        "recall_distance_evaluations": 0,
        "recall_measurement_reason": "no distance budget remained for recall calibration",
        "approximation_reason": (
            "exact neighbor scan exceeded max_distance_evaluations; bounded sampled candidates were used"
        ),
    }


def _representative_strategy_seed(strategy):
    if strategy is None:
        return 0, "farthest_first"

    from metric.strategies import Coverage, FarthestFirst, KCenter

    if isinstance(strategy, FarthestFirst):
        return _normalize_neighbor_count(strategy.seed_index, None), "farthest_first"
    if isinstance(strategy, Coverage):
        return _normalize_neighbor_count(strategy.seed_index, None), "coverage"
    if isinstance(strategy, KCenter):
        return _normalize_neighbor_count(strategy.seed_index, None), "k_center"

    _require_native_binding(
        "find_representatives(...)", f"{type(strategy).__name__} representative selection"
    )


def _representative_diagnostics(record_count, distances):
    if record_count == 0:
        return 0.0, 0.0
    coverage_radius = max(distances) if distances else 0.0
    average_nearest_distance = float(sum(distances) / record_count) if distances else 0.0
    return coverage_radius, average_nearest_distance


def _assign_to_representatives(records, metric, representatives):
    assignments, distances = _native_metric_module().assign_to_representatives(
        list(records),
        metric,
        list(representatives),
    )
    return tuple(assignments), tuple(distances)


def _compression_representative_measure(assignments, representative_count, source_record_count):
    if len(assignments) != source_record_count:
        raise MetricComputationError(
            "compression assignments must match source record count"
        )

    multiplicities = [0] * representative_count
    for assignment in assignments:
        representative_index = int(assignment)
        if representative_index < 0 or representative_index >= representative_count:
            raise MetricComputationError(
                "compression assignment references an unknown representative"
            )
        multiplicities[representative_index] += 1

    if source_record_count:
        weights = tuple(multiplicity / source_record_count for multiplicity in multiplicities)
    else:
        weights = tuple(0.0 for _ in multiplicities)
    return tuple(multiplicities), weights


def _compression_validity(strategy):
    return (
        f"weighted metric-measure compression by {strategy}; kept records are an unmodified subset under the "
        "source metric; assignments induce representative multiplicities and normalized weights; not dimension "
        "reduction"
    )


def _radius_coverage_representative_indices(records, metric, radius):
    if radius < 0:
        raise ValueError("coverage radius must be non-negative")
    if not records:
        raise ValueError("cannot compress an empty metric space")

    covered = [False] * len(records)
    selected = []
    covered_count = 0
    while covered_count < len(records):
        seed_index = next(index for index, is_covered in enumerate(covered) if not is_covered)
        selected.append(seed_index)
        seed_record = records[seed_index]
        for index, record in enumerate(records):
            if not covered[index] and metric(seed_record, record) <= radius:
                covered[index] = True
                covered_count += 1
    return tuple(selected)


def _regular_thin_indices(record_count, count, offset=0):
    count = _normalize_neighbor_count(count, None)
    offset = _normalize_neighbor_count(offset, None)
    if count == 0:
        return ()
    if record_count == 0:
        raise ValueError("cannot thin a non-empty set from an empty record set")
    if count > record_count:
        raise ValueError("thin count cannot exceed record count")
    if offset >= record_count:
        raise IndexError("thin offset is outside the record set")
    return tuple(((index * record_count) // count + offset) % record_count for index in range(count))


def _assign_positions_to_selected(records, metric, selected):
    if not selected:
        return (), ()
    assignments = []
    distances = []
    for record in records:
        best_position = 0
        best_distance = metric(record, records[selected[0]])
        for candidate_position, selected_index in enumerate(selected[1:], start=1):
            distance = metric(record, records[selected_index])
            if distance < best_distance or (
                distance == best_distance and selected_index < selected[best_position]
            ):
                best_position = candidate_position
                best_distance = distance
        assignments.append(best_position)
        distances.append(best_distance)
    return tuple(assignments), tuple(distances)


def _average_nearest_other_distance(records, metric):
    if len(records) < 2:
        return 0.0
    total = 0.0
    for index, record in enumerate(records):
        best_distance = None
        for other_index, other in enumerate(records):
            if index == other_index:
                continue
            distance = metric(record, other)
            if best_distance is None or distance < best_distance:
                best_distance = distance
        total += float(best_distance)
    return total / len(records)


def _average_local_volume(records, metric, radius):
    if not records:
        return 0.0, 0.0
    total_count = 0.0
    for record in records:
        count = 0
        for other in records:
            if metric(record, other) <= radius:
                count += 1
        total_count += count
    average_count = total_count / len(records)
    return average_count, average_count / len(records)


def _uniform_density_diagnostics(records, metric, selected, radius):
    assignments, distances = _assign_positions_to_selected(records, metric, selected)
    multiplicities, weights = _compression_representative_measure(
        assignments,
        len(selected),
        len(records),
    )
    selected_records = [records[index] for index in selected]
    coverage_radius = max(distances) if distances else 0.0
    average_assignment_distance = (sum(float(distance) for distance in distances) / len(distances)) if distances else 0.0
    source_knn = _average_nearest_other_distance(records, metric)
    target_knn = _average_nearest_other_distance(selected_records, metric)
    source_volume_count, source_volume_density = _average_local_volume(records, metric, radius)
    target_volume_count, target_volume_density = _average_local_volume(selected_records, metric, radius)
    return {
        "diagnostic": "uniform_density_thinning",
        "policy": "maximal_radius_net",
        "radius": radius,
        "source_record_count": len(records),
        "target_record_count": len(selected),
        "assignments": assignments,
        "nearest_representative_distances": distances,
        "representative_multiplicities": multiplicities,
        "representative_weights": weights,
        "coverage_radius": coverage_radius,
        "average_assignment_distance": average_assignment_distance,
        "source_average_nearest_neighbor_distance": source_knn,
        "target_average_nearest_neighbor_distance": target_knn,
        "local_density_drift": target_knn - source_knn,
        "local_volume_radius": radius,
        "source_average_local_volume_count": source_volume_count,
        "target_average_local_volume_count": target_volume_count,
        "local_volume_count_drift": target_volume_count - source_volume_count,
        "source_average_local_volume_density": source_volume_density,
        "target_average_local_volume_density": target_volume_density,
        "local_volume_density_drift": target_volume_density - source_volume_density,
        "empirical_density_preserved": False,
    }


def _clustering_result_from_payload(payload):
    return ClusteringResult(
        assignments=tuple(payload["assignments"]),
        medoids=tuple(payload["medoids"]),
        core_records=tuple(payload["core_records"]),
        unassigned_records=tuple(payload["unassigned_records"]),
        cluster_sizes=tuple(payload["cluster_sizes"]),
        record_count=int(payload["record_count"]),
        cluster_count=int(payload["cluster_count"]),
        unassigned_count=int(payload["unassigned_count"]),
        iterations=int(payload["iterations"]),
        converged=bool(payload["converged"]),
        algorithm=str(payload["algorithm"]),
        representation=str(payload["representation"]),
    )


def _outlier_result_from_payload(payload):
    return OutlierResult(
        outliers=tuple(Outlier(record_id=int(record_id), score=score) for record_id, score in payload["outliers"]),
        record_count=int(payload["record_count"]),
        cluster_count=int(payload["cluster_count"]),
        unassigned_count=int(payload["unassigned_count"]),
        exact=bool(payload["exact"]),
        operator_name=str(payload["operator_name"]),
        strategy=str(payload["strategy"]),
        representation=str(payload["representation"]),
    )


def _normalize_positive_count(value, name):
    requested = _normalize_neighbor_count(value, None)
    if requested <= 0:
        raise ValueError(f"{name} must be positive")
    return requested


def _resolve_distance_profile_strategy(strategy):
    """Resolve the comparison strategy to its native algorithm name.

    The promoted Python path supports distance-profile (Pearson) correlation.
    ``None`` and a :class:`metric.strategies.DistanceProfileCorrelation` token
    both select that native path; any other strategy stays native-only.
    """

    if strategy is None:
        return "distance_profile_correlation"

    from metric.strategies import DistanceProfileCorrelation

    if isinstance(strategy, DistanceProfileCorrelation):
        if strategy.method != "pearson":
            raise StrategyUnavailableError(
                "compare/correlate only promote the 'pearson' distance-profile method in Python; "
                f"{strategy.method!r} is native-only"
            )
        return "distance_profile_correlation"

    _require_native_binding(
        "compare_spaces(...)/correlate_spaces(...)",
        f"{type(strategy).__name__} cross-space comparison",
    )


def _correlation_result_from_payload(
    payload,
    *,
    matched_ids,
    dropped_left_ids,
    dropped_right_ids,
    align,
    p_value,
):
    diagnostics = payload["diagnostics"]
    diagnostics = dict(diagnostics) if diagnostics is not None else None
    return CorrelationResult(
        value=float(payload["value"]),
        left_record_count=int(payload["left_record_count"]),
        right_record_count=int(payload["right_record_count"]),
        pair_count=int(payload["pair_count"]),
        exact=bool(payload["exact"]),
        algorithm=str(payload["algorithm"]),
        strategy=str(payload["strategy"]),
        left_representation=str(payload["left_representation"]),
        right_representation=str(payload["right_representation"]),
        statistic_name=str(payload["statistic_name"]),
        p_value=p_value,
        matched_ids=tuple(matched_ids),
        dropped_left_ids=tuple(dropped_left_ids),
        dropped_right_ids=tuple(dropped_right_ids),
        align=align,
        local_scores=(),
        diagnostics=diagnostics,
    )


def _require_aligned_record_counts(left_count, right_count):
    """Marshal-time guard: aligned comparison needs equal record counts."""
    if left_count != right_count:
        raise IncompatibleSpaceError(
            "aligned compare/correlate requires equal record counts; "
            f"left space has {left_count} records and right space has {right_count} records"
        )


def _raise_unsupported_inverse(result):
    if getattr(result, "operator_name", None) == "density_filter":
        raise UnsupportedOperationError(
            "density-filter results do not support inverse_transform() because source records were dropped. "
            "Use result.space for the filtered metric space, space.embed(...) for a derived coordinate view, "
            "or a strategy that declares inverse_supported=True when available."
        )

    raise UnsupportedOperationError(
        f"{type(result).__name__} does not support inverse_transform(); "
        "the result declares inverse_supported=False. Use result.space for the derived metric space "
        "or choose a strategy that declares inverse_supported=True when available."
    )


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

    def to_dict(self):
        return {
            "strategy": self.strategy,
            "record_count": self.record_count,
            "edge_count": self.edge_count,
            "directed": self.directed,
            "self_loops": self.self_loops,
            "exact": self.exact,
            "k": self.k,
            "radius": self.radius,
            "edge_payload": self.edge_payload,
            "weighting": self.weighting,
            "symmetrization": self.symmetrization,
            "normalization": self.normalization,
            "tie_break": self.tie_break,
            "max_out_degree": self.max_out_degree,
            "sparsification": self.sparsification,
        }


@dataclass(frozen=True)
class GraphConstructionResult:
    """Graph construction result with directed edge tuples and metadata."""

    edges: tuple
    metadata: GraphConstructionMetadata

    def to_dict(self):
        return {
            "edges": self.edges,
            "metadata": self.metadata.to_dict(),
        }

    def to_numpy(self):
        return _numpy().asarray(self.edges, dtype=object)

    def to_pandas(self):
        try:
            import pandas as pd
        except ModuleNotFoundError:
            raise OptionalDependencyError(
                "GraphConstructionResult.to_pandas() requires pandas. Install pandas or use to_dict()."
            ) from None

        metadata = self.metadata.to_dict()
        rows = []
        for source_record_id, target_record_id, distance in self.edges:
            rows.append(
                {
                    "source_record_id": source_record_id,
                    "target_record_id": target_record_id,
                    "distance": distance,
                    "strategy": metadata["strategy"],
                    "directed": metadata["directed"],
                    "edge_payload": metadata["edge_payload"],
                    "weighting": metadata["weighting"],
                    "symmetrization": metadata["symmetrization"],
                    "normalization": metadata["normalization"],
                    "sparsification": metadata["sparsification"],
                }
            )
        return pd.DataFrame.from_records(rows)


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

    def to_dict(self):
        return {
            "record_count": self.record_count,
            "edge_count": self.edge_count,
            "directed": self.directed,
            "degrees": self.degrees,
            "out_degrees": self.out_degrees,
            "in_degrees": self.in_degrees,
            "isolated_count": self.isolated_count,
            "max_degree": self.max_degree,
            "average_degree": self.average_degree,
            "degree_policy": self.degree_policy,
        }

    def to_numpy(self):
        return _numpy().asarray(self.degrees, dtype=int)

    def to_pandas(self):
        try:
            import pandas as pd
        except ModuleNotFoundError:
            raise OptionalDependencyError(
                "GraphDegreeDiagnostics.to_pandas() requires pandas. Install pandas or use to_dict()."
            ) from None

        rows = []
        for record_id, degree in enumerate(self.degrees):
            rows.append(
                {
                    "record_id": record_id,
                    "degree": degree,
                    "out_degree": self.out_degrees[record_id],
                    "in_degree": self.in_degrees[record_id],
                    "directed": self.directed,
                    "degree_policy": self.degree_policy,
                }
            )
        return pd.DataFrame.from_records(rows)


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

    def to_dict(self):
        return {
            "record_count": self.record_count,
            "edge_count": self.edge_count,
            "directed": self.directed,
            "component_labels": self.component_labels,
            "component_count": self.component_count,
            "isolated_count": self.isolated_count,
            "largest_component_size": self.largest_component_size,
            "connected": self.connected,
            "connectivity_policy": self.connectivity_policy,
        }

    def to_numpy(self):
        return _numpy().asarray(self.component_labels, dtype=int)

    def to_pandas(self):
        try:
            import pandas as pd
        except ModuleNotFoundError:
            raise OptionalDependencyError(
                "GraphConnectivityDiagnostics.to_pandas() requires pandas. Install pandas or use to_dict()."
            ) from None

        rows = []
        for record_id, component_label in enumerate(self.component_labels):
            rows.append(
                {
                    "record_id": record_id,
                    "component_label": component_label,
                    "directed": self.directed,
                    "connected": self.connected,
                    "connectivity_policy": self.connectivity_policy,
                }
            )
        return pd.DataFrame.from_records(rows)


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

    def to_dict(self):
        return {
            "record_count": self.record_count,
            "edge_count": self.edge_count,
            "directed": self.directed,
            "pair_count": self.pair_count,
            "reachable_pair_count": self.reachable_pair_count,
            "unreachable_pair_count": self.unreachable_pair_count,
            "zero_metric_pair_count": self.zero_metric_pair_count,
            "max_stretch": self.max_stretch,
            "average_stretch": self.average_stretch,
            "stretch_policy": self.stretch_policy,
        }

    def to_numpy(self):
        return _numpy().asarray(
            [
                self.pair_count,
                self.reachable_pair_count,
                self.unreachable_pair_count,
                self.zero_metric_pair_count,
                self.max_stretch,
                self.average_stretch,
            ],
            dtype=float,
        )

    def to_pandas(self):
        try:
            import pandas as pd
        except ModuleNotFoundError:
            raise OptionalDependencyError(
                "GraphStretchDiagnostics.to_pandas() requires pandas. Install pandas or use to_dict()."
            ) from None

        return pd.DataFrame.from_records([self.to_dict()])


@dataclass(frozen=True)
class Neighbor:
    """One neighbor record in a finite metric space."""

    id: int
    record: object
    distance: object
    rank: int

    def to_dict(self):
        return {
            "id": self.id,
            "record": self.record,
            "distance": self.distance,
            "rank": self.rank,
        }

    def __eq__(self, other):
        if isinstance(other, Neighbor):
            return (
                self.id == other.id
                and self.record == other.record
                and self.distance == other.distance
                and self.rank == other.rank
            )
        return False


@dataclass(frozen=True)
class NeighborResult:
    """Neighbor result container."""

    query: object
    query_id: object
    neighbors: tuple
    rows: tuple
    distances: tuple
    exact: bool
    strategy: str
    representation: str
    diagnostics: object = None
    route: str = "source_metric"
    provenance: object = None

    def __len__(self):
        return len(self.rows) if self.rows else len(self.neighbors)

    def __iter__(self):
        if self.rows:
            return iter([list(row) for row in self.rows])
        return iter(self.neighbors)

    def __getitem__(self, index):
        if self.rows:
            return list(self.rows[index])
        return self.neighbors[index]

    def __eq__(self, other):
        if isinstance(other, NeighborResult):
            return (
                self.query == other.query
                and self.query_id == other.query_id
                and self.neighbors == other.neighbors
                and self.rows == other.rows
                and self.distances == other.distances
                and self.exact == other.exact
                and self.strategy == other.strategy
                and self.representation == other.representation
                and self.diagnostics == other.diagnostics
                and self.route == other.route
                and self.provenance == other.provenance
            )
        return False

    def to_dict(self):
        return {
            "query": self.query,
            "query_id": self.query_id,
            "neighbors": [neighbor.to_dict() for neighbor in self.neighbors],
            "rows": [
                [neighbor.to_dict() for neighbor in row]
                for row in self.rows
            ],
            "distances": self.distances,
            "exact": self.exact,
            "strategy": self.strategy,
            "representation": self.representation,
            "diagnostics": self.diagnostics,
            "route": self.route,
            "provenance": self.provenance,
        }

    def to_numpy(self):
        try:
            return _numpy().asarray(self.distances, dtype=float)
        except ValueError:
            return _numpy().asarray(self.distances, dtype=object)

    def to_pandas(self):
        try:
            import pandas as pd
        except ModuleNotFoundError:
            raise OptionalDependencyError(
                "NeighborResult.to_pandas() requires pandas. Install pandas or use to_dict()."
            ) from None

        records = []
        if self.rows:
            for source_id, row in enumerate(self.rows):
                for neighbor in row:
                    record = neighbor.to_dict()
                    record["source_id"] = source_id
                    records.append(record)
        else:
            records = [neighbor.to_dict() for neighbor in self.neighbors]
        return pd.DataFrame.from_records(records)


@dataclass(frozen=True)
class ClusteringResult:
    """Engine-style grouping result with assignments and cluster metadata."""

    unassigned_label: ClassVar[int] = -1

    assignments: tuple
    medoids: tuple
    core_records: tuple
    unassigned_records: tuple
    cluster_sizes: tuple
    record_count: int
    cluster_count: int
    unassigned_count: int
    iterations: int
    converged: bool
    algorithm: str
    representation: str

    def to_dict(self):
        return {
            "assignments": self.assignments,
            "medoids": self.medoids,
            "core_records": self.core_records,
            "unassigned_records": self.unassigned_records,
            "cluster_sizes": self.cluster_sizes,
            "record_count": self.record_count,
            "cluster_count": self.cluster_count,
            "unassigned_count": self.unassigned_count,
            "iterations": self.iterations,
            "converged": self.converged,
            "algorithm": self.algorithm,
            "representation": self.representation,
            "unassigned_label": self.unassigned_label,
        }

    def to_numpy(self):
        return _numpy().asarray(self.assignments, dtype=int)

    def to_pandas(self):
        try:
            import pandas as pd
        except ModuleNotFoundError:
            raise OptionalDependencyError(
                "ClusteringResult.to_pandas() requires pandas. Install pandas or use to_dict()."
            ) from None

        medoids = set(self.medoids)
        core_records = set(self.core_records)
        unassigned_records = set(self.unassigned_records)
        rows = []
        for record_id, cluster_label in enumerate(self.assignments):
            cluster_size = None
            if 0 <= cluster_label < len(self.cluster_sizes):
                cluster_size = self.cluster_sizes[cluster_label]
            rows.append(
                {
                    "record_id": record_id,
                    "cluster_label": cluster_label,
                    "cluster_size": cluster_size,
                    "is_medoid": record_id in medoids,
                    "is_core": record_id in core_records,
                    "is_unassigned": record_id in unassigned_records or cluster_label == self.unassigned_label,
                    "algorithm": self.algorithm,
                    "representation": self.representation,
                }
            )
        return pd.DataFrame.from_records(rows)


@dataclass(frozen=True)
class Outlier:
    """One outlier record ID with a deterministic isolation score."""

    record_id: int
    score: object

    def to_dict(self):
        return {
            "record_id": self.record_id,
            "score": self.score,
        }


@dataclass(frozen=True)
class OutlierResult:
    """Engine-style outlier result with strategy and representation metadata."""

    outliers: tuple
    record_count: int
    cluster_count: int
    unassigned_count: int
    exact: bool
    operator_name: str
    strategy: str
    representation: str

    def to_dict(self):
        return {
            "outliers": [outlier.to_dict() for outlier in self.outliers],
            "record_count": self.record_count,
            "cluster_count": self.cluster_count,
            "unassigned_count": self.unassigned_count,
            "exact": self.exact,
            "operator_name": self.operator_name,
            "strategy": self.strategy,
            "representation": self.representation,
        }

    def to_numpy(self):
        scores = [outlier.score for outlier in self.outliers]
        try:
            return _numpy().asarray(scores, dtype=float)
        except (TypeError, ValueError):
            return _numpy().asarray(scores, dtype=object)

    def to_pandas(self):
        try:
            import pandas as pd
        except ModuleNotFoundError:
            raise OptionalDependencyError(
                "OutlierResult.to_pandas() requires pandas. Install pandas or use to_dict()."
            ) from None

        rows = []
        for rank, outlier in enumerate(self.outliers):
            record = outlier.to_dict()
            record["rank"] = rank
            record["operator_name"] = self.operator_name
            record["strategy"] = self.strategy
            record["representation"] = self.representation
            record["exact"] = self.exact
            rows.append(record)
        return pd.DataFrame.from_records(rows)


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

    def to_dict(self):
        return {
            "representatives": self.representatives,
            "nearest_representative_distances": self.nearest_representative_distances,
            "record_count": self.record_count,
            "requested_count": self.requested_count,
            "coverage_radius": self.coverage_radius,
            "average_nearest_distance": self.average_nearest_distance,
            "exact": self.exact,
            "strategy": self.strategy,
            "representation": self.representation,
        }

    def to_numpy(self):
        return _numpy().asarray(self.representatives, dtype=int)

    def to_pandas(self):
        try:
            import pandas as pd
        except ModuleNotFoundError:
            raise OptionalDependencyError(
                "RepresentativeSet.to_pandas() requires pandas. Install pandas or use to_dict()."
            ) from None

        representatives = set(self.representatives)
        rows = []
        for record_id, distance in enumerate(self.nearest_representative_distances):
            rows.append(
                {
                    "record_id": record_id,
                    "nearest_representative_distance": distance,
                    "is_representative": record_id in representatives,
                    "strategy": self.strategy,
                    "representation": self.representation,
                }
            )
        return pd.DataFrame.from_records(rows)


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

    def inverse_transform(self, *args, **kwargs):
        _raise_unsupported_inverse(self)

    def to_dict(self):
        return {
            "source_record_ids": self.source_record_ids,
            "assignments": self.assignments,
            "nearest_representative_distances": self.nearest_representative_distances,
            "source_record_count": self.source_record_count,
            "reduced_record_count": self.reduced_record_count,
            "exact": self.exact,
            "operator_name": self.operator_name,
            "strategy": self.strategy,
            "representation": self.representation,
            "inverse_supported": self.inverse_supported,
        }

    def to_numpy(self):
        return _numpy().asarray(self.assignments, dtype=int)

    def to_pandas(self):
        try:
            import pandas as pd
        except ModuleNotFoundError:
            raise OptionalDependencyError(
                "ReductionResult.to_pandas() requires pandas. Install pandas or use to_dict()."
            ) from None

        selected = set(self.source_record_ids)
        rows = []
        for source_record_id, reduced_record_id in enumerate(self.assignments):
            rows.append(
                {
                    "source_record_id": source_record_id,
                    "reduced_record_id": reduced_record_id,
                    "representative_source_id": self.source_record_ids[reduced_record_id],
                    "nearest_representative_distance": self.nearest_representative_distances[source_record_id],
                    "is_representative": source_record_id in selected,
                    "strategy": self.strategy,
                    "representation": self.representation,
                }
            )
        return pd.DataFrame.from_records(rows)


@dataclass(frozen=True)
class CompressionResult:
    """Compressed metric-space result with source-record lineage."""

    space: object
    source_record_ids: tuple
    assignments: tuple
    nearest_representative_distances: tuple
    representative_multiplicities: tuple
    representative_weights: tuple
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
    metric_status: str = "unknown"
    validity: str = ""

    def inverse_transform(self, *args, **kwargs):
        _raise_unsupported_inverse(self)

    def to_dict(self):
        return {
            "source_record_ids": self.source_record_ids,
            "assignments": self.assignments,
            "nearest_representative_distances": self.nearest_representative_distances,
            "representative_multiplicities": self.representative_multiplicities,
            "representative_weights": self.representative_weights,
            "source_record_count": self.source_record_count,
            "compressed_record_count": self.compressed_record_count,
            "compression_ratio": self.compression_ratio,
            "exact": self.exact,
            "operator_name": self.operator_name,
            "compression": self.compression,
            "strategy": self.strategy,
            "representation": self.representation,
            "lossy": self.lossy,
            "inverse_supported": self.inverse_supported,
            "metric_status": self.metric_status,
            "validity": self.validity,
        }

    def to_numpy(self):
        return _numpy().asarray(self.assignments, dtype=int)

    def to_pandas(self):
        try:
            import pandas as pd
        except ModuleNotFoundError:
            raise OptionalDependencyError(
                "CompressionResult.to_pandas() requires pandas. Install pandas or use to_dict()."
            ) from None

        selected = set(self.source_record_ids)
        rows = []
        for source_record_id, compressed_record_id in enumerate(self.assignments):
            rows.append(
                {
                    "source_record_id": source_record_id,
                    "compressed_record_id": compressed_record_id,
                    "representative_source_id": self.source_record_ids[compressed_record_id],
                    "nearest_representative_distance": self.nearest_representative_distances[source_record_id],
                    "representative_multiplicity": self.representative_multiplicities[compressed_record_id],
                    "representative_weight": self.representative_weights[compressed_record_id],
                    "is_representative": source_record_id in selected,
                    "compression": self.compression,
                    "strategy": self.strategy,
                    "representation": self.representation,
                    "metric_status": self.metric_status,
                    "validity": self.validity,
                }
            )
        return pd.DataFrame.from_records(rows)


@dataclass(frozen=True)
class MappingResult:
    """Mapped metric-space result with source-to-target lineage.

    ``metric_status`` mirrors the C++ pipeline contract where the target metric
    law is known. Python callable metrics generally report ``"unknown"`` rather
    than pretending that an arbitrary callable has been validated as a metric.
    """

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
    source_records: tuple = ()
    representative_records: tuple = ()
    assignments: tuple = ()
    nearest_representative_distances: tuple = ()
    representative_multiplicities: tuple = ()
    representative_weights: tuple = ()
    coverage_radius: object = None
    average_assignment_distance: object = None
    metric_status: str = "unknown"
    out_of_sample_supported: bool = False
    validity: str = ""
    mapping_artifact: object = None
    diagnostics: object = None

    def inverse_transform(self, records=None):
        if self.inverse_supported and self.mapping_artifact is not None:
            latent_records = self.space.records if records is None else records
            return self.mapping_artifact.inverse_transform(latent_records)
        _raise_unsupported_inverse(self)

    def to_dict(self):
        return {
            "source_record_ids": self.source_record_ids,
            "source_record_count": self.source_record_count,
            "target_record_count": self.target_record_count,
            "exact": self.exact,
            "operator_name": self.operator_name,
            "mapping": self.mapping,
            "strategy": self.strategy,
            "representation": self.representation,
            "inverse_supported": self.inverse_supported,
            "source_records": self.source_records,
            "representative_records": self.representative_records,
            "assignments": self.assignments,
            "nearest_representative_distances": self.nearest_representative_distances,
            "representative_multiplicities": self.representative_multiplicities,
            "representative_weights": self.representative_weights,
            "coverage_radius": self.coverage_radius,
            "average_assignment_distance": self.average_assignment_distance,
            "metric_status": self.metric_status,
            "out_of_sample_supported": self.out_of_sample_supported,
            "validity": self.validity,
            "diagnostics": self.diagnostics,
        }

    def to_numpy(self):
        return _numpy().asarray(self.source_record_ids, dtype=object)

    def to_pandas(self):
        try:
            import pandas as pd
        except ModuleNotFoundError:
            raise OptionalDependencyError(
                "MappingResult.to_pandas() requires pandas. Install pandas or use to_dict()."
            ) from None

        rows = []
        for target_record_id, source_record_id in enumerate(self.source_record_ids):
            row = {
                "target_record_id": target_record_id,
                "source_record_id": source_record_id,
                "mapping": self.mapping,
                "strategy": self.strategy,
                "representation": self.representation,
                "metric_status": self.metric_status,
                "out_of_sample_supported": self.out_of_sample_supported,
                "validity": self.validity,
            }
            if self.source_records:
                row["source_records"] = self.source_records[target_record_id]
            if self.representative_records:
                row["representative_record"] = self.representative_records[target_record_id]
            if self.representative_multiplicities:
                row["representative_multiplicity"] = self.representative_multiplicities[target_record_id]
            if self.representative_weights:
                row["representative_weight"] = self.representative_weights[target_record_id]
            if self.coverage_radius is not None:
                row["coverage_radius"] = self.coverage_radius
            if self.average_assignment_distance is not None:
                row["average_assignment_distance"] = self.average_assignment_distance
            rows.append(row)
        return pd.DataFrame.from_records(rows)


@dataclass(frozen=True)
class StructureDescription:
    """Finite-space structure diagnostics."""

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
    diagnostics: object = None

    def to_dict(self):
        return {
            "record_count": self.record_count,
            "pair_count": self.pair_count,
            "zero_distance_pair_count": self.zero_distance_pair_count,
            "minimum_nonzero_distance": self.minimum_nonzero_distance,
            "maximum_distance": self.maximum_distance,
            "average_distance": self.average_distance,
            "intrinsic_dimension": self.intrinsic_dimension,
            "has_nonzero_distances": self.has_nonzero_distances,
            "exact": self.exact,
            "strategy": self.strategy,
            "representation": self.representation,
            "diagnostics": self.diagnostics,
        }

    def to_numpy(self):
        return _numpy().asarray(
            [
                self.record_count,
                self.pair_count,
                self.zero_distance_pair_count,
                self.minimum_nonzero_distance,
                self.maximum_distance,
                self.average_distance,
                self.intrinsic_dimension,
            ],
            dtype=float,
        )

    def to_pandas(self):
        try:
            import pandas as pd
        except ModuleNotFoundError:
            raise OptionalDependencyError(
                "StructureDescription.to_pandas() requires pandas. Install pandas or use to_dict()."
            ) from None

        return pd.DataFrame.from_records([self.to_dict()])


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
    statistic_name: str = "distance_profile_correlation"
    p_value: object = None
    matched_ids: tuple = ()
    dropped_left_ids: tuple = ()
    dropped_right_ids: tuple = ()
    align: str = "position"
    local_scores: tuple = ()
    diagnostics: object = None

    def to_dict(self):
        return {
            "value": self.value,
            "left_record_count": self.left_record_count,
            "right_record_count": self.right_record_count,
            "pair_count": self.pair_count,
            "exact": self.exact,
            "algorithm": self.algorithm,
            "strategy": self.strategy,
            "left_representation": self.left_representation,
            "right_representation": self.right_representation,
            "statistic_name": self.statistic_name,
            "p_value": self.p_value,
            "matched_ids": self.matched_ids,
            "dropped_left_ids": self.dropped_left_ids,
            "dropped_right_ids": self.dropped_right_ids,
            "align": self.align,
            "local_scores": self.local_scores,
            "diagnostics": self.diagnostics,
        }

    def to_numpy(self):
        return _numpy().asarray([self.value], dtype=float)

    def to_pandas(self):
        try:
            import pandas as pd
        except ModuleNotFoundError:
            raise OptionalDependencyError(
                "CorrelationResult.to_pandas() requires pandas. Install pandas or use to_dict()."
            ) from None

        return pd.DataFrame.from_records([self.to_dict()])


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

    def to_dict(self):
        return {
            "raw_stress": self.raw_stress,
            "normalized_stress": self.normalized_stress,
            "distance_correlation": self.distance_correlation,
            "trustworthiness": self.trustworthiness,
            "neighbor_k": self.neighbor_k,
            "finite_coordinates": self.finite_coordinates,
            "coordinate_scale": self.coordinate_scale,
        }


@dataclass(frozen=True)
class EmbeddingArtifact:
    """Metadata for a deterministic embedding artifact."""

    method: str
    dimensions: int
    source_record_ids: tuple

    def to_dict(self):
        return {
            "method": self.method,
            "dimensions": self.dimensions,
            "source_record_ids": self.source_record_ids,
        }


@dataclass(frozen=True)
class EmbeddingResult:
    """Derived coordinate view of a finite metric space."""

    coordinates: object
    embedded_space: object
    source_space: object
    mapping_artifact: EmbeddingArtifact
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

    def to_dict(self):
        return {
            "coordinates": _numpy().asarray(self.coordinates).tolist(),
            "mapping_artifact": self.mapping_artifact.to_dict(),
            "source_record_ids": self.source_record_ids,
            "source_record_count": self.source_record_count,
            "dimensions": self.dimensions,
            "stress": self.stress,
            "trustworthiness": self.trustworthiness,
            "exact": self.exact,
            "operator_name": self.operator_name,
            "strategy": self.strategy,
            "representation": self.representation,
            "diagnostics": self.diagnostics.to_dict(),
        }

    def to_numpy(self):
        return _numpy().array(self.coordinates, dtype=float, copy=True)

    def to_pandas(self):
        try:
            import pandas as pd
        except ModuleNotFoundError:
            raise OptionalDependencyError(
                "EmbeddingResult.to_pandas() requires pandas. Install pandas or use to_dict()."
            ) from None

        coordinates = self.to_numpy()
        if coordinates.ndim == 1:
            coordinates = coordinates.reshape((-1, 1))
        rows = []
        for row_index, source_record_id in enumerate(self.source_record_ids):
            row = {"source_record_id": source_record_id}
            for dimension in range(coordinates.shape[1]):
                row[f"coordinate_{dimension}"] = coordinates[row_index, dimension]
            row["strategy"] = self.strategy
            row["representation"] = self.representation
            rows.append(row)
        return pd.DataFrame.from_records(rows)


# ---------------------------------------------------------------------------
# Adapter boundary
#
# The result dataclasses above are pure marshaling containers: they hold and
# reshape values, they do not compute METRIC framework results. Everything
# below this line is the operator surface. METRIC is a native C++ finite
# metric-space framework; promoted pairwise/exact-scan loops and production
# algorithms must run in native C++ and be reached through a binding. The Python
# package is an adapter only:
#
#   * helpers that merely invoke the caller's metric/transform callable or
#     marshal native results into the dataclasses above stay live, and
#   * named algorithms without a promoted binding raise StrategyUnavailableError.
# ---------------------------------------------------------------------------


def _require_native_binding(operation, kind):
    raise StrategyUnavailableError(
        f"{operation} requires native C++ binding support. The METRIC Python package is an "
        f"adapter-only surface: {kind} must run in native C++ and be reached through a binding, "
        f"and is not implemented in Python. No native binding for this operation is exposed in "
        f"metric._impl yet."
    )


def _refuse_dense_materialization(operation, plan):
    fallback = ", ".join(plan.fallback)
    raise MetricComputationError(
        f"{operation} refused dense all-pairs materialization: "
        f"records={plan.record_count}, "
        f"estimated_bytes={plan.memory_bytes_estimate}, "
        f"budget_bytes={plan.max_memory_bytes}, "
        f"estimated_distance_evaluations={plan.estimated_distance_evaluations}, "
        f"distance_evaluation_budget={plan.max_distance_evaluations}, "
        f"max_dense_records={plan.max_dense_records}. "
        f"Reason: {plan.reason}. Suggested fallback: {fallback}."
    )


def pairwise_distance_matrix(
    records,
    metric,
    *,
    max_memory_bytes=None,
    max_distance_evaluations=None,
    max_dense_records=None,
):
    """Materialize the explicit finite metric-space distance matrix.

    Native adapter: the all-pairs loop runs in C++ and invokes the supplied
    metric callable for each pair after the dense materialization budget accepts
    the requested matrix.
    """
    record_list = list(records)
    from metric.spaces import _make_plan

    plan = _make_plan(
        "pairwise_distances",
        len(record_list),
        exact=True,
        max_memory_bytes=max_memory_bytes,
        max_distance_evaluations=max_distance_evaluations,
        max_dense_records=max_dense_records,
        allow_approximate=False,
        allow_chunking=False,
    )
    if plan.decision == "refused":
        _refuse_dense_materialization("pairwise_distance_matrix(...)", plan)
    return _native_metric_module().pairwise_distance_matrix(record_list, metric)


def neighbor_result(
    records,
    *,
    query=None,
    query_id=None,
    neighbors=None,
    rows=None,
    exact=True,
    strategy="exact_scan",
    representation="metric_space",
    diagnostics=None,
    route=None,
    provenance=None,
):
    """Marshal raw ``(record_id, distance)`` neighbor data into a NeighborResult.

    Adapter: this reshapes neighbor tuples (as a native neighbor binding would
    return them) into the public result object. It performs no search.
    """

    def make_neighbor(raw_neighbor, rank):
        record_id, distance = raw_neighbor
        return Neighbor(
            id=record_id,
            record=records[record_id],
            distance=distance,
            rank=rank,
        )

    if rows is not None:
        result_rows = tuple(
            tuple(make_neighbor(raw_neighbor, rank) for rank, raw_neighbor in enumerate(row))
            for row in rows
        )
        distances = tuple(
            tuple(neighbor.distance for neighbor in row)
            for row in result_rows
        )
        result_neighbors = ()
    else:
        result_neighbors = tuple(
            make_neighbor(raw_neighbor, rank)
            for rank, raw_neighbor in enumerate(() if neighbors is None else neighbors)
        )
        result_rows = ()
        distances = tuple(neighbor.distance for neighbor in result_neighbors)

    return NeighborResult(
        query=query,
        query_id=query_id,
        neighbors=result_neighbors,
        rows=result_rows,
        distances=distances,
        exact=exact,
        strategy=strategy,
        representation=representation,
        diagnostics=diagnostics,
        route=route or _neighbor_search_route(strategy, representation),
        provenance=provenance,
    )


def _neighbor_search_route(strategy, representation):
    if representation == "metric_space":
        return f"source_metric:{strategy}"
    return f"representation:{representation}:{strategy}"


def nearest_neighbors(
    records,
    metric,
    query,
    k=None,
    count=None,
    *,
    representation="metric_space",
    provenance=None,
    exact=True,
    max_distance_evaluations=None,
    allow_approximate=True,
):
    """Exact-scan k-nearest-neighbor search through the native C++ binding."""
    record_list = list(records)
    requested = _normalize_neighbor_count(k, count)
    budget = _normalize_optional_distance_budget(max_distance_evaluations)
    if budget is not None and len(record_list) > budget:
        if exact or not allow_approximate:
            _refuse_over_budget_neighbor_helper("nearest_neighbors(...)", len(record_list), budget)
        sample_count, holdout_count = _sampled_neighbor_budget_split(len(record_list), budget)
        sampled_indices = _sample_indices(len(record_list), sample_count)
        holdout_indices = _sampled_neighbor_holdout_indices(
            len(record_list),
            sampled_indices,
            holdout_count,
        )
        pairs, sampled_distances = _sampled_neighbor_scan(
            record_list,
            metric,
            query,
            count=requested,
            indices=sampled_indices,
        )
        diagnostics = _sampled_neighbor_diagnostics(
            len(record_list),
            sample_count,
            budget=budget,
            estimated_distance_evaluations=len(record_list),
        )
        recall_diagnostics = _sampled_neighbor_recall_diagnostics(
            record_list,
            metric,
            (query,),
            (sampled_distances,),
            sampled_indices=sampled_indices,
            holdout_indices=holdout_indices,
            remaining_distance_budget=budget - sample_count,
            count=requested,
        )
        diagnostics.update(recall_diagnostics)
        diagnostics["distance_evaluations"] = (
            diagnostics["sampled_distance_evaluations"]
            + diagnostics["recall_distance_evaluations"]
        )
        return neighbor_result(
            record_list,
            query=query,
            neighbors=pairs,
            exact=False,
            strategy="sampled_knn_budget_guard",
            representation="sampled_live_scan",
            diagnostics=diagnostics,
            provenance=provenance,
        )
    pairs = _native_metric_module().exact_scan_neighbors(
        record_list,
        metric,
        query,
        requested,
    )
    return neighbor_result(
        record_list,
        query=query,
        neighbors=pairs,
        exact=True,
        strategy="exact_scan",
        representation=representation,
        provenance=provenance,
    )


def range_neighbors(
    records,
    metric,
    query,
    radius,
    *,
    representation="metric_space",
    provenance=None,
    exact=True,
    max_distance_evaluations=None,
    allow_approximate=True,
):
    """Exact-scan radius neighbor search through the native C++ binding."""
    record_list = list(records)
    budget = _normalize_optional_distance_budget(max_distance_evaluations)
    if budget is not None and len(record_list) > budget:
        if exact or not allow_approximate:
            _refuse_over_budget_neighbor_helper("range_neighbors(...)", len(record_list), budget)
        sample_count, holdout_count = _sampled_neighbor_budget_split(len(record_list), budget)
        sampled_indices = _sample_indices(len(record_list), sample_count)
        holdout_indices = _sampled_neighbor_holdout_indices(
            len(record_list),
            sampled_indices,
            holdout_count,
        )
        pairs, sampled_distances = _sampled_neighbor_scan(
            record_list,
            metric,
            query,
            radius=radius,
            indices=sampled_indices,
        )
        diagnostics = _sampled_neighbor_diagnostics(
            len(record_list),
            sample_count,
            budget=budget,
            estimated_distance_evaluations=len(record_list),
        )
        recall_diagnostics = _sampled_neighbor_recall_diagnostics(
            record_list,
            metric,
            (query,),
            (sampled_distances,),
            sampled_indices=sampled_indices,
            holdout_indices=holdout_indices,
            remaining_distance_budget=budget - sample_count,
            radius=radius,
        )
        diagnostics.update(recall_diagnostics)
        diagnostics["distance_evaluations"] = (
            diagnostics["sampled_distance_evaluations"]
            + diagnostics["recall_distance_evaluations"]
        )
        return neighbor_result(
            record_list,
            query=query,
            neighbors=pairs,
            exact=False,
            strategy="sampled_range_budget_guard",
            representation="sampled_live_scan",
            diagnostics=diagnostics,
            provenance=provenance,
        )
    pairs = _native_metric_module().exact_scan_radius_neighbors(record_list, metric, query, radius)
    return neighbor_result(
        record_list,
        query=query,
        neighbors=pairs,
        exact=True,
        strategy="exact_range",
        representation=representation,
        provenance=provenance,
    )


def exact_knn_graph(records, metric, k):
    """Exact directed kNN graph construction (native-only)."""
    _require_native_binding("exact_knn_graph(...)", "exact kNN graph construction")


def exact_knn_graph_edges(records, metric, k):
    """Exact directed kNN graph edges (native-only)."""
    _require_native_binding("exact_knn_graph_edges(...)", "exact kNN graph construction")


def exact_radius_graph(records, metric, radius):
    """Exact directed radius graph construction (native-only)."""
    _require_native_binding("exact_radius_graph(...)", "exact radius graph construction")


def exact_radius_graph_edges(records, metric, radius):
    """Exact directed radius graph edges (native-only)."""
    _require_native_binding("exact_radius_graph_edges(...)", "exact radius graph construction")


def symmetrize_graph(graph, policy="union", weighting="minimum_distance"):
    """Symmetrize a graph construction result (native-only)."""
    _require_native_binding("symmetrize_graph(...)", "graph symmetrization")


def prune_graph_out_degree(graph, max_out_degree):
    """Prune a directed graph by out-degree (native-only)."""
    _require_native_binding("prune_graph_out_degree(...)", "graph sparsification")


def graph_degree_diagnostics(graph):
    """Degree diagnostics for a graph construction result (native-only)."""
    _require_native_binding("graph_degree_diagnostics(...)", "graph degree analysis")


def graph_connectivity_diagnostics(graph):
    """Connectivity diagnostics for a graph construction result (native-only)."""
    _require_native_binding("graph_connectivity_diagnostics(...)", "graph connectivity analysis")


def graph_stretch_diagnostics(records, metric, graph):
    """Shortest-path stretch diagnostics for a graph result (native-only)."""
    _require_native_binding("graph_stretch_diagnostics(...)", "graph shortest-path stretch analysis")


def kmedoids(records, metric, groups, max_iterations=100, *, representation="metric_space"):
    """Deterministic k-medoids grouping through the native C++ binding."""
    return _clustering_result_from_payload(
        _native_metric_module().kmedoids(
            list(records),
            metric,
            _normalize_positive_count(groups, "groups"),
            _normalize_positive_count(max_iterations, "max_iterations"),
            representation,
        )
    )


def dbscan(records, metric, radius, min_points, *, representation="metric_space"):
    """Deterministic DBSCAN grouping through the native C++ binding."""
    return _clustering_result_from_payload(
        _native_metric_module().dbscan(
            list(records),
            metric,
            radius,
            _normalize_positive_count(min_points, "min_points"),
            representation,
        )
    )


def find_groups(records, metric, strategy, *, representation="metric_space"):
    """Group records through a promoted native clustering strategy."""
    from metric.strategies import DBSCAN, KMedoids

    if isinstance(strategy, int):
        return kmedoids(records, metric, strategy, representation=representation)
    if isinstance(strategy, KMedoids):
        return kmedoids(records, metric, strategy.groups, strategy.max_iterations, representation=representation)
    if isinstance(strategy, DBSCAN):
        return dbscan(records, metric, strategy.radius, strategy.min_points, representation=representation)
    _require_native_binding("find_groups(...)", f"{type(strategy).__name__} clustering")


def find_outliers(records, metric, strategy, *, representation="metric_space"):
    """Find unusual records through a promoted native scoring strategy."""
    from metric.strategies import DBSCAN

    if isinstance(strategy, DBSCAN):
        return _outlier_result_from_payload(
            _native_metric_module().dbscan_outliers(
                list(records),
                metric,
                strategy.radius,
                _normalize_positive_count(strategy.min_points, "min_points"),
                representation,
            )
        )
    if isinstance(strategy, int):
        return _outlier_result_from_payload(
            _native_metric_module().nearest_neighbor_outliers(
                list(records),
                metric,
                _normalize_neighbor_count(strategy, None),
                representation,
            )
        )
    _require_native_binding("find_outliers(...)", f"{type(strategy).__name__} outlier detection")


def density_filter_space(records, metric, strategy, *, representation="metric_space"):
    """Filter DBSCAN-unassigned records into a derived finite metric space."""
    from metric.strategies import DBSCAN
    from metric.spaces import Space

    if not isinstance(strategy, DBSCAN):
        _require_native_binding("density_filter_space(...)", f"{type(strategy).__name__} density filtering")

    record_list = list(records)
    clustering = dbscan(record_list, metric, strategy.radius, strategy.min_points, representation=representation)
    unassigned = set(clustering.unassigned_records)
    kept_ids = tuple(index for index in range(len(record_list)) if index not in unassigned)
    filtered = [record_list[index] for index in kept_ids]
    validity = (
        "uneven-sampling correction by removing DBSCAN-unassigned records; kept records are an "
        "unmodified subset under the source metric; in-sample only"
    )
    return MappingResult(
        space=Space(
            filtered,
            metric,
            metadata={
                "mapping": "density_filter",
                "strategy": "dbscan_density_filter",
                "metric_status": "unknown",
                "out_of_sample_supported": False,
                "validity": validity,
            },
        ),
        source_record_ids=kept_ids,
        source_record_count=len(record_list),
        target_record_count=len(filtered),
        exact=True,
        operator_name="density_filter",
        mapping="density_filter",
        strategy="dbscan_density_filter",
        representation=representation,
        inverse_supported=False,
        source_records=tuple((record_id,) for record_id in kept_ids),
        representative_records=kept_ids,
        metric_status="unknown",
        out_of_sample_supported=False,
        validity=validity,
    )


def _resample_space(records, metric, count=None, strategy=None, *, radius=None, representation="metric_space", mapping="thin"):
    from metric.strategies import PreserveDistribution, UniformDensity
    from metric.spaces import Space

    if radius is not None:
        if isinstance(strategy, UniformDensity) and strategy.radius != radius:
            raise ValueError("use either radius= or UniformDensity(...), not conflicting radii")
        if strategy is not None and not isinstance(strategy, UniformDensity):
            raise ValueError("uniform-density thinning cannot combine radius= with another strategy")
        strategy = UniformDensity(radius)
    if strategy is None:
        strategy = PreserveDistribution()
    if not isinstance(strategy, (PreserveDistribution, UniformDensity)):
        _require_native_binding("thin_space(...)", f"{type(strategy).__name__} thinning")

    record_list = list(records)
    diagnostics = None
    assignments = ()
    nearest_distances = ()
    representative_multiplicities = ()
    representative_weights = ()
    coverage_radius = None
    average_assignment_distance = None
    if isinstance(strategy, UniformDensity):
        if count is not None:
            raise ValueError("uniform-density thinning chooses the representative count; omit count")
        selected = _radius_coverage_representative_indices(record_list, metric, strategy.radius)
        if mapping == "equalize":
            validity = (
                "density-equalizing deterministic thinning by maximal metric radius net; kept records are an "
                "unmodified radius-separated and radius-covering subset under the source metric; empirical density "
                "is intentionally normalized toward uniform metric coverage"
            )
        else:
            validity = (
                "uniform-density deterministic thinning by maximal metric radius net; kept records are an "
                "unmodified radius-separated and radius-covering subset under the source metric; empirical density "
                "is intentionally flattened"
            )
        strategy_name = "uniform_density_radius_net"
        diagnostics = _uniform_density_diagnostics(record_list, metric, selected, strategy.radius)
        assignments = diagnostics["assignments"]
        nearest_distances = diagnostics["nearest_representative_distances"]
        representative_multiplicities = diagnostics["representative_multiplicities"]
        representative_weights = diagnostics["representative_weights"]
        coverage_radius = diagnostics["coverage_radius"]
        average_assignment_distance = diagnostics["average_assignment_distance"]
    else:
        if mapping == "equalize":
            raise ValueError("equalize requires UniformDensity(...) or radius=")
        if count is None:
            raise ValueError("preserve-distribution thinning requires count")
        selected = _regular_thin_indices(len(record_list), count, strategy.offset)
        representative_multiplicities = tuple(1 for _ in selected)
        representative_weights = (
            tuple(1.0 / len(selected) for _ in selected)
            if selected
            else ()
        )
        validity = (
            "distribution-preserving deterministic thinning; kept records are an unmodified regular sample "
            "under the source metric; retained records carry normalized sample weights and no full-source "
            "assignment map is implied"
        )
        strategy_name = "preserve_distribution_regular"
    return MappingResult(
        space=Space(
            [record_list[index] for index in selected],
            metric,
            metadata={
                "mapping": mapping,
                "strategy": strategy_name,
                "metric_status": "unknown",
                "out_of_sample_supported": False,
                "validity": validity,
                "diagnostics": diagnostics,
            },
        ),
        source_record_ids=selected,
        source_record_count=len(record_list),
        target_record_count=len(selected),
        exact=True,
        operator_name=mapping,
        mapping=mapping,
        strategy=strategy_name,
        representation=representation,
        inverse_supported=False,
        source_records=tuple((index,) for index in selected),
        representative_records=selected,
        assignments=assignments,
        nearest_representative_distances=nearest_distances,
        representative_multiplicities=representative_multiplicities,
        representative_weights=representative_weights,
        coverage_radius=coverage_radius,
        average_assignment_distance=average_assignment_distance,
        metric_status="unknown",
        out_of_sample_supported=False,
        validity=validity,
        diagnostics=diagnostics,
    )


def thin_space(records, metric, count=None, strategy=None, *, radius=None, representation="metric_space"):
    """Distribution-preserving deterministic thinning by regular source order."""
    return _resample_space(
        records,
        metric,
        count=count,
        strategy=strategy,
        radius=radius,
        representation=representation,
        mapping="thin",
    )


def distribution_sample_space(records, metric, count, strategy=None, *, representation="metric_space"):
    """Alias for distribution-preserving thinning."""
    return thin_space(records, metric, count, strategy=strategy, representation=representation)


def uniform_density_sample_space(records, metric, radius, *, representation="metric_space"):
    """Alias for uniform-density thinning by maximal radius net."""
    from metric.strategies import UniformDensity

    return thin_space(records, metric, strategy=UniformDensity(radius), representation=representation)


def equalize_space(records, metric, radius=None, strategy=None, *, representation="metric_space"):
    """Density-equalizing thinning by maximal metric radius net."""
    from metric.strategies import UniformDensity

    if radius is None and strategy is None:
        raise ValueError("equalize_space requires radius= or UniformDensity(...)")
    if strategy is not None and not isinstance(strategy, UniformDensity):
        _require_native_binding("equalize_space(...)", f"{type(strategy).__name__} equalization")
    return _resample_space(
        records,
        metric,
        strategy=strategy,
        radius=radius,
        representation=representation,
        mapping="equalize",
    )


def representative_indices(records, metric, k, seed_index=0):
    """Farthest-first representative selection by index through the native C++ binding."""
    return tuple(
        _native_metric_module().representative_indices(
            list(records),
            metric,
            _normalize_neighbor_count(k, None),
            _normalize_neighbor_count(seed_index, None),
        )
    )


def find_representatives(records, metric, k=None, strategy=None, *, count=None, representation="metric_space"):
    """Representative selection result through the native C++ binding."""
    record_list = list(records)
    requested_count = _normalize_neighbor_count(k, count)
    if record_list and requested_count == 0:
        raise ValueError("representative count must be positive for a non-empty record set")
    seed_index, strategy_name = _representative_strategy_seed(strategy)
    selected = representative_indices(record_list, metric, requested_count, seed_index=seed_index)
    assignments, distances = _assign_to_representatives(record_list, metric, selected)
    coverage_radius, average_nearest_distance = _representative_diagnostics(len(record_list), distances)
    return RepresentativeSet(
        representatives=selected,
        nearest_representative_distances=distances,
        record_count=len(record_list),
        requested_count=requested_count,
        coverage_radius=coverage_radius,
        average_nearest_distance=average_nearest_distance,
        exact=True,
        strategy=strategy_name,
        representation=representation,
    )


def reduce_space(records, metric, count=None, strategy=None, *, representation="metric_space"):
    """Reduce a finite metric space to representative records through the native C++ binding."""
    record_list = list(records)
    requested_count = _normalize_neighbor_count(count, None)
    seed_index, strategy_name = _representative_strategy_seed(strategy)
    selected = representative_indices(record_list, metric, requested_count, seed_index=seed_index)
    assignments, distances = _assign_to_representatives(record_list, metric, selected)

    from metric.spaces import Space

    return ReductionResult(
        space=Space([record_list[index] for index in selected], metric),
        source_record_ids=selected,
        assignments=assignments,
        nearest_representative_distances=distances,
        source_record_count=len(record_list),
        reduced_record_count=len(selected),
        exact=True,
        operator_name="reduce",
        strategy=strategy_name,
        representation=representation,
        inverse_supported=False,
    )


def compress_space(records, metric, count=None, strategy=None, *, radius=None, representation="metric_space"):
    """Compress a finite metric space by retaining representatives through the native C++ binding."""
    from metric.strategies import KMedoids, RadiusCoverage

    record_list = list(records)
    if radius is not None:
        if isinstance(strategy, RadiusCoverage) and strategy.radius != radius:
            raise ValueError("use either radius= or RadiusCoverage(...), not conflicting radii")
        if strategy is not None and not isinstance(strategy, RadiusCoverage):
            raise ValueError("radius compression cannot combine radius= with a count-based strategy")
        strategy = RadiusCoverage(radius)

    if isinstance(strategy, RadiusCoverage):
        if count is not None:
            raise ValueError("radius_coverage compression chooses the representative count; omit count")
        selected = _radius_coverage_representative_indices(record_list, metric, strategy.radius)
        assignments, distances = _assign_to_representatives(record_list, metric, selected)
        multiplicities, weights = _compression_representative_measure(
            assignments,
            len(selected),
            len(record_list),
        )

        from metric.spaces import Space

        ratio = (len(selected) / len(record_list)) if record_list else 0.0
        validity = _compression_validity("radius coverage")
        return CompressionResult(
            space=Space(
                [record_list[index] for index in selected],
                metric,
                metadata={
                    "operator_name": "compress",
                    "strategy": "radius_coverage",
                    "metric_status": "unknown",
                    "validity": validity,
                },
            ),
            source_record_ids=selected,
            assignments=assignments,
            nearest_representative_distances=distances,
            representative_multiplicities=multiplicities,
            representative_weights=weights,
            source_record_count=len(record_list),
            compressed_record_count=len(selected),
            compression_ratio=ratio,
            exact=True,
            operator_name="compress",
            compression="representatives",
            strategy="radius_coverage",
            representation=representation,
            lossy=len(selected) < len(record_list),
            inverse_supported=False,
            metric_status="unknown",
            validity=validity,
        )

    if isinstance(strategy, KMedoids):
        if count is not None and count != strategy.groups:
            raise ValueError("use either count= or KMedoids(groups=...), not conflicting group counts")
        groups = kmedoids(
            record_list,
            metric,
            strategy.groups,
            strategy.max_iterations,
            representation=representation,
        )
        selected = groups.medoids
        assignments, distances = _assign_to_representatives(record_list, metric, selected)
        multiplicities, weights = _compression_representative_measure(
            assignments,
            len(selected),
            len(record_list),
        )

        from metric.spaces import Space

        ratio = (len(selected) / len(record_list)) if record_list else 0.0
        validity = _compression_validity("k-medoids")
        return CompressionResult(
            space=Space(
                [record_list[index] for index in selected],
                metric,
                metadata={
                    "operator_name": "compress",
                    "strategy": "k_medoids",
                    "metric_status": "unknown",
                    "validity": validity,
                },
            ),
            source_record_ids=selected,
            assignments=assignments,
            nearest_representative_distances=distances,
            representative_multiplicities=multiplicities,
            representative_weights=weights,
            source_record_count=len(record_list),
            compressed_record_count=len(selected),
            compression_ratio=ratio,
            exact=True,
            operator_name="compress",
            compression="representatives",
            strategy="k_medoids",
            representation=representation,
            lossy=len(selected) < len(record_list),
            inverse_supported=False,
            metric_status="unknown",
            validity=validity,
        )

    reduction = reduce_space(record_list, metric, count=count, strategy=strategy, representation=representation)
    ratio = (reduction.reduced_record_count / reduction.source_record_count) if reduction.source_record_count else 0.0
    multiplicities, weights = _compression_representative_measure(
        reduction.assignments,
        reduction.reduced_record_count,
        reduction.source_record_count,
    )
    return CompressionResult(
        space=reduction.space,
        source_record_ids=reduction.source_record_ids,
        assignments=reduction.assignments,
        nearest_representative_distances=reduction.nearest_representative_distances,
        representative_multiplicities=multiplicities,
        representative_weights=weights,
        source_record_count=reduction.source_record_count,
        compressed_record_count=reduction.reduced_record_count,
        compression_ratio=ratio,
        exact=True,
        operator_name="compress",
        compression="representatives",
        strategy=reduction.strategy,
        representation=reduction.representation,
        lossy=reduction.reduced_record_count < reduction.source_record_count,
        inverse_supported=False,
        metric_status="unknown",
        validity=_compression_validity(reduction.strategy),
    )


def map_space(records, transform, metric, *, representation="metric_space", source_ids=None):
    """Map records through a caller-supplied deterministic transform.

    Adapter: this applies the caller's own ``transform`` callable to each record
    and wraps the mapped records in a Space using the caller's ``metric``. It is
    element-wise data adaptation (analogous to ``map``), not a derived
    finite-space mapping operator, so it stays on the Python side. Native
    derived-coordinate artifacts are reached through ``metric.mappings``.
    """
    if not callable(transform):
        raise TypeError("transform must be callable")
    if not callable(metric):
        raise TypeError("metric must be callable")

    records = list(records)
    mapped_records = [transform(record) for record in records]
    if source_ids is None:
        source_ids = tuple(range(len(records)))
    else:
        source_ids = tuple(source_ids)
    if len(source_ids) != len(records):
        raise ValueError("source_ids must match the mapped record count")

    from metric.spaces import Space
    validity = (
        "deterministic per-record transform; applicable out-of-sample when the caller's "
        "transform and target metric accept the new record"
    )

    return MappingResult(
        space=Space(
            mapped_records,
            metric,
            ids=source_ids,
            metadata={
                "mapping": "deterministic_transform",
                "strategy": "deterministic_transform",
                "metric_status": "unknown",
                "out_of_sample_supported": True,
                "validity": validity,
            },
        ),
        source_record_ids=source_ids,
        source_record_count=len(records),
        target_record_count=len(mapped_records),
        exact=True,
        operator_name="map",
        mapping="deterministic_transform",
        strategy="deterministic_transform",
        representation=representation,
        inverse_supported=False,
        source_records=tuple((record_id,) for record_id in source_ids),
        representative_records=source_ids,
        metric_status="unknown",
        out_of_sample_supported=True,
        validity=validity,
    )


def embed_space(records, metric, dimensions=2, strategy=None, *, representation="metric_space"):
    """Embed a finite metric space into Euclidean coordinates (native-only)."""
    _require_native_binding("embed_space(...)", "metric-space embedding (e.g. classical MDS)")


def representatives(records, metric, k, seed_index=0):
    """Farthest-first representative records through the native C++ binding."""
    record_list = list(records)
    return tuple(record_list[index] for index in representative_indices(record_list, metric, k, seed_index))


def medoid_index(records, metric):
    """Index of the record with the smallest total distance through the native C++ binding."""
    return _native_metric_module().medoid_index(list(records), metric)


def medoid(records, metric):
    """Record with the smallest total distance to all records through the native C++ binding."""
    record_list = list(records)
    return record_list[medoid_index(record_list, metric)]


def separated_representative_indices(records, metric, minimum_distance):
    """Greedy minimum-distance separated representatives by index through the native C++ binding."""
    return tuple(
        _native_metric_module().separated_representative_indices(
            list(records),
            metric,
            minimum_distance,
        )
    )


def separated_representatives(records, metric, minimum_distance):
    """Greedy minimum-distance separated representative records through the native C++ binding."""
    record_list = list(records)
    return tuple(record_list[index] for index in separated_representative_indices(record_list, metric, minimum_distance))


def coverage_representative_indices(records, metric, radius):
    """Greedy radius-cover representatives by index through the native C++ binding."""
    return tuple(
        _native_metric_module().coverage_representative_indices(
            list(records),
            metric,
            radius,
        )
    )


def coverage_representatives(records, metric, radius):
    """Greedy radius-cover representative records through the native C++ binding."""
    record_list = list(records)
    return tuple(record_list[index] for index in coverage_representative_indices(record_list, metric, radius))


def compare_spaces(
    left_records,
    left_metric,
    right_records,
    right_metric,
    strategy=None,
    *,
    left_representation="records",
    right_representation="records",
    align="position",
    matched_ids=None,
    dropped_left_ids=(),
    dropped_right_ids=(),
    p_value=None,
):
    """Compare aligned finite metric spaces by distance-profile correlation.

    Native adapter: the off-diagonal pairwise distance profiles of both spaces
    are computed in C++ and reduced to a Pearson correlation. The Python side
    only marshals records, validates the aligned ``align="position"`` contract,
    and wraps the native payload in a :class:`CorrelationResult`.

    Only equal-length, ``align="position"`` comparisons are promoted. Mismatched
    record counts raise :class:`metric.exceptions.IncompatibleSpaceError`. Other
    alignment modes (e.g. ``"ids"``) stay native-only. A degenerate profile
    (fewer than two records or zero-variance distances) returns
    ``value=0.0`` with ``diagnostics["defined"] is False``.
    """
    algorithm = _resolve_distance_profile_strategy(strategy)
    if align != "position":
        _require_native_binding(
            "compare_spaces(...)", f"{align!r}-aligned cross-space distance-profile correlation"
        )

    left = list(left_records)
    right = list(right_records)
    _require_aligned_record_counts(len(left), len(right))

    payload = _native_metric_module().distance_profile_correlation(
        left,
        left_metric,
        right,
        right_metric,
        left_representation,
        right_representation,
    )
    matched = tuple(range(len(left))) if matched_ids is None else tuple(matched_ids)
    return _correlation_result_from_payload(
        payload,
        matched_ids=matched,
        dropped_left_ids=dropped_left_ids,
        dropped_right_ids=dropped_right_ids,
        align=align,
        p_value=p_value,
    )


def correlate_spaces(
    left_records,
    left_metric,
    right_records,
    right_metric,
    strategy=None,
    *,
    left_representation="records",
    right_representation="records",
):
    """Correlate aligned finite metric spaces by distance-profile correlation.

    Native adapter alias of :func:`compare_spaces` for the promoted
    ``align="position"`` path; see that function for the full contract.
    """
    return compare_spaces(
        left_records,
        left_metric,
        right_records,
        right_metric,
        strategy,
        left_representation=left_representation,
        right_representation=right_representation,
        align="position",
    )


def intrinsic_dimension(records, metric):
    """Estimate intrinsic expansion dimension through the native C++ binding."""
    return float(_native_metric_module().intrinsic_dimension(list(records), metric))


def describe_structure(records, metric, *, representation="metric_space"):
    """Describe exact finite-space structure through the native C++ binding."""
    payload = _native_metric_module().describe_structure(list(records), metric, representation)
    return StructureDescription(
        record_count=int(payload["record_count"]),
        pair_count=int(payload["pair_count"]),
        zero_distance_pair_count=int(payload["zero_distance_pair_count"]),
        minimum_nonzero_distance=payload["minimum_nonzero_distance"],
        maximum_distance=payload["maximum_distance"],
        average_distance=float(payload["average_distance"]),
        intrinsic_dimension=float(payload["intrinsic_dimension"]),
        has_nonzero_distances=bool(payload["has_nonzero_distances"]),
        exact=bool(payload["exact"]),
        strategy=str(payload["strategy"]),
        representation=str(payload["representation"]),
        diagnostics=payload.get("diagnostics"),
    )


def intrinsic_dimension_from_distances(distances):
    """Estimate intrinsic expansion dimension from a precomputed distance matrix."""
    return float(_native_metric_module().intrinsic_dimension_from_distances(list(distances)))


__all__ = [
    "ClusteringResult",
    "CompressionResult",
    "CorrelationResult",
    "EmbeddingDiagnostics",
    "EmbeddingArtifact",
    "EmbeddingResult",
    "GraphConnectivityDiagnostics",
    "GraphDegreeDiagnostics",
    "GraphStretchDiagnostics",
    "GraphConstructionMetadata",
    "GraphConstructionResult",
    "MappingResult",
    "Neighbor",
    "NeighborResult",
    "Outlier",
    "OutlierResult",
    "RepresentativeSet",
    "ReductionResult",
    "StructureDescription",
    "compare_spaces",
    "correlate_spaces",
    "compress_space",
    "dbscan",
    "density_filter_space",
    "describe_structure",
    "distribution_sample_space",
    "equalize_space",
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
    "native_binding_available",
    "native_metric_module_or_none",
    "neighbor_result",
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
    "thin_space",
    "uniform_density_sample_space",
]
