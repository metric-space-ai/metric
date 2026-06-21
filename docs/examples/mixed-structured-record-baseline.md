# Mixed Structured Record Baseline

This report records the first CI-safe mixed-record hero fixture for METRIC
production readiness.

Command:

```bash
build/core/examples/engine/engine_mixed_structured_records
```

Fixture:

- source records combine numeric temperature summaries, categorical status,
  short text, histogram-like spectrum bins, and short process curves
- METRIC path uses one native C++ composite domain metric with per-field
  contributions
- baseline path projects each record to a padded numeric vector containing only
  the directly numeric fields and runs ordinary Euclidean nearest-neighbor
  search
- benchmark queries are constructed so the numeric shadow is close as a vector
  but semantically wrong as a mixed industrial record

Expected benchmark output:

```text
mixed benchmark records = 8
mixed benchmark queries = 4
mixed benchmark metric correct = 4/4
mixed benchmark vector mismatches = 4/4
mixed benchmark average metric margin = 0.358734
mixed benchmark query evaluations = 32
```

Interpretation:

- the composed metric selects the expected mixed-record family for every query
- the numeric vector baseline selects a shadow record for every query because
  it ignores categorical and text semantics
- the metric margin is measured in the mixed-record metric: it is the distance
  from the query to the vector winner minus the distance to the metric winner
- the fixture remains deterministic and runs as part of the existing C++
  `example_engine_mixed_structured_records` smoke test
