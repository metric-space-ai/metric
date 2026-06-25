# Process Curve parametric diffusion coordinates Map

This report records the first CI-safe non-vector parametric diffusion coordinate pipeline fixture for
METRIC production readiness.

Command:

```bash
build/core/examples/engine/engine_process_curve_diffusion_coordinate_map
```

Fixture:

- source records are custom `ProcessCurve` records, not vector rows
- source geometry is defined by native aligned-curve distance
- ordinary vector baseline treats the same curve values as padded point vectors
- native `record_coordinate_codec` supplies fixed-size solver input coordinates without
  replacing the source metric
- parametric diffusion coordinate-style geometry uses the executable pipeline hooks
  `distance_table_pairwise_distances`, `exponential_affinity_kernel`, and
  `lazy_row_normalized_diffusion_operator`
- native calibration uses the C++ coordinate solver path with reconstruction
  error plus coordinate-target error

Expected output:

```text
process diffusion coordinate records = 10
process diffusion coordinate queries = 4
process diffusion coordinate source metric = aligned_curve_distance
process diffusion coordinate record-coordinate codec = process_curve_coordinate_codec
process diffusion-coordinate pipeline components = 11
process diffusion-coordinate pipeline codec = record_coordinate_codec
process diffusion coordinate distance provider = distance_table_pairwise_distances
process diffusion coordinate affinity kernel = exponential_affinity_kernel
process diffusion-coordinate operator = lazy_row_normalized_diffusion_operator
process diffusion-coordinate target target evaluations = 100
process diffusion coordinate metric nearest family = late_ramp
process diffusion coordinate vector baseline family = flat
process diffusion coordinate latent nearest family = late_ramp
process diffusion coordinate metric correct = 4/4
process diffusion coordinate vector misses = 4/4
process diffusion coordinate latent correct = 4/4
process diffusion coordinate query = query_delayed_ramp:metric=late_ramp,vector=flat,latent=late_ramp
process diffusion coordinate query = query_delayed_ramp_long:metric=late_ramp,vector=flat,latent=late_ramp
process diffusion coordinate query = query_delayed_ramp_short:metric=late_ramp,vector=normal,latent=late_ramp
process diffusion coordinate query = query_delayed_ramp_deep_delay:metric=late_ramp,vector=flat,latent=late_ramp
process diffusion coordinate target error = 0.6213 -> 0.160793
process diffusion coordinate out-of-sample support = parametric_space_transform
process diffusion coordinate out-of-sample anchor recall = 0.5
process parametric diffusion coordinate mapped-anchor source rank = 1.75
process diffusion coordinate max mapped-anchor source rank = 3
process diffusion coordinate original-metric distance penalty = 0.25
process diffusion coordinate max original-metric distance penalty = 1
process diffusion coordinate inverse support = none_record_coordinate_codec
process diffusion coordinate deterministic seed = 29
```

Interpretation:

- METRIC constructs the source space from records plus a domain metric; it does
  not require vectorizing the records first
- the raw padded-vector baseline misses all four delayed-ramp query families
- the derived native parametric diffusion coordinate mapping preserves the alignment-metric family for
  all four held-out queries in latent space
- out-of-sample diagnostics report mapped-anchor source rank and original-metric
  distance penalty, so parametric transform quality is visible beyond a binary
  family hit count
- codec selection is explicit pipeline provenance: `record_coordinate_codec` is a
  native C++ component, not Python-side coordinate math
- inverse transform is intentionally unavailable because this record-coordinate codec has
  no native decoder back to `ProcessCurve`
