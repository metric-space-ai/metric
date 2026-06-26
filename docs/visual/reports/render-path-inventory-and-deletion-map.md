# Render Path Inventory And Deletion Map

Date: 2026-06-26

Scope: read-only inventory. I inspected `visual/src/metric-visual.js`, `visual/src/views/`,
`visual/examples/*/index.html`, and `docs/site/index.html`. I ran the required
`rg` searches; generated/embedded bundles produced very large matches, so the
actionable findings below cite the source lines that matter. GRAE10 files were
not changed. GRAE10 cleanup is intentionally not scheduled here.

Follow-up status, same day: finding F4 was implemented after this inventory.
The unused `metric-visual.js` facade helpers `pairedRecordBridgeDescriptor`,
`sharedGroundDescriptor`, and `offsetPositionMap` were removed. The next
cleanup priority is F2, wrapping or demoting the public descriptor injection
escape hatches.

Target architecture:

```text
metric.visual.v1 evidence -> semantic view -> layer descriptors -> MetricVisualRuntime
```

## Prioritized Deletion/Extraction Order

1. Done: delete the dead legacy descriptor helpers at the bottom of
   `visual/src/metric-visual.js`: `pairedRecordBridgeDescriptor`,
   `sharedGroundDescriptor`, and `offsetPositionMap`.
2. Next: wrap or demote `MetricVisualSurface.setLayerDescriptors()` and
   `addLayerDescriptors()`. They are public descriptor injection escape hatches
   that let page code bypass semantic views.
3. Wrap process-curve miniature bundle/factory exports so process-curve look
   galleries can request a semantic process-curve scene without assembling
   descriptors and runtimes page-locally.
4. Quarantine direct-runtime examples (`native-engine-probe` and
   `miniature-look-gallery`) outside public-gallery paths, or rewrite them to use
   `createMetricVisual()` plus semantic commands.
5. Delete/quarantine the external MNIST/Babyplots HTML demo. It is a separate
   renderer and is not compatible with the one-engine rule.
6. Keep the existing public hero examples that call `createMetricVisual()` and
   `show*` commands; validate they never import `MetricVisualRuntime` directly.

## Findings

### F1: Canonical Public Runtime Entry

- Path/line: `visual/src/metric-visual.js:36`, `visual/src/metric-visual.js:52`
- Symbol: `createMetricVisual()`
- Current role: public entry point that normalizes evidence, owns the single
  `MetricVisualRuntime`, attaches miniature hero style, and returns a
  `MetricVisualSurface`.
- Classification: keep.
- One-engine status: satisfies the rule. This is the intended
  evidence-to-runtime bridge; `new MetricVisualRuntime` is acceptable here
  because the public surface owns it centrally.
- Exact next action: keep as the only public helper that constructs
  `MetricVisualRuntime`.
- Safest owner scope: `visual/src/metric-visual.js`,
  `visual/src/runtime/`, and public API tests only.
- Validation command: `rg -n "new MetricVisualRuntime" visual/src visual/examples docs/site --glob '!visual/examples/grae10-*/*'`
  should show this file plus explicitly quarantined/internal probes only.

### F2: Public Descriptor Injection Escape Hatch

- Path/line: `visual/src/metric-visual.js:209`,
  `visual/src/metric-visual.js:222`, `visual/src/metric-visual.js:242`
- Symbol: `MetricVisualSurface.setViews()`,
  `MetricVisualSurface.setLayerDescriptors()`,
  `MetricVisualSurface.addLayerDescriptors()`
- Current role: direct descriptor setters on the public surface.
- Classification: wrap.
- One-engine status: partially violates the rule. `setViews()` is safe when it
  receives semantic views, but it also accepts raw descriptors. `setLayerDescriptors()`
  and `addLayerDescriptors()` let callers bypass semantic view ownership.
- Exact next action: demote descriptor setters to internal/private API or gate
  them behind an explicitly internal diagnostics option. Public examples should
  use `show*` commands or semantic view objects only.
