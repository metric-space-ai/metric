"""Finite metric-space helpers for the revived Python API."""

from collections.abc import Mapping
import math
import operator

from metric.exceptions import MissingMetricError, StaleRepresentationError
from metric.runtime import require_exact_runtime


def _coerce_non_negative_integer(value, name):
    try:
        value = operator.index(value)
    except TypeError:
        raise TypeError(f"{name} must be an integer") from None
    if value < 0:
        raise ValueError(f"{name} must be non-negative")
    return value


class FiniteMetricSpace:
    """Explicit finite metric space backed by a pairwise distance matrix."""

    def __init__(
        self,
        records,
        metric=None,
        source_space=None,
        representation="metric_space",
        ids=None,
        name=None,
        metadata=None,
    ):
        if metric is None:
            raise MissingMetricError(
                "Space requires an explicit metric for arbitrary records. "
                "Use Space(records, metric=Edit()) or another callable metric."
            )
        self.records = list(records)
        self.ids = list(range(len(self.records))) if ids is None else list(ids)
        if len(self.ids) != len(self.records):
            raise ValueError("ids must have the same length as records")
        if len(set(self.ids)) != len(self.ids):
            raise ValueError("ids must be unique")
        self._id_to_index = {record_id: index for index, record_id in enumerate(self.ids)}
        self.metric = metric
        self.name = name
        self.metadata = {} if metadata is None else dict(metadata)
        self.source_space = source_space
        self.source_version = source_space.version() if source_space is not None else None
        self.representation = representation
        self._version = 0
        self._distances = [
            [metric(lhs, rhs) for rhs in self.records]
            for lhs in self.records
        ]

    def __len__(self):
        return len(self.records)

    def __getitem__(self, index):
        return self.records[index]

    def __call__(self, lhs_index, rhs_index):
        return self.distance(lhs_index, rhs_index)

    @classmethod
    def from_dataframe(cls, dataframe, metric=None, id_column=None, **kwargs):
        if metric is None:
            raise MissingMetricError(
                "Space.from_dataframe requires an explicit metric. "
                "Use Space.from_dataframe(df, metric=callable)."
            )
        if id_column is not None and "ids" in kwargs:
            raise ValueError("use either id_column or ids, not both")
        if not hasattr(dataframe, "to_dict"):
            raise TypeError("dataframe must provide to_dict('records')")

        try:
            rows = dataframe.to_dict("records")
        except TypeError:
            rows = dataframe.to_dict(orient="records")

        ids = None
        records = list(rows)
        if id_column is not None:
            ids = []
            feature_records = []
            for row in records:
                if not isinstance(row, Mapping):
                    raise TypeError("dataframe records must be mappings when id_column is used")
                if id_column not in row:
                    raise KeyError(f"id_column {id_column!r} is not present in every row")
                ids.append(row[id_column])
                feature_records.append({
                    column: value
                    for column, value in row.items()
                    if column != id_column
                })
            records = feature_records

        return cls(records, metric=metric, ids=ids, **kwargs)

    def version(self):
        return self._version

    def touch(self):
        self._version += 1
        return self._version

    def is_stale(self):
        return self.source_space is not None and self.source_space.version() != self.source_version

    def ensure_fresh(self):
        if self.is_stale():
            raise StaleRepresentationError(
                f"{self.representation} representation is stale: source version {self.source_space.version()} "
                f"does not match representation version {self.source_version}. "
                "Rebuild the representation from the source space."
            )
        return self

    def distance(self, lhs_index, rhs_index):
        self.ensure_fresh()
        return self._distances[lhs_index][rhs_index]

    def record(self, record_id):
        return self.records[self._id_to_index[record_id]]

    def pairwise_distances(self):
        self.ensure_fresh()
        return [row[:] for row in self._distances]

    def pairwise(self, ids=None):
        self.ensure_fresh()
        if ids is None:
            return self.pairwise_distances()

        positions = [self._id_to_index[record_id] for record_id in ids]
        return [
            [self._distances[lhs][rhs] for rhs in positions]
            for lhs in positions
        ]

    def to_matrix(self):
        return FiniteMetricSpace(
            self.records,
            self.metric,
            source_space=self,
            representation="matrix",
            ids=self.ids,
            name=self.name,
            metadata=self.metadata,
        )

    def to_tree(self):
        from metric.representations import TreeIndex

        return TreeIndex(self)

    def to_graph(self, count):
        from metric.representations import GraphIndex

        return GraphIndex(self, count)

    def knn(self, query, k=1):
        self.ensure_fresh()
        k = _coerce_non_negative_integer(k, "k")
        distances = [
            (index, self.metric(record, query))
            for index, record in enumerate(self.records)
        ]
        return sorted(distances, key=lambda item: item[1])[:k]

    def nn(self, query):
        return self.knn(query, 1)[0]

    def rnn(self, query, radius):
        return [
            (index, distance)
            for index, distance in self.knn(query, len(self.records))
            if distance <= radius
        ]


