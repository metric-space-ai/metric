"""Python interface for METRIC metric-space numerics.

The revived core API is organized around explicit finite metric spaces:

- ``metric.core`` for central finite metric-space building blocks
- ``metric.metrics`` for metric constructors
- ``metric.spaces`` for finite metric-space helpers
- ``metric.exceptions`` for public METRIC error types
- ``metric.operators`` for small metric-space operators
- ``metric.runtime`` for runtime policy objects
- ``metric.mappings`` and ``metric.transforms`` for beta compatibility bridges
- ``metric.compat`` for lazy legacy import-path discovery

Legacy compiled extension names remain available when their modules are present.
"""

__version__ = "0.3.4"

try:
    from metric._impl.metric import *
except ModuleNotFoundError:
    pass

from . import compat, core, exceptions, intent, mapping_pipeline, mappings, metrics, operators, representations, runtime, spaces, strategies, transforms
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
from .runtime import CachePolicy, RuntimeDiagnostics, RuntimePolicy, runtime_diagnostics
from .spaces import FiniteMetricSpace, MatrixSpace, RecordId, Space


def available():
    """Report which native capabilities this installed wheel actually provides.

    Returns a dict mapping each native extension module to whether it imports, so
    callers can check what runs vs. what raises StrategyUnavailableError /
    OptionalDependencyError before relying on an operation. ``distance``/``space``/
    ``metric``/``transform`` ship in the core wheel; ``entropy``/``mgc``
    (correlation) require the FULL build.
    """

    import importlib

    modules = {
        "distance": "metric._impl.distance",
        "space": "metric._impl.space",
        "metric": "metric._impl.metric",
        "transform": "metric._impl.transform",
        "entropy": "metric._impl.entropy",
        "mgc": "metric._impl.mgc",
    }
    capabilities = {}
    for name, module_path in modules.items():
        try:
            importlib.import_module(module_path)
            capabilities[name] = True
        except Exception:
            capabilities[name] = False
    return capabilities


__all__ = sorted(
    name
    for name in globals()
    if not name.startswith("_") and name not in {"annotations"}
)
