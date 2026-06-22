# Metric-Space Mapping Pipeline (hero)

This is the flagship demonstration of METRIC's **finite metric-space mapping**: a
source finite metric space is transformed into a derived coordinate space by a
pipeline of interchangeable, role-based components. PHATE-AE appears here not as
an opaque machine-learning model but as the *solver* of one stage in that
pipeline — the metric stays authoritative and visible from the source records
through target construction, training, the persisted artifact, the lineage, and
the diagnostics.

The companion [PHATE-AE pipeline workflow](phate-ae-pipeline-workflow.md) and the
[process-curve PHATE map](process-curve-phate-map.md) cover the feature-codec
(non-vector) path; this hero covers the decoder-capable vector-row path and adds
an explicit, fair comparison against a classical multidimensional-scaling
baseline computed from the *same metric values*.

## Command

```sh
cmake --preset core
cmake --build build/core --target engine_metric_space_mapping_pipeline
./build/core/examples/engine/engine_metric_space_mapping_pipeline
```

The gate equivalent is `ctest --preset core -R metric_space_mapping_pipeline`,
which runs both the example and the `metric_space_mapping_pipeline_smoke`
contract test.

## Fixture

A 16-record finite metric space sampled along a half-circle arc in `R^2` (a
curved one-manifold), governed by the Euclidean metric — a true metric law. The
intrinsic structure is one-dimensional while the ambient space is planar, the
canonical "unrolling" fixture.

The pipeline is composed of these interchangeable components (the hero promotes
the non-default variant in each swappable slot):

| Role | Component used | Default alternative |
| --- | --- | --- |
| `codec` | `vector_record_codec` | `feature_record_codec` |
| `pairwise_distances` | `distance_table_pairwise_distances` | `exact_space_distances` |
| `affinity_kernel` | `exponential_affinity_kernel` | `gaussian_affinity_kernel` |
| `diffusion_operator` | `lazy_row_normalized_diffusion_operator` | `row_normalized_diffusion_operator` |
| `target_generator` | `diffusion_potential_anchor_coordinates` | — |
| `trainer` | `native_dnn_autoencoder_trainer` | — |
| `mapping_model` | `native_autoencoder_mapping_model` | — |
| `artifact` | `native_mapping_artifact` | — |

The naive control is **classical multidimensional scaling**: double-centre the
squared distance matrix into a Gram matrix and read off the top eigenvector with
deterministic power iteration. It consumes the same metric values, embeds to the
same target dimension, and is scored with the same neighbor-preservation
diagnostic on identical one-to-one lineage.

## Expected output

```
mapping_pipeline_source_space = finite_metric_space
mapping_pipeline_source_metric = euclidean
mapping_pipeline_source_records = 16
mapping_pipeline_source_dimension = 2
mapping_pipeline_metric_mean_distance = 1.029598
mapping_pipeline_metric_dense_evaluations = 256
mapping_pipeline_target_method = diffusion_potential_anchor_coordinates
mapping_pipeline_target_dimensions = 1
mapping_pipeline_target_diffusion_steps = 5
mapping_pipeline_target_kernel_scale = 1.029598
mapping_pipeline_component_codec = vector_record_codec
mapping_pipeline_component_pairwise_distances = distance_table_pairwise_distances
mapping_pipeline_component_affinity_kernel = exponential_affinity_kernel
mapping_pipeline_component_diffusion_operator = lazy_row_normalized_diffusion_operator
mapping_pipeline_component_count = 11
mapping_pipeline_solver = native_dnn_autoencoder_trainer
mapping_pipeline_solver_epochs = 400
mapping_pipeline_solver_seed = 29
mapping_pipeline_solver_bottleneck_loss = 0.183192 -> 0.020674
mapping_pipeline_artifact_format = metric.native_phate_autoencoder_artifact
mapping_pipeline_artifact_backend = native_dnn
mapping_pipeline_artifact_roundtrip = transform_and_lineage_parity
mapping_pipeline_lineage = one_to_one_source_records
mapping_pipeline_inverse_support = decoder_inverse_transform
mapping_pipeline_reconstruction_mse = 0.126603
mapping_pipeline_phate_neighbor_recall = 0.854167
mapping_pipeline_phate_out_of_sample_records = 3
mapping_pipeline_phate_out_of_sample_anchor_recall = 0.777778
mapping_pipeline_baseline = classical_mds
mapping_pipeline_baseline_inverse_support = none
mapping_pipeline_baseline_out_of_sample_support = none
mapping_pipeline_baseline_neighbor_recall = 0.875000
mapping_pipeline_neighbor_recall_margin = -0.020833
mapping_pipeline_parametric_advantage = out_of_sample_and_inverse_vs_none
```