class Space(FiniteMetricSpace):
    """Intent-first facade for a finite metric space.

    The revived Python engine facade starts from operations backed by stable
    strategy and result contracts. Learned embedding paths stay out of the core
    facade until they have deterministic coverage.
    """

    def _neighbor_count(self, k=None, count=None):
        if k is not None and count is not None:
            raise ValueError("use either k or count, not both")
        value = 1 if k is None and count is None else count if count is not None else k
        return _coerce_non_negative_integer(value, "count")

    def _neighbor_row(self, source_index, count=None, radius=None, include_self=False):
        self.ensure_fresh()
        neighbors = []
        for candidate_index in range(len(self.records)):
            if not include_self and candidate_index == source_index:
                continue
            distance = self.distance(candidate_index, source_index)
            if radius is None or distance <= radius:
                neighbors.append((candidate_index, distance))
        neighbors.sort(key=lambda item: item[1])
        if count is not None:
            neighbors = neighbors[:count]
        return neighbors

    def neighbors(
        self,
        query=None,
        k=None,
        count=None,
        radius=None,
        *,
        include_self=False,
        strategy=None,
        representation=None,
        runtime=None,
    ):
        require_exact_runtime(runtime)
        if strategy is not None:
            raise ValueError("strategy overrides are not promoted for Python neighbors yet")
        if query is not None and representation is not None:
            return representation.neighbors(query, k=k, count=count, radius=radius)
        if representation is not None:
            representation.ensure_fresh()

        has_explicit_count = k is not None or count is not None
        neighbor_count = self._neighbor_count(k, count) if has_explicit_count or radius is None else None
        if query is None:
            return [
                self._neighbor_row(source_index, neighbor_count, radius=radius, include_self=include_self)
                for source_index in range(len(self.records))
            ]

        if radius is not None:
            neighbors = self.rnn(query, radius)
            return neighbors if neighbor_count is None else neighbors[:neighbor_count]
        return self.knn(query, neighbor_count)

    def nearest(self, query):
        return self.nn(query)

    def within_radius(self, query, radius):
        return self.rnn(query, radius)

    def groups(self, strategy=None, *, count="auto", radius=None, min_size=1, representation=None, runtime=None):
        require_exact_runtime(runtime)
        if representation is not None:
            representation.ensure_fresh()
        if strategy is not None and (count != "auto" or radius is not None):
            raise ValueError("use either strategy or count/radius, not both")

        from metric.operators import find_groups
        from metric.strategies import DBSCAN, KMedoids

        if strategy is None:
            if radius is not None:
                strategy = DBSCAN(radius=radius, min_points=min_size)
            else:
                if count == "auto":
                    count = 1 if len(self.records) <= 2 else 2
                strategy = KMedoids(groups=count)

        return find_groups(self.records, self.metric, strategy)

    def _nearest_other_distance(self, source_index):
        candidates = [
            self.distance(source_index, candidate_index)
            for candidate_index in range(len(self.records))
            if candidate_index != source_index
        ]
        return min(candidates) if candidates else 0

    def _outlier_count(self, count=None, fraction=0.05):
        if count is not None:
            return _coerce_non_negative_integer(count, "count")
        if fraction < 0 or fraction > 1:
            raise ValueError("fraction must be between 0 and 1")
        if not self.records or fraction == 0:
            return 0
        return max(1, math.ceil(len(self.records) * fraction))

    def outliers(
        self,
        strategy=None,
        *,
        count=None,
        fraction=0.05,
        threshold=None,
        representation=None,
        runtime=None,
    ):
        require_exact_runtime(runtime)
        if representation is not None:
            representation.ensure_fresh()
        from metric.operators import find_outliers

        if strategy is not None:
            if count is not None or threshold is not None or fraction != 0.05:
                raise ValueError("use either strategy or count/fraction/threshold, not both")
            return find_outliers(self.records, self.metric, strategy)

        from metric.operators import Outlier, OutlierResult

        self.ensure_fresh()
        scored = [
            Outlier(record_id=record_index, score=self._nearest_other_distance(record_index))
            for record_index in range(len(self.records))
        ]
        if threshold is not None:
            scored = [outlier for outlier in scored if outlier.score >= threshold]
        scored.sort(key=lambda outlier: outlier.record_id)
        scored.sort(key=lambda outlier: outlier.score, reverse=True)
        if threshold is None:
            scored = scored[:self._outlier_count(count=count, fraction=fraction)]

        return OutlierResult(
            outliers=tuple(scored),
            record_count=len(self.records),
            cluster_count=0,
            noise_count=len(scored),
            exact=True,
            operator_name="find_outliers",
            strategy="nearest_neighbor_distance",
            representation="metric_space",
        )

    def denoise(
        self,
        strategy=None,
        *,
        count=None,
        fraction=0.05,
        threshold=None,
        strength="auto",
        representation=None,
        runtime=None,
    ):
        require_exact_runtime(runtime)
        if representation is not None:
            representation.ensure_fresh()
        from metric.operators import denoise_space

        if strategy is not None:
            if count is not None or threshold is not None or fraction != 0.05 or strength != "auto":
                raise ValueError("use either strategy or count/fraction/threshold/strength, not both")
            return denoise_space(self.records, self.metric, strategy)

        if strength != "auto":
            if count is not None or threshold is not None or fraction != 0.05:
                raise ValueError("strength cannot be combined with count, fraction, or threshold")
            try:
                fraction = float(strength)
            except (TypeError, ValueError):
                raise ValueError("strength must be 'auto' or a fraction between 0 and 1") from None

        from metric.operators import MappingResult

        outlier_ids = {
            outlier.record_id
            for outlier in self.outliers(count=count, fraction=fraction, threshold=threshold).outliers
        }
        kept_record_ids = tuple(
            record_index
            for record_index in range(len(self.records))
            if record_index not in outlier_ids
        )
        denoised_records = [self.records[index] for index in kept_record_ids]

        return MappingResult(
            space=Space(denoised_records, self.metric),
            source_record_ids=kept_record_ids,
            source_record_count=len(self.records),
            target_record_count=len(denoised_records),
            exact=True,
            operator_name="denoise",
            mapping="nearest_neighbor_outlier_filter",
            strategy="nearest_neighbor_distance",
            representation="metric_space",
            inverse_supported=False,
        )

    def representatives(self, k, strategy=None, *, runtime=None):
        require_exact_runtime(runtime)
        from metric.operators import find_representatives

        return find_representatives(self.records, self.metric, k, strategy=strategy)

    def reduce(self, count=None, strategy=None, *, runtime=None):
        require_exact_runtime(runtime)
        from metric.operators import reduce_space

        return reduce_space(self.records, self.metric, count, strategy=strategy)

    def compress(self, count=None, strategy=None, *, runtime=None):
        require_exact_runtime(runtime)
        from metric.operators import compress_space

        return compress_space(self.records, self.metric, count, strategy=strategy)

    def map(self, transform, metric=None, *, runtime=None):
        require_exact_runtime(runtime)
        from metric.operators import map_space

        return map_space(self.records, transform, self.metric if metric is None else metric)

    def embed(self, dimensions=2, strategy=None, *, runtime=None):
        require_exact_runtime(runtime)
        from metric.operators import embed_space

        return embed_space(self.records, self.metric, dimensions=dimensions, strategy=strategy)

    def describe(self, *, runtime=None):
        require_exact_runtime(runtime)
        from metric.operators import describe_structure

        return describe_structure(self.records, self.metric)

    def describe_structure(self, *, runtime=None):
        return self.describe(runtime=runtime)

    def compare(self, other, strategy=None, *, runtime=None):
        require_exact_runtime(runtime)
        from metric.operators import compare_spaces

        return compare_spaces(
            self.records,
            self.metric,
            other.records,
            other.metric,
            strategy,
            left_representation="metric_space",
            right_representation="metric_space",
        )

    def correlate(self, other, strategy=None, *, runtime=None):
        return self.compare(other, strategy=strategy, runtime=runtime)


MatrixSpace = FiniteMetricSpace

__all__ = ["FiniteMetricSpace", "MatrixSpace", "Space"]
