# Mapping Dimensionality Scale Exporter Report

Date: 2026-06-25
Branch: `codex/visual-engine-evidence-exporters`

Status note, 2026-06-26: this is a historical exporter execution report. The
current authoritative scale/gate state is
`docs/visual/reports/native-hero-scale-evidence-upgrade-plan.md` and
`docs/visual/reports/visual-regression-performance-workstream.md`. The stale
record-count blocker described below has since been removed from the active
Visual Brief state; the remaining blocker is screenshot/composition acceptance.

## Changed Files

- `examples/engine/mapping_dimensionality_visual_export.cpp`
- `docs/examples/assets/mapping-dimensionality/metric.visual.json`
- `docs/visual/reports/mapping-dimensionality-scale-exporter.md`

## Final Counts

- Records: 1,000
- Relations: 2
- Relation values: two `1000 x 1000` dense numeric matrices; the source asset
  stays under GitHub's 50 MB warning threshold while preserving the 1,000-record
  native scale evidence
- Coordinates: 3, each with 1,000 record positions
- Timelines: 1 coordinate-morph timeline with 3 exported steps
- Properties: 9
- Graphs: 1 kNN graph with 5,000 native edges
- Diagnostics: 4

## Native Mapping Evidence

- Source records are deterministic native process curves across 5 families and 200 variants per family.
- Every record keeps its original time-series payload plus native mapping diagnostics in `payload.mapping_diagnostics`.
- C++ computes aligned process-curve distances, metric-law diagnostics, single-step diffusion potential anchor targets, ridge-linear parametric latent coordinates, latent Euclidean distances, local mapping distortion, diffusion target residuals, neighborhood preservation error, source-nearest latent rank, kNN graph edges and query stability diagnostics.
- JavaScript validation/rendering does not compute embeddings, residuals, preservation errors, kNN edges or metric values.
- Provenance keeps `native_export: true` and does not set `provenance.synthetic`.

## Commands Run

```bash
cmake --build build/core --target engine_mapping_dimensionality_visual_export -- -j4
build/core/examples/engine/engine_mapping_dimensionality_visual_export --export-dir docs/examples/assets/mapping-dimensionality
ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure
node visual/tools/check-visual-document.mjs docs/examples/assets/mapping-dimensionality/metric.visual.json
node visual/tools/check-native-hero-evidence-scale.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-visual-regression-public-examples.mjs
```

Results:

- Build: passed.
- Export: passed and wrote `docs/examples/assets/mapping-dimensionality/metric.visual.json`.
- CTest visual export/validate: passed, 14/14 tests.
- Visual document check: passed, records 1,000, relations 2, coordinates 3, properties 9, timelines 1.
- Historical native scale gate result from this isolated task: failed globally
  because the then-out-of-scope public brief manifest still listed stale
  record-count blockers. The mapping row itself was already
  `nativeScaleReady: true` with `recordCount: 1000`; the current parent gate
  has since been refreshed and passes for this row.
- Public gallery evidence: passed.
- Public visual regression: passed, 8/8 examples.

## Remaining Blocker

The regenerated native asset has `recordCount: 1000` and meets
`minRecordCountForHero: 1000`. The active remaining blocker is
`visual-composition-not-human-accepted`, so human screenshot/composition
acceptance remains required.
