"""Strategy objects for the revived Python engine facade."""

from dataclasses import dataclass


@dataclass(frozen=True)
class ClassicMDS:
    """Classical MDS embedding strategy over an exact distance matrix."""

    dimensions: int = 2


@dataclass(frozen=True)
class FarthestFirst:
    """Deterministic farthest-first representative-selection strategy."""

    seed_index: int = 0


@dataclass(frozen=True)
class KMedoids:
    """Deterministic k-medoids grouping strategy."""

    groups: int
    max_iterations: int = 100


@dataclass(frozen=True)
class DBSCAN:
    """Deterministic DBSCAN grouping strategy over a finite metric space."""

    radius: float
    min_points: int


@dataclass(frozen=True)
class DistanceProfileCorrelation:
    """Compare two spaces by Pearson correlation of pairwise distance profiles."""

    method: str = "pearson"


__all__ = ["ClassicMDS", "DBSCAN", "DistanceProfileCorrelation", "FarthestFirst", "KMedoids"]
