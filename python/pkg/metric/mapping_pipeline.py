"""Metric-Space Coordinate Pipeline -- Python adapter.

This module exposes a native finite metric-space coordinate pipeline to Python
as an adapter only. It marshals row data to the native C++ engine and
re-packages the engine's results; it never computes diffusion geometry,
coordinate calibration, or neighbor-preservation scoring in Python. When the
native binding is absent, every entry point raises
:class:`StrategyUnavailableError` rather than falling back to a Python
reimplementation.

The pipeline is framed as a finite metric-space transform, not a neural-network
product: a source finite metric space (records + a metric) is mapped into a
derived coordinate space through interchangeable, role-based components
(codec, pairwise distances, affinity kernel, diffusion operator, target
generator, calibration component, coordinate-map component, artifact). The artifact handle surfaces those
components, the one-to-one lineage, and native diagnostics.
"""

from dataclasses import dataclass

from metric.exceptions import StrategyUnavailableError

STABILITY = "beta"

_UNAVAILABLE_MESSAGE = (
    "The Metric-Space Mapping Pipeline requires the native C++ binding. "
    "This adapter does not reimplement metric-space diffusion or neighbor "
    "preservation in Python."
)

def _load_native_engine_module():
    try:
        from metric._impl import metric as native_metric
    except (ImportError, ModuleNotFoundError):
        return None
    return native_metric


def _require_native(attribute):
    native_metric = _load_native_engine_module()
    if native_metric is None or not hasattr(native_metric, attribute):
        raise StrategyUnavailableError(_UNAVAILABLE_MESSAGE)
    return native_metric


def _normalize_component(component):
    return dict(component)


@dataclass(frozen=True)
class MetricSpaceMappingArtifact:
    """Adapter handle for a native metric-space mapping artifact.

    All properties and methods delegate to the native artifact; this class holds
    no mapping state and performs no numerical work.
    """

    _artifact: object
    distance_provider: str
    affinity_kernel: str
    diffusion_operator: str

    @property
    def mapping(self):
        return "parametric_diffusion_coordinates"

    @property
    def strategy(self):
        return "native_metric_diffusion_coordinate_solver"

    @property
    def source_record_count(self):
        return self._artifact.source_record_count

    @property
    def latent_dimension(self):
        return self._artifact.latent_dimension

    @property
    def inverse_supported(self):
        return self._artifact.inverse_supported

    @property
    def components(self):
        """The interchangeable pipeline components, as (role, name) records."""
        return tuple(_normalize_component(component) for component in self._artifact.components)

    def lineage(self):
        """One-to-one provenance from the derived space back to the source."""
        return dict(self._artifact.lineage())

    def transform(self, records):
        """Map a finite metric space (rows) into the derived coordinate space."""
        return self._artifact.transform(records)

    def inverse_transform(self, latent_records):
        """Decode derived coordinates back into source-shaped records."""
        return self._artifact.inverse_transform(latent_records)

    def neighbor_recall(self, neighbor_count=3):
        """Native neighbor-preservation recall of the derived coordinate space."""
        return self._artifact.neighbor_recall(neighbor_count)

    def calibration_report(self):
        return dict(self._artifact.calibration_report())

    def to_dict(self):
        return {
            "mapping": self.mapping,
            "strategy": self.strategy,
            "source_record_count": self.source_record_count,
            "latent_dimension": self.latent_dimension,
            "inverse_supported": self.inverse_supported,
            "distance_provider": self.distance_provider,
            "affinity_kernel": self.affinity_kernel,
            "diffusion_operator": self.diffusion_operator,
            "components": list(self.components),
            "lineage": self.lineage(),
        }


def pipeline_components(
    *,
    reconstruction_weight=0.05,
    geometry_weight=1.0,
    distance_provider="exact_metric_space_distance_provider",
    affinity_kernel="gaussian_affinity_kernel",
    diffusion_operator="row_normalized_diffusion_operator",
):
    """Return the interchangeable components of the pipeline plan.

    A pure-metadata adapter: it resolves the plan in the native engine and
    returns its role-based components without deriving any artifact.
    """
    native_metric = _require_native("_metric_space_mapping_pipeline_plan")
    components = native_metric._metric_space_mapping_pipeline_plan(
        reconstruction_weight=reconstruction_weight,
        geometry_weight=geometry_weight,
        distance_provider=distance_provider,
        affinity_kernel=affinity_kernel,
        diffusion_operator=diffusion_operator,
    )
    return tuple(_normalize_component(component) for component in components)


def derive_mapping_pipeline(
    records,
    *,
    dimensions=1,
    calibration_steps=100,
    step_size=0.01,
    diffusion_steps=3,
    kernel_scale=1.0,
    reconstruction_weight=0.05,
    geometry_weight=1.0,
    seed=29,  # reserved for API stability; the native map is deterministic (no effect)
    distance_provider="exact_metric_space_distance_provider",
    affinity_kernel="gaussian_affinity_kernel",
    diffusion_operator="row_normalized_diffusion_operator",
):
    """Derive the native metric-space coordinate artifact for vector-row records.

    Binding adapter only: list/NumPy rows are marshalled to the native C++
    pipeline, which performs all target construction and calibration.

    The native map is DETERMINISTIC by construction (closed-form weight
    initialization and no batch shuffling), so a given input always yields the
    same coordinates. ``seed`` is accepted for API stability but has no effect on
    the result; reproducibility is guaranteed structurally, not via the seed (see
    the determinism test in tests/core/test_mapping_pipeline_adapter.py).
    """
    native_metric = _require_native("_metric_space_mapping_pipeline_derive")
    artifact = native_metric._metric_space_mapping_pipeline_derive(
        records,
        dimensions=dimensions,
        calibration_steps=calibration_steps,
        step_size=step_size,
        diffusion_steps=diffusion_steps,
        kernel_scale=kernel_scale,
        reconstruction_weight=reconstruction_weight,
        geometry_weight=geometry_weight,
        seed=seed,
        distance_provider=distance_provider,
        affinity_kernel=affinity_kernel,
        diffusion_operator=diffusion_operator,
    )
    return MetricSpaceMappingArtifact(
        artifact,
        distance_provider=distance_provider,
        affinity_kernel=affinity_kernel,
        diffusion_operator=diffusion_operator,
    )
