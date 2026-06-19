"""Representation facade for the revived Python engine API."""

import operator

from metric.exceptions import StaleRepresentationError
from .spaces import FiniteMetricSpace, MatrixSpace, Space


def _coerce_count(value, name):
    try:
        value = operator.index(value)
    except TypeError:
        raise TypeError(f"{name} must be an integer") from None
    if value < 0:
        raise ValueError(f"{name} must be non-negative")
    return value


def _resolve_neighbor_count(k=None, count=None):
    if k is not None and count is not None:
        raise ValueError("use either k or count, not both")
    return _coerce_count(1 if k is None and count is None else count if count is not None else k, "count")


def _source_version(space):
    return space.version() if hasattr(space, "version") else None


def _raise_if_stale(representation):
    if representation.is_stale():
        raise StaleRepresentationError(
            f"{representation.representation} representation is stale: "
            f"source version {representation.source_space.version()} does not match "
            f"representation version {representation.source_version}. "
            "Rebuild the representation from the source space."
        )


class TreeIndex:
    """Exact tree-style neighbor index over a Space-like source."""

    exact = True
    representation = "exact_tree_index"
    strategy = "exact_scan"

    def __init__(self, space):
        self.source_space = space
        self.source_version = _source_version(space)
        self.records = list(space.records)
        self.metric = space.metric
        self.record_count = len(self.records)

    def __len__(self):
        return self.record_count

    def __getitem__(self, index):
        return self.records[index]

    def is_stale(self):
        return self.source_version is not None and self.source_space.version() != self.source_version

    def ensure_fresh(self):
        _raise_if_stale(self)
        return self

    def distance(self, lhs_index, rhs_index):
        self.ensure_fresh()
        return self.source_space.distance(lhs_index, rhs_index)

    def knn(self, query, k=None, count=None):
        self.ensure_fresh()
        return self.source_space.knn(query, _resolve_neighbor_count(k, count))

    def neighbors(self, query, k=None, count=None, radius=None):
        if radius is not None:
            if k is not None or count is not None:
                raise ValueError("radius cannot be combined with k or count")
            return self.rnn(query, radius)
        return self.knn(query, k=k, count=count)

    def rnn(self, query, radius):
        self.ensure_fresh()
        return self.source_space.rnn(query, radius)


class GraphIndex:
    """Exact kNN graph representation over a Space-like source."""

    exact = True
    representation = "exact_knn_graph"

    def __init__(self, space, count):
        from metric.operators import exact_knn_graph

        self.source_space = space
        self.source_version = _source_version(space)
        self.records = list(space.records)
        self.metric = space.metric
        self.count = _coerce_count(count, "count")
        self.graph = exact_knn_graph(self.records, self.metric, self.count)
        self.edges = self.graph.edges
        self.metadata = self.graph.metadata
        self.strategy = self.metadata.strategy
        self.record_count = self.metadata.record_count
        self.edge_count = self.metadata.edge_count

    def __len__(self):
        return self.record_count

    def __getitem__(self, index):
        return self.records[index]

    def is_stale(self):
        return self.source_version is not None and self.source_space.version() != self.source_version

    def ensure_fresh(self):
        _raise_if_stale(self)
        return self

    def distance(self, lhs_index, rhs_index):
        self.ensure_fresh()
        return self.source_space.distance(lhs_index, rhs_index)

    def neighbors(self, source_index):
        self.ensure_fresh()
        source_index = _coerce_count(source_index, "source_index")
        if source_index >= self.record_count:
            raise IndexError("source_index is outside the graph")
        return tuple(
            (target_index, distance)
            for edge_source, target_index, distance in self.edges
            if edge_source == source_index
        )


def matrix(space):
    """Return an explicit matrix-backed view of a Space-like object."""
    return space.to_matrix()


def tree(space):
    """Return an exact tree-style index over a Space-like object."""
    return space.to_tree()


def graph(space, count):
    """Return an exact kNN graph index over a Space-like object."""
    return space.to_graph(count=count)


def matrix_space(records, metric):
    """Construct an explicit finite matrix-space representation."""
    return MatrixSpace(records, metric)


__all__ = [
    "FiniteMetricSpace",
    "GraphIndex",
    "MatrixSpace",
    "Space",
    "TreeIndex",
    "graph",
    "matrix",
    "matrix_space",
    "tree",
]
