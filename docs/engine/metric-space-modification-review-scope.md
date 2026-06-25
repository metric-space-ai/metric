# Metric-Space Modification Review Scope

Use this file as the review cut for the finite metric-space modification work.
The repository worktree currently contains many unrelated edits; do not stage
them together with this change unless the PR intentionally expands scope.

## Include In This Review

Roadmap and public docs:

- `docs/engine/metric-space-modification-plan.md`
- `docs/engine/metric-space-research-track.md`
- `docs/engine/metric-space-modification-review-scope.md`
- `docs/engine/metric-space-modification-completion-audit.md`
- `docs/examples/customer-focus-groups-metric-space.md`
- `docs/examples/structured-data.md`
- `docs/python/intents.md`
- `docs/python/results.md`
- `docs/api/python.md`
- `docs/testing-and-ci.md`
- `.github/workflows/core-cpp.yml`

Python API and examples:

- `python/pkg/metric/__init__.py`
- `python/pkg/metric/intent.py`
- `python/pkg/metric/experimental.py`
- `python/pkg/metric/strategies.py`
- `python/pkg/metric/operators.py`
- `python/pkg/metric/spaces.py`
- `python/pkg/metric/mapping_pipeline.py`
- `python/pkg/metric/mappings.py`
- `python/pkg/metric/distance/__init__.py`
- `python/examples/engine/metric_space_modification_objectives.py`
- `python/examples/engine/customer_focus_groups_metric_space.py`

Tests and C++ smoke source:

- `metric/core/result.hpp`
- `metric/modify.hpp`
- `metric/modify/reduce/compress.hpp`
- `metric/modify/resample/thin.hpp`
- `python/tests/core/test_revival_api.py`
- `python/tests/core/test_binding_adapter_boundary.py`
- `python/tests/core/test_mapping_pipeline_adapter.py`
- `tests/core_smoke/modify_level2_components_smoke.cpp`
- `tests/core_smoke/modify_thin_smoke.cpp`
- `tests/core_smoke/CMakeLists.txt`

## Exclude From This Review

All unrelated deletions of older examples/mapping files, visual assets, broad
engine refactors, numeric smoke changes, generated `output/` content, and other
workspace edits that are not needed for the finite metric-space modification
roadmap.

## Verification Evidence

Safe checks currently passing:

```bash
PYTHONPATH=python/pkg /Users/michaelwelsch/.local/bin/uvx --python /Users/michaelwelsch/.local/bin/python3.12 --with numpy pytest python/tests/core/test_revival_api.py python/tests/core/test_binding_adapter_boundary.py python/tests/core/test_mapping_pipeline_adapter.py
PYTHONPATH=python/pkg /Users/michaelwelsch/.local/bin/python3.12 -m py_compile python/pkg/metric/__init__.py python/pkg/metric/intent.py python/pkg/metric/strategies.py python/pkg/metric/operators.py python/pkg/metric/spaces.py python/pkg/metric/mapping_pipeline.py python/pkg/metric/mappings.py python/pkg/metric/experimental.py
PYTHONPATH=python/pkg /Users/michaelwelsch/.local/bin/python3.12 python/examples/engine/customer_focus_groups_metric_space.py
PYTHONPATH=python/pkg /Users/michaelwelsch/.local/bin/python3.12 python/examples/engine/metric_space_modification_objectives.py
cmake --build build/core --target modify_thin_smoke
cmake --build build/core --target modify_level2_components_smoke
cmake --build build/core --target metric_pipeline_contracts_smoke
ctest --test-dir build/core -N -L metric_modify_components
ctest --test-dir build/core -L metric_modify_components --output-on-failure
```

Verification status:

- No open verification gap for this review cut. Stale macOS `U`/`UE` processes from earlier aborted
  `assert()`-based smoke runs may still be visible on the host, but the current
  modify smoke harness returns ordinary non-zero test status on failed checks and
  the `metric_modify_components` CTest label now passes locally.
