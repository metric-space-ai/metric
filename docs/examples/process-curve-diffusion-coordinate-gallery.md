# Process Curve parametric diffusion coordinates Gallery

This report records the larger native C++ parametric diffusion coordinate process-curve gallery
fixture for METRIC production readiness.

Command:

```bash
build/core/examples/engine/engine_process_curve_diffusion_coordinate_gallery
```

Fixture:

- source records are 15 custom `ProcessCurve` records, not vector rows
- held-out queries are six delayed-ramp process curves
- source geometry is defined by native aligned-curve distance
- ordinary vector nearest-neighbor search treats the same curve values as
  padded point vectors
- native `record_coordinate_codec` supplies fixed-size solver input coordinates without
  replacing the source metric
- parametric diffusion coordinate-style geometry uses `distance_table_pairwise_distances`,
  `exponential_affinity_kernel`, and `lazy_row_normalized_diffusion_operator`
- native calibration uses the C++ coordinate solver path with reconstruction
  error plus coordinate-target error

Expected output:

```text
process diffusion-coordinate gallery records = 15
process diffusion-coordinate gallery queries = 6
process diffusion-coordinate gallery source metric = aligned_curve_distance
process diffusion-coordinate gallery record-coordinate codec = process_curve_gallery_coordinate_codec
process diffusion-coordinate gallery pipeline components = 11
process diffusion-coordinate gallery distance provider = distance_table_pairwise_distances
process diffusion-coordinate gallery affinity kernel = exponential_affinity_kernel
process diffusion-coordinate gallery diffusion operator = lazy_row_normalized_diffusion_operator
process diffusion-coordinate gallery target evaluations = 225
process diffusion-coordinate gallery metric nearest family = late_ramp
process diffusion-coordinate gallery vector baseline family = flat
process diffusion-coordinate gallery latent nearest family = late_ramp
process diffusion-coordinate gallery metric correct = 6/6
process diffusion-coordinate gallery vector misses = 6/6
process diffusion-coordinate gallery latent correct = 6/6
process diffusion-coordinate gallery average metric margin = 2.16667
process diffusion-coordinate gallery query = gallery_delay_A:metric=late_ramp,vector=flat,latent=late_ramp
process diffusion-coordinate gallery query = gallery_delay_B:metric=late_ramp,vector=flat,latent=late_ramp
process diffusion-coordinate gallery query = gallery_delay_C:metric=late_ramp,vector=normal,latent=late_ramp
process diffusion-coordinate gallery query = gallery_delay_D:metric=late_ramp,vector=normal,latent=late_ramp
process diffusion-coordinate gallery query = gallery_delay_E:metric=late_ramp,vector=flat,latent=late_ramp
process diffusion-coordinate gallery query = gallery_delay_F:metric=late_ramp,vector=flat,latent=late_ramp
process diffusion-coordinate gallery neighbor preservation = 0.4
process diffusion-coordinate gallery OOS anchor recall = 0.833333
process diffusion-coordinate gallery first-anchor match rate = 0.833333
process diffusion-coordinate gallery mapped-anchor source rank = 1.33333
process diffusion-coordinate gallery max mapped-anchor source rank = 3
process diffusion-coordinate gallery original-metric distance penalty = 0.166667
process diffusion-coordinate gallery max original-metric distance penalty = 1
process diffusion-coordinate gallery target error = 0.46694 -> 0.0860003
process diffusion-coordinate gallery inverse support = none_record_coordinate_codec
process diffusion-coordinate gallery deterministic seed = 41
```

Interpretation:

- the source space is built from records plus an alignment metric; the coordinate solver never
  defines the source geometry
- raw padded-vector nearest-neighbor search misses all six delayed-ramp held-out
  query families
- the derived native parametric diffusion coordinate mapping preserves the alignment-metric family for
  all six held-out queries in latent space
- the record-coordinate codec is explicit native pipeline provenance, not Python-side
  coordinate math
- local-neighbor preservation, out-of-sample anchor stability, mapped-anchor
  source rank, and original-metric distance penalty are reported as native
  diagnostics; they are interpretation signals, not hidden success criteria
- inverse transform is intentionally unavailable because this record-coordinate codec has
  no native decoder back to `ProcessCurve`
