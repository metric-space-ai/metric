# Mapping Dimensionality Scale Exporter Report

Date: 2026-06-25
Branch: `codex/visual-engine-evidence-exporters`

## Changed Files

- `examples/engine/mapping_dimensionality_visual_export.cpp`
- `docs/examples/assets/mapping-dimensionality/metric.visual.json`
- `docs/visual/reports/mapping-dimensionality-scale-exporter.md`

## Final Counts

- Records: 1,000
- Relations: 2
- Relation values: 500,500 upper-triangle entries per relation
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
- Native scale gate: failed globally because the public brief manifest still lists stale `record-count-below-hero-minimum` blockers. The mapping row itself is `nativeScaleReady: true` with `recordCount: 1000`.
- Public gallery evidence: passed.
- Public visual regression: passed, 8/8 examples.

## Remaining Blocker

`node visual/tools/check-native-hero-evidence-scale.mjs` still exits nonzero because `visual/hero-visual-briefs.manifest.json` contains `record-count-below-hero-minimum` for `mapping-dimensionality-hero` even though the regenerated native asset has `recordCount: 1000` and meets `minRecordCountForHero: 1000`.

That manifest is outside this task's owner scope. The mapping row also still carries `visual-composition-not-human-accepted`, so human screenshot/composition acceptance remains a separate blocker.
