"""Mapping namespace for finite-space derived-coordinate and quotient surfaces.

Mapping surfaces are currently beta extensions. This module provides a stable
import location without making them part of the core finite-space API.
"""

from collections.abc import Mapping as MappingABC
from dataclasses import dataclass
import copy
import json
import operator

from metric.exceptions import MetricComputationError, MetricInputError, StrategyUnavailableError


STABILITY = "beta"
_NATIVE_MAPPING_ARTIFACT_FORMATS = frozenset({
    "metric.parametric_diffusion_coordinate_artifact",
})
_PARAMETRIC_DIFFUSION_DENSE_CELL_BYTES = 64
_CLUSTERED_SPACE_DENSE_CELL_BYTES = 32


@dataclass(frozen=True)
class ClusterRecord:
    """One derived clustered-space record with source-record lineage."""

    label: int
    representative: object
    members: tuple
    unassigned: bool = False


@dataclass(frozen=True)
class ClusteredSpaceDerivation:
    """Derived artifact that produces a cluster-level finite metric space."""

    records: tuple
    distances: tuple
    source_records: tuple
    representative_records: tuple
    source_record_count: int
    strategy: str
    representation: str

    def transform(self, space=None):
        if space is not None and len(space) != self.source_record_count:
            raise ValueError("source space size does not match clustered-space derivation")

        distances = self.distances

        def cluster_distance(lhs, rhs):
            return distances[lhs.label][rhs.label]

        from metric.operators import MappingResult
        from metric.spaces import Space

        validity = (
            "cluster-representative space; pairwise distances are inherited from the source metric "
            "between representatives; in-sample only"
        )
        return MappingResult(
            space=Space(
                list(self.records),
                metric=cluster_distance,
                metadata={
                    "mapping": "clustered_space",
                    "strategy": self.strategy,
                    "metric_status": "unknown",
                    "out_of_sample_supported": False,
                    "validity": validity,
                },
            ),
            source_record_ids=tuple(self.representative_records),
            source_record_count=self.source_record_count,
            target_record_count=len(self.records),
            exact=True,
            operator_name="map",
            mapping="clustered_space",
            strategy=self.strategy,
            representation=self.representation,
            inverse_supported=False,
            source_records=self.source_records,
            representative_records=self.representative_records,
            metric_status="unknown",
            out_of_sample_supported=False,
            validity=validity,
        )

    def inverse_supported(self):
        return False


@dataclass(frozen=True)
class ClusteredSpaceMapping:
    """Mapping adapter from a ClusteringResult into a clustered Space."""

    clustering: object
    runtime: object = None
    max_memory_bytes: object = None
    max_distance_evaluations: object = None
    max_dense_records: object = None

    def derive_from(
        self,
        space,
        *,
        runtime=None,
        max_memory_bytes=None,
        max_distance_evaluations=None,
        max_dense_records=None,
    ):
        return _build_clustered_space_derivation(
            space,
            self.clustering,
            runtime=self.runtime if runtime is None else runtime,
            max_memory_bytes=(
                self.max_memory_bytes if max_memory_bytes is None else max_memory_bytes
            ),
            max_distance_evaluations=(
                self.max_distance_evaluations
                if max_distance_evaluations is None
                else max_distance_evaluations
            ),
            max_dense_records=(
                self.max_dense_records if max_dense_records is None else max_dense_records
            ),
        )


