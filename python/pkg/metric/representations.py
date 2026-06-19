"""Representation facade for the revived Python engine API."""

from .spaces import FiniteMetricSpace, MatrixSpace, Space


def matrix(space):
    """Return an explicit matrix-backed view of a Space-like object."""
    return space.to_matrix()


def matrix_space(records, metric):
    """Construct an explicit finite matrix-space representation."""
    return MatrixSpace(records, metric)


__all__ = ["FiniteMetricSpace", "MatrixSpace", "Space", "matrix", "matrix_space"]
