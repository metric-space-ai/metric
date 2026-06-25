"""Mapping namespace for finite-space derived-coordinate and quotient surfaces.

Mapping surfaces are currently beta extensions. This module provides a stable
import location without making them part of the core finite-space API.
"""

from collections.abc import Mapping as MappingABC
from dataclasses import dataclass
import copy
import json
import operator

from metric.exceptions import MetricInputError, StrategyUnavailableError


STABILITY = "beta"
_NATIVE_MAPPING_ARTIFACT_FORMATS = frozenset({
    "metric.parametric_diffusion_coordinate_artifact",
})


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

    def derive_from(self, space):
        return _build_clustered_space_derivation(space, self.clustering)


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

    def transform(self, records):
        return self._artifact.transform(records)

    def inverse_transform(self, latent_records):
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


def derive_from(mapping, space):
    """Derive a mapping artifact from a finite metric space."""
    return mapping.derive_from(space)


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
):
    """Derive a native C++ parametric diffusion coordinate artifact.

    This is a binding adapter. It marshals Python list/NumPy row data to the
    native C++ implementation and does not compute diffusion geometry or
    coordinate calibration in Python.

    The native path is deterministic by construction: a given input always
    yields the same coordinates.
    """

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

    try:
        artifact = derive_vectors(
            records,
            dimensions=dimensions,
            calibration_steps=calibration_steps,
            step_size=step_size,
            diffusion_steps=diffusion_steps,
            kernel_scale=kernel_scale,
            reconstruction_weight=reconstruction_weight,
            geometry_weight=geometry_weight,
            seed=seed,
            distance_provider=distance_provider,
            affinity_kernel=affinity_kernel,
            diffusion_operator=diffusion_operator,
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


def make_clustered_space_mapping(clustering):
    """Create a mapping adapter from an engine-style clustering result."""
    return ClusteredSpaceMapping(clustering)


def clustered_space(space, clustering):
    """Derive a cluster-level Space from a source Space and clustering result."""
    return make_clustered_space_mapping(clustering).derive_from(space).transform(space)


def _build_clustered_space_derivation(space, clustering):
    record_count = _coerce_count(getattr(clustering, "record_count", None), "record_count")
    if record_count != len(space):
        raise ValueError("clustering record count does not match source space")

    assignments = tuple(getattr(clustering, "assignments", ()))
    if len(assignments) != record_count:
        raise ValueError("clustering assignments do not match record count")

    cluster_count = _coerce_count(getattr(clustering, "cluster_count", None), "cluster_count")
    if cluster_count == 0:
        raise ValueError("cannot derive a clustered space without clusters")

    unassigned_label = getattr(clustering, "unassigned_label", -1)
    source_ids = tuple(getattr(space, "ids", tuple(range(record_count))))
    source_records = [[] for _ in range(cluster_count)]

    for index, label in enumerate(assignments):
        if label == unassigned_label:
            continue
        label = _coerce_count(label, "cluster assignment")
        if label >= cluster_count:
            raise ValueError("clustering assignment references an unknown cluster")
        source_records[label].append(source_ids[index])

    medoids = tuple(getattr(clustering, "medoids", ()))
    representative_positions = []
    representative_records = []
    records = []

    for label, members in enumerate(source_records):
        if not members:
            raise ValueError("cannot derive a clustered space with empty clusters")
        if label < len(medoids):
            medoid = _coerce_count(medoids[label], "cluster medoid")
            try:
                representative_position = _position_for_source_id(space, medoid)
            except ValueError:
                representative_position = medoid
        else:
            representative_position = _position_for_source_id(space, members[0])
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


def _position_for_source_id(space, source_id):
    try:
        return space.ids.index(source_id)
    except ValueError:
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
