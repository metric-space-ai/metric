# C29 — Native Compare/Correlate Python Binding

## Goal

Promote exact aligned-space comparison into the Python adapter by delegating to
a native C++ distance-profile correlation. Close the `StrategyUnavailableError`
path for `compare_spaces`, `correlate_spaces`, `Space.compare`, and
`Space.correlate` for equal-length / `align="position"` finite metric spaces.
Python only marshals records/strategies/IDs/results; all math is in C++.

## Native binding added

File: `python/src/exact_scan.cpp`

- New free function `native_distance_profile_correlation(...)` (in the same TU
  as `native_pairwise_distance_matrix`), plus a small anonymous-namespace helper
  `condensed_distance_profile(records, metric)` that builds the off-diagonal
  (upper-triangle) distance profile using the existing `checked_distance(...)`
  validation (real/finite/non-negative).
- Registered in `export_exact_scan(py::module &m)` as:

  ```cpp
  m.def("distance_profile_correlation", &native_distance_profile_correlation,
        py::arg("left_records"), py::arg("left_metric"),
        py::arg("right_records"), py::arg("right_metric"),
        py::arg("left_representation") = "records",
        py::arg("right_representation") = "records");
  ```

  Exposed name: `metric._impl.metric.distance_profile_correlation`.

The math (means, covariance, both variances, Pearson `cov / sqrt(varL*varR)`,
clamp to `[-1, 1]`) is computed entirely in C++ over the two condensed profiles.
A C++ side count guard also throws `std::invalid_argument` when left/right record
counts differ (defense in depth; the Python adapter raises first — see below).

## Result contract

Native payload (`py::dict`) marshaled by `operators._correlation_result_from_payload`
into the existing `CorrelationResult` dataclass:

- `value: float` — Pearson correlation in `[-1, 1]` (clamped).
- `left_record_count`, `right_record_count`, `pair_count` (off-diagonal pairs).
- `exact = True`.
- `algorithm = strategy = statistic_name = "distance_profile_correlation"`.
- `align = "position"`.
- `matched_ids` — positional `tuple(range(n))` for the promoted path (override
  accepted via `compare_spaces(..., matched_ids=...)`).
- `dropped_left_ids`, `dropped_right_ids` — pass-through (default `()`).
- `left_representation`, `right_representation` — pass-through metadata.
- `p_value` — pass-through (default `None`; no permutation test in this path).
- `local_scores = ()`.
- `diagnostics` — dict: `defined`, `degenerate`, `left_variance`,
  `right_variance`, `left_mean`, `right_mean`, `covariance`.

`correlate_spaces(...)` and `Space.correlate(...)` are aliases of the compare
path (`align="position"`).

## Edge cases / tested failure modes

