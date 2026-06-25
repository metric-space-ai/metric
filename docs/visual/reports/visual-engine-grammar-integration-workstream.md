# Visual Engine Grammar Integration Workstream

Date: 2026-06-25

## Scope

This slice integrates three reusable engine improvements without promoting any
public preview to hero status:

- public command diagnostics now expose evidence counts, evidence-kind signals,
  provenance summaries and descriptor-kind counts;
- typed-record glyph and property-field grammars now carry first-class
  diagnostics and semantic contracts;
- mapping, dynamics and solver-trace views now expose motion/timeline evidence
  descriptors rather than page-local animation logic.

The runtime also reports layer diagnostics through `MetricVisualRuntime.getState()`.
That lets browser gates inspect engine capabilities such as heat-field grammar
diagnostics and relation-matrix readability diagnostics directly.

## Engine Changes

- `MetricVisualSurface` diagnostics now include record, space, coordinate,
  relation, property, graph, timeline and declared-view counts plus an
  evidence report.
- `createRecordGlyphGrammar()` emits
  `metric.visual.record_glyph_diagnostics.v1` for identity, render, material
  and label contracts.
- `SpacePropertiesView` emits
  `metric.visual.property_field_grammar.v1` for density, entropy, anomaly,
  outlier, residual and generic scalar fields.
- `HeatFieldLayer.getGrammarDiagnostics()` reports scalar channel, record
  identity, ramp mode and selection-linking state.
- `MappingView` attaches `metric.visual.mapping_motion_evidence.v1` to morph
  and residual/error descriptors.
- `DynamicsView` attaches `metric.visual.timeline_evidence.v1` to timeline,
  trajectory, field and animation descriptors.
- `SolverTraceView` now emits a reusable `CurveRibbonLayer` timeline curve with
  `metric.visual.solver_trace_evidence.v1`.
- The mapping preview page now references the current native coordinate IDs:
  `parametric-coordinate-latent-2d` and `source-coordinate-layout-3d`.

## Acceptance Commands

Passed:

```bash
node visual/tools/check-glyph-record-grammar.mjs
node visual/tools/check-field-property-grammar.mjs
node visual/tools/check-mapping-motion-grammar.mjs
node visual/tools/check-dynamics-motion-grammar.mjs
node visual/tools/check-timeline-motion-contract.mjs
node visual/tools/check-visual-command-api.mjs
node visual/tools/check-views.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-relation-matrix-readability.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-visual-performance-large-scenes.mjs
node visual/tools/check-hero-screenshot-review.mjs
```

## Browser Evidence

- Public regression: `7` checked, `0` failed,
  generated `2026-06-25T09:12:18.508Z`.
- Performance: large-scene rows plus `6` public grammar rows passed,
  generated `2026-06-25T09:12:38.389Z`.
- Screenshot review: pass, `1` accepted hero and `6` review-pending previews,
  generated `2026-06-25T09:17:38.382Z`.

The relation-matrix performance row now verifies layer diagnostics for the
tile-summary LOD texture. Field/dynamics rows expose heat-field grammar
diagnostics through runtime state.

## Remaining Gaps

- Public previews are still not hero-accepted. Screenshot review keeps them in
  review-pending status.
- The current native preview evidence sizes are still small for several
  application families.
- Runtime layer diagnostics are intentionally descriptive. They do not replace
  visual review or public-scale evidence.
