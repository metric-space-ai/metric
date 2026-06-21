# PHATE-AE Pipeline Hero

This report records the CI-safe learned metric-space pipeline demonstrator for
METRIC production readiness.

The vector-row fixture below proves the decoder-capable special case. The
non-vector process-curve fixture in [Process Curve PHATE Map](process-curve-phate-map.md)
proves the same native pipeline-builder model with `feature_record_codec`,
where source geometry comes from an alignment metric and the codec only supplies
DNN input features. The larger [Process Curve PHATE Gallery](process-curve-phate-gallery.md)
extends that non-vector path to 15 source records and six held-out delayed-ramp
queries.

Command:

```bash
build/core/examples/engine/engine_phate_autoencoder_map
```

Fixture:

- source records are aligned vector rows with an explicit Euclidean metric
- native PHATE-style geometry uses the executable pipeline hooks
  `matrix_cache_distance_provider`, `exponential_affinity_kernel`, and
  `lazy_row_normalized_diffusion_operator`
- the native pipeline exposes `vector_record_codec` as an explicit component
  rather than hiding feature encoding inside the fit call
- native training uses the C++ `metric::dnn` autoencoder path with
  reconstruction loss plus bottleneck-coordinate geometry loss
- ordinary vector nearest-neighbor search is kept as a baseline for
  out-of-sample handling: it can return a nearest source anchor but does not
  provide a reusable parametric transform or decoder inverse

Expected hero output:

```text
pipeline = native_phate_autoencoder_pipeline
pipeline components = 11
pipeline codec = vector_record_codec
pipeline distance provider = matrix_cache_distance_provider
pipeline affinity kernel = exponential_affinity_kernel
pipeline diffusion operator = lazy_row_normalized_diffusion_operator
PHATE-AE bottleneck loss: 0.248425 -> 0.160057
PHATE-AE neighbor preservation = 0.8
PHATE-AE reconstruction MSE: 2.04967
vector baseline out-of-sample support = nearest_anchor_only
vector baseline inverse support = no_decoder
out-of-sample support = parametric_space_transform
inverse support = decoder_inverse_transform
out-of-sample anchor recall = 1
```

Interpretation:

- the promoted hero path is an explicit native pipeline, not an opaque
  one-shot algorithm call
- the fitted model carries component provenance through the pipeline plan
- codec selection is now part of that provenance; the companion process-curve
  PHATE map uses `feature_record_codec` for non-vector records without
  Python-side algorithm work
- the larger process-curve PHATE gallery shows the same native pipeline
  preserving 6/6 held-out alignment-metric families while the padded-vector
  nearest-neighbor baseline misses 6/6
- bottleneck geometry loss decreases during native training
- local-neighbor preservation and out-of-sample anchor stability are reported
  from native mapping diagnostics
- the vector baseline can identify an existing anchor, but it is not a learned
  reusable mapping model and has no decoder inverse
