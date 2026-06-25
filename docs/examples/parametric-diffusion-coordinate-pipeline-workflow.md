# Parametric Diffusion Coordinate Pipeline Workflow

This report records the CI-safe parametric diffusion coordinate preset workflow for METRIC production
readiness. In METRIC terms, the workflow builds diffusion compression from
source distances, calibrates a native coordinate map, and reports lineage plus diagnostics.

The vector-row fixture below proves the decoder-capable special case. The
non-vector process-curve fixture in [Process Curve parametric diffusion coordinates Map](process-curve-diffusion-coordinate-map.md)
proves the same native pipeline-builder contract with `record_coordinate_codec`,
where source geometry comes from an alignment metric and the codec only supplies
solver input coordinates. The larger [Process Curve parametric diffusion coordinates Gallery](process-curve-diffusion-coordinate-gallery.md)
extends that non-vector path to 15 source records and six held-out delayed-ramp
queries.

Command:

```bash
build/core/examples/engine/engine_parametric_diffusion_coordinate_map
```

Fixture:

- source records are aligned vector rows with an explicit Euclidean metric
- parametric diffusion coordinate-style geometry uses the executable pipeline hooks
  `distance_table_pairwise_distances`, `exponential_affinity_kernel`, and
  `lazy_row_normalized_diffusion_operator`
- the native pipeline exposes `vector_record_codec` as an explicit component
  rather than hiding coordinate encoding inside the derivation call
- native calibration uses the C++ coordinate solver path as a derived-map component with
  reconstruction error plus coordinate-target error
- ordinary vector nearest-neighbor search is kept as a baseline for
  out-of-sample handling: it can return a nearest source anchor but does not
  provide a reusable parametric transform or decoder inverse

Expected workflow output:

```text
pipeline = parametric_diffusion_coordinate_pipeline
pipeline components = 11
pipeline codec = vector_record_codec
pipeline distance provider = distance_table_pairwise_distances
pipeline affinity kernel = exponential_affinity_kernel
pipeline diffusion operator = lazy_row_normalized_diffusion_operator
parametric diffusion coordinate target error: 0.248425 -> 0.160057
parametric diffusion coordinate neighbor preservation = 0.8
parametric diffusion coordinate reconstruction MSE: 2.04967
vector baseline out-of-sample support = nearest_anchor_only
vector baseline inverse support = no_decoder
out-of-sample support = parametric_space_transform
inverse support = decoder_inverse_transform
out-of-sample anchor recall = 1
```

Interpretation:

- the promoted workflow path is an explicit native pipeline, not an opaque
  one-shot algorithm call
- the derived mapping artifact carries component provenance through the pipeline plan
- codec selection is now part of that provenance; the companion process-curve
  parametric diffusion coordinate map uses `record_coordinate_codec` for non-vector records without
  Python-side algorithm work
- the larger process-curve diffusion-coordinate gallery shows the same native pipeline
  preserving 6/6 held-out alignment-metric families while the padded-vector
  nearest-neighbor baseline misses 6/6
- coordinate-target error decreases during native calibration
- local-neighbor preservation and out-of-sample anchor stability are reported
  from native mapping diagnostics
- the vector comparison can identify an existing anchor, but it is not a derived
  reusable mapping artifact and has no decoder inverse
