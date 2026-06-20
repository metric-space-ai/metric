"""Mapping namespace for the revived Python API.

Mapping algorithms are currently a beta/compatibility surface. This module
provides a stable import location without forcing the core wheel to import the
broader legacy compiled mapping bindings.
"""

from dataclasses import dataclass
import operator


STABILITY = "beta"


@dataclass(frozen=True)
class ClusterRecord:
    """One derived clustered-space record with source-record lineage."""

    label: int
    representative: object
    members: tuple
    noise: bool = False


@dataclass(frozen=True)
class ClusteredSpaceModel:
    """Fitted model that derives a cluster-level metric space."""

    records: tuple
    distances: tuple
    source_records: tuple
    representative_records: tuple
    source_record_count: int
    strategy: str
    representation: str

    def transform(self, space=None):
        if space is not None and len(space) != self.source_record_count:
            raise ValueError("source space size does not match fitted clustered-space mapping")

        distances = self.distances

        def cluster_distance(lhs, rhs):
            return distances[lhs.label][rhs.label]

        from metric.operators import MappingResult
        from metric.spaces import Space

        return MappingResult(
            space=Space(list(self.records), metric=cluster_distance),
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
        )

    def inverse_supported(self):
        return False


@dataclass(frozen=True)
class ClusteredSpaceMapping:
    """Mapping adapter from a ClusteringResult into a clustered Space."""

    clustering: object

    def fit(self, space):
        return _build_clustered_space_model(space, self.clustering)


def _load_legacy_module():
    try:
        from metric import mapping
    except (ImportError, ModuleNotFoundError):
        return None
    return mapping


def available():
    """Return public mapping names available in the installed wheel."""
    module = _load_legacy_module()
    if module is None:
        return tuple()
    return tuple(sorted(name for name in dir(module) if not name.startswith("_")))


def legacy_module():
    """Return the legacy mapping module when the installed wheel provides it."""
    module = _load_legacy_module()
    if module is None:
        raise ImportError("metric.mapping is not available in this wheel")
    return module


def fit(mapping, space):
    """Fit a promoted mapping adapter to a finite metric space."""
    return mapping.fit(space)


def transform(model, space=None):
    """Transform a finite metric space through a fitted mapping model."""
    return model.transform(space)


def make_clustered_space_mapping(clustering):
    """Create a mapping adapter from an engine-style clustering result."""
    return ClusteredSpaceMapping(clustering)


def clustered_space(space, clustering):
    """Derive a cluster-level Space from a source Space and clustering result."""
    return make_clustered_space_mapping(clustering).fit(space).transform(space)


def _build_clustered_space_model(space, clustering):
    record_count = _coerce_count(getattr(clustering, "record_count", None), "record_count")
    if record_count != len(space):
        raise ValueError("clustering record count does not match source space")

    assignments = tuple(getattr(clustering, "assignments", ()))
    if len(assignments) != record_count:
        raise ValueError("clustering assignments do not match record count")

    cluster_count = _coerce_count(getattr(clustering, "cluster_count", None), "cluster_count")
    if cluster_count == 0:
        raise ValueError("cannot derive a clustered space without clusters")

    noise_label = getattr(clustering, "noise_label", -1)
    source_ids = tuple(getattr(space, "ids", tuple(range(record_count))))
    source_records = [[] for _ in range(cluster_count)]

    for index, label in enumerate(assignments):
        if label == noise_label:
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
        representative_position = (
            _coerce_count(medoids[label], "cluster medoid")
            if label < len(medoids)
            else _position_for_source_id(space, members[0])
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

    return ClusteredSpaceModel(
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


def _position_for_source_id(space, source_id):
    try:
        return space.ids.index(source_id)
    except ValueError:
        raise ValueError("cluster source record is not present in the source space") from None


__all__ = [
    "STABILITY",
    "ClusterRecord",
    "ClusteredSpaceMapping",
    "ClusteredSpaceModel",
    "available",
    "clustered_space",
    "fit",
    "legacy_module",
    "make_clustered_space_mapping",
    "transform",
]