- Safest owner scope: `visual/src/metric-visual.js` plus public API/export tests.
- Validation command: `rg -n "setLayerDescriptors\\(|addLayerDescriptors\\(" visual/src visual/examples docs/site --glob '!visual/src/metric-visual.js'`
  should have no public page calls after the wrap.

### F3: Canonical Semantic Commands

- Path/line: `visual/src/metric-visual.js:270`,
  `visual/src/metric-visual.js:284`, `visual/src/metric-visual.js:291`,
  `visual/src/metric-visual.js:298`, `visual/src/metric-visual.js:309`,
  `visual/src/metric-visual.js:318`, `visual/src/metric-visual.js:348`,
  `visual/src/metric-visual.js:395`, `visual/src/metric-visual.js:418`,
  `visual/src/metric-visual.js:434`, `visual/src/metric-visual.js:450`,
  `visual/src/metric-visual.js:474`
- Symbol: `showMetricSpace()`, `showRelationMatrix()`,
  `showNeighborhoodGraph()`, `showSpaceProperties()`, `showMapping()`,
  `showDynamics()`, `showConditionMonitoring()`, `showProcessCurves()`,
  `showMixedRecords()`, `showCrossSpace()`,
  `showRelationMatrixNeighborhood()`, `showSolverTrace()`
- Current role: semantic public commands that instantiate semantic view classes
  and feed their descriptors into the runtime.
- Classification: keep.
- One-engine status: satisfies the rule at the public API level. The commands
  convert normalized evidence into semantic views before reaching descriptors.
- Exact next action: keep commands as public surface. Avoid adding any new
  public command that assembles raw descriptors without a semantic view class.
- Safest owner scope: individual command plus corresponding view class and hero
  example.
- Validation command: `rg -n "visual\\.show|show[A-Z].*\\(" visual/examples/*/index.html docs/site/index.html`
  and check that public heroes call `createMetricVisual()` plus `show*`.

### F4: Legacy Facade Cross-Space Descriptor Helpers

- Path/line: `visual/src/metric-visual.js:1407`,
  `visual/src/metric-visual.js:1436`, `visual/src/metric-visual.js:1459`
- Symbol: `pairedRecordBridgeDescriptor()`, `sharedGroundDescriptor()`,
  `offsetPositionMap()`
- Current role: legacy descriptor construction helpers left in the public facade
  file.
- Classification: delete.
- One-engine status: violates ownership boundaries. Equivalent logic now belongs
  to `CrossSpaceView` (`sharedGroundDescriptor()` at
  `visual/src/views/CrossSpaceView.js:266` and `bridgeDescriptor()` at
  `visual/src/views/CrossSpaceView.js:292`). These helpers are currently unused,
  but their presence invites facade-level descriptor assembly.
- Exact next action: done; the three functions were removed from
  `metric-visual.js` after confirming no imports/references existed.
- Safest owner scope: `visual/src/metric-visual.js` only.
- Validation command: `rg -n "pairedRecordBridgeDescriptor|sharedGroundDescriptor|offsetPositionMap" visual/src/metric-visual.js visual/src/views`
  should return only `CrossSpaceView` methods or no facade hits.

### F5: Cross-Space Semantic View

- Path/line: `visual/src/views/CrossSpaceView.js:229`,
  `visual/src/views/CrossSpaceView.js:266`,
  `visual/src/views/CrossSpaceView.js:292`
- Symbol: `CrossSpaceView.toLayerDescriptors()`,
  `CrossSpaceView.sharedGroundDescriptor()`,
  `CrossSpaceView.bridgeDescriptor()`
- Current role: semantic owner for paired-space stage, side descriptors, and
  dependence bridge descriptors.
- Classification: keep.
- One-engine status: satisfies the rule. It owns the semantic contract and
  emits layer descriptors for runtime consumption.
- Exact next action: keep this as the owner of paired-space bridge/ground logic;
  delete facade duplicates in F4.