@dataclass(frozen=True)
class NativeMappingArtifact:
    """Read-only Python projection of a native C++ mapping artifact.

    This object exposes manifest, provenance, and pipeline metadata only. It is
    not a Python implementation of the native transform.
    """

    manifest: object
    diagnostics: object = None
    network_byte_count: object = None

    def __post_init__(self):
        manifest = _coerce_manifest(self.manifest, "manifest")
        diagnostics = (
            None
            if self.diagnostics is None
            else _coerce_manifest(self.diagnostics, "diagnostics")
        )
        network_byte_count = self.network_byte_count
        if network_byte_count is None:
            network_byte_count = _nested(manifest, ("network", "byte_count"), 0)
        network_byte_count = _coerce_count(network_byte_count, "network_byte_count")

        _validate_native_mapping_manifest(manifest)
        object.__setattr__(self, "manifest", manifest)
        object.__setattr__(self, "diagnostics", diagnostics)
        object.__setattr__(self, "network_byte_count", network_byte_count)

    @property
    def format(self):
        return self.manifest["format"]

    @property
    def format_version(self):
        return self.manifest["format_version"]

    @property
    def backend(self):
        return self.manifest["backend"]

    @property
    def scalar_type(self):
        return self.manifest.get("scalar_type")

    @property
    def mapping(self):
        return self.manifest["mapping"]["name"]

    @property
    def strategy(self):
        return self.manifest["mapping"]["strategy"]

    @property
    def target(self):
        return self.manifest["mapping"].get("target")

    @property
    def pipeline(self):
        return copy.deepcopy(self.manifest.get("pipeline", {}))

    @property
    def pipeline_name(self):
        return _nested(self.manifest, ("pipeline", "name"))

    @property
    def components(self):
        return tuple(copy.deepcopy(_nested(self.manifest, ("pipeline", "components"), ())))

    @property
    def component_names(self):
        return tuple(
            component.get("name")
            for component in self.components
            if isinstance(component, MappingABC) and component.get("name") is not None
        )

    @property
    def source_record_count(self):
        return _nested(self.manifest, ("source", "record_count"))

    @property
    def source_coordinate_count(self):
        return _nested(self.manifest, ("source", "source_coordinate_count"))

    @property
    def source_space_version(self):
        return _nested(self.manifest, ("source", "space_version"))

    @property
    def transform_supported(self):
        return False

    @property
    def inverse_supported(self):
        return False

    def transform(self, space=None):
        raise StrategyUnavailableError(
            "Native mapping artifact transform requires the native C++ binding. "
            "Python exposes artifact metadata only and must not reimplement mapping math."
        )

    def inverse_transform(self, records=None):
        raise StrategyUnavailableError(
            "Native mapping artifact inverse transform requires the native C++ binding. "
            "Python exposes artifact metadata only and must not reimplement decoder math."
        )

    def to_dict(self):
        return {
            "format": self.format,
            "format_version": self.format_version,
            "backend": self.backend,
            "scalar_type": self.scalar_type,
            "mapping": self.mapping,
            "strategy": self.strategy,
            "target": self.target,
            "pipeline": self.pipeline_name,
            "components": self.components,
            "source_record_count": self.source_record_count,
            "source_coordinate_count": self.source_coordinate_count,
            "source_space_version": self.source_space_version,
            "network_byte_count": self.network_byte_count,
            "transform_supported": self.transform_supported,
            "inverse_supported": self.inverse_supported,
            "manifest": copy.deepcopy(self.manifest),
            "diagnostics": copy.deepcopy(self.diagnostics),
        }


@dataclass(frozen=True)
class ParametricDiffusionCoordinateArtifact:
    """Python handle for a native C++ parametric coordinate artifact."""

    _artifact: object
    distance_provider: str
    affinity_kernel: str
    diffusion_operator: str

    @property
    def mapping(self):
        return "parametric_diffusion_coordinates"

    @property
    def strategy(self):
        return "native_metric_diffusion_coordinate_solver"

    @property
    def source_record_count(self):
        return self._artifact.source_record_count

    @property
    def latent_dimension(self):
        return self._artifact.latent_dimension

    @property
    def inverse_supported(self):
        return self._artifact.inverse_supported

    @property
    def transform_supported(self):
        return True

    def transform(
        self,
        records,
        *,
        runtime=None,
        max_memory_bytes=None,
        max_distance_evaluations=None,
        max_dense_records=None,
    ):
        records = _preflight_parametric_diffusion_records(
            "ParametricDiffusionCoordinateArtifact.transform(...)",
            records,
            runtime=runtime,
            max_memory_bytes=max_memory_bytes,
            max_distance_evaluations=max_distance_evaluations,
            max_dense_records=max_dense_records,
        )
        return self._artifact.transform(records)

    def inverse_transform(
        self,
        latent_records,
        *,
        runtime=None,
        max_memory_bytes=None,
        max_distance_evaluations=None,
        max_dense_records=None,
    ):
        latent_records = _preflight_parametric_diffusion_records(
            "ParametricDiffusionCoordinateArtifact.inverse_transform(...)",
            latent_records,
            runtime=runtime,
            max_memory_bytes=max_memory_bytes,
            max_distance_evaluations=max_distance_evaluations,
            max_dense_records=max_dense_records,
        )
        return self._artifact.inverse_transform(latent_records)

    def calibration_report(self):
        return dict(self._artifact.calibration_report())

    def to_dict(self):
        return {
            "mapping": self.mapping,
            "strategy": self.strategy,
            "source_record_count": self.source_record_count,
            "latent_dimension": self.latent_dimension,
            "distance_provider": self.distance_provider,
            "affinity_kernel": self.affinity_kernel,
            "diffusion_operator": self.diffusion_operator,
            "transform_supported": self.transform_supported,
            "inverse_supported": self.inverse_supported,
            "calibration_report": self.calibration_report(),
        }


