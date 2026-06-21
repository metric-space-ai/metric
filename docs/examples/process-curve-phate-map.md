# Process Curve PHATE Map

This report records the first CI-safe non-vector PHATE-AE pipeline fixture for
METRIC production readiness.

Command:

```bash
build/core/examples/engine/engine_process_curve_phate_map
```

Fixture:

- source records are custom `ProcessCurve` records, not vector rows
- source geometry is defined by native aligned-curve distance
- ordinary vector baseline treats the same curve values as padded point vectors
- native `feature_record_codec` supplies fixed-size DNN input features without
  replacing the source metric
- native PHATE-style geometry uses the executable pipeline hooks
  `matrix_cache_distance_provider`, `exponential_affinity_kernel`, and
  `lazy_row_normalized_diffusion_operator`
- native training uses the C++ `metric::dnn` autoencoder path with
  reconstruction loss plus bottleneck-coordinate geometry loss

Expected output:

```text
process PHATE records = 10
process PHATE queries = 4
process PHATE source metric = aligned_curve_distance
process PHATE feature codec = process_curve_feature_codec
process PHATE pipeline components = 11
process PHATE pipeline codec = feature_record_codec
process PHATE distance provider = matrix_cache_distance_provider
process PHATE affinity kernel = exponential_affinity_kernel
process PHATE diffusion operator = lazy_row_normalized_diffusion_operator
process PHATE geometry target evaluations = 100
process PHATE metric nearest family = late_ramp
process PHATE vector baseline family = flat
process PHATE latent nearest family = late_ramp
process PHATE metric correct = 4/4
process PHATE vector misses = 4/4
process PHATE latent correct = 4/4
process PHATE query = query_delayed_ramp:metric=late_ramp,vector=flat,latent=late_ramp
process PHATE query = query_delayed_ramp_long:metric=late_ramp,vector=flat,latent=late_ramp
process PHATE query = query_delayed_ramp_short:metric=late_ramp,vector=normal,latent=late_ramp
process PHATE query = query_delayed_ramp_deep_delay:metric=late_ramp,vector=flat,latent=late_ramp
process PHATE bottleneck loss = 0.6213 -> 0.160793
process PHATE out-of-sample support = parametric_space_transform
process PHATE out-of-sample anchor recall = 0.5
process PHATE mapped-anchor source rank = 1.75
process PHATE max mapped-anchor source rank = 3
process PHATE original-metric distance penalty = 0.25
process PHATE max original-metric distance penalty = 1
process PHATE inverse support = none_feature_codec
process PHATE deterministic seed = 29
```

Interpretation:

- METRIC constructs the source space from records plus a domain metric; it does
  not require vectorizing the records first
- the raw padded-vector baseline misses all four delayed-ramp query families
- the fitted native PHATE-AE mapping preserves the alignment-metric family for
  all four held-out queries in latent space
- out-of-sample diagnostics report mapped-anchor source rank and original-metric
  distance penalty, so parametric transform quality is visible beyond a binary
  family hit count
- codec selection is explicit pipeline provenance: `feature_record_codec` is a
  native C++ component, not Python-side PHATE or DNN math
- inverse transform is intentionally unavailable because this feature codec has
  no native decoder back to `ProcessCurve`
