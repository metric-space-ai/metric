# Histogram Transport Baseline

This report records the first CI-safe histogram transport baseline fixture for
METRIC production readiness.

Command:

```bash
build/core/examples/engine/engine_histogram_transport_space
```

Fixture:

- source records are one-dimensional histograms with shifted unit mass and
  split-mass vector decoys
- METRIC path uses a native C++ cumulative transport distance over histogram
  bins
- baseline path treats the same histograms as raw coordinate vectors and runs
  ordinary Euclidean nearest-neighbor search
- benchmark queries are shifted impulses where transport should select the
  adjacent shifted mass, while raw vector distance prefers a split-mass decoy
  that shares half the query coordinate

Expected benchmark output:

```text
histogram benchmark records = 8
histogram benchmark queries = 4
histogram benchmark metric correct = 4/4
histogram benchmark vector mismatches = 4/4
histogram benchmark average metric margin = 3.25
histogram benchmark dense evaluations = 64
```

Interpretation:

- the transport metric selects the expected shifted-mass family for every query
- the raw vector baseline selects split-mass decoys for every query because it
  rewards coordinate overlap instead of movement cost
- the metric margin is measured in the histogram transport metric: it is the
  distance from the query to the vector winner minus the distance to the metric
  winner
- the fixture remains deterministic and runs as part of the existing C++
  `example_engine_histogram_transport_space` smoke test
