"""Metric-Space Mapping Pipeline -- Python adapter.

This module exposes the native finite metric-space mapping pipeline (PHATE-AE)
to Python *as an adapter only*. It marshals row data to the native C++ engine
and re-packages the engine's results; it never computes PHATE geometry,
diffusion, neural training, or neighbor-preservation scoring in Python. When the
native binding is absent (e.g. a wheel built without it), every entry point
raises :class:`StrategyUnavailableError` rather than falling back to a Python
reimplementation -- Python must not own the mapping math.

The pipeline is framed as a finite metric-space transform, not a neural-network
product: a source finite metric space (records + a metric) is mapped into a
derived coordinate space through interchangeable, role-based components
(codec, pairwise distances, affinity kernel, diffusion operator, target
generator, trainer, mapping model, artifact). The model handle surfaces those
components, the one-to-one lineage, and native diagnostics.
"""

from dataclasses import dataclass

from metric.exceptions import StrategyUnavailableError

STABILITY = "beta"

_UNAVAILABLE_MESSAGE = (
    "The Metric-Space Mapping Pipeline requires the native C++ binding. "
    "This adapter does not reimplement PHATE-AE, diffusion, or neighbor "
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


@dataclass(frozen=True)
class MetricSpaceMappingModel:
    """Adapter handle for a native fitted metric-space mapping.

    All properties and methods delegate to the native model; this class holds
    no mapping state and performs no numerical work.
    """

    _model: object
    distance_provider: str
    affinity_kernel: str
    diffusion_operator: str

    @property
    def mapping(self):
        return self._model.mapping

    @property
    def strategy(self):
        return self._model.strategy

    @property
    def source_record_count(self):
        return self._model.source_record_count

    @property
    def latent_dimension(self):
        return self._model.latent_dimension

    @property
    def inverse_supported(self):
        return self._model.inverse_supported

    @property
    def components(self):
        """The interchangeable pipeline components, as (role, name) records."""
        return tuple(dict(component) for component in self._model.components)

    def lineage(self):
        """One-to-one provenance from the derived space back to the source."""
        return dict(self._model.lineage())

    def transform(self, records):
        """Map a finite metric space (rows) into the derived coordinate space."""
        return self._model.transform(records)

    def inverse_transform(self, latent_records):
        """Decode derived coordinates back into source-shaped records."""
        return self._model.inverse_transform(latent_records)

    def neighbor_recall(self, neighbor_count=3):
        """Native neighbor-preservation recall of the fitted derived space."""
        return self._model.neighbor_recall(neighbor_count)

    def training_report(self):
        return self._model.training_report()

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
    returns its role-based components without fitting anything.
    """
    native_metric = _require_native("_metric_space_mapping_pipeline_plan")
    components = native_metric._metric_space_mapping_pipeline_plan(
        reconstruction_weight=reconstruction_weight,
        geometry_weight=geometry_weight,
        distance_provider=distance_provider,
        affinity_kernel=affinity_kernel,
        diffusion_operator=diffusion_operator,
    )
    return tuple(dict(component) for component in components)


def fit_mapping_pipeline(
    records,
    *,
    dimensions=1,
    epochs=100,
    learning_rate=0.01,
    diffusion_steps=3,
    kernel_scale=1.0,
    reconstruction_weight=0.05,
    geometry_weight=1.0,
    seed=29,
    distance_provider="exact_metric_space_distance_provider",
    affinity_kernel="gaussian_affinity_kernel",
    diffusion_operator="row_normalized_diffusion_operator",
):
    """Fit the native metric-space mapping pipeline to vector-row records.

    Binding adapter only: list/NumPy rows are marshalled to the native C++
    pipeline, which performs all target construction and training.
    """
    native_metric = _require_native("_metric_space_mapping_pipeline_fit")
    model = native_metric._metric_space_mapping_pipeline_fit(
        records,
        dimensions=dimensions,
        epochs=epochs,
        learning_rate=learning_rate,
        diffusion_steps=diffusion_steps,
        kernel_scale=kernel_scale,
        reconstruction_weight=reconstruction_weight,
        geometry_weight=geometry_weight,
        seed=seed,
        distance_provider=distance_provider,
        affinity_kernel=affinity_kernel,
        diffusion_operator=diffusion_operator,
    )
    return MetricSpaceMappingModel(
        model,
        distance_provider=distance_provider,
        affinity_kernel=affinity_kernel,
        diffusion_operator=diffusion_operator,
    )
