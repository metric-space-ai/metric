"""Metric-space operator surface for the revived Python API.

This module is an adapter boundary, not an algorithm library. The result
dataclasses marshal native results into stable public objects, and a few
helpers invoke the caller's own metric/transform callable. Exact-scan pair,
neighbor, representative-selection, reduction, compression, intrinsic-dimension,
structure-description, clustering, outlier, and denoise loops are exposed
through the native C++ binding; higher METRIC algorithms (graph construction,
embedding, correlation) remain native-only facades until their binding is
exposed. METRIC's production numerics live in native C++.
"""

from dataclasses import dataclass
import operator
from typing import ClassVar

from metric.exceptions import OptionalDependencyError, StrategyUnavailableError, UnsupportedOperationError


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


def _representative_strategy_seed(strategy):
    if strategy is None:
        return 0, "farthest_first"

    from metric.strategies import FarthestFirst

    if isinstance(strategy, FarthestFirst):
        return _normalize_neighbor_count(strategy.seed_index, None), "farthest_first"

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


def _clustering_result_from_payload(payload):
    return ClusteringResult(
        assignments=tuple(payload["assignments"]),
        medoids=tuple(payload["medoids"]),
        core_records=tuple(payload["core_records"]),
        noise_records=tuple(payload["noise_records"]),
        cluster_sizes=tuple(payload["cluster_sizes"]),
        record_count=int(payload["record_count"]),
        cluster_count=int(payload["cluster_count"]),
        noise_count=int(payload["noise_count"]),
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
        noise_count=int(payload["noise_count"]),
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


def _raise_unsupported_inverse(result):
    if getattr(result, "operator_name", None) == "denoise":
        raise UnsupportedOperationError(
            "denoise results do not support inverse_transform() because density filtering drops source records. "
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
    """One neighbor record with compatibility tuple behavior."""

    id: int
    record: object
    distance: object
    rank: int

    def as_tuple(self):
        return (self.id, self.distance)

    def to_dict(self):
        return {
            "id": self.id,
            "record": self.record,
            "distance": self.distance,
            "rank": self.rank,
        }

    def __iter__(self):
        return iter(self.as_tuple())

    def __len__(self):
        return 2

    def __getitem__(self, index):
        return self.as_tuple()[index]

    def __eq__(self, other):
        if isinstance(other, Neighbor):
            return (
                self.id == other.id
                and self.record == other.record
                and self.distance == other.distance
                and self.rank == other.rank
            )
        if isinstance(other, (list, tuple)) and len(other) == 2:
            return self.as_tuple() == tuple(other)
        return False


@dataclass(frozen=True)
class NeighborResult:
    """Engine-style neighbor result with sequence compatibility."""

    query: object
    query_id: object
    neighbors: tuple
    rows: tuple
    distances: tuple
    exact: bool
    strategy: str
    representation: str
    diagnostics: object = None

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
            )
        return list(self) == other

    def as_tuples(self):
        if self.rows:
            return [[neighbor.as_tuple() for neighbor in row] for row in self.rows]
        return [neighbor.as_tuple() for neighbor in self.neighbors]

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

    def to_dict(self):
        return {
            "assignments": self.assignments,
            "medoids": self.medoids,
            "core_records": self.core_records,
            "noise_records": self.noise_records,
            "cluster_sizes": self.cluster_sizes,
            "record_count": self.record_count,
            "cluster_count": self.cluster_count,
            "noise_count": self.noise_count,
            "iterations": self.iterations,
            "converged": self.converged,
            "algorithm": self.algorithm,
            "representation": self.representation,
            "noise_label": self.noise_label,
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
        noise_records = set(self.noise_records)
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
                    "is_noise": record_id in noise_records or cluster_label == self.noise_label,
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
    noise_count: int
    exact: bool
    operator_name: str
    strategy: str
    representation: str

    def to_dict(self):
        return {
            "outliers": [outlier.to_dict() for outlier in self.outliers],
            "record_count": self.record_count,
            "cluster_count": self.cluster_count,
            "noise_count": self.noise_count,
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

    def inverse_transform(self, *args, **kwargs):
        _raise_unsupported_inverse(self)

    def to_dict(self):
        return {
            "source_record_ids": self.source_record_ids,
            "assignments": self.assignments,
            "nearest_representative_distances": self.nearest_representative_distances,
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
                    "is_representative": source_record_id in selected,
                    "compression": self.compression,
                    "strategy": self.strategy,
                    "representation": self.representation,
                }
            )
        return pd.DataFrame.from_records(rows)


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
    source_records: tuple = ()
    representative_records: tuple = ()
    fitted_model: object = None

    def inverse_transform(self, records=None):
        if self.inverse_supported and self.fitted_model is not None:
            latent_records = self.space.records if records is None else records
            return self.fitted_model.inverse_transform(latent_records)
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
            }
            if self.source_records:
                row["source_records"] = self.source_records[target_record_id]
            if self.representative_records:
                row["representative_record"] = self.representative_records[target_record_id]
            rows.append(row)
        return pd.DataFrame.from_records(rows)


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
class EmbeddingModel:
    """Metadata for a deterministic embedding model."""

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

    def to_dict(self):
        return {
            "coordinates": _numpy().asarray(self.coordinates).tolist(),
            "model": self.model.to_dict(),
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


def pairwise_distance_matrix(records, metric):
    """Materialize the explicit finite metric-space distance matrix.

    Native adapter: the all-pairs loop runs in C++ and invokes the supplied
    metric callable for each pair.
    """
    return _native_metric_module().pairwise_distance_matrix(list(records), metric)


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
    )


def nearest_neighbors(records, metric, query, k=None, count=None):
    """Exact-scan k-nearest-neighbor search through the native C++ binding."""
    record_list = list(records)
    pairs = _native_metric_module().exact_scan_neighbors(
        record_list,
        metric,
        query,
        _normalize_neighbor_count(k, count),
    )
    return neighbor_result(
        record_list,
        query=query,
        neighbors=pairs,
        exact=True,
        strategy="exact_scan",
        representation="metric_space",
    )


def range_neighbors(records, metric, query, radius):
    """Exact-scan radius neighbor search through the native C++ binding."""
    record_list = list(records)
    pairs = _native_metric_module().exact_scan_radius_neighbors(record_list, metric, query, radius)
    return neighbor_result(
        record_list,
        query=query,
        neighbors=pairs,
        exact=True,
        strategy="exact_range",
        representation="metric_space",
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


def denoise_space(records, metric, strategy, *, representation="metric_space"):
    """Filter DBSCAN noise records into a derived metric space through the native C++ binding."""
    from metric.strategies import DBSCAN
    from metric.spaces import Space

    if not isinstance(strategy, DBSCAN):
        _require_native_binding("denoise_space(...)", f"{type(strategy).__name__} denoising")

    record_list = list(records)
    clustering = dbscan(record_list, metric, strategy.radius, strategy.min_points, representation=representation)
    noise = set(clustering.noise_records)
    kept_ids = tuple(index for index in range(len(record_list)) if index not in noise)
    filtered = [record_list[index] for index in kept_ids]
    return MappingResult(
        space=Space(filtered, metric),
        source_record_ids=kept_ids,
        source_record_count=len(record_list),
        target_record_count=len(filtered),
        exact=True,
        operator_name="denoise",
        mapping="dbscan_noise_filter",
        strategy="dbscan_noise",
        representation=representation,
        inverse_supported=False,
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


def compress_space(records, metric, count=None, strategy=None, *, representation="metric_space"):
    """Compress a finite metric space by retaining representatives through the native C++ binding."""
    record_list = list(records)
    reduction = reduce_space(record_list, metric, count=count, strategy=strategy, representation=representation)
    ratio = (reduction.reduced_record_count / reduction.source_record_count) if reduction.source_record_count else 0.0
    return CompressionResult(
        space=reduction.space,
        source_record_ids=reduction.source_record_ids,
        assignments=reduction.assignments,
        nearest_representative_distances=reduction.nearest_representative_distances,
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
    )


def map_space(records, transform, metric, *, representation="metric_space"):
    """Map records through a caller-supplied deterministic transform.

    Adapter: this applies the caller's own ``transform`` callable to each record
    and wraps the mapped records in a Space using the caller's ``metric``. It is
    element-wise data adaptation (analogous to ``map``), not a METRIC mapping
    algorithm (PCFA/SOM/KOC/DSPCC/PHATE-AE), so it stays on the Python side.
    Learned/structural mappings are reached through ``Space.map(strategy=...)``
    and the native bindings in ``metric.mappings``.
    """
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
        representation=representation,
        inverse_supported=False,
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
    """Compare aligned finite metric spaces by distance-profile correlation (native-only)."""
    _require_native_binding("compare_spaces(...)", "cross-space distance-profile correlation")


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
    """Correlate aligned finite metric spaces (native-only)."""
    _require_native_binding("correlate_spaces(...)", "cross-space distance-profile correlation")


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
    )


def intrinsic_dimension_from_distances(distances):
    """Estimate intrinsic expansion dimension from a precomputed distance matrix."""
    return float(_native_metric_module().intrinsic_dimension_from_distances(list(distances)))


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
]
