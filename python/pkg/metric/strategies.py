"""Strategy objects for the revived Python engine facade."""

from dataclasses import dataclass


@dataclass(frozen=True)
class ClassicMDS:
    """Classical MDS embedding strategy over an exact distance matrix."""

    dimensions: int = 2


@dataclass(frozen=True)
class MDS(ClassicMDS):
    """Preferred public name for the promoted classical MDS strategy."""


@dataclass(frozen=True)
class DiffusionEmbedding:
    """Roadmap PHATE-style diffusion embedding strategy.

    The strategy object is importable so examples can use stable engine
    vocabulary, but execution is promoted only after deterministic fixtures and
    CI-backed result contracts exist.
    """

    dimensions: int = 2
    knn: object = None
    diffusion_time: object = "auto"


@dataclass(frozen=True)
class PCFA:
    """Roadmap PCFA reduction or mapping strategy."""

    dimensions: int = 2


@dataclass(frozen=True)
class SOM:
    """Roadmap SOM/KOC reduction or mapping strategy."""

    grid: tuple = (6, 6)


@dataclass(frozen=True)
class PhateAE:
    """Roadmap PHATE-AE-style learnable mapping strategy."""

    dimensions: int = 2
    epochs: int = 100


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


__all__ = [
    "ClassicMDS",
    "DBSCAN",
    "DiffusionEmbedding",
    "DistanceProfileCorrelation",
    "FarthestFirst",
    "KMedoids",
    "MDS",
    "PCFA",
    "PhateAE",
    "SOM",
]
