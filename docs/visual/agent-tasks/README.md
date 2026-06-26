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
| `process-curve-external-visual-exporter.md` | native external UCR process-curve visual exporter and preview | integrated; preview-only |
| `exporter-quality-audit.md` | read-only/patch-light audit of native exporters | ready to assign |
| `visual-command-api-gallery-workstream.md` | public command API and gallery wiring | integrated |
| `relation-matrix-graph-grammar-workstream.md` | relation matrix, graph and pair-selection grammar | integrated |
| `record-glyph-field-grammar-workstream.md` | typed record glyphs and property fields | integrated |
| `mapping-dynamics-motion-grammar-workstream.md` | mapping, dynamics, timeline and solver-trace grammar | integrated |
| `visual-regression-performance-workstream.md` | browser regression and performance gates | integrated |
| `cpp-visual-export-core-workstream.md` | reusable C++ `metric.visual.v1` writer core and single-exporter refactor rules | integrated |
| `gpu-picking-preview-workstream.md` | GPU/CPU picking, record preview and pair preview contracts | integrated |
| `preview-panel-presentation-workstream.md` | bounded record/pair preview presentation for exported payloads | integrated |
| `relation-matrix-readability-workstream.md` | matrix readability, row/column/cell focus and large-matrix diagnostics | integrated |
| `visual-acceptance-gates-workstream.md` | regression, performance and screenshot gates that cannot promote smoke tests to heroes | integrated |
| `process-curve-scene-view-consolidation.md` | process-curve command/view consolidation into one semantic engine view | integrated |
| `native-hero-scale-evidence-upgrade-plan.md` | concrete native evidence scale upgrade plan per preview | integrated |
| `condition-monitoring-scale-exporter.md` | scale native condition-monitoring evidence to hero-record threshold | integrated; historical report superseded by current scale gate |
| `mixed-records-scale-exporter.md` | scale native mixed-record evidence to typed hero threshold | integrated; historical report superseded by current scale gate |
| `cross-space-scale-exporter.md` | scale native cross-space dependency evidence to paired hero threshold | integrated; historical report superseded by current scale gate |
| `dynamics-scale-exporter.md` | scale native finite-dynamics evidence to hero-record threshold | integrated; historical report superseded by current scale gate |
| `mapping-dimensionality-scale-exporter.md` | scale native mapping/dimensionality evidence to hero-record threshold | integrated; historical report superseded by current scale gate |
| `relation-matrix-hero-readability.md` | first relation-matrix readability hardening wave | integrated; superseded by composition acceptance wave |
| `hero-acceptance-manifest-workstream.md` | explicit manifest-backed hero acceptance gate | integrated |
| `render-path-inventory-and-deletion-map.md` | read-only inventory of remaining noncanonical render paths | integrated |
| `legacy-render-facade-deletion.md` | delete old `metric.evidence.v1` WebGL/2D render facade from public API | integrated |
| `process-curve-descriptor-factory-quarantine.md` | remove process-curve descriptor-factory calls from example/public paths | integrated |
| `direct-runtime-harness-quarantine.md` | quarantine direct-runtime probes and standalone renderer exports | integrated |
| `condition-monitoring-visual-acceptance.md` | visual acceptance candidate for native condition monitoring preview | review-pending candidate integrated |
| `mixed-records-visual-acceptance.md` | visual acceptance candidate for native mixed-record preview | review-pending candidate integrated |
| `cross-space-dependency-visual-acceptance.md` | visual acceptance candidate for native cross-space preview | review-pending candidate integrated |
| `mapping-dimensionality-visual-acceptance.md` | visual acceptance candidate for native mapping preview | review-pending candidate integrated |
| `dynamics-noise-visual-acceptance.md` | visual acceptance candidate for native dynamics preview | review-pending candidate integrated |
| `relation-matrix-visual-acceptance.md` | visual acceptance candidate for native relation-matrix preview | review-pending candidate integrated |
| `process-curve-external-scale-exporter.md` | scale external process-curve preview only with real licensed source windows | integrated; review-pending visual composition |
| `native-provenance-command-contract-hardening.md` | harden explicit native provenance and public command coverage | integrated |
| `evidence-strictness-trajectory-property-residual.md` | prevent record-order, scalar-default and residual fallback evidence claims | integrated |
| `project-site-visual-portfolio-redesign.md` | reshape project page as visual-first native evidence portfolio | integrated |
| `visual-exporter-cli-contract.md` | make Process-Curve and Solver-Trace exporters emit JSON on stdout by default | integrated |
| `redif-native-public-evidence.md` | make Redif visual evidence native, non-synthetic, published and checker-enforced | integrated |
| `relation-matrix-reproducibility-audit.md` | normalize or document relation-matrix export reproducibility | integrated |
| `project-gallery-native-evidence.md` | project-page gallery limited to accepted/native evidence | integrated |
| `descriptor-injection-hardening.md` | remove public raw descriptor injection escape hatches | integrated |
| `mnist-babyplot-quarantine.md` | remove old Babyplots/Babylon MNIST standalone renderer from active examples | integrated |
| `solver-trace-native-evidence.md` | back `showSolverTrace()` with native METRIC evidence or document precise blockers | integrated |
| `project-site-copy-contract.md` | public project-page copy reads as visual portfolio, not an internal review log | integrated |
| `redif-preview-integration.md` | native Redif visual evidence rendered through `showDynamics()` as a review-pending engine preview | integrated |
| `process-curve-real-data-inventory.md` | read-only local inventory for real UCR source windows | integrated; blocker confirmed |
| `condition-monitoring-composition-acceptance.md` | improve condition-monitoring field/trajectory composition through reusable engine views | integrated; still review-pending |
| `mapping-residual-composition-acceptance.md` | make mapping morph and residual evidence readable without page-local rendering | integrated; still review-pending |
| `relation-matrix-composition-acceptance.md` | improve relation-matrix readability, graph balance and pair preview placement | integrated; still review-pending |
| `mixed-records-composition-acceptance.md` | make heterogeneous typed-glyph and cross-type relation structure readable | integrated; still review-pending |
| `cross-space-composition-acceptance.md` | make paired-space dependence bridges and linked selection readable | integrated; still review-pending |
| `dynamics-noise-composition-acceptance.md` | make finite metric dynamics trajectory, field and timeline structure readable | integrated; still review-pending |