- **Mismatched record counts** → `metric.exceptions.IncompatibleSpaceError`,
  naming both counts (e.g. "left space has 3 records and right space has 2
  records"). Enforced Python-side in `operators._require_aligned_record_counts`
  *before* calling native (marshaling, not math). C++ also throws as a backstop.
  Tested: `test_compare_mismatched_counts_names_the_mismatch`.
- **Degenerate profile** (Pearson undefined): fewer than two records (no pairs)
  or zero-variance distance profile (all pairwise distances equal). Documented
  deterministic behavior decided in C++ (the math owner): return `value=0.0`
  with `diagnostics["defined"] is False` and `diagnostics["degenerate"] is True`
  — no exception, no division by zero. Tested:
  `test_compare_degenerate_profile_returns_undefined_sentinel`.
- **`embed` stays unpromoted**: `Space.embed(...)` and `embed_space(...)` still
  raise `StrategyUnavailableError`. Tested:
  `test_embed_stays_unpromoted_after_compare_is_promoted` and the
  binding-adapter-boundary loop (which keeps `space.embed(1)`).
- **Strategy token**: `None` and `metric.strategies.DistanceProfileCorrelation()`
  both select the native path; a non-`pearson` method or any other strategy
  raises `StrategyUnavailableError`. Non-`position` alignment (`align="ids"`)
  raises `StrategyUnavailableError`.

## Changed files

- `python/src/exact_scan.cpp` — native binding + helper.
- `python/pkg/metric/operators.py` — promoted `compare_spaces`/`correlate_spaces`
  bodies, added `_resolve_distance_profile_strategy`,
  `_correlation_result_from_payload`, `_require_aligned_record_counts`; imported
  `IncompatibleSpaceError`; updated module docstring. `embed_space` unchanged
  (still raises).
- `python/pkg/metric/spaces.py` — rewired `Space.compare`/`Space.correlate` to
  delegate to the operator functions via a shared `_compare_aligned` helper
  (`align="position"`); `Space.embed` unchanged (still raises).
- `python/tests/core/test_revival_api.py` — flipped the four
  `assertRequiresNative(...)` compare/correlate assertions to positive
  `CorrelationResult` assertions (≈ lines 994-995, 1012-1013, 1273); added four
  focused tests (aligned positive, mismatch, degenerate, embed-still-raises).
  The intent-name callable list (compare/correlate/embed) is unchanged because
  it only asserts callability, which all three still are.
- `python/tests/core/test_binding_adapter_boundary.py` — removed
  `space.compare(space)` from the "still raises" loop (kept `space.embed(1)`),
  added a positive `CorrelationResult` assertion; imported `CorrelationResult`.
- `python/notebooks/04_compare_aligned_spaces.ipynb` — demonstrates the promoted
  positional `compare`/`correlate` path and the `IncompatibleSpaceError`
  mismatch case; no stored outputs; first code cell unchanged; no
  `metric.strategies` import; nbformat 4.
- `docs/api/python.md`, `docs/python/intents.md`, `docs/python/space.md` —
  documented the promoted equal-length `align="position"` path, the
  `CorrelationResult` contract, the mismatch error, and the degenerate sentinel;
  removed stale "unavailable" claims for this path.

### File touched outside the listed allow-set (flagged)

- `python/examples/engine/cross_space_dependency.py` — **not** in the task's
  allowed-files list, but it is executed by the in-scope test
  `test_revival_api.py::test_promoted_metric_space_examples_run`, and it
  asserted the now-obsolete contract (`compare` must raise
  `StrategyUnavailableError`). After promotion that assertion fails, turning an
  in-scope test red. Updated the example to demonstrate the promoted path and
  the mismatch error. This is a Python example file (no public C++ header, no
  dependency change). Flagged here for transparency; if policy requires it
  untouched, the alternative is to skip that example in the test runner, which
  would hide a real regression.

## Exact test output

Wheel build: `Successfully built mtrc-0.3.4-cp312-cp312-macosx_26_0_arm64.whl`
`ls build/wheel-smoke/mtrc-0.3.4-*.whl` →
`build/wheel-smoke/mtrc-0.3.4-cp312-cp312-macosx_26_0_arm64.whl`

pytest (`test_revival_api.py` + `test_binding_adapter_boundary.py`):

```
..................................                          [100%]
34 passed, 13 subtests passed in 0.77s
```

notebook smoke:

```
notebook smoke ok: 00_strings_as_metric_space.ipynb
notebook smoke ok: 01_dataframe_records_custom_metric.ipynb
notebook smoke ok: 02_neighbors_groups_outliers.ipynb
notebook smoke ok: 03_embed_map_denoise.ipynb
notebook smoke ok: 04_compare_aligned_spaces.ipynb
notebook smoke ok: 05_strategy_overrides.ipynb
```

`compileall python/pkg python/examples` → exit 0.
`git diff --check` → exit 0 (no whitespace errors).

## Remaining unpromoted comparison/correlation paths

- `embed` / `embed_space` — still `StrategyUnavailableError` (intentionally not
  promoted).
- Non-aligned comparison (`align="ids"` and any non-`position` alignment) —
  native-only.
- Non-Pearson distance-profile methods and any non-`DistanceProfileCorrelation`
  comparison strategy — native-only.
- MGC / `metric.correlation` (Entropy/MGC) cross-space dependence — unchanged,
  full-build only; not promoted here.
- `p_value` / permutation significance for distance-profile correlation —
  pass-through only; not computed in this path.
