"""Finite metric-space helpers for the revived Python API."""

import operator


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

    def __init__(self, records, metric):
        self.records = list(records)
        self.metric = metric
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

    def distance(self, lhs_index, rhs_index):
        return self._distances[lhs_index][rhs_index]

    def pairwise_distances(self):
        return [row[:] for row in self._distances]

    def to_matrix(self):
        return FiniteMetricSpace(self.records, self.metric)

    def to_tree(self):
        from metric.representations import TreeIndex

        return TreeIndex(self)

    def to_graph(self, count):
        from metric.representations import GraphIndex

        return GraphIndex(self, count)

    def knn(self, query, k=1):
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

    def neighbors(self, query, k=None, count=None, radius=None):
        if radius is not None:
            if k is not None or count is not None:
                raise ValueError("radius cannot be combined with k or count")
            return self.rnn(query, radius)

        if k is not None and count is not None:
            raise ValueError("use either k or count, not both")

        return self.knn(query, 1 if k is None and count is None else count if count is not None else k)

    def nearest(self, query):
        return self.nn(query)

    def within_radius(self, query, radius):
        return self.rnn(query, radius)

    def groups(self, strategy):
        from metric.operators import find_groups

        return find_groups(self.records, self.metric, strategy)

    def outliers(self, strategy):
        from metric.operators import find_outliers

        return find_outliers(self.records, self.metric, strategy)

    def denoise(self, strategy):
        from metric.operators import denoise_space

        return denoise_space(self.records, self.metric, strategy)

    def representatives(self, k, strategy=None):
        from metric.operators import find_representatives

        return find_representatives(self.records, self.metric, k, strategy=strategy)

    def reduce(self, count=None, strategy=None):
        from metric.operators import reduce_space

        return reduce_space(self.records, self.metric, count, strategy=strategy)

    def compress(self, count=None, strategy=None):
        from metric.operators import compress_space

        return compress_space(self.records, self.metric, count, strategy=strategy)

    def map(self, transform, metric=None):
        from metric.operators import map_space

        return map_space(self.records, transform, self.metric if metric is None else metric)

    def embed(self, dimensions=2, strategy=None):
        from metric.operators import embed_space

        return embed_space(self.records, self.metric, dimensions=dimensions, strategy=strategy)

    def describe(self):
        from metric.operators import describe_structure

        return describe_structure(self.records, self.metric)

    def describe_structure(self):
        return self.describe()

    def compare(self, other, strategy=None):
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

    def correlate(self, other, strategy=None):
        return self.compare(other, strategy=strategy)


MatrixSpace = FiniteMetricSpace

__all__ = ["FiniteMetricSpace", "MatrixSpace", "Space"]