## Next Slots

The current native evidence exporter and first visual-acceptance candidate waves
are integrated. The next parallel wave starts from reusable engine capabilities,
visual grammars and gallery assets, not from synthetic data or one-off pages:

- prioritize `relation-matrix-composition-acceptance.md` next: improve
  reusable matrix readability, row/column/cell focus and graph/matrix z-order
  in `RelationMatrixView`, `RelationMatrixLayer` and relation descriptors
- prioritize a shared trajectory/field hierarchy pass across
  `condition-monitoring-composition-acceptance.md` and
  `dynamics-noise-composition-acceptance.md`: path, property field, current
  state and labels need stable visual rank in shared view/layer modules
- prioritize a shared relation-edge legibility pass across
  `mixed-records-composition-acceptance.md`,
  `cross-space-composition-acceptance.md` and
  `mapping-residual-composition-acceptance.md`: edges and residual vectors must
  be lanes, bundles, samples or ranked visual evidence, not hairballs
- keep the integrated visual-acceptance candidates review-pending until human
  screenshot review accepts their composition
- continue the external process-curve scale-up task only with real licensed
  source windows; if the current CSV assets are insufficient, record the exact
  data blocker instead of inflating the preview synthetically
- keep the public command API gate green so examples call semantic commands
- keep the integrated exporter CLI contract green: native visual exporters emit
  `metric.visual.v1` JSON to stdout by default and write files only through an
  explicit file-output argument
- keep Redif visual evidence native, non-synthetic and checker-enforced before
  any Redif preview is promoted
- keep relation-matrix public assets reproducible from the native exporter
  without hidden patches
- continue the process-curve preview from the `showProcessCurves` command,
  not from generic matrix/point-cloud fallback code
- consolidate native exporter writer code through `visual/cpp/mtrc_visual.hpp`
- make relation, glyph/field and mapping/dynamics grammars first-class engine
  capabilities
- make GPU/CPU picking, selected-record preview and selected-pair preview
  reusable engine capabilities
- run screenshot/performance review against the GRAE10 visual reference and
  each grammar brief
- keep public gallery promotion behind `check-public-gallery-evidence.mjs`
- keep every review-pending preview out of accepted-hero status until screenshot
  review explicitly accepts it
- keep the project-page copy contract green so internal review state stays in
  tooling attributes and not visible public copy
- keep the public miniature scene contract green so every public preview keeps
  the shared photographic runtime contract instead of drifting into page-local
  render/style code
- keep the Redif preview on native `metric.visual.v1` evidence and
  `showDynamics()`; do not promote it to a public hero without a separate
  visual brief, gallery decision and screenshot review
- process-curve external scale is now integrated with 576 real licensed source
  windows; remaining work is visual composition acceptance, not source padding
- use the integrated composition task files for condition monitoring, mapping
  residuals and relation matrix readability as the baseline for the next
  acceptance wave; each follow-up task must still improve reusable engine/view
  modules, not only page layout
- use the integrated composition task files for mixed records, cross-space
  dependency and dynamics/noise as the baseline for later human visual review;
  they remain review-pending until screenshot acceptance clears the blockers

## Shared Validation Commands

```bash
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-views.mjs
node visual/tools/check-cpp-export.mjs
node visual/tools/check-project-site-copy-contract.mjs
node visual/tools/check-public-miniature-scene-contract.mjs
node visual/tools/check-redif-preview.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-visual-performance-large-scenes.mjs
node visual/tools/check-visual-document.mjs <exported metric.visual.json>
node visual/tools/check-visual-agent-tasks.mjs
ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure
ctest --test-dir build/core -L 'metric_application_evidence|metric_diffusion_coordinate_pipeline|metric_mnist|metric_visual_integrity|metric_benchmark_report' --output-on-failure
```
