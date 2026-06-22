# C28: Python Correlation Import Boundary

## Goal

Make `import metric.correlation` / `from metric import correlation` fail cleanly
with `metric.OptionalDependencyError` (naming the missing native binding) when the
native correlation extensions are unavailable, instead of leaking a raw
`ModuleNotFoundError`. Adapter-boundary only: no correlation algorithm is promoted.

## Environment fact

The native correlation extensions `metric._impl.entropy` and `metric._impl.mgc`
do NOT exist in this wheel. Only `metric._impl.{metric,distance,space,transform}.so`
are present (and gitignored). `import metric` does not import `correlation`, so it
already succeeds and continues to succeed.

## Changed files

1. `python/pkg/metric/correlation/entropy.py` — wrapped `import metric._impl.entropy`
   in try/except. A `ModuleNotFoundError` for the native binding (`name` is `None`
   or starts with `metric._impl`) is converted to `OptionalDependencyError(...)`
   using `from None` so no raw `ModuleNotFoundError` is chained as the visible cause.
   An unrelated missing dependency (name outside `metric._impl`) is re-raised
   unchanged.
2. `python/pkg/metric/correlation/mgc.py` — same pattern for `metric._impl.mgc`.
3. `python/pkg/metric/correlation/__init__.py` — added an adapter-boundary docstring
   and an `__all__`; still re-exports `Entropy`, `MGC`, `MGC_direct`, so importing the
   package naturally triggers the `OptionalDependencyError` at import time.
4. `python/pkg/metric/exceptions.py` — `OptionalDependencyError` now derives from
   `(MetricError, ImportError)` instead of `MetricError` alone (see "Allowed-file
   limit / cross-coupling" below). Public hierarchy and `__all__` are otherwise
   intact; it is still a `MetricError`.
5. `python/tests/core/test_binding_adapter_boundary.py` — added
   `test_import_metric_succeeds_without_native_correlation` and
   `test_correlation_import_raises_optional_dependency_error_when_native_absent`
   (skips when native correlation is importable; asserts message names the binding,
   surfaced error is not a `ModuleNotFoundError`, and `__cause__ is None`; loads
   `mgc.py` standalone to confirm its own binding name in the message).
6. `python/tests/core/test_revival_api.py` — added one additive method
   `test_correlation_import_is_clean_adapter_boundary` to `RevivalApiTest`
   (skips when native correlation is present). No existing test was rewritten.
7. `docs/api/python.md` — corrected the top "Availability" note (was: correlation
   "otherwise raises `ModuleNotFoundError`") and added one focused paragraph in the
   Error Model section stating the adapter boundary and `OptionalDependencyError`
   contract.
8. `.codex/claude-reports/C28_CORRELATION_IMPORT_BOUNDARY.md` — this report.

## Behavior before / after

Before (native correlation absent):
```
import metric                 -> OK
import metric.correlation     -> builtins.ModuleNotFoundError: No module named 'metric._impl.entropy'
```

After (native correlation absent):
```
import metric                 -> OK
import metric.correlation     -> metric.OptionalDependencyError:
    native correlation binding metric._impl.entropy is unavailable; correlation is
    an adapter boundary until the native binding is promoted
from metric import correlation -> same OptionalDependencyError
```
`__cause__` is `None` (used `from None`); the surfaced error is not a
`ModuleNotFoundError`. When importing the package, `__init__` imports `entropy`
before `mgc`, so the package-level message names `metric._impl.entropy`; loading
`mgc.py` on its own names `metric._impl.mgc`.

If native correlation IS present in some environment, the imports succeed and behave
as before (factories returned); the new tests skip their raise-assertions via an
`importlib.util.find_spec` availability probe.

## Allowed-file limit / cross-coupling (important)

`python/pkg/metric/compat.py` (NOT in the allowed-file list) defines
`available_modules()`, which calls `legacy_module("correlation")` and only catches
`(ImportError, ModuleNotFoundError)`. `test_revival_api.py::test_metric_concepts_are_importable`
(a pre-existing test) calls `compat.available_modules()`.

With the boundary raising a plain `MetricError`-only `OptionalDependencyError`, that
exception escaped `compat`'s `except (ImportError, ModuleNotFoundError)` and broke
the pre-existing test (reproduced: baseline correlation files -> test passes; with
the boundary change alone -> test fails inside `compat.available_modules()`).

Rather than edit the disallowed `compat.py`, the fix was made in the allowed
`exceptions.py`: `OptionalDependencyError` now also subclasses `ImportError`. This
is semantically correct — it is raised only for unavailable optional imports/native
bindings (correlation here, missing `numpy` in `operators.py`/`spaces.py`) — and
lets import-aware discovery treat an unavailable optional binding like any other
unimportable legacy module. `MetricError` remains a base, so `issubclass(
OptionalDependencyError, MetricError)` still holds and `__all__` is unchanged. It is
NOT a `ModuleNotFoundError`, so the "no raw ModuleNotFoundError surfaced" assertion
still holds. No allowed-file limit blocked a better fix; this fix stayed inside the
allowed set and is arguably cleaner than special-casing `compat`.

## Exact test output

Command (PYTHONPATH=python/pkg required — see note):
```
PYTHONDONTWRITEBYTECODE=1 PYTHONPATH=python/pkg uv run --python 3.12 --with pytest --with numpy python -m pytest \
  python/tests/core/test_binding_adapter_boundary.py \
  python/tests/core/test_revival_api.py -q
```
Output:
```
.................................                           [100%]
33 passed, 13 subtests passed in 0.57s
```

compileall:
```
PYTHONDONTWRITEBYTECODE=1 uv run --python 3.12 python -m compileall -q python/pkg
```
Output: (no output; exit 0)

git diff --check:
```
(no output; exit 0)
```

Full `python/tests/core` directory: `36 passed, 13 subtests passed`.

## PYTHONPATH note

The literal task `pytest` command without `PYTHONPATH=python/pkg` fails: at the repo
root `import metric` resolves to the C++ `metric/` namespace directory and raises
`ModuleNotFoundError`. `PYTHONPATH=python/pkg` must be prepended so `metric` resolves
to the Python package. `compileall` does not need it.

## Residual risks

- The legacy suites `python/tests/correlation/test_entropy.py` and `test_mgc.py`
  import `Entropy`/`MGC` at module top-level, so they still error during pytest
  collection when native correlation is absent. They errored identically on baseline
  (raw `ModuleNotFoundError`, now `OptionalDependencyError`); both files are outside
  the allowed-file list and outside the active `core` test gate. Not changed.
- `OptionalDependencyError` is now also an `ImportError` globally. Reviewed: no test
  asserts it is NOT an `ImportError`, and all current raisers (correlation boundary,
  missing `numpy`) are genuine import/optional-dependency failures, so the widened
  base is accurate. `from metric import KMedoids` and similar `ImportError`
  expectations are produced elsewhere and are unaffected.
