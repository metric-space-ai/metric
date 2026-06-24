# W4 — Condition Monitoring Hero

Summary: a dense finite-metric-space sculpture of windowed process records,
colored by anomaly with a local-density ground field, rendered on the miniature
hero rig (camera-depth DoF, photographic postFx). This is the first
visually-accepted hero and the quality bar for W5–W8.

## Preflight

- Visual claim: entropy/density and metric neighborhood expose abnormal process
  states as a 3D metric-space sculpture before a raw time-series plot does.
- Evidence fields available: `condition-3d`/`condition-2d` coordinates;
  `anomaly`, `entropy`, `density` scalar properties; `cluster` (regime)
  categorical; `condition-metric` sparse kNN relation; per-record time-series
  payloads.
- Record count: 1536 windows across 16 degrading/recovering runs, 4 regimes
  (healthy/drift/fault/recovery). Relation: 9216 kNN edges.
- Viewport composition: dense cloud fills the frame; rising fault ridge in the
  center; blue density field on the bounded floor; anomaly burns blue→red;
  shallow depth-of-field focuses the ridge.
- Layer stack: density ground field (DenseFieldView) → semantic ground
  projection (GroundProjectionView) → instanced point sculpture (PointCloudView),
  composed by `createMiniatureHeroSceneBundle` and applied via the miniature
  style.
- Interaction plan: orbit camera; hover preview (time-series) is a follow-up.
- Screenshot target state: dense sculpture, anomaly ridge clearly hottest.
- Expected failure modes: too-sparse data (mitigated: 1536 records); washed-out
  anomaly (mitigated: scalar ramp + DoF).

## Files changed

- `visual/examples/condition-monitoring-hero/index.html`
- `visual/examples/condition-monitoring-hero/evidence.json` (generated)
- `visual/tools/build-hero-evidence.mjs` (evidence generator)
- `visual/src/views/MetricSpaceHeroScene.js` (shared hero-scene helper)

## Evidence fixture

`visual/examples/condition-monitoring-hero/evidence.json` (1536 records).

## Screenshot

`visual/output/W4-condition-monitoring-hero.png`

## Browser command

```
METRIC_VISUAL_EXAMPLES=condition-monitoring-hero \
  NODE_PATH=<npm-cache>/node_modules \
  node visual/tools/check-visual-examples.mjs
```

## Console error summary

No console errors, no page errors. Layer state `ready`, 4 layer instances.

## Frame / performance notes

Headless swiftshader (CPU floor). The 1536-record cloud + field + projection
renders interactively; the 60k GRAE10 flagship remains the dense-performance
reference.

## Visual acceptance checklist

- [x] first viewport communicates one metric-space idea (degradation → fault)
- [x] dense and intentional (1536 records, swirling ridge)
- [x] composition fills the frame without clipping the structure
- [x] color encodes a named property (anomaly score)
- [x] floor carries information (local-density field, not decoration)
- [x] camera/floor/DoF reinforce spatial understanding
- [x] screenshot usable on the project page without debug UI

## Visual failure checklist (must all be false)

- [ ] tiny point cluster in a huge empty frame — FALSE
- [ ] points without floor/field/property encoding — FALSE
- [ ] generic template reused for an unrelated concept — FALSE
- [ ] side panel carries the explanation while the scene says nothing — FALSE
- [ ] synthetic decoration not backed by evidence fields — FALSE (color = anomaly
      field, floor = density field, both exported)

## Remaining gaps

- Hover time-series preview not yet wired in this rebuilt page (was present in
  the earlier version; re-add as interaction polish).
- Evidence is dense synthetic, not a real C++ export; a `mtrc::visual` exporter
  exists for the contract but the dense condition export pipeline is future work.

## Status

```
Status:
- loads: yes
- renders: yes
- interactive: partial (camera orbit; hover preview pending)
- visually accepted: yes
- complete: yes (visual); interaction polish (hover preview) tracked as a gap
```
