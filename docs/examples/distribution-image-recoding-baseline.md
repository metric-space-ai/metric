# Distribution Image Recoding Baseline

This report records the first CI-safe distribution/image recoding workflow fixture
for METRIC production readiness.

Command:

```bash
build/core/examples/engine/engine_distribution_image_recoding
```

Fixture:

- source records are small 5x5 image-like mass distributions
- METRIC path uses a native C++ transport cost with Manhattan ground distance
  over grid cells
- baseline path treats each patch as a flat 25-dimensional vector and uses
  coordinate-wise Euclidean nearest-neighbor search
- benchmark queries preserve shape under one-cell shifts, where transport cost
  should stay low and coordinate-wise vector distance should over-penalize the
  shift

Expected benchmark output:

```text
distribution benchmark records = 8
distribution benchmark queries = 4
distribution benchmark metric correct = 4/4
distribution benchmark vector mismatches = 4/4
distribution benchmark average metric margin = 0.416667
distribution benchmark graph edges = 2
```

Interpretation:

- the transport metric selects the expected shifted structure for every query
- the flat vector comparison selects colocated but structurally wrong patterns for
  every query
- the metric margin is measured in the transport metric: it is the distance from
  the query to the vector winner minus the distance to the metric winner
- dense materialization is explicit in the executable output, and graph edges
  record the current local neighborhood threshold
