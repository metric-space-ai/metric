"""Finite metric-space helpers for the revived Python API."""


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

    def knn(self, query, k=1):
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

    The revived Python engine facade starts with neighbor access because it is
    backed by the current core representation. Higher-level intents such as
    embedding and mapping are added only after they have stable strategy and
    result contracts.
    """

    def neighbors(self, query, k=1):
        return self.knn(query, k)

    def nearest(self, query):
        return self.nn(query)

    def within_radius(self, query, radius):
        return self.rnn(query, radius)

    def groups(self, strategy):
        from metric.operators import find_groups

        return find_groups(self.records, self.metric, strategy)

    def representatives(self, k, strategy=None):
        from metric.operators import find_representatives

        return find_representatives(self.records, self.metric, k, strategy=strategy)

    def describe(self):
        from metric.operators import describe_structure

        return describe_structure(self.records, self.metric)

    def describe_structure(self):
        return self.describe()


MatrixSpace = FiniteMetricSpace

__all__ = ["FiniteMetricSpace", "MatrixSpace", "Space"]
