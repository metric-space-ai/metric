# C30 — Promote TWED into the default Python wheel

## Goal
Promote exactly one true signature metric — `TWED` (Time Warp Edit Distance) for
numeric time-series records — into the DEFAULT Python wheel, delegating to native
C++. No Python-side math. No other metric promoted.

## Changed files
- `python/src/distance/CMakeLists.txt` — added `TWED.cpp` to the DEFAULT
  (non-FULL `else()`) `DISTANCE_SOURCE_FILES`:
  `init.cpp Edit.cpp TWED.cpp subs/standards.cpp`. EMD/RandomEMD/kohonen/
  sorensen/SSIM and the `subs` subdir remain FULL-only.
- `python/src/distance/init.cpp` — moved the `export_metric_TWED` forward
  declaration out of `#ifdef METRIC_PYTHON_BUILD_FULL` so it is declared
  unconditionally, and moved the `export_metric_TWED(m)` call next to
  `export_metric_Edit(m)` so it runs in both the default and full builds.
  EMD/RandomEMD/kohonen/sorensen/SSIM stay FULL-only.
- `python/pkg/metric/metrics.py` — added `TWED = _optional_metric("TWED")`,
  added `"TWED": TWED` to the `available()` mapping, and added `"TWED"` to
  `__all__`.
- `python/tests/distance/test_twed.py` — deterministic native fixture (see
  below). Also fixed the pre-existing `test_ok` which used the NumPy-2.0-removed
  `numpy.float_` (replaced with `numpy.asarray(..., dtype=float)`); this file is
  owned solely by C30.
- `python/tests/core/test_revival_api.py` — additive test
  `test_twed_signature_metric_is_promoted_in_default_wheel`: asserts
  `"TWED" in metrics.available()`, `metric.metrics.TWED` is not None and is in
  `metric.metrics.__all__`, constructs it, and checks native identity-is-zero and
  symmetry. No unrelated tests rewritten.
- `python/examples/metric_space/time_series_alignment_space.py` — added a focused
  native `metrics.TWED(penalty=0, elastic=1)` demonstration over the same short
  curves (identity, symmetry, pairwise matrix, nearest search). Deterministic and
  runs under the default wheel.
- `docs/api/python.md` — TWED documented as the first promoted signature metric
  (constructor, native delegation, parameter gates, `available()` membership);
  EMD/RandomEMD/Kohonen/Sorensen/SSIM marked as still unavailable in the default
  wheel.
- `docs/metrics/true-metric-catalog.md` — added a Python-availability fact to the
  existing TWED row's note column without altering its metric-law content.
- `.codex/claude-reports/C30_TWED_DEFAULT_WHEEL.md` — this report.

`python/pkg/metric/distance/__init__.py` was NOT modified: it does
`from metric._impl.distance import *`, so `from metric.distance import TWED`
works automatically once TWED is compiled into the default extension.

## Exact Python API
```python
from metric import metrics
twed = metrics.TWED(penalty=0, elastic=1)   # lambda >= 0, nu > 0
twed([0.0, 1.0, 2.0, 3.0], [0.0, 1.0, 2.0, 4.0])  # -> 1.0

# equivalently
from metric.distance import TWED
TWED()  # penalty=0, elastic=1, is_zero_padded=False
```
Read-only attributes: `penalty`, `elastic`, `is_zero_padded`.
`metrics.available()` includes `"TWED"`.

## Native binding evidence
- The Python class comes from `python/src/distance/TWED.cpp`, which defines
  `void export_metric_TWED(py::module &m)` wrapping `mtrc::TWED<double>` from
  `metric/metric/catalog/structured/TWED.hpp`. The math recurrence lives in
  `metric/metric/catalog/structured/TWED.cpp` (included by the header). No Python
  implements TWED.
- TWED.hpp pulls only `<metric/core/metric_traits.hpp>` and
  `<metric/numeric/Math.h>` plus standard headers; it is header-only with no
  extra link dependencies. The lightweight default wheel built and linked
  cleanly (the already-present `subs/standards.cpp` -> `Standards.hpp` already
  pulls the numeric headers into the default build's reach), so NO scope
  expansion or extra dependency was needed.
- Verified under the installed default wheel:
  ```
  metrics.TWED: <class 'metric._impl.distance.TWED'>
  ```
  i.e. the class is provided by the compiled `metric._impl.distance` extension.

## Test fixture explanation (`python/tests/distance/test_twed.py`)
- `test_ok` — two known distances over numpy curves (7.0 close, 15.0 far),
  pinned against the native binding.
- `test_construct_default` / `test_construct_override` — constructor params and
  read-only attributes.
- `test_identity_is_zero` — `d(x, x) == 0.0` (identity of indiscernibles).
- `test_symmetry_holds` — `d(a, b) == d(b, a)`.
- `test_known_nonzero_distance_is_stable` — a single stable known non-zero
  distance: `d([0,1,2,3], [0,1,2,4]) == 1.0`.
- `test_empty_input_fails_clearly` — empty sequences raise (native
  `std::invalid_argument` -> Python exception).
- `test_invalid_parameters_fail_clearly` — `elastic <= 0` and `penalty < 0`
  rejected by the native parameter gate.

## Exact command output
Build:
```
Successfully built mtrc-0.3.4-cp312-cp312-macosx_26_0_arm64.whl
```
Wheel:
```
build/wheel-smoke/mtrc-0.3.4-cp312-cp312-macosx_26_0_arm64.whl
```
TWED present in default wheel:
```
TWED OK
available: ('Edit', 'Euclidean', 'Euclidean_thresholded', 'Manhattan', 'Minkowski', 'Chebyshev', 'P_norm', 'ThresholdedEuclidean', 'TWED')
metrics.TWED: <class 'metric._impl.distance.TWED'>
```
Pytest:
```
34 passed, 13 subtests passed in 0.77s
```
Example:
```
records = baseline, shifted, flat, spike
distance(baseline, shifted) = 2.0
nearest(query) = baseline distance = 1.0
intrinsic_dimension = 1.0
TWED(baseline, shifted) = 7.0
TWED(baseline, flat)    = 11.0
TWED nearest(query) = baseline distance = 1.0
```
`git diff --check`: clean (no whitespace errors).

## Signature metrics that remain UNPROMOTED in the default wheel
- `EMD`
- `RandomEMD`
- `Kohonen`
- `Sorensen`
- `SSIM`

These are still compiled only into the FULL extension set (`*.cpp` glob under
`METRIC_PYTHON_BUILD_FULL`); `metric.metrics.<name>` is `None` for them in the
default wheel.

## Blockers
None. TWED.hpp compiled in the default build without extra deps or headers, so no
disallowed files were touched.