The seed (`29`) is reported because the native solver is deterministic with
shuffling disabled; the exact loss and recall values are stable on the reference
toolchain. The gate test asserts robust bounds rather than exact equalities, so
floating-point variation across platforms does not break CI.

## Interpretation

**Metric values are the single source of truth.** The mean pairwise distance and
the `256` dense evaluations (`16 x 16`) describe the materialized distance
matrix. That matrix feeds *both* the diffusion target and the classical-MDS
baseline — neither sees the raw records, which is what makes the comparison a
metric-space comparison rather than an algorithm bake-off.

**Target construction is an explicit, swappable stage.** The diffusion-potential
anchor coordinates are produced from the affinity kernel and diffusion operator
named in the plan. Swapping `gaussian_affinity_kernel` for
`exponential_affinity_kernel`, or the strict diffusion operator for its lazy
variant, changes the target geometry without touching the source metric or the
solver — these are component slots, not buried hyperparameters.

**The solver fits the geometry, and it converges.** The bottleneck-coordinate
loss falls from `0.183` to `0.021` over `400` epochs: the native C++ autoencoder
learns to place its latent axis on the diffusion-potential target. No Python and
no external math library participate.

**The artifact is a real boundary.** The fitted model serializes to a
`metric.native_phate_autoencoder_artifact`, and reloading it reproduces the
derived space and the inverse transform exactly (`transform_and_lineage_parity`).
The one-to-one source lineage survives the roundtrip, so every derived record can
be traced back to its source record after persistence.

**The baseline comparison is honest.** On this simple convex manifold the linear
distance embedding already preserves neighbors well (`0.875`), so the diffusion
map does *not* win the in-sample recall race (`0.854`, margin `-0.021`) — and the
demo does not pretend it does. The decisive advantage is **structural and
always true**: the PHATE-AE map is *parametric* (it transforms held-out records,
here at `0.778` anchor recall on three unseen points) and *invertible* (it has a
decoder), while classical MDS is a one-shot, in-sample-only, non-invertible
embedding that must be refit from scratch to place any new point. Comparable
neighbor fidelity *plus* out-of-sample transform *plus* an inverse is the value
the pipeline delivers over the naive baseline.

## What this demonstrates

- **Level 1 (finite metric-space question).** Fit a map from a source finite
  metric space into a derived coordinate space that respects the source's
  diffusion geometry, and keep the derived space traceable to the source.
- **Level 2 (namespace owners).** The pipeline lives under
  `mtrc::modify::compose` (plan composition) and `mtrc::modify::map`
  (target construction, fit/transform, artifact, lineage, diagnostics), over
  `mtrc::solve::parametric::dnn` (the native solver) and
  `mtrc::space::storage` (the metric values).
- **Level 3 (implementation).** PHATE-style diffusion-potential anchor
  coordinates as the target, a native C++ autoencoder as the solver, a
  serialized mapping artifact as the boundary, and a classical-MDS control for
  the baseline — all deterministic under a fixed seed.

The same pattern — metric semantics, an explicit target, an interchangeable
solver, a persisted artifact, preserved lineage, and transparent diagnostics —
is METRIC's general approach to mapping; PHATE-AE is one exemplary instance, not
a special case. See the
[cross-space dependency baseline](cross-space-dependency-baseline.md) and the
[mixed structured record baseline](mixed-structured-record-baseline.md) for the
same discipline applied to other questions.
