# Direct Runtime Harness Quarantine

Snapshot: `codex/visual-engine-evidence-exporters` on 2026-06-25.

This task kept the public METRIC Visual API centered on `createMetricVisual()`
and semantic `show*()` commands. The canonical runtime remains intact; this work
only hides or documents direct-runtime and standalone renderer paths.

## Remaining Direct Runtime Or Probe Paths

| Path | Status | Why it remains |
| --- | --- | --- |
| `visual/examples/grae10-metric-engine/index.html` | protected accepted reference | GRAE10 is the current accepted golden visual and remains under its existing hash/golden gates. |
| `visual/examples/grae10-metric-engine/index.original.html` | protected reference copy | Retained as the original GRAE10 comparison artifact; not treated as a public gallery entry by the new checker. |
| `visual/examples/grae10-engine-module/index.html` | protected module diagnostic | GRAE10 module smoke page; outside this task's edit scope. |
| `visual/examples/native-engine-probe/index.html` | internal diagnostic harness | Retained as a dev-only process-curve miniature runtime probe. It now has HTML metadata, dataset flags, visible copy, and `window.metricVisualProbeMetadata` marking it as non-public. |
| `visual/examples/miniature-look-gallery/index.html` | internal miniature style atlas | Still uses direct `MetricVisualRuntime` setup, but is not linked from the public gallery and is explicitly allow-listed by the quarantine checker. |
| `visual/examples/process-curve-condition-monitoring/index.html` | legacy process-curve dev page | Still uses direct runtime setup, but process-curve example pages are outside this task's owner scope and it is not an accepted/public gallery hero. |

`visual/examples/miniature-hero-frame/index.html` is documented in the checker as
an internal miniature frame harness, but the current file no longer matches the
direct-runtime patterns.

## Exports Removed Or Marked Internal

- Removed the root `miniatureField` namespace export from `visual/src/index.js`.
- Marked `visual/src/miniature-field/index.js` as internal diagnostic exports.
- Marked `visual/src/miniature-field/instanced-box-field.js` as an internal
  standalone renderer retained for diagnostics only.
- `InstancedBoxField` and `createInstancedBoxField` remain available only by
  direct internal module import; `rg` found no usage outside the module after
  removing the root namespace export.

## Checker Added

Added `visual/tools/check-direct-runtime-harness-quarantine.mjs`.

The checker:

- lists allowed direct-runtime harnesses and why they are allowed;
- enforces the internal marker on `native-engine-probe`;
- fails if a public gallery page uses a direct runtime harness unless it is the
  protected GRAE10 reference;
- fails if an accepted hero uses direct runtime or standalone renderer modules
  unless explicitly allowed;
- fails if `visual/src/index.js` re-exports `miniatureField`.

## Validation

| Command | Result |
| --- | --- |
| `node --check visual/src/index.js` | pass |
| `node visual/tools/check-single-render-pipeline.mjs` | pass |
| `node visual/tools/check-public-gallery-evidence.mjs` | pass, `ok: true`, no issues |
| `node visual/tools/check-runtime-picking-preview.mjs` | pass |
| `node visual/tools/check-views.mjs` | pass, `ok: true`, `total: 80`, `failed: 0` |
| `node visual/tools/check-grae10-golden.mjs` | pass, GRAE10 hash `464f6a90c36c1e9c6b4ec90068500dc226740d65b251918aca567f99d64d3d5e` |
| `rg -n "miniatureField\|InstancedBoxField\|createInstancedBoxField" visual/src/index.js visual/examples docs/site` | no matches; `rg` exited 1 because the expected result is empty |
| `rg -n "new MetricVisualRuntime\|createLayerFromDescriptor" visual/examples docs/site` | pass with only non-public dev/probe pages: `process-curve-condition-monitoring`, `miniature-look-gallery`, and `native-engine-probe` |
| `node visual/tools/check-direct-runtime-harness-quarantine.mjs` | pass, `ok: true`, no issues |

Additional syntax checks also passed for:

- `node --check visual/src/miniature-field/index.js`
- `node --check visual/src/miniature-field/instanced-box-field.js`
- `node --check visual/tools/check-direct-runtime-harness-quarantine.mjs`

## Next Cleanup

- Convert or further quarantine `visual/examples/miniature-look-gallery/index.html`
  when that page is in scope.
- Convert or remove the direct runtime usage in
  `visual/examples/process-curve-condition-monitoring/index.html` when
  process-curve example pages are in scope.
- Keep GRAE10 direct renderer paths under the existing GRAE10 owner/golden
  workflow; do not use them as a general public API pattern.
