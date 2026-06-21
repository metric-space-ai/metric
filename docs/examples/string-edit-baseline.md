# String Edit Baseline

This report records the first CI-safe string edit-space baseline fixture for
METRIC production readiness.

Command:

```bash
build/core/examples/engine/engine_strings_edit_space
```

Fixture:

- source records are short symbolic strings with typo variants, suffix variants,
  and anagram decoys
- METRIC path uses the native C++ edit-distance metric over the source strings
- baseline path projects each string to a 26-letter count vector plus length and
  runs ordinary Euclidean nearest-neighbor search
- benchmark queries are constructed so the bag-of-characters vector is closest
  to an anagram decoy while edit distance stays closest to the expected string
  family

Expected benchmark output:

```text
string benchmark records = 12
string benchmark queries = 4
string benchmark metric correct = 4/4
string benchmark vector mismatches = 4/4
string benchmark average metric margin = 2
string benchmark dense evaluations = 144
```

Interpretation:

- the edit metric selects the expected symbolic family for every query
- the vector baseline selects an anagram decoy for every query because it
  discards order
- the metric margin is measured in the string edit metric: it is the distance
  from the query to the vector winner minus the distance to the metric winner
- the fixture remains deterministic and runs as part of the existing C++
  `example_engine_strings_edit_space` smoke test
