"""Python interface for METRIC metric-space numerics.

The revived core API is organized around explicit finite metric spaces:

- ``metric.metrics`` for metric constructors
- ``metric.spaces`` for finite metric-space helpers
- ``metric.operators`` for small metric-space operators
- ``metric.mappings`` and ``metric.transforms`` for beta compatibility bridges

Legacy compiled extension names remain available when their modules are present.
"""

__version__ = "0.3.2"

try:
    from metric._impl.metric import *
except ModuleNotFoundError:
    pass

from . import mappings, metrics, operators, spaces, transforms
from .metrics import Edit
from .operators import (
    GraphConnectivityDiagnostics,
    GraphDegreeDiagnostics,
    GraphStretchDiagnostics,
    GraphConstructionMetadata,
    GraphConstructionResult,
    coverage_representative_indices,
    coverage_representatives,
    exact_knn_graph,
    exact_knn_graph_edges,
    exact_radius_graph,
    exact_radius_graph_edges,
    graph_connectivity_diagnostics,
    graph_degree_diagnostics,
    graph_stretch_diagnostics,
    intrinsic_dimension,
    medoid,
    medoid_index,
    nearest_neighbors,
    pairwise_distance_matrix,
    prune_graph_out_degree,
    range_neighbors,
    representative_indices,
    representatives,
    separated_representative_indices,
    separated_representatives,
    symmetrize_graph,
)
from .spaces import FiniteMetricSpace, MatrixSpace, Space

__all__ = sorted(
    name
    for name in globals()
    if not name.startswith("_") and name not in {"annotations"}
)