- Safest owner scope: `visual/src/views/CrossSpaceView.js` and
  cross-space hero/evidence fixtures.
- Validation command: `rg -n "dependence-bridge|shared-paired-space-stage|CrossSpaceView" visual/src/views visual/examples/cross-space-dependency-hero`
  should show the semantic owner and public hero only.

### F6: Process-Curve Semantic View

- Path/line: `visual/src/views/ProcessCurveSceneView.js:16`,
  `visual/src/views/ProcessCurveSceneView.js:118`
- Symbol: `ProcessCurveSceneView`, `ProcessCurveSceneView.toLayerDescriptors()`
- Current role: semantic process-curve view that resolves evidence and emits
  morph/projection/field/track/label/matrix/neighborhood descriptors.
- Classification: keep.
- One-engine status: satisfies the rule when reached through
  `MetricVisualSurface.showProcessCurves()` or `showConditionMonitoring()`.
- Exact next action: keep as canonical process-curve semantic owner.
- Safest owner scope: `visual/src/views/ProcessCurveSceneView.js` plus
  process-curve examples.
- Validation command: `rg -n "showProcessCurves|showConditionMonitoring|ProcessCurveSceneView.fromVisualSpace" visual/src visual/examples docs/site`
  should show public commands/pages, not page-local descriptor factories.

### F7: Process-Curve Miniature Bundle Factory

- Path/line: `visual/src/views/ProcessCurveSceneView.js:388`,
  `visual/src/views/ProcessCurveSceneView.js:396`,
  `visual/src/views/ProcessCurveSceneView.js:408`
- Symbol: `createProcessCurveMiniatureLayerDescriptors()`,
  `createProcessCurveLayerDescriptors()`,
  `createProcessCurveMiniatureSceneBundle()`
- Current role: helper/factory path that creates process-curve descriptors and
  miniature scene bundles outside the public surface.
- Classification: wrap.
- One-engine status: risky. Internally it delegates to semantic subviews, but
  the exported bundle factory allows callers to skip `createMetricVisual()` and
  feed descriptors/style bundles directly into a runtime.
- Exact next action: move bundle creation behind a semantic view or style API
  that accepts a `ProcessCurveSceneView`/surface, not raw descriptors. Keep
  private helpers if only `ProcessCurveSceneView.toLayerDescriptors()` calls them.
- Safest owner scope: `visual/src/views/ProcessCurveSceneView.js`,
  `visual/src/style/miniature/`, and the direct-runtime examples that use the
  bundle.
- Validation command: `rg -n "createProcessCurveMiniatureSceneBundle|createProcessCurveMiniatureLayerDescriptors|createProcessCurveLayerDescriptors" visual/src visual/examples docs/site`
  should show no public example import after wrapping.

### F8: Process-Curve Raw Label/Skyline Descriptor Construction

- Path/line: `visual/src/views/ProcessCurveSceneView.js:719`,
  `visual/src/views/ProcessCurveSceneView.js:837`
- Symbol: `createProcessCurveLabelDescriptor()`,
  `createProcessCurveSkylineDescriptor()`
- Current role: process-curve-specific raw descriptor builders.
- Classification: wrap.
- One-engine status: acceptable only while private to `ProcessCurveSceneView`.
  They produce raw descriptor objects instead of using dedicated semantic view
  classes, so they should not be exported or reused by pages.
- Exact next action: either keep private and document as view-internal, or
  extract to `ProcessCurveLabelView` and `ProcessCurveSkylineView` if another
  owner needs them.
- Safest owner scope: `visual/src/views/ProcessCurveSceneView.js` only, unless
  new view files are introduced.
- Validation command: `rg -n "createProcessCurveLabelDescriptor|createProcessCurveSkylineDescriptor" visual/src visual/examples docs/site`
  should show private use in `ProcessCurveSceneView.js` only.

### F9: Direct Runtime Internal Probe

