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
| `visual-command-api-gallery-workstream.md` | public command API and gallery wiring | ready to assign |
| `relation-matrix-graph-grammar-workstream.md` | relation matrix, graph and pair-selection grammar | ready to assign |
| `record-glyph-field-grammar-workstream.md` | typed record glyphs and property fields | ready to assign |
| `mapping-dynamics-motion-grammar-workstream.md` | mapping, dynamics, timeline and solver-trace grammar | ready to assign |
| `visual-regression-performance-workstream.md` | browser regression and performance gates | ready to assign |
| `cpp-visual-export-core-workstream.md` | reusable C++ `metric.visual.v1` writer core and single-exporter refactor rules | integrated |
| `gpu-picking-preview-workstream.md` | GPU/CPU picking, record preview and pair preview contracts | ready to assign |
| `preview-panel-presentation-workstream.md` | bounded record/pair preview presentation for exported payloads | integrated |
| `relation-matrix-readability-workstream.md` | matrix readability, row/column/cell focus and large-matrix diagnostics | integrated |
| `visual-acceptance-gates-workstream.md` | regression, performance and screenshot gates that cannot promote smoke tests to heroes | integrated |

## Next Slots

The current native evidence exporter wave is integrated. The next parallel wave
starts from reusable engine capabilities, visual grammars and gallery assets,
not from synthetic data or one-off pages:

- harden the public command API so examples call semantic commands
- consolidate native exporter writer code through `visual/cpp/mtrc_visual.hpp`
- make relation, glyph/field and mapping/dynamics grammars first-class engine
  capabilities
- make GPU/CPU picking, selected-record preview and selected-pair preview
  reusable engine capabilities
- run screenshot/performance review against the GRAE10 visual reference and
  each grammar brief
- keep public gallery promotion behind `check-public-gallery-evidence.mjs`

## Shared Validation Commands

```bash
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-views.mjs
node visual/tools/check-cpp-export.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-visual-performance-large-scenes.mjs
node visual/tools/check-visual-document.mjs <exported metric.visual.json>
ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure
ctest --test-dir build/core -L 'metric_application_evidence|metric_phate_pipeline|metric_mnist|metric_visual_integrity|metric_benchmark_report' --output-on-failure
```