def _load_native_engine_module():
    try:
        from metric._impl import metric as native_metric
    except (ImportError, ModuleNotFoundError):
        return None
    return native_metric


def _preflight_parametric_diffusion_records(
    operation,
    records,
    *,
    runtime=None,
    max_memory_bytes=None,
    max_distance_evaluations=None,
    max_dense_records=None,
):
    from metric.operators import _ensure_exact_metric_work_allowed

    try:
        record_count = len(records)
    except TypeError:
        materialized = []
        for record in records:
            observed = len(materialized) + 1
            _ensure_exact_metric_work_allowed(
                operation,
                observed,
                runtime=runtime,
                max_memory_bytes=max_memory_bytes,
                max_distance_evaluations=max_distance_evaluations,
                max_dense_records=max_dense_records,
                dense=True,
                dense_cell_bytes=_PARAMETRIC_DIFFUSION_DENSE_CELL_BYTES,
            )
            materialized.append(record)
        records = materialized
        record_count = len(records)

    _ensure_exact_metric_work_allowed(
        operation,
        record_count,
        runtime=runtime,
        max_memory_bytes=max_memory_bytes,
        max_distance_evaluations=max_distance_evaluations,
        max_dense_records=max_dense_records,
        dense=True,
        dense_cell_bytes=_PARAMETRIC_DIFFUSION_DENSE_CELL_BYTES,
    )
    return records


def _parametric_diffusion_native_budgets(
    *,
    runtime=None,
    max_memory_bytes=None,
    max_distance_evaluations=None,
    max_dense_records=None,
):
    from metric.operators import _resolve_runtime_budgets, _runtime_policy_or_none
    from metric.spaces import (
        _DEFAULT_MAX_DENSE_RECORDS,
        _DEFAULT_MAX_DISTANCE_EVALUATIONS,
        _DEFAULT_MAX_MEMORY_BYTES,
        _normalize_optional_budget,
    )

    policy = _runtime_policy_or_none(runtime)
    max_memory_bytes, max_distance_evaluations, max_dense_records = _resolve_runtime_budgets(
        policy,
        max_memory_bytes=max_memory_bytes,
        max_distance_evaluations=max_distance_evaluations,
        max_dense_records=max_dense_records,
    )
    return {
        "max_memory_bytes": _normalize_optional_budget(
            max_memory_bytes,
            "max_memory_bytes",
            _DEFAULT_MAX_MEMORY_BYTES,
        ),
        "max_distance_evaluations": _normalize_optional_budget(
            max_distance_evaluations,
            "max_distance_evaluations",
            _DEFAULT_MAX_DISTANCE_EVALUATIONS,
        ),
        "max_dense_records": _normalize_optional_budget(
            max_dense_records,
            "max_dense_records",
            _DEFAULT_MAX_DENSE_RECORDS,
        ),
    }


def _call_native_with_optional_diffusion_budgets(function, records, native_kwargs):
    native_kwargs = dict(native_kwargs)
    optional_budget_keys = (
        "max_memory_bytes",
        "max_distance_evaluations",
        "max_dense_records",
    )
    while True:
        try:
            return function(records, **native_kwargs)
        except TypeError as exc:
            unsupported_key = _unsupported_optional_budget_keyword(
                str(exc),
                native_kwargs,
                optional_budget_keys,
            )
            if unsupported_key is None:
                raise
            native_kwargs.pop(unsupported_key)


def _unsupported_optional_budget_keyword(message, native_kwargs, optional_budget_keys):
    lowered = message.lower()
    signature_mismatch = (
        "unexpected keyword" in lowered
        or "invalid keyword" in lowered
        or "incompatible function arguments" in lowered
    )
    if not signature_mismatch:
        return None
    for key in optional_budget_keys:
        if key in native_kwargs and key in message:
            return key
    return None


