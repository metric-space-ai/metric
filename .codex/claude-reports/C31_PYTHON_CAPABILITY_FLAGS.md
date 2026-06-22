# C31: Python Capability Flags

## Goal

Add a small, factual `metric.available()` capability report so users and docs
can ask what the installed wheel actually supports. The report inspects native
bindings and public adapter state; it never guesses, promises roadmap behavior,
or exposes a raw `ModuleNotFoundError`.

## Changed files

- `python/pkg/metric/runtime.py` — added `capabilities()` and its
  `available()` alias plus three internal probe helpers
  (`_native_core_available`, `_distance_metrics_available`,
  `_correlation_package_available`); added `available`/`capabilities` to
  `__all__`.
- `python/pkg/metric/operators.py` — added adapter probe helpers
  `native_metric_module_or_none()` (imports `metric._impl.metric` defensively,
  returning `None` on missing extension) and `native_binding_available(name)`
  (`hasattr` check on the native module); added both to `__all__`.
- `python/pkg/metric/__init__.py` — re-exported `available` and `capabilities`
  from `.runtime` so `metric.available()` is the top-level entry point. They
  enter `metric.__all__` automatically (it is derived from `globals()`).
- `python/tests/core/test_binding_adapter_boundary.py` — added two additive,
  task-prefixed pytest tests (`test_c31_*`).
- `python/tests/core/test_revival_api.py` — added one additive, task-prefixed
  unittest method (`test_c31_available_capability_flags_are_dynamic_bools`).
- `docs/api/python.md` — added a "Capability Introspection" section (factual,
  no marketing).
- `README.md` — added a factual `metric.available()` paragraph + snippet in the
  Python Binding section.
- `.codex/claude-reports/C31_PYTHON_CAPABILITY_FLAGS.md` — this report.

No C++ changes. No behavior promotion. No new dependencies.

## Capability keys and meaning

`metric.available()` returns a plain `dict[str, bool]` with these stable keys:

| key | meaning | True iff |
| --- | --- | --- |
| `native_core` | native C++ extension present | `metric._impl.metric` imports |
| `distance_metrics` | at least one distance metric constructor | `len(metric.metrics.available()) > 0` (Edit guaranteed) |
| `neighbors` | exact-scan neighbor search | native `exact_scan_neighbors` present |
| `pairwise` | explicit all-pairs distance matrix | native `pairwise_distance_matrix` present |
| `representatives` | farthest-first representative selection | native `representative_indices` present |
| `reduce_compress` | representative reduction/compression | native `representative_indices` AND `assign_to_representatives` present |
| `structure` | exact finite-space structure description | native `describe_structure` present |
| `groups` | k-medoids / DBSCAN grouping | native `kmedoids` AND `dbscan` present |
| `outliers` | nearest-neighbor / DBSCAN outliers | native `nearest_neighbor_outliers` AND `dbscan_outliers` present |
| `denoise` | DBSCAN noise filtering into a derived space | native `dbscan` present |
| `embed` | metric-space embedding | native `embed` present (never promoted in this batch) |
| `compare_correlate` | cross-space distance-profile comparison | native `compare_spaces` present |
| `correlation_package` | optional native correlation package | `import metric.correlation` succeeds without raising |

A `True` value means the path is reachable in this build; a `False` value means
it is not promoted (or its optional dependency is absent) here.

## How each flag is probed (proving it is dynamic, not hardcoded)

Every flag is computed at call time:

- `native_core` — attempts `from metric._impl import metric`, wrapped in
  `except (ImportError, ModuleNotFoundError)` returning `False`.
- `distance_metrics` — `len(metric.metrics.available()) > 0`, guarded so any
  failure yields `False`.
