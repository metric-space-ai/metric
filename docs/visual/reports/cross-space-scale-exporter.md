# Cross-Space Scale Exporter Report

Date: 2026-06-25

## Changed Files

- `examples/engine/cross_space_dependency_visual_export.cpp`
- `docs/examples/assets/cross-space-dependency/metric.visual.json`
- `docs/visual/reports/cross-space-scale-exporter.md`

## Final Counts

- Paired records: 512
- Relations: 3
  - `event-log-edit-distance`
  - `process-curve-twed-distance`
  - `cross-space-dependence-bridge-relation`
- Coordinates: 2 coordinate states, each with 512 positions
- Properties: 6
- Pair-property values: 130816
- Graphs: 1
- Bridge graph edges: 96
- Diagnostics: 4

## Native Computation

- Event-log distances are exported from native C++ `mtrc::Edit<char>` distance matrices.
- Process-curve distances are exported from native C++ `mtrc::TWED<double>` distance matrices.
- Local distance-profile alignment is computed in C++ from the two native distance matrices.
- Pair distance-profile contribution is computed in C++ across all off-diagonal pairs.
- Local dependence contribution is computed in C++ by aggregating pair contribution values per paired observation.
- Local densities are computed in C++ from native row mean distances.
- Global dependence uses native `mtrc::stats::correlate::mgc_estimate` with sample count 160 and 3 sample iterations.
- Exact full-grid MGC permutation significance at 512 records was not tractable for the exporter gate. A 63-permutation exact run completed in about 136 seconds, and a reduced exact-permutation/control attempt was still over two minutes. The final exporter omits permutation significance explicitly with `permutation_status: omitted_cost_control_exact_mgc_permutations_not_tractable_at_export_scale`.

## Bridge Evidence

The exporter adds:

- `cross-space-dependence-bridge-relation`: sparse native bridge-strength relation over selected paired observations.
- `cross-space-dependence-bridges`: graph with 96 self-pair bridge edges. Each edge carries source/target space IDs, source/target coordinate IDs, local contribution, local distance-profile alignment and bridge strength.
- `cross-space-bridge-evidence`: diagnostic tying the bridge graph/relation to the top native local-dependence contributors.

## Commands Run

- `cmake --build build/core --target engine_cross_space_dependency_visual_export -- -j4` passed.
- `/usr/bin/time -p build/core/examples/engine/engine_cross_space_dependency_visual_export --export-dir docs/examples/assets/cross-space-dependency` passed in 47.15 seconds for the final exporter.
- `node visual/tools/check-visual-document.mjs docs/examples/assets/cross-space-dependency/metric.visual.json` passed.
- `ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure` passed, 14/14 tests.
- `node visual/tools/check-native-hero-evidence-scale.mjs` failed overall. The cross-space row is native-scale-ready with 512 records and 3 relations, but `visual/hero-visual-briefs.manifest.json` still lists `record-count-below-hero-minimum`, which is outside this task's owner scope.
- `node visual/tools/check-public-gallery-evidence.mjs` passed.
- `node visual/tools/check-visual-regression-public-examples.mjs` failed before rendering examples because the Chrome DevTools endpoint did not start; the process then hung in cleanup and was interrupted.

## Remaining Blockers

- `cross-space-dependency-hero` now passes the evidence scale condition in the native-scale report, but the full native-scale gate does not pass because the out-of-scope visual brief manifest still has the stale `record-count-below-hero-minimum` blocker for this row.
- Browser screenshot/composition review is still blocked: the public regression runner failed to start Chrome DevTools, and the manifest still carries `visual-composition-not-human-accepted`.