def derive_from(mapping, space, **kwargs):
    """Derive a mapping artifact from a finite metric space."""
    return mapping.derive_from(space, **kwargs)


def transform(mapping_artifact, space=None):
    """Transform a finite metric space through a derived mapping artifact."""
    return mapping_artifact.transform(space)


def load_native_mapping_artifact(artifact):
    """Project a native mapping artifact into Python metadata.

    Accepts a manifest mapping, a JSON manifest string/bytes object, a bundle
    with a ``manifest`` field, or a future bound C++ artifact object with a
    ``manifest`` attribute.
    """

    manifest = artifact
    diagnostics = None
    network_byte_count = None

    if hasattr(artifact, "manifest"):
        manifest = artifact.manifest
        diagnostics = getattr(artifact, "diagnostics", None)
        network = getattr(artifact, "network_cereal", None)
        if network is not None:
            network_byte_count = len(network)
    elif isinstance(artifact, MappingABC) and "manifest" in artifact:
        manifest = artifact["manifest"]
        diagnostics = artifact.get("diagnostics")
        network = artifact.get("network_cereal", artifact.get("network_bytes"))
        if network is not None:
            network_byte_count = len(network)

    return NativeMappingArtifact(
        manifest=manifest,
        diagnostics=diagnostics,
        network_byte_count=network_byte_count,
    )


def derive_parametric_diffusion_coordinates(
    records,
    *,
    dimensions=1,
    calibration_steps=100,
    step_size=0.001,
    diffusion_steps=2,
    kernel_scale=1.0,
    reconstruction_weight=0.05,
    geometry_weight=1.0,
    seed=23,
    distance_provider="exact_metric_space_distance_provider",
    affinity_kernel="gaussian_affinity_kernel",
    diffusion_operator="row_normalized_diffusion_operator",
    runtime=None,
    max_memory_bytes=None,
    max_distance_evaluations=None,
    max_dense_records=None,
):
    """Derive a native C++ parametric diffusion coordinate artifact.

    This is a binding adapter. It marshals Python list/NumPy row data to the
    native C++ implementation and does not compute diffusion geometry or
    coordinate calibration in Python.

    The native path is deterministic by construction: a given input always
    yields the same coordinates.
    """

    records = _preflight_parametric_diffusion_records(
        "derive_parametric_diffusion_coordinates(...)",
        records,
        runtime=runtime,
        max_memory_bytes=max_memory_bytes,
        max_distance_evaluations=max_distance_evaluations,
        max_dense_records=max_dense_records,
    )

    native_metric = _load_native_engine_module()
    if native_metric is None:
        raise StrategyUnavailableError(
            "Native parametric diffusion coordinate derivation requires the native C++ binding. "
            "Python must not reimplement the metric-space dynamics."
        )

    derive_vectors = getattr(native_metric, "_parametric_diffusion_coordinate_derive_vectors", None)
    if derive_vectors is None:
        raise StrategyUnavailableError(
            "Native parametric diffusion coordinate derivation requires the native C++ binding. "
            "Python must not reimplement the metric-space dynamics."
        )

    native_kwargs = {
        "dimensions": dimensions,
        "calibration_steps": calibration_steps,
        "step_size": step_size,
        "diffusion_steps": diffusion_steps,
        "kernel_scale": kernel_scale,
        "reconstruction_weight": reconstruction_weight,
        "geometry_weight": geometry_weight,
        "seed": seed,
        "distance_provider": distance_provider,
        "affinity_kernel": affinity_kernel,
        "diffusion_operator": diffusion_operator,
        **_parametric_diffusion_native_budgets(
            runtime=runtime,
            max_memory_bytes=max_memory_bytes,
            max_distance_evaluations=max_distance_evaluations,
            max_dense_records=max_dense_records,
        ),
    }
    try:
        artifact = _call_native_with_optional_diffusion_budgets(
            derive_vectors,
            records,
            native_kwargs,
        )
    except ValueError as exc:
        message = str(exc)
        if "distance provider" in message:
            raise ValueError("unsupported parametric diffusion coordinate distance provider") from exc
        if "affinity kernel" in message:
            raise ValueError("unsupported parametric diffusion coordinate affinity kernel") from exc
        if "diffusion operator" in message:
            raise ValueError("unsupported parametric diffusion coordinate diffusion operator") from exc
        raise
    return ParametricDiffusionCoordinateArtifact(
        artifact,
        distance_provider=distance_provider,
        affinity_kernel=affinity_kernel,
        diffusion_operator=diffusion_operator,
    )


