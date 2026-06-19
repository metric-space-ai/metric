"""Small metric-space operators covered by the core Python smoke path."""

from metric.spaces import FiniteMetricSpace


def pairwise_distance_matrix(records, metric):
    return FiniteMetricSpace(records, metric).pairwise_distances()


def nearest_neighbors(records, metric, query, k=1):
    return FiniteMetricSpace(records, metric).knn(query, k)


def range_neighbors(records, metric, query, radius):
    return FiniteMetricSpace(records, metric).rnn(query, radius)


__all__ = ["pairwise_distance_matrix", "nearest_neighbors", "range_neighbors"]
