# Process Curve PHATE Gallery

This report records the larger native C++ PHATE-AE process-curve gallery
fixture for METRIC production readiness.

Command:

```bash
build/core/examples/engine/engine_process_curve_phate_gallery
```

Fixture:

- source records are 15 custom `ProcessCurve` records, not vector rows
- held-out queries are six delayed-ramp process curves
- source geometry is defined by native aligned-curve distance
- ordinary vector nearest-neighbor search treats the same curve values as
  padded point vectors
- native `feature_record_codec` supplies fixed-size DNN input features without
  replacing the source metric
- native PHATE-style geometry uses `distance_table_pairwise_distances`,
  `exponential_affinity_kernel`, and `lazy_row_normalized_diffusion_operator`
- native training uses C++ `mtrc::solve::parametric::dnn` with reconstruction loss plus
  bottleneck-coordinate geometry loss

Expected output:

```text
process PHATE gallery records = 15
process PHATE gallery queries = 6
process PHATE gallery source metric = aligned_curve_distance
process PHATE gallery feature codec = process_curve_gallery_feature_codec
process PHATE gallery pipeline components = 11
process PHATE gallery distance provider = distance_table_pairwise_distances
process PHATE gallery affinity kernel = exponential_affinity_kernel
process PHATE gallery diffusion operator = lazy_row_normalized_diffusion_operator
process PHATE gallery target evaluations = 225
process PHATE gallery metric nearest family = late_ramp
process PHATE gallery vector baseline family = flat
process PHATE gallery latent nearest family = late_ramp
process PHATE gallery metric correct = 6/6
process PHATE gallery vector misses = 6/6
process PHATE gallery latent correct = 6/6
process PHATE gallery average metric margin = 2.16667
process PHATE gallery query = gallery_delay_A:metric=late_ramp,vector=flat,latent=late_ramp
process PHATE gallery query = gallery_delay_B:metric=late_ramp,vector=flat,latent=late_ramp
process PHATE gallery query = gallery_delay_C:metric=late_ramp,vector=normal,latent=late_ramp
process PHATE gallery query = gallery_delay_D:metric=late_ramp,vector=normal,latent=late_ramp
process PHATE gallery query = gallery_delay_E:metric=late_ramp,vector=flat,latent=late_ramp
process PHATE gallery query = gallery_delay_F:metric=late_ramp,vector=flat,latent=late_ramp
process PHATE gallery neighbor preservation = 0.4
process PHATE gallery OOS anchor recall = 0.833333
process PHATE gallery first-anchor match rate = 0.833333
process PHATE gallery mapped-anchor source rank = 1.33333
process PHATE gallery max mapped-anchor source rank = 3
process PHATE gallery original-metric distance penalty = 0.166667
process PHATE gallery max original-metric distance penalty = 1
process PHATE gallery bottleneck loss = 0.46694 -> 0.0860003
process PHATE gallery inverse support = none_feature_codec
process PHATE gallery deterministic seed = 41
```

Interpretation:

- the source space is built from records plus an alignment metric; the DNN never
  defines the source geometry
- raw padded-vector nearest-neighbor search misses all six delayed-ramp held-out
  query families
- the fitted native PHATE-AE mapping preserves the alignment-metric family for
  all six held-out queries in latent space
- the feature codec is explicit native pipeline provenance, not Python-side
  PHATE or DNN math
- local-neighbor preservation, out-of-sample anchor stability, mapped-anchor
  source rank, and original-metric distance penalty are reported as native
  diagnostics; they are interpretation signals, not hidden success criteria
- inverse transform is intentionally unavailable because this feature codec has
  no native decoder back to `ProcessCurve`
