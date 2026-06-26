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
    """Roadmap diffusion-coordinate embedding strategy.

    The strategy object is importable so examples can use stable engine
    vocabulary, but execution is promoted only after deterministic fixtures and
    CI-backed result contracts exist.
    """

    dimensions: int = 2
    knn: object = None
    diffusion_time: object = "auto"


@dataclass(frozen=True)
class ParametricDiffusionCoordinates:
    """Native parametric coordinate derivation over a finite metric space.

    The native derivation is deterministic by construction, so a given input
    yields the same coordinates. ``seed`` is reserved for native solvers that
    need it; the current native path is structurally deterministic.
    """

    dimensions: int = 2
    calibration_steps: int = 100
    step_size: float = 0.05
    diffusion_steps: int = 2
    kernel_scale: object = None
    reconstruction_weight: float = 0.05
    geometry_weight: float = 1.0
    seed: int = 23
    distance_provider: str = "exact_metric_space_distance_provider"
    affinity_kernel: str = "gaussian_affinity_kernel"
    diffusion_operator: str = "row_normalized_diffusion_operator"
    max_memory_bytes: object = None
    max_distance_evaluations: object = None
    max_dense_records: object = None


@dataclass(frozen=True)
class RedifDynamics:
    """Metric-induced Redif measure dynamics over a finite metric space."""

    neighbors: int = 10
    iterations: int = 15
    euler_step: float = 0.25
    adaptive_geometry: bool = True
    scale_policy: str = "mean_local_distance"
    stability_tolerance: float = 1.0e-12
    marginal_stability_tolerance: float = 1.0e-8


@dataclass(frozen=True)
class ForwardDynamics(RedifDynamics):
    """Forward metric-induced dynamics, interpreted as adding noise."""


@dataclass(frozen=True)
class InverseDynamics(RedifDynamics):
    """Inverse metric-induced dynamics, interpreted as removing noise."""


@dataclass(frozen=True)
class TransportPath(RedifDynamics):
    """Score atoms by Redif transport path length."""


@dataclass(frozen=True)
class FarthestFirst:
    """Deterministic farthest-first representative-selection strategy."""

    seed_index: int = 0


@dataclass(frozen=True)
class Coverage:
    """Coverage/k-center representative-selection strategy by requested count."""

    seed_index: int = 0


@dataclass(frozen=True)
class KCenter:
    """Explicit k-center alias for coverage-style representative selection."""

    seed_index: int = 0


@dataclass(frozen=True)
class RadiusCoverage:
    """Radius-bounded coverage strategy that chooses the representative count."""

    radius: float


@dataclass(frozen=True)
class PreserveDistribution:
    """Deterministic regular thinning that preserves empirical sampling order."""

    offset: int = 0


@dataclass(frozen=True)
class UniformDensity:
    """Uniform-density thinning by maximal metric radius net."""

    radius: float


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
    "Coverage",
    "DBSCAN",
    "DiffusionEmbedding",
    "DistanceProfileCorrelation",
    "FarthestFirst",
    "ForwardDynamics",
    "InverseDynamics",
    "KCenter",
    "KMedoids",
    "MDS",
    "ParametricDiffusionCoordinates",
    "PreserveDistribution",
    "RadiusCoverage",
    "RedifDynamics",
    "TransportPath",
    "UniformDensity",
]
