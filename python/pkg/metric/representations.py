"""Representation facade for the revived Python engine API.

Representations are explicit views over a finite metric space. The matrix view
and the lifecycle/staleness bookkeeping are pure adapters. Indexed neighbor
search (tree) and graph construction are METRIC algorithms; they must run in
native C++, so those operations raise :class:`StrategyUnavailableError` until a
native binding is exposed.
"""

import operator

from metric.exceptions import StaleRepresentationError, StrategyUnavailableError
from .spaces import FiniteMetricSpace, MatrixSpace, Space


def _require_native_binding(operation, kind):
    raise StrategyUnavailableError(
        f"{operation} requires native C++ binding support. The METRIC Python package is an "
        f"adapter-only surface: {kind} must run in native C++ and be reached through a binding, "
        f"and is not implemented in Python. No native binding for this operation is exposed in "
        f"metric._impl yet."
    )


def _coerce_count(value, name):
    try:
        value = operator.index(value)
    except TypeError:
        raise TypeError(f"{name} must be an integer") from None
    if value < 0:
        raise ValueError(f"{name} must be non-negative")
    return value


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
        """Indexed k-nearest-neighbor search (native-only)."""
        _require_native_binding("TreeIndex.knn(...)", "indexed neighbor search")

    def neighbors(self, query, k=None, count=None, radius=None):
        """Indexed neighbor search (native-only)."""
        _require_native_binding("TreeIndex.neighbors(...)", "indexed neighbor search")

    def rnn(self, query, radius):
        """Indexed radius search (native-only)."""
        _require_native_binding("TreeIndex.rnn(...)", "indexed radius search")


class GraphIndex:
    """Exact kNN graph representation over a Space-like source (native-only).

    Constructing an exact kNN graph is a METRIC graph-construction algorithm, so
    the Python facade cannot build it; it raises until a native binding exists.
    """

    exact = True
    representation = "exact_knn_graph"

    def __init__(self, space, count):
        self.source_space = space
        self.source_version = _source_version(space)
        self.records = list(space.records)
        self.metric = space.metric
        self.count = _coerce_count(count, "count")
        _require_native_binding("Space.to_graph(...) / GraphIndex(...)", "exact kNN graph construction")


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
