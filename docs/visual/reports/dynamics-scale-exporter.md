# Dynamics Scale Exporter Report

Date: 2026-06-25

Status note, 2026-06-26: this is a historical exporter execution report. The
current authoritative scale/gate state is
`docs/visual/reports/native-hero-scale-evidence-upgrade-plan.md` and
`docs/visual/reports/visual-regression-performance-workstream.md`. The stale
record-count blocker described below has since been removed from the active
Visual Brief state; the remaining blocker is screenshot/composition acceptance.

## Changed Files

- `examples/engine/finite_metric_dynamics_fixture.hpp`
- `examples/engine/finite_metric_dynamics_visual_export.cpp`
- `docs/examples/assets/dynamics-noise/metric.visual.json`
- `docs/visual/reports/dynamics-scale-exporter.md`

## Final Evidence Counts

- Records: 512
- Relations: 2 (`finite-euclidean-metric`, `geometry-derived-transition`)
- Relation shape: 512 x 512 dense matrix for both relations
- Graphs: 1 (`transition-graph`)
- Transition graph edges: 27,586
- Coordinates: 48
- Timelines: 2 (`forward-diffusion` with 7 steps, `reverse-reconstruction` with 41 steps)
- Properties: 12
- Diagnostics: 25, all passing
- Record state-history payloads: 512 of 512 records
- Generated asset size: about 15 MB

## Native Computation And Timeline Policy

The visual exporter now uses the scaled deterministic fixture path from
`finite_metric_dynamics_fixture.hpp` with 512 native vector records. The original
28-record default fixture remains available for the console example.

All METRIC evidence is computed in C++:

- Euclidean metric values
- geometry-derived transition matrix with native k=48 schedule
- transition graph
- forward diffusion
- reverse reconstruction
- seeded random-walk visit counts and empirical distribution
- stationary distribution
- Dirichlet energy
- MSE-to-clean and reconstruction errors
- coordinate states for timeline playback
- compact per-record state-history preview payloads

Timeline export remains bounded: all 7 forward states and all 41 reverse
coordinate states are exported. Each record payload carries full forward preview
history plus sampled reverse preview history, including the best reverse step,
so previews prove state history without duplicating every reverse coordinate
frame in every record payload.

## Commands Run

```bash
cmake --build build/core --target engine_finite_metric_dynamics_visual_export -- -j4
```

Result: passed.

```bash
build/core/examples/engine/engine_finite_metric_dynamics_visual_export --export-dir docs/examples/assets/dynamics-noise
```

Result: passed; regenerated `docs/examples/assets/dynamics-noise/metric.visual.json`.

```bash
cmake --build build/core --target engine_finite_metric_dynamics -- -j4 && build/core/examples/engine/engine_finite_metric_dynamics
```

Result: passed; confirms the shared default fixture still satisfies the existing
console example.

```bash
ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure
```

Result: passed, 14/14 tests.

```bash
node visual/tools/check-visual-document.mjs docs/examples/assets/dynamics-noise/metric.visual.json
```

Result: passed; validator reports 512 records, 2 relations, 48 coordinates and
native checks passing.

```bash
node visual/tools/check-native-hero-evidence-scale.mjs
```

Result from this historical isolated task: full command failed. The
`dynamics-noise-hero` row was already native-scale-ready with 512 records and 2
relations, but the then-unowned visual-brief manifest still carried stale
record-count blockers. The current parent gate has since been refreshed and
passes for this row.

```bash
node visual/tools/check-public-gallery-evidence.mjs
```

Result: passed.

```bash
node visual/tools/check-visual-regression-public-examples.mjs
```

Result: passed, 8/8 public examples. The dynamics row loaded and rendered
512 native records, 2 relations and 48 coordinates with no issues.

## Remaining Blocker

`dynamics-noise-hero` now passes the native scale evidence requirements, but
the full native-scale gate still fails on stale acceptance blockers in the
unowned brief manifest. Updating `visual/hero-visual-briefs.manifest.json` is
outside this task's owner scope.

The preview also remains blocked on human screenshot review:
`visual-composition-not-human-accepted`.
