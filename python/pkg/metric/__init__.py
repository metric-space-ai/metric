"""Python interface for METRIC metric-space numerics.

The revived core API is organized around explicit finite metric spaces:

- ``metric.metrics`` for metric constructors
- ``metric.spaces`` for finite metric-space helpers
- ``metric.operators`` for small metric-space operators
- ``metric.exceptions`` for public METRIC error types
- ``metric.mappings`` and ``metric.transforms`` for beta compatibility bridges

Legacy compiled extension names remain available when their modules are present.
"""

__version__ = "0.3.2"

try:
    from metric._impl.metric import *
except ModuleNotFoundError:
    pass

from . import exceptions, intent, mappings, metrics, operators, representations, spaces, strategies, transforms
from .exceptions import (
    AmbiguousIntentError,
    IncompatibleSpaceError,
    MetricComputationError,
    MetricContractError,
    MetricError,
    MetricInputError,
    MissingMetricError,
    NotFittedError,
    OptionalDependencyError,
    RepresentationError,
    StaleRepresentationError,
    StrategyError,
    StrategyParameterError,
    StrategyUnavailableError,
    UnsupportedOperationError,
)
from .metrics import Edit, Metric
from .operators import (
    ClusteringResult,
    CompressionResult,
    CorrelationResult,
    EmbeddingDiagnostics,
    EmbeddingModel,
    EmbeddingResult,
    GraphConnectivityDiagnostics,
    GraphDegreeDiagnostics,
    GraphStretchDiagnostics,
    GraphConstructionMetadata,
    GraphConstructionResult,
    MappingResult,
    Outlier,
    OutlierResult,
    RepresentativeSet,
    ReductionResult,
    StructureDescription,
    coverage_representative_indices,
    coverage_representatives,
    compress_space,
    compare_spaces,
    correlate_spaces,
    dbscan,
    denoise_space,
    describe_structure,
    embed_space,
    exact_knn_graph,
    exact_knn_graph_edges,
    exact_radius_graph,
    exact_radius_graph_edges,
    find_groups,
    find_outliers,
    find_representatives,
    graph_connectivity_diagnostics,
    graph_degree_diagnostics,
    graph_stretch_diagnostics,
    intrinsic_dimension,
    kmedoids,
    map_space,
    medoid,
    medoid_index,
    nearest_neighbors,
    pairwise_distance_matrix,
    prune_graph_out_degree,
    range_neighbors,
    reduce_space,
    representative_indices,
    representatives,
    separated_representative_indices,
    separated_representatives,
    symmetrize_graph,
)
from .spaces import FiniteMetricSpace, MatrixSpace, Space
from .strategies import ClassicMDS, DBSCAN, DistanceProfileCorrelation, FarthestFirst, KMedoids

__all__ = sorted(
    name
    for name in globals()
    if not name.startswith("_") and name not in {"annotations"}
)
