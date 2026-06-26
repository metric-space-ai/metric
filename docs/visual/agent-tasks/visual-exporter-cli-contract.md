# Visual Exporter CLI Contract

## Owner

This task owns only:

- `examples/engine/process_curve_external_visual_export.cpp`
- `examples/engine/solver_trace_visual_export.cpp`
- optional report update under `docs/visual/reports/visual-exporter-cli-contract.md`

Do not edit other exporters, CMake, public pages, runtime code, generated
assets, or unrelated dirty files.

## Purpose

Native `metric.visual.v1` exporters must behave like reusable evidence
generators, not status-printing demo programs. The standard contract is:

- no arguments: emit a complete `metric.visual.v1` JSON document to stdout
- `--export-dir <dir>` or `--export-dir=<dir>`: write `<dir>/metric.visual.json`
- `--export-json <path>` where already supported: write the exact path
- status/progress text goes to stderr only, or is omitted
- stdout must be parseable JSON whenever no file-output argument is used

This task fixes the two known offenders from the quality audit:

- `process_curve_external_visual_export.cpp`
- `solver_trace_visual_export.cpp`

## Non-Negotiables

- C++ computes the evidence.
- JavaScript must not compute or patch process-curve or solver-trace evidence.
- Do not mark preview evidence as accepted hero evidence.
- Do not touch the protected GRAE10 visual reference.
- Do not use synthetic data to bypass the process-curve scale blocker.
- Do not revert other agents' work.

## Required Behavior

For `process_curve_external_visual_export.cpp`:

1. Preserve current `--export-dir` behavior for CTest output.
2. Preserve current `--export-json <path>` behavior if present.
3. Change the no-argument path so it prints the generated document JSON to
   stdout instead of writing to `docs/examples/assets/...` and printing status.
4. Ensure stdout contains only JSON in no-argument mode.
5. Move any optional summary/status lines to stderr only for file-output modes,
   or remove them.

For `solver_trace_visual_export.cpp`:

1. Preserve current `--export-dir` behavior for CTest output.
2. Change the no-argument path so it prints the generated document JSON to
   stdout instead of writing to `docs/examples/assets/...` and printing status.
3. Ensure stdout contains only JSON in no-argument mode.
4. Move any optional summary/status lines to stderr only for file-output modes,
   or remove them.

## Validation

Run all commands that are possible in the current workspace:

```bash
cmake --build build/core --target \
  engine_process_curve_external_visual_export \
  engine_solver_trace_visual_export -- -j4

build/core/examples/engine/engine_process_curve_external_visual_export \
  > /tmp/process_curve_external.metric.visual.json
node visual/tools/check-visual-document.mjs \
  /tmp/process_curve_external.metric.visual.json

build/core/examples/engine/engine_solver_trace_visual_export \
  > /tmp/solver_trace.metric.visual.json
node visual/tools/check-visual-document.mjs \
  /tmp/solver_trace.metric.visual.json

tmp_dir="$(mktemp -d)"
build/core/examples/engine/engine_process_curve_external_visual_export \
  --export-dir "$tmp_dir/process"
node visual/tools/check-visual-document.mjs \
  "$tmp_dir/process/metric.visual.json"

build/core/examples/engine/engine_solver_trace_visual_export \
  --export-dir "$tmp_dir/solver"
node visual/tools/check-visual-document.mjs \
  "$tmp_dir/solver/metric.visual.json"

ctest --test-dir build/core -R 'example_engine_(process_curve_external|solver_trace)_visual_(export|validate)' --output-on-failure
```

If `build/core` is unavailable, run `cmake -S . -B build/core` first.

## Done

The task is done when both exporters satisfy the stdout/file-output contract,
their generated JSON validates, and the report names the exact commands run.