- Path/line: `visual/examples/native-engine-probe/index.html:104`,
  `visual/examples/native-engine-probe/index.html:161`,
  `visual/examples/native-engine-probe/index.html:166`,
  `visual/examples/native-engine-probe/index.html:180`,
  `visual/examples/native-engine-probe/index.html:191`
- Symbol: module imports, `createMiniaturePhotographicStyle()`,
  `new MetricVisualRuntime()`, `runtime.setDocument()`, `style.applyBundle()`
- Current role: explicitly marked internal direct-runtime diagnostic harness.
- Classification: quarantine.
- One-engine status: violates the public one-engine rule if linked from a public
  hero. It constructs the runtime and applies a descriptor bundle page-locally.
  The page metadata says `publicGallery: false`, so it can remain only as an
  internal diagnostic.
- Exact next action: keep out of `docs/site/index.html` and public galleries; if
  it must become public, rewrite through `createMetricVisual()` plus a semantic
  `showProcessCurves()` path.
- Safest owner scope: `visual/examples/native-engine-probe/index.html` and
  internal diagnostics docs only.
- Validation command: `rg -n "native-engine-probe|new MetricVisualRuntime|style\\.applyBundle" docs/site visual/examples --glob '!visual/examples/grae10-*/*'`
  should show the probe only, never public site links.

### F10: Miniature Look Gallery Direct Runtime Loop

- Path/line: `visual/examples/miniature-look-gallery/index.html:163`,
  `visual/examples/miniature-look-gallery/index.html:184`,
  `visual/examples/miniature-look-gallery/index.html:201`,
  `visual/examples/miniature-look-gallery/index.html:207`,
  `visual/examples/miniature-look-gallery/index.html:246`,
  `visual/examples/miniature-look-gallery/index.html:250`
- Symbol: `ProcessCurveSceneView.fromVisualSpace()`,
  `sceneView.toLayerDescriptors()`, `createMiniatureLookSceneAtlas()`,
  `new MetricVisualRuntime()`, `style.applyBundle()`
- Current role: style/look gallery that builds descriptors once, creates many
  runtimes, and applies style bundles page-locally.
- Classification: wrap.
- One-engine status: bypasses the public surface. It starts with a semantic
  view, but then exports descriptors into a page-local atlas and runtime loop.
- Exact next action: provide a semantic look-gallery wrapper that owns the
  runtime creation internally, or move this page to an internal diagnostics area
  with explicit quarantine metadata.
- Safest owner scope: `visual/examples/miniature-look-gallery/index.html`,
  `visual/src/style/miniature/`, and process-curve miniature APIs.
- Validation command: `rg -n "miniature-look-gallery|new MetricVisualRuntime|toLayerDescriptors\\(\\)|createMiniatureLookSceneAtlas" visual/examples docs/site`
  should show no public site iframe/link and no direct runtime if wrapped.

### F11: External MNIST/Babyplots Demo

- Path/line: `visual/examples/mnist-dimension-reduction/index.html:24`,
  `visual/examples/mnist-dimension-reduction/index.html:58`
- Symbol: embedded `Baby` bundle, `vis.doRender()`
- Current role: standalone generated HTML with embedded Babyplots/Babylon-style
  renderer and page-local render loop.
- Classification: delete.
- One-engine status: violates the rule. It is a separate renderer and cannot be
  made safe by tests passing.
- Exact next action: delete the example or quarantine it outside public visual
  examples. If MNIST remains needed, re-export as `metric.visual.v1` evidence and
  render with `createMetricVisual()`.
- Safest owner scope: `visual/examples/mnist-dimension-reduction/index.html`
  only, plus docs links if any are found.
- Validation command: `rg -n "Baby;|Baby\\.|doRender\\(|mnist-dimension-reduction" visual docs/site`
  should show no public render path after deletion/quarantine.

### F12: Public Hero Examples Using Canonical Surface

