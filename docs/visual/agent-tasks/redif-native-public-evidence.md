# Redif Native Public Evidence

## Owner

This task owns only:

- `examples/engine/redif_metric_dynamics_visual_export.cpp`
- `visual/tools/check-redif-visual-export.mjs`
- `docs/examples/assets/redif-metric-dynamics/metric.visual.json`
- optional report update under `docs/visual/reports/redif-native-public-evidence.md`

Do not edit other exporters, public pages, runtime code, CMake, or unrelated
dirty files.

## Purpose

Redif visual evidence currently proves useful native computation, but the
exporter still violates the public native-evidence contract:

- generated JSON is marked `provenance.synthetic: true`
- generated JSON does not set `provenance.native_export: true`
- there is no checked public
  `docs/examples/assets/redif-metric-dynamics/metric.visual.json`
- default stdout behavior is status text rather than JSON

Fix Redif so it is a real native visual evidence artifact. It is still a
preview, not an accepted hero.

## Non-Negotiables

- C++ computes Redif measure paths, entropy, transport path lengths and
  diagnostics.
- JavaScript may validate and inspect only. It must not compute Redif.
- Do not add Python, Node packages, NPM build steps, CDNs, or runtime
  dependencies.
- Do not use fake or synthetic evidence labels.
- Do not promote Redif as an accepted public hero.
- Do not touch the protected GRAE10 reference.
- Do not revert other agents' work.

## Required Behavior

1. Update exporter provenance:
   - set `native_export: true`
   - remove `synthetic: true` or set `synthetic: false`
   - add `synthetic_js: false`
   - keep a source/writer value that names the native exporter
2. Update CLI behavior:
   - no arguments emits complete `metric.visual.v1` JSON to stdout
   - `--export-dir <dir>` writes `<dir>/metric.visual.json`
   - stdout contains only JSON in no-argument mode
3. Publish a real generated public evidence asset:
   - `docs/examples/assets/redif-metric-dynamics/metric.visual.json`
4. Harden `visual/tools/check-redif-visual-export.mjs`:
   - require `provenance.native_export === true`
   - reject `provenance.synthetic === true`
   - require `provenance.synthetic_js === false` when present
   - keep the existing check that `visual/src` contains no Redif operator
     implementation

## Validation

Run all commands that are possible in the current workspace:

```bash
cmake --build build/core --target engine_redif_metric_dynamics_visual_export -- -j4

build/core/examples/engine/engine_redif_metric_dynamics_visual_export \
  > /tmp/redif_metric_dynamics.metric.visual.json
node visual/tools/check-visual-document.mjs \
  /tmp/redif_metric_dynamics.metric.visual.json
node visual/tools/check-redif-visual-export.mjs \
  /tmp/redif_metric_dynamics.metric.visual.json

tmp_dir="$(mktemp -d)"
build/core/examples/engine/engine_redif_metric_dynamics_visual_export \
  --export-dir "$tmp_dir/redif"
node visual/tools/check-visual-document.mjs \
  "$tmp_dir/redif/metric.visual.json"
node visual/tools/check-redif-visual-export.mjs \
  "$tmp_dir/redif/metric.visual.json"

build/core/examples/engine/engine_redif_metric_dynamics_visual_export \
  --export-dir docs/examples/assets/redif-metric-dynamics
node visual/tools/check-visual-document.mjs \
  docs/examples/assets/redif-metric-dynamics/metric.visual.json
node visual/tools/check-redif-visual-export.mjs \
  docs/examples/assets/redif-metric-dynamics/metric.visual.json

ctest --test-dir build/core -R 'redif_visual_export_validate|example_engine_redif_metric_dynamics_visual_export' --output-on-failure
```

If `build/core` is unavailable, run `cmake -S . -B build/core` first.

## Done

The task is done when Redif has native, non-synthetic provenance, stdout/file
export modes both work, the public asset exists and validates, and the checker
fails if Redif evidence is marked synthetic again.
