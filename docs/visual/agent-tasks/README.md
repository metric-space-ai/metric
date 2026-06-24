# METRIC Visual Agent Tasks

These task files are execution contracts for parallel agents. They are stricter
than the high-level visual plan because each file defines one owner, one write
set, and one validation path.

## Coordination Rules

- Work on branch `codex/visual-engine-evidence-exporters`.
- Do not revert changes made by other agents.
- Keep write scopes disjoint.
- C++ computes METRIC results. JavaScript validates, indexes, renders and
  inspects exported evidence.
- A public hero requires native evidence, visual grammar, screenshot review and
  `visual/tools/check-public-gallery-evidence.mjs`.
- Existing `visual/examples/*/evidence.json` files with
  `provenance.synthetic: true` are renderer fixtures only.

## Current Parallel Slots

| Task file | Owner scope | Status |
| --- | --- | --- |
| `mixed-records-exporter.md` | native mixed-record visual exporter | integrated |
| `cross-space-exporter.md` | native paired-space visual exporter | integrated |
| `dynamics-exporter.md` | native dynamics visual exporter | integrated |
| `relation-matrix-exporter.md` | native relation-matrix visual exporter | integrated |
| `visual-api-hardening.md` | public JS API and checks | integrated |
| `condition-monitoring-exporter.md` | native condition-monitoring visual exporter | integrated |
| `mapping-dimensionality-exporter.md` | native mapping/dimensionality visual exporter | integrated |
| `exporter-quality-audit.md` | read-only/patch-light audit of native exporters | ready to assign |

## Next Slots

The current native evidence exporter wave is integrated. The next parallel wave
should start from visual grammars and gallery assets, not from synthetic data:

- build `metric.visual.v1` assets from the integrated exporters
- design one distinct visual grammar per hero
- run screenshot review against the GRAE10 visual reference
- keep public gallery promotion behind `check-public-gallery-evidence.mjs`

## Shared Validation Commands

```bash
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-views.mjs
node visual/tools/check-cpp-export.mjs
node visual/tools/check-visual-document.mjs <exported metric.visual.json>
ctest --test-dir build/core -L 'metric_application_evidence|metric_phate_pipeline|metric_mnist|metric_visual_integrity|metric_benchmark_report' --output-on-failure
```