- Path/line: `visual/examples/condition-monitoring-hero/index.html:46`,
  `visual/examples/condition-monitoring-hero/index.html:52`,
  `visual/examples/cross-space-dependency-hero/index.html:53`,
  `visual/examples/cross-space-dependency-hero/index.html:58`,
  `visual/examples/dynamics-noise-hero/index.html:61`,
  `visual/examples/dynamics-noise-hero/index.html:66`,
  `visual/examples/mapping-dimensionality-hero/index.html:84`,
  `visual/examples/mapping-dimensionality-hero/index.html:95`,
  `visual/examples/mixed-record-hero/index.html:80`,
  `visual/examples/mixed-record-hero/index.html:85`,
  `visual/examples/process-curve-external-hero/index.html:79`,
  `visual/examples/process-curve-external-hero/index.html:84`,
  `visual/examples/relation-matrix-neighborhood/index.html:47`,
  `visual/examples/relation-matrix-neighborhood/index.html:52`
- Symbol: `createMetricVisual()` plus `visual.show*()`
- Current role: public examples/heroes.
- Classification: keep.
- One-engine status: satisfies the public rule. These pages load evidence and
  use the semantic public surface instead of constructing runtime/layers.
- Exact next action: keep. Future page edits should not import
  `MetricVisualRuntime` or `createLayerFromDescriptor`.
- Safest owner scope: the individual example page and matching exported evidence.
- Validation command: `rg -n "MetricVisualRuntime|createLayerFromDescriptor|setLayerDescriptors|toLayerDescriptors" visual/examples/*-hero/index.html visual/examples/relation-matrix-neighborhood/index.html`
  should return no public hero matches.

### F13: Process-Curve Condition Monitoring Example

- Path/line: `visual/examples/process-curve-condition-monitoring/index.html:213`,
  `visual/examples/process-curve-condition-monitoring/index.html:255`
- Symbol: `createMetricVisual()`, `visual.showProcessCurves()`
- Current role: process-curve example with UI controls and descriptor diagnostics.
- Classification: keep.
- One-engine status: render path is canonical. It inspects descriptors for UI
  status, but it does not construct the runtime or descriptors page-locally.
- Exact next action: keep descriptor inspection read-only; do not add page-local
  descriptor mutation.
- Safest owner scope: `visual/examples/process-curve-condition-monitoring/index.html`.
- Validation command: `rg -n "new MetricVisualRuntime|createLayerFromDescriptor|setLayerDescriptors|toLayerDescriptors" visual/examples/process-curve-condition-monitoring/index.html`
  should have no matches.

### F14: Miniature Hero Frame Example

- Path/line: `visual/examples/miniature-hero-frame/index.html:141`,
  `visual/examples/miniature-hero-frame/index.html:172`,
  `visual/examples/miniature-hero-frame/index.html:206`
- Symbol: `createMetricVisual()`, `visual.showProcessCurves()`,
  descriptor reporting from `visual.descriptors`
- Current role: process-curve hero frame capture/reporting example.
- Classification: keep.
- One-engine status: render path is canonical. It reads descriptors after the
  semantic command for reporting.
- Exact next action: keep read-only descriptor reporting; do not use it as a
  descriptor injection surface.
- Safest owner scope: `visual/examples/miniature-hero-frame/index.html`.
- Validation command: `rg -n "new MetricVisualRuntime|createLayerFromDescriptor|setLayerDescriptors|toLayerDescriptors" visual/examples/miniature-hero-frame/index.html`
  should have no direct runtime/descriptors-as-input matches.

### F15: Docs Site Iframe Routing

- Path/line: `docs/site/index.html:420`, `docs/site/index.html:446`,
  `docs/site/index.html:475`, `docs/site/index.html:504`,
  `docs/site/index.html:533`, `docs/site/index.html:562`,
  `docs/site/index.html:591`, `docs/site/index.html:620`
