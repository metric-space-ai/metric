# Solver Trace Native Evidence

Status: ready to assign

## Owner Scope

- `visual/src/views/SolverTraceView.js`
- `visual/src/metric-visual.js` only for `showSolverTrace()` wiring
- `visual/tools/check-visual-command-api.mjs`
- new or existing solver-trace validation tools under `visual/tools/`
- `docs/examples/assets/solver-trace/`
- `examples/engine/*solver*visual*` and matching CMake entries, only if a native
  exporter is required
- `docs/visual/metric-visual-progress.md`

Do not edit unrelated visual grammars, public gallery layout, GRAE10 files, or
non-visual C++ framework code.

## Goal

Make `showSolverTrace()` a real engine capability backed by exported METRIC
evidence, not only a synthetic command fixture. The browser may render the
trace, but JavaScript must not invent solver iterations, residuals, objectives
or convergence values.

## Required Change

1. Inspect the current `SolverTraceView` evidence contract.
2. If native solver-trace evidence already exists, wire the command/gate to it.
3. If no native evidence exists, add a small native C++ exporter that writes
   `metric.visual.v1` solver trace evidence into
   `docs/examples/assets/solver-trace/metric.visual.json`.
4. The exported document must contain explicit native provenance:
   `provenance.native_export === true`.
5. The visual grammar must expose an objective/residual timeline and enough
   metadata for hover/preview inspection.
6. `check-visual-command-api.mjs` should report `showSolverTrace()` as native
   evidence once the exporter exists. If implementation proves too large, write
   a precise blocker report instead of adding fake synthetic evidence.

## Non-Negotiables

- No JavaScript computation of solver results.
- No fake convergence curves.
- No hero promotion.
- No page-local renderer.
- Keep GRAE10 untouched.

## Validation

Run at minimum:

```bash
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-views.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-visual-document.mjs docs/examples/assets/solver-trace/metric.visual.json
```

If a C++ exporter is added, also run the corresponding build target and CTest
entry if available.

## Report

Update `docs/visual/metric-visual-progress.md` with the evidence source,
exported trace fields, command behavior and remaining blockers.