- `neighbors`, `pairwise`, `representatives`, `reduce_compress`, `structure`,
  `groups`, `outliers`, `denoise` — resolved via
  `operators.native_binding_available("<fn>")`, which defensively imports the
  native module and returns `hasattr(native, "<fn>")`. The exact binding names
  were read from `operators.py` (`pairwise_distance_matrix`,
  `exact_scan_neighbors`, `representative_indices` + `assign_to_representatives`,
  `describe_structure`, `kmedoids`/`dbscan`,
  `nearest_neighbor_outliers`/`dbscan_outliers`, `dbscan` for denoise).
- `embed` — `native_binding_available("embed")`. `embed_space()` only calls
  `_require_native_binding`, so there is no promoted binding; the probe returns
  `False`. It will flip automatically if a native `embed` binding is ever added.
- `compare_correlate` — `native_binding_available("compare_spaces")`. False on
  this branch (compare unpromoted at origin/main). A parallel task adds the
  native `compare_spaces` binding; this probe then returns `True` with no code
  change, because it reads the live native module.
- `correlation_package` — attempts `import metric.correlation` inside
  `except Exception` (covers `ImportError`, `ModuleNotFoundError`, and the
  `OptionalDependencyError` a parallel task introduces) returning `False`.
  Currently the import raises `ModuleNotFoundError: No module named
  'metric._impl.entropy'`, so the flag is `False`; it flips to `True`
  automatically once correlation imports cleanly.

No probe imports an optional/missing module in a way that surfaces a raw
`ModuleNotFoundError`: every probe catches the import failure and reports
`False`.

Snapshot of `metric.available()` on this branch (origin/main, native `.so`
present):

```
native_core         True
distance_metrics    True
neighbors           True
pairwise            True
representatives     True
reduce_compress     True
structure           True
groups              True
outliers            True
denoise             True
embed               False
compare_correlate   False
correlation_package False
```

## Tests

The tests assert the dynamic contract, never a frozen snapshot for the flags
that integration will flip:

- dict shape + `set(result) >= {13 required keys}` + all values are `bool`;
- `native_core is True`, `distance_metrics is True`;
- always-promoted paths True (`neighbors`, `pairwise`, `representatives`,
  `reduce_compress`, `structure`, `groups`, `outliers`, `denoise`);
- `embed is False`;
- internal consistency: `compare_correlate == hasattr(<native>, "compare_spaces")`
  and `correlation_package == <import metric.correlation succeeds>`, so the test
  passes both standalone and post-integration.

### Invocation note

`PYTHONPATH=python/pkg` is required on the pytest command. The repo-root C++
`metric/` directory otherwise shadows the Python package and import fails. The
literal task command in the prompt omits it; the version run here prepends it.

### Exact test output

```
$ cd /Users/michaelwelsch/Documents/metric-c31 && \
  PYTHONDONTWRITEBYTECODE=1 PYTHONPATH=python/pkg uv run --python 3.12 --with pytest --with numpy python -m pytest \
    python/tests/core/test_binding_adapter_boundary.py \
    python/tests/core/test_revival_api.py -q
.................................                           [100%]
33 passed, 13 subtests passed in 0.56s
```

```
$ cd /Users/michaelwelsch/Documents/metric-c31 && \
  PYTHONDONTWRITEBYTECODE=1 uv run --python 3.12 python -m compileall -q python/pkg
COMPILEALL_OK   (clean, no output from compileall)
```

```
$ cd /Users/michaelwelsch/Documents/metric-c31 && git diff --check
(clean, no output)
```

## Known limitations

- `denoise` is probed via the native `dbscan` binding because `denoise_space()`
  builds the derived space from a `dbscan(...)` result; there is no dedicated
  native `denoise` binding to probe.
- `distance_metrics` checks `metrics.available()` length, not each individual
  constructor. It is effectively always `True` because Edit is guaranteed.
- The flags are coarse promotion flags: they indicate that a binding/path
  exists, not that a specific strategy variant or parameterization is
  supported.
- `available()` is a thin wrapper around `capabilities()`; they return equal
  dicts but are distinct function objects.