def make_clustered_space_mapping(
    clustering,
    *,
    runtime=None,
    max_memory_bytes=None,
    max_distance_evaluations=None,
    max_dense_records=None,
):
    """Create a mapping adapter from an engine-style clustering result."""
    return ClusteredSpaceMapping(
        clustering,
        runtime=runtime,
        max_memory_bytes=max_memory_bytes,
        max_distance_evaluations=max_distance_evaluations,
        max_dense_records=max_dense_records,
    )


def clustered_space(
    space,
    clustering,
    *,
    runtime=None,
    max_memory_bytes=None,
    max_distance_evaluations=None,
    max_dense_records=None,
):
    """Derive a cluster-level Space from a source Space and clustering result."""
    return (
        make_clustered_space_mapping(
            clustering,
            runtime=runtime,
            max_memory_bytes=max_memory_bytes,
            max_distance_evaluations=max_distance_evaluations,
            max_dense_records=max_dense_records,
        )
        .derive_from(space)
        .transform(space)
    )


def _build_clustered_space_derivation(
    space,
    clustering,
    *,
    runtime=None,
    max_memory_bytes=None,
    max_distance_evaluations=None,
    max_dense_records=None,
):
    record_count = _coerce_count(getattr(clustering, "record_count", None), "record_count")
    if record_count != len(space):
        raise ValueError("clustering record count does not match source space")

    cluster_count = _coerce_count(getattr(clustering, "cluster_count", None), "cluster_count")
    if cluster_count == 0:
        raise ValueError("cannot derive a clustered space without clusters")

    from metric.operators import _ensure_exact_metric_work_allowed

    _ensure_exact_metric_work_allowed(
        "clustered_space(...)",
        cluster_count,
        runtime=runtime,
        max_memory_bytes=max_memory_bytes,
        max_distance_evaluations=max_distance_evaluations,
        max_dense_records=max_dense_records,
        dense=True,
        dense_cell_bytes=_CLUSTERED_SPACE_DENSE_CELL_BYTES,
    )

    assignments = _materialize_clustered_sequence(
        "assignments",
        getattr(clustering, "assignments", ()),
        record_count,
        "record_count",
    )
    if len(assignments) != record_count:
        raise ValueError("clustering assignments do not match record count")

    unassigned_label = getattr(clustering, "unassigned_label", -1)
    source_ids = getattr(space, "ids", None)
    if source_ids is None:
        source_ids = tuple(range(record_count))
    source_id_positions = _source_id_position_index(space, source_ids)
    source_records = [[] for _ in range(cluster_count)]

    for index, label in enumerate(assignments):
        if label == unassigned_label:
            continue
        label = _coerce_count(label, "cluster assignment")
        if label >= cluster_count:
            raise ValueError("clustering assignment references an unknown cluster")
        source_records[label].append(source_ids[index])

    medoids = _materialize_clustered_sequence(
        "medoids",
        getattr(clustering, "medoids", ()),
        cluster_count,
        "cluster_count",
    )
    representative_positions = []
    representative_records = []
    records = []

    for label, members in enumerate(source_records):
        if not members:
            raise ValueError("cannot derive a clustered space with empty clusters")
        if label < len(medoids):
            medoid = _coerce_count(medoids[label], "cluster medoid")
            try:
                representative_position = _position_for_source_id(
                    source_id_positions,
                    source_ids,
                    medoid,
                )
            except ValueError:
                representative_position = medoid
        else:
            representative_position = _position_for_source_id(
                source_id_positions,
                source_ids,
                members[0],
            )
        if representative_position >= record_count:
            raise ValueError("clustering medoid references an unknown source record")
        representative_positions.append(representative_position)
        representative = source_ids[representative_position]
        representative_records.append(representative)
        records.append(
            ClusterRecord(label=label, representative=representative, members=tuple(members))
        )

    distances = tuple(
        tuple(space.distance(lhs, rhs) for rhs in representative_positions)
        for lhs in representative_positions
    )

    return ClusteredSpaceDerivation(
        records=tuple(records),
        distances=distances,
        source_records=tuple(tuple(members) for members in source_records),
        representative_records=tuple(representative_records),
        source_record_count=record_count,
        strategy=getattr(clustering, "algorithm", "clustering"),
        representation=getattr(clustering, "representation", "metric_space"),
    )