- Symbol: public site `<iframe>` routes
- Current role: public gallery shell embedding visual examples.
- Classification: keep for canonical hero iframes; quarantine for any iframe
  that points at non-canonical renderers.
- One-engine status: the site itself does not render, but it can expose bypass
  renderers. The non-GRAE gallery iframes point at canonical `createMetricVisual`
  examples. The GRAE10 hero is outside this cleanup request and was not touched.
- Exact next action: do not add `native-engine-probe`,
  `miniature-look-gallery`, or `mnist-dimension-reduction` iframes here until
  they are wrapped/deleted.
- Safest owner scope: `docs/site/index.html` plus the specific embedded example.
- Validation command: `rg -n "native-engine-probe|miniature-look-gallery|mnist-dimension-reduction|MetricVisualRuntime|Baby;" docs/site/index.html`
  should return no matches.

### F16: Semantic View Layer Descriptor Classes

- Path/line: `visual/src/views/BaseView.js:33`,
  `visual/src/views/VisualLayer.js:3`, `visual/src/views/PointCloudView.js:84`,
  `visual/src/views/MetricSpaceView.js:213`,
  `visual/src/views/RelationMatrixView.js:127`,
  `visual/src/views/NeighborhoodGraphView.js:131`,
  `visual/src/views/DynamicsView.js:211`,
  `visual/src/views/TrajectoryPathView.js:174`,
  `visual/src/views/PropertyFieldView.js:137`,
  `visual/src/views/MixedRecordView.js:131`,
  `visual/src/views/MappingView.js:123`,
  `visual/src/views/SolverTraceView.js:67`
- Symbol: `BaseView.toLayerDescriptors()`, `VisualLayer`, and concrete
  `*View.toLayerDescriptors()` implementations
- Current role: semantic view layer descriptor generation.
- Classification: keep.
- One-engine status: satisfies the rule. These are the intended middle layer
  between evidence and runtime.
- Exact next action: keep new rendering logic here or in similarly scoped
  semantic view classes, not in pages or the public facade.
- Safest owner scope: one view class per cleanup task, with matching example and
  visual regression fixture.
- Validation command: `rg -n "new MetricVisualRuntime|new MetricScene3D|CanvasRenderingContext2D" visual/src/views`
  should return no matches.

## Next Three Non-Overlapping Cleanup Tasks

1. Facade cleanup: remove `pairedRecordBridgeDescriptor`,
   `sharedGroundDescriptor`, and `offsetPositionMap` from
   `visual/src/metric-visual.js`; separately wrap/demote public descriptor
   setters.
2. Process-curve miniature API cleanup: wrap
   `createProcessCurveMiniatureSceneBundle` and direct descriptor factories so
   look-gallery/probe pages cannot assemble runtimes from descriptors.
3. Example quarantine/deletion: move or rewrite `native-engine-probe` and
   `miniature-look-gallery`; delete or quarantine
   `visual/examples/mnist-dimension-reduction/index.html`. Do not include GRAE10
   in this task.

## Search Log

Required searches were run:

```bash
rg -n "new MetricVisualRuntime|new MetricScene3D|CanvasRenderingContext2D|create.*Descriptor|LayerDescriptor|setLayerDescriptors|this.views = \\[\\]|this.views = \\[\\]|function .*Descriptor|class Metric" visual/src visual/examples docs/site
rg -n "createProcessCurveMiniatureLayerDescriptors|pairedRecordBridgeDescriptor|sharedGroundDescriptor|offsetPositionMap|MetricScene3D|MetricRecordGallery|MetricHeatmap|MetricQueryInspector|loadMetricEvidence" visual/src/metric-visual.js visual/src/views visual/examples docs/site
```

Focused follow-up searches inspected `visual/src/metric-visual.js`,
`visual/src/views`, all `visual/examples/*/index.html`, and
`docs/site/index.html`. The broad searches also match embedded/generated bundles
inside HTML files; those were treated as evidence for quarantine/delete only
when they expose a non-METRIC renderer path.
