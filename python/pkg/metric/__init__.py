"""Python interface for METRIC metric-space numerics.

The revived core API is organized around explicit finite metric spaces:

- ``metric.core`` for central finite metric-space building blocks
- ``metric.metrics`` for metric constructors
- ``metric.spaces`` for finite metric-space helpers
- ``metric.exceptions`` for public METRIC error types
- ``metric.operators`` for small metric-space operators
- ``metric.runtime`` for runtime policy objects
- ``metric.mappings`` for beta native mapping artifacts
- ``metric.transforms`` for transform adapters

Legacy mapping modules are not part of the core Python surface.
"""

__version__ = "0.3.4"

try:
    from metric._impl.metric import *
except ModuleNotFoundError:
    pass

from . import core, exceptions, intent, mapping_pipeline, mappings, metrics, operators, representations, runtime, spaces, strategies, transforms
from .exceptions import (
    AmbiguousIntentError,
    IncompatibleSpaceError,
    MetricComputationError,
    MetricContractError,
    MetricError,
    MetricInputError,
    MissingMetricError,
    NotDerivedError,
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
    EmbeddingArtifact,
    EmbeddingResult,
    GraphConnectivityDiagnostics,
    GraphDegreeDiagnostics,
    GraphStretchDiagnostics,
    GraphConstructionMetadata,
    GraphConstructionResult,
    MappingResult,
    Neighbor,
    NeighborResult,
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
    density_filter_space,
    describe_structure,
    distribution_sample_space,
    equalize_space,
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
    thin_space,
    uniform_density_sample_space,
)
from .runtime import CachePolicy, RuntimeDiagnostics, RuntimePolicy, available, capabilities, runtime_diagnostics
from .spaces import FiniteMetricSpace, MatrixSpace, RecordId, Space

# Low-level native numeric containers are pulled in by the wildcard import above.
# They are implementation detail, not part of the documented top-level adapter
# surface, so keep them out of __all__.
_PRIVATE_NATIVE_NAMES = {"CompressedMatrix", "DynamicMatrix"}

__all__ = sorted(
    name
    for name in globals()
    if not name.startswith("_") and name not in {"annotations"} | _PRIVATE_NATIVE_NAMES
)