def _coerce_count(value, name):
    try:
        value = operator.index(value)
    except TypeError:
        raise TypeError(f"{name} must be an integer") from None
    if value < 0:
        raise ValueError(f"{name} must be non-negative")
    return value


def _materialize_clustered_sequence(name, values, max_count, max_count_name):
    try:
        known_count = len(values)
    except TypeError:
        materialized = []
        for value in values:
            if len(materialized) >= max_count:
                raise MetricComputationError(
                    f"clustered_space(...) refused {name} materialization: "
                    f"observed_{name}>{max_count}, {max_count_name}={max_count}. "
                    "Suggested fallback: pass a bounded clustering result sequence."
                )
            materialized.append(value)
        return tuple(materialized)

    if known_count > max_count:
        raise MetricComputationError(
            f"clustered_space(...) refused {name} materialization: "
            f"{name}={known_count}, {max_count_name}={max_count}. "
            "Suggested fallback: pass a bounded clustering result sequence."
        )
    return tuple(values)


def _coerce_manifest(value, name):
    if isinstance(value, bytes):
        value = value.decode("utf-8")
    if isinstance(value, str):
        try:
            value = json.loads(value)
        except json.JSONDecodeError as exc:
            raise MetricInputError(f"{name} must be valid JSON") from exc
    if not isinstance(value, MappingABC):
        raise MetricInputError(f"{name} must be a mapping or JSON object")
    return copy.deepcopy(dict(value))


def _validate_native_mapping_manifest(manifest):
    artifact_format = manifest.get("format")
    if artifact_format not in _NATIVE_MAPPING_ARTIFACT_FORMATS:
        raise MetricInputError(
            "unsupported native mapping artifact format; expected one of "
            f"{sorted(_NATIVE_MAPPING_ARTIFACT_FORMATS)!r}"
        )
    if manifest.get("format_version") != 1:
        raise MetricInputError("unsupported native mapping artifact version")
    if manifest.get("backend") != "native_dnn":
        raise MetricInputError("unsupported native mapping artifact backend")

    mapping = manifest.get("mapping")
    if not isinstance(mapping, MappingABC):
        raise MetricInputError("native mapping artifact must include mapping provenance")
    if not mapping.get("name") or not mapping.get("strategy"):
        raise MetricInputError("native mapping artifact mapping provenance is incomplete")

    if artifact_format == "metric.parametric_diffusion_coordinate_artifact":
        if mapping.get("name") != "parametric_diffusion_coordinates" or mapping.get("strategy") != "native_metric_diffusion_coordinate_solver":
            raise MetricInputError("unexpected parametric diffusion coordinate provenance")

    source = manifest.get("source")
    if not isinstance(source, MappingABC):
        raise MetricInputError("native mapping artifact must include source metadata")
    _coerce_count(source.get("record_count"), "source.record_count")


def _nested(mapping, path, default=None):
    value = mapping
    for key in path:
        if not isinstance(value, MappingABC) or key not in value:
            return default
        value = value[key]
    return value


def _source_id_position_index(space, source_ids):
    position_index = getattr(space, "_id_to_index", None)
    if position_index is not None:
        return position_index
    try:
        return {record_id: index for index, record_id in enumerate(source_ids)}
    except TypeError:
        return None


def _position_for_source_id(position_index, source_ids, source_id):
    if position_index is not None:
        try:
            return position_index[source_id]
        except (KeyError, TypeError):
            raise ValueError("cluster source record is not present in the source space") from None
    try:
        return source_ids.index(source_id)
    except (AttributeError, ValueError):
        raise ValueError("cluster source record is not present in the source space") from None


__all__ = [
    "STABILITY",
    "ClusterRecord",
    "ClusteredSpaceMapping",
    "ClusteredSpaceDerivation",
    "NativeMappingArtifact",
    "ParametricDiffusionCoordinateArtifact",
    "clustered_space",
    "derive_parametric_diffusion_coordinates",
    "derive_from",
    "load_native_mapping_artifact",
    "make_clustered_space_mapping",
    "transform",
]
