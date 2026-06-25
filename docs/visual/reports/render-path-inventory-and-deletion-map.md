# Render Path Inventory And Deletion Map

Snapshot: `codex/visual-engine-evidence-exporters`, current worktree on 2026-06-25.
The tree was already dirty and `visual/src/views/ProcessCurveSceneView.js` changed
concurrently during inspection; this report reflects the current file contents at
write time. This inventory did not edit source, examples, generated assets, site
pages, tests, or GRAE10.

One-engine rule:

```text
metric.visual.v1 evidence -> semantic view -> layer descriptors -> MetricVisualRuntime
```

Required searches were run across `visual/src`, `visual/examples`, and
`docs/site`; `visual/src/metric-visual.js`, `visual/src/views/`,
`visual/examples/*/index.html`, and `docs/site/index.html` were inspected.
`docs/site/index.html` has no active visual renderer matches.

## Prioritized deletion/extraction order

1. Delete the legacy public exports at the bottom of `visual/src/metric-visual.js`
   first: `loadMetricEvidence`, `assertMetricEvidence`, `MetricSelection`,
   `MetricScene3D`, `MetricRecordGallery`, `MetricHeatmap`,
   `MetricQueryInspector`, `MetricProcessCurveApp`, plus their raw WebGL/2D
   helper functions. They are the highest-risk alternate render stack.
2. Finish the remaining process-curve cleanup. `MetricVisualSurface.showProcessCurves()`
   now uses `ProcessCurveSceneView`, but the public descriptor factory and two
   non-GRAE10 example/probe pages still call
   `createProcessCurveMiniatureLayerDescriptors()` directly.
3. Convert or quarantine pages that instantiate `MetricVisualRuntime` directly:
   `process-curve-condition-monitoring`, `miniature-hero-frame`,
   `miniature-look-gallery`, and `native-engine-probe`.
4. Tighten public descriptor escape hatches after the above migrations:
   `MetricVisualSurface.setLayerDescriptors()`,
   `MetricVisualSurface.addLayerDescriptors()`, runtime raw-descriptor fallback,
   and `applyMiniatureSceneBundle()`.
5. Quarantine standalone or external renderers that are not public-hero paths:
   `miniature-field/InstancedBoxField`, GRAE10, and the embedded Babylon MNIST
   demo. Do not touch GRAE10 without its owner.

## Findings

### P0 - `metric-visual.js` Legacy Exports

| Path and line | Symbol | Current role | Classification | Why | Exact next action | Safest owner scope | Validation command |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `visual/src/metric-visual.js:35` | `createMetricVisual()` | Public entry point that loads or normalizes a visual document, creates `MetricVisualRuntime`, and returns `MetricVisualSurface`. | keep | This is the intended public gateway into the one-engine path. | Keep as the only public page entry point for heroes. | Public command API worker. | `node visual/tools/check-visual-command-api.mjs && node visual/tools/check-single-render-pipeline.mjs` |
| `visual/src/metric-visual.js:159` | `MetricVisualSurface` | Semantic command facade over one runtime. | keep | The class is the right public surface when callers use `show*()` commands. | Keep, but remove public descriptor bypasses listed below. | Public command API worker. | `node visual/tools/check-visual-command-api.mjs` |
| `visual/src/metric-visual.js:212` | `MetricVisualSurface.setLayerDescriptors()` / `addLayerDescriptors()` at `:229` | Publicly commits raw descriptors to the runtime. | wrap | It lets callers skip semantic views and go straight to descriptors. | Split into a private/internal commit helper, keep semantic `show*()` methods as the public API, and remove or gate direct descriptor calls. | `visual/src/metric-visual.js` only, plus command API checks. | `rg -n "\\.setLayerDescriptors\\(" visual/examples docs/site visual/src/metric-visual.js && rg -n "\\.addLayerDescriptors\\(" visual/examples docs/site visual/src/metric-visual.js` should show only internal surface/runtime plumbing after migration. |
| `visual/src/metric-visual.js:422` | `MetricVisualSurface.showProcessCurves()` | Public semantic command now routed through `ProcessCurveSceneView`. | keep | The surface no longer imports or calls `createProcessCurveMiniatureLayerDescriptors()` and now records `this.views = [view]`. | Keep the gate that rejects reintroducing direct process-curve descriptor helpers in `metric-visual.js`. | `visual/src/metric-visual.js`, process-curve checks. | `! rg -n "createProcessCurveMiniatureLayerDescriptors" visual/src/metric-visual.js && node visual/tools/check-process-curve-scene-view.mjs && node visual/tools/check-single-render-pipeline.mjs` |
| `visual/src/metric-visual.js:1404` | `pairedRecordBridgeDescriptor()` | Unused legacy paired-space descriptor helper. | delete | It creates descriptors outside current semantic views and is not referenced outside its definition. | Delete with the legacy block after confirming `CrossSpaceView` owns paired-space descriptors. | `visual/src/metric-visual.js` only. | `rg -n "pairedRecordBridgeDescriptor" visual/src visual/examples docs/site` returns no matches. |
| `visual/src/metric-visual.js:1433` | `sharedGroundDescriptor()` | Unused duplicate ground descriptor helper. | delete | `CrossSpaceView.sharedGroundDescriptor()` at `visual/src/views/CrossSpaceView.js:232` is the semantic-view version. | Delete the dead helper. | `visual/src/metric-visual.js` only. | `rg -n "function sharedGroundDescriptor" visual/src/metric-visual.js` returns no matches. |
| `visual/src/metric-visual.js:1456` | `offsetPositionMap()` | Unused legacy position mutator. | delete | No current call site; it belongs to the removed paired-space path. | Delete with the surrounding legacy helpers. | `visual/src/metric-visual.js` only. | `rg -n "offsetPositionMap" visual/src visual/examples docs/site` returns no matches. |
| `visual/src/metric-visual.js:1488` | `loadMetricEvidence()` / `assertMetricEvidence()` at `:1498` | Fetches and validates old `metric.evidence.v1`. | delete | It bypasses `metric.visual.v1`, `loadVisualDocument()`, and current evidence adapters. | Replace the one remaining page import with `metric.visual.json` plus `createMetricVisual()`, then delete both exports. | Legacy panel removal worker. | `! rg -n "loadMetricEvidence" visual/src visual/examples docs/site && ! rg -n "assertMetricEvidence" visual/src visual/examples docs/site && ! rg -n "metric\\.evidence\\.v1" visual/src visual/examples docs/site` |
| `visual/src/metric-visual.js:1509` | `MetricSelection` | Standalone pub/sub state for legacy panels. | delete | Current runtime has its own selection/picking state; this class only supports legacy canvases. | Delete after `process-curve-condition-monitoring` stops importing it. | Legacy panel removal worker. | `rg -n "MetricSelection" visual/src visual/examples docs/site --glob '!docs/visual/reports/*'` returns no matches. |
| `visual/src/metric-visual.js:1529` | `MetricScene3D` | Raw WebGL scene renderer. | delete | This is a second renderer with its own shaders, buffers, picking, animation, and canvas event model. | Delete after no page imports legacy panel exports. | Legacy panel removal worker. | `! rg -n "MetricScene3D" visual/src visual/examples docs/site && ! rg -n "new MetricScene3D" visual/src visual/examples docs/site` |
| `visual/src/metric-visual.js:1927` | `MetricRecordGallery` | 2D canvas record-gallery renderer. | delete | It renders outside semantic views and outside `MetricVisualRuntime`. | Replace with `RecordPreviewPanel`/runtime preview semantics or remove the panel. | Legacy panel removal worker. | `rg -n "MetricRecordGallery" visual/src visual/examples docs/site` returns no matches. |
| `visual/src/metric-visual.js:1979` | `MetricHeatmap` | 2D canvas pair-value heatmap renderer. | delete | `RelationMatrixView` and `RelationMatrixLayer` are the semantic/runtime path. | Replace with `RelationMatrixView` through `createMetricVisual().showRelationMatrix*()`; delete class. | Legacy panel removal worker. | `rg -n "MetricHeatmap" visual/src visual/examples docs/site` returns no matches and `node visual/tools/check-relation-matrix-picker.mjs` passes. |
| `visual/src/metric-visual.js:2044` | `MetricQueryInspector` | 2D canvas query/winner inspector. | delete | It is a page-local style renderer over old evidence, not a semantic view or runtime layer. | Fold useful preview payloads into `RecordPreviewPanel` or a semantic inspector view before deleting. | Preview/interaction worker after legacy panel removal. | `rg -n "MetricQueryInspector" visual/src visual/examples docs/site` returns no matches. |
| `visual/src/metric-visual.js:2106` | `MetricProcessCurveApp` | Legacy app shell that instantiates `MetricScene3D` and panel canvases. | delete | It composes the second renderer and old 2D panels. No current external usage was found. | Delete with the legacy renderer classes. | `visual/src/metric-visual.js` only. | `! rg -n "MetricProcessCurveApp" visual/src visual/examples docs/site && ! rg -n "new MetricProcessCurveApp" visual/src visual/examples docs/site` |
| `visual/src/metric-visual.js:2187` and `:2496` | `createPointProgram()` family, `prepare2D()`, curve/color helpers | Raw WebGL shader support and 2D canvas utilities for legacy classes. | delete | These functions exist only to support the alternate renderer/panel stack. | Delete after deleting legacy classes; do not move them to another renderer. | `visual/src/metric-visual.js` only. | `! rg -n "createPointProgram" visual/src/metric-visual.js && ! rg -n "createGroundProgram" visual/src/metric-visual.js && ! rg -n "createProjectionProgram" visual/src/metric-visual.js && ! rg -n "createMiniatureCompositeProgram" visual/src/metric-visual.js && ! rg -n "prepare2D" visual/src/metric-visual.js` |

### P0/P1 - Process-Curve Semantic View Handoff

| Path and line | Symbol | Current role | Classification | Why | Exact next action | Safest owner scope | Validation command |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `visual/src/views/ProcessCurveSceneView.js:15` | `ProcessCurveSceneView` | Semantic view for process curves. | keep | It supplies the process-curve view object and owns neighborhood/matrix support for `showProcessCurves()`. | Keep using it from `MetricVisualSurface.showProcessCurves()`; migrate remaining examples to semantic commands and then narrow or privatize descriptor helpers. | `visual/src/views/ProcessCurveSceneView.js`, `visual/src/views/index.js`, targeted tests. | `node --check visual/src/views/ProcessCurveSceneView.js && node visual/tools/check-process-curve-scene-view.mjs && node visual/tools/check-views.mjs` |
| `visual/src/views/ProcessCurveSceneView.js:387` | `createProcessCurveMiniatureLayerDescriptors()` | Public descriptor factory compatibility path. | wrap | It still exposes direct evidence -> descriptors without requiring a semantic view. | Keep only as a transitional wrapper around `ProcessCurveSceneView`, or make it private after callers migrate. | Process-curve scene-view consolidation worker. | `rg -n "createProcessCurveMiniatureLayerDescriptors" visual/src/metric-visual.js visual/examples` should return no public surface/page call sites. |
| `visual/src/views/ProcessCurveSceneView.js:395` | `createProcessCurveLayerDescriptors()` and descriptor helper family | Internal descriptor assembly for process-curve layers. | wrap | The logic is useful, but it must stay behind `ProcessCurveSceneView.toLayerDescriptors()`. | Keep private, ensure exports do not invite page-level descriptor construction, and add a check that the public surface uses the view. | Process-curve scene-view consolidation worker. | `node visual/tools/check-process-curve-scene-view.mjs && node visual/tools/check-single-render-pipeline.mjs` |
| `visual/examples/miniature-hero-frame/index.html:156` and `:197` | Direct `MetricVisualRuntime` plus `createProcessCurveMiniatureLayerDescriptors()` | Example page creates runtime and descriptors itself. | wrap | It uses the canonical runtime, but bypasses the public semantic command. | Convert to `createMetricVisual({ evidence }).showProcessCurves(...)` or use `ProcessCurveSceneView` only in a dev-only harness. | Example thin-shell worker, not GRAE10. | `! rg -n "MetricVisualRuntime" visual/examples/miniature-hero-frame/index.html && ! rg -n "createProcessCurveMiniatureLayerDescriptors" visual/examples/miniature-hero-frame/index.html && ! rg -n "createLayerFromDescriptor" visual/examples/miniature-hero-frame/index.html` |
| `visual/examples/miniature-look-gallery/index.html:186` and `:235` | Look gallery direct descriptor/runtime loop | Gallery renders many runtimes from descriptor bundles. | wrap | Useful look-atlas logic, but not a public hero path and still skips public semantic commands. | Keep as an internal style gallery or convert each entry to start from `ProcessCurveSceneView`; exclude from public gallery acceptance. | Style/gallery worker. | `node visual/tools/check-single-render-pipeline.mjs` rejects public use while a targeted gallery check still passes. |
| `visual/examples/native-engine-probe/index.html:156` | `new MetricVisualRuntime(...)` probe | Native engine probe harness. | quarantine | It is a diagnostic harness, not a public semantic page; it should not be accepted as a hero. | Move under an explicitly dev/probe-only contract or convert to `createMetricVisual()` if it remains public. | Visual diagnostics worker. | `rg -n "native-engine-probe" visual/hero-*.json docs/visual` shows it is not an accepted public hero. |

### P1 - Page-Level Legacy Panels And Evidence

| Path and line | Symbol | Current role | Classification | Why | Exact next action | Safest owner scope | Validation command |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `visual/examples/process-curve-condition-monitoring/index.html:186` | Imports `MetricVisualRuntime` and layer factory directly | Page builds the runtime manually. | wrap | Runtime is canonical, but the page bypasses `createMetricVisual()` and semantic commands. | Convert the scene to `createMetricVisual({ evidence: metric.visual.json }).showProcessCurves()` or a dedicated semantic command. | Same worker as legacy panel removal. | `! rg -n "MetricVisualRuntime" visual/examples/process-curve-condition-monitoring/index.html && ! rg -n "createLayerFromDescriptor" visual/examples/process-curve-condition-monitoring/index.html` |
| `visual/examples/process-curve-condition-monitoring/index.html:195` | Imports `MetricHeatmap`, `MetricQueryInspector`, `MetricRecordGallery`, `MetricSelection`, `loadMetricEvidence` | Uses legacy public exports. | delete | These are the only external references keeping the legacy export block alive. | Replace/remove panels, switch from `evidence.json` to `metric.visual.json`, then remove imports. | Legacy panel removal worker. | `! rg -n "MetricHeatmap" visual/examples/process-curve-condition-monitoring/index.html && ! rg -n "MetricQueryInspector" visual/examples/process-curve-condition-monitoring/index.html && ! rg -n "MetricRecordGallery" visual/examples/process-curve-condition-monitoring/index.html && ! rg -n "MetricSelection" visual/examples/process-curve-condition-monitoring/index.html && ! rg -n "loadMetricEvidence" visual/examples/process-curve-condition-monitoring/index.html` |
| `visual/examples/process-curve-condition-monitoring/index.html:226` | `new MetricRecordGallery`, `new MetricQueryInspector`, `new MetricHeatmap` | Page-local 2D canvas panels. | delete | They are separate render paths and duplicate relation/preview capabilities. | Replace with runtime preview and `RelationMatrixView` or remove this legacy page from public scope. | Legacy panel removal worker. | `node visual/tools/check-runtime-picking-preview.mjs && node visual/tools/check-relation-matrix-picker.mjs` |
| `visual/examples/process-curve-condition-monitoring/index.html:237` | `loadMetricEvidence(.../evidence.json)` | Loads old evidence schema. | delete | Public heroes must start from `metric.visual.v1`, not old `metric.evidence.v1`. | Use `docs/examples/assets/process-curve-external/metric.visual.json` or a new native export. | Native evidence/page worker. | `! rg -n "evidence\\.json" visual/examples/process-curve-condition-monitoring/index.html && ! rg -n "metric\\.evidence\\.v1" visual/examples/process-curve-condition-monitoring/index.html && ! rg -n "loadMetricEvidence" visual/examples/process-curve-condition-monitoring/index.html` |

### Keep - Canonical Runtime, Views, Layers, And Thin Heroes

| Path and line | Symbol | Current role | Classification | Why | Exact next action | Safest owner scope | Validation command |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `visual/src/runtime/runtime.js:344` | `MetricVisualRuntime` | Canonical renderer that owns scene, camera, layers, picking, postprocess, and render loop. | keep | This is the single renderer required by the one-engine rule. | Keep; do not introduce another renderer. | Runtime owner only. | `node visual/tools/check-single-render-pipeline.mjs` |
| `visual/src/runtime/runtime.js:524` | `setViewDescriptors()` | Runtime path from view descriptors to layer descriptors. | keep | It preserves the semantic-view entry when callers provide views. | Keep as canonical document/view ingestion path. | Runtime owner only. | `node visual/tools/check-views.mjs` |
| `visual/src/runtime/runtime.js:542` | `setLayerDescriptors()` / `addLayerDescriptor()` | Runtime commit point for descriptors. | keep | The architecture ends at layer descriptors -> runtime; the method is acceptable inside runtime-owned APIs. | Keep, but do not let public pages call it directly. | Runtime plus command API owner. | `rg -n "setLayerDescriptors\\(" visual/examples docs/site` returns no public-page matches except quarantined/dev pages. |
| `visual/src/runtime/runtime.js:1616` | `layerDescriptorsFromViews()` raw descriptor fallback at `:1630` | Accepts objects that already look like layer descriptors. | wrap | It can turn `setViewDescriptors()` into a descriptor bypass. | Restrict raw descriptor fallback to internal compatibility or require a semantic wrapper. | Runtime hardening worker after page migrations. | `node visual/tools/check-single-render-pipeline.mjs` with a negative fixture for raw layer descriptors in document views. |
| `visual/src/layers/LayerFactory.js:42` | `createLayerFromDescriptor()` | Maps descriptors to layer classes. | keep | This is the layer-materialization step inside the runtime. | Keep; callers should reach it only through runtime setup. | Layer/runtime owner. | `node --check visual/src/layers/LayerFactory.js && node visual/tools/check-single-render-pipeline.mjs` |
| `visual/src/views/BaseView.js:3` and `:33` | `BaseView.toLayerDescriptors()` contract | Base semantic view contract. | keep | This is the intended semantic view -> descriptor boundary. | Keep and require new grammars to subclass/use this contract. | Views owner. | `node visual/tools/check-views.mjs` |
| `visual/src/views/MetricSpaceView.js:59`, `MappingView.js:23`, `DynamicsView.js:33`, `CrossSpaceView.js:21`, `MixedRecordView.js:43`, `RelationMatrixView.js:14`, `NeighborhoodGraphView.js:20`, `TrajectoryPathView.js:21`, `PropertyFieldView.js:19`, `SpacePropertiesView.js:28`, `SolverTraceView.js:13` | Semantic view classes | Canonical view-to-descriptor implementations. | keep | They satisfy the one-engine path when invoked by `MetricVisualSurface.show*()` or runtime view descriptors. | Keep; do not replace with page-local canvas/SVG fallbacks. | Views owner. | `node visual/tools/check-views.mjs && node visual/tools/check-hero-grammar-contract.mjs` |
| `visual/src/curves/descriptors.js:15`, `:35`, `:87`, `:110`; `visual/src/relational/descriptors.js:13`, `:116`, `:175`; `visual/src/timeline/index.js:427`, `:533`, `:564` | Low-level descriptor factories | Descriptor emitters consumed by semantic views/layers. | keep | They are not renderers by themselves and are acceptable when called behind semantic views. | Keep, but checks should reject public page calls into these factories. | Curves/relational/timeline owners. | `! rg -n "create.*LayerDescriptor" visual/examples docs/site && ! rg -n "createTimeline.*Descriptor" visual/examples docs/site` |
| `visual/src/style/miniature/composition.js:58` | `applyMiniatureSceneBundle()` | Applies staged descriptor bundles to a runtime. | wrap | It is useful internally, but it calls `runtime.setLayerDescriptors()` and is used by direct-runtime examples. | Keep for style internals after semantic-view call sites are migrated; do not expose it as a public hero path. | Miniature style owner. | `rg -n "applyBundle" visual/examples docs/site && rg -n "applyMiniatureSceneBundle" visual/examples docs/site && rg -n "createMiniature.*SceneBundle" visual/examples docs/site` only shows dev/quarantined pages. |
| `visual/examples/condition-monitoring-hero/index.html:38`, `cross-space-dependency-hero/index.html:42`, `dynamics-noise-hero/index.html:39`, `mapping-dimensionality-hero/index.html:39`, `mixed-record-hero/index.html:37`, `relation-matrix-neighborhood/index.html:35`, `process-curve-external-hero/index.html:68` | Thin public hero pages using `createMetricVisual().show*()` | Current public command pattern. | keep | These pages start from `metric.visual.v1` evidence and call semantic commands. | Keep this as the only public hero pattern. | Public gallery worker. | `node visual/tools/check-public-gallery-evidence.mjs && node visual/tools/check-visual-regression-public-examples.mjs` |
| `docs/site/index.html:769` | Static site index | Project page, no active visual runtime. | keep | Required inspection found no renderer or descriptor path in this page. | No action. | None. | `! rg -n "MetricVisualRuntime" docs/site/index.html && ! rg -n "MetricScene3D" docs/site/index.html && ! rg -n "create.*Descriptor" docs/site/index.html && ! rg -n "setLayerDescriptors" docs/site/index.html` |

### Quarantine / Standalone Renderers

| Path and line | Symbol | Current role | Classification | Why | Exact next action | Safest owner scope | Validation command |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `visual/src/miniature-field/instanced-box-field.js:78` | `InstancedBoxField` / `createInstancedBoxField()` at `:596` | Standalone WebGL field renderer with its own context and loop. | quarantine | It bypasses `MetricVisualRuntime`; no current public page usage was found, but it is exported through `visual/src/index.js`. | Either remove from public exports and delete if unused, or wrap the useful visual idea as descriptors for `InstancedBoxLayer`. | Separate miniature-field cleanup worker, not part of P0 legacy export removal. | `rg -n "InstancedBoxField" visual/src visual/examples docs/site && rg -n "createInstancedBoxField" visual/src visual/examples docs/site` shows no usage outside its module/export after cleanup. |
| `visual/src/miniature-engine/grae10-engine.js:11`, `visual/examples/grae10-engine-module/index.html:190`, `visual/examples/grae10-metric-engine/index.html:190` | GRAE10 miniature engine | Golden/reference renderer using `metric-webgl` and custom 2D labels. | quarantine | It is explicitly outside this task's edit scope and should not be used as a public-hero fallback. | Do not touch here. Maintain as GRAE10-only until its owner schedules migration or exemption. | GRAE10 owner only. | `node visual/tools/check-grae10-golden.mjs` and `rg -n "mountGrae10MetricEngine" visual/examples docs/site` confirms limited scope. |
| `visual/examples/mnist-dimension-reduction/index.html:41` | `new Baby.Plots(...)` | Embedded Babylon/BabyPlots demo. | quarantine | It is a generated/external-style renderer and not compatible with the one-engine rule. | Remove from public gallery routing or replace with `metric.visual.v1` plus semantic views in a separate example cleanup. | Example archival worker, not GRAE10. | `! rg -n "Baby\\.Plots" visual/examples docs/site && ! rg -n "var Baby" visual/examples docs/site` for public gallery paths after cleanup. |
| `visual/src/interaction/record-preview.js:464` | `renderSparkline()` | 2D canvas inside runtime-managed record preview UI. | keep | This is an interaction adjunct sourced from `metric.visual.v1`, not an alternate scene renderer. | Keep, but do not use it to recreate page-level render panels. | Preview/interaction owner. | `node visual/tools/check-record-preview-presentation.mjs && node visual/tools/check-runtime-picking-preview.mjs` |
| `visual/src/layers/BillboardLabelLayer.js:143` and `:180` | Label atlas canvas | 2D texture generation inside a runtime layer. | keep | This canvas is an implementation detail of a `MetricVisualRuntime` layer. | Keep behind `BillboardLabelLayer`. | Layer owner. | `node --check visual/src/layers/BillboardLabelLayer.js && node visual/tools/check-views.mjs` |

## Next Three Non-Overlapping Cleanup Tasks

1. **Legacy facade export deletion**
   - Scope: `visual/src/metric-visual.js` legacy export block and
     `visual/examples/process-curve-condition-monitoring/index.html`.
   - Goal: remove all imports/usages of `MetricScene3D`, `MetricRecordGallery`,
     `MetricHeatmap`, `MetricQueryInspector`, `MetricProcessCurveApp`,
     `MetricSelection`, `loadMetricEvidence`, and `assertMetricEvidence`.
   - Validation: `rg` commands above plus preview/relation checks.

2. **Process-curve descriptor factory quarantine**
   - Scope: `visual/src/views/ProcessCurveSceneView.js`,
     `visual/examples/miniature-hero-frame/index.html`,
     `visual/examples/miniature-look-gallery/index.html` and targeted checks.
   - Goal: remove public/example calls into
     `createProcessCurveMiniatureLayerDescriptors()` or mark them explicitly as
     dev-only harnesses; keep `showProcessCurves()` as the public semantic path.
   - Validation: `node visual/tools/check-process-curve-scene-view.mjs`,
     `node visual/tools/check-single-render-pipeline.mjs`, and targeted `rg`
     scans showing no public page reaches process-curve descriptors directly.

3. **Direct runtime harness quarantine**
   - Scope: non-GRAE10 example/probe pages and standalone exports:
     `visual/examples/miniature-hero-frame/index.html`,
     `visual/examples/miniature-look-gallery/index.html`,
     `visual/examples/native-engine-probe/index.html`, and
     `visual/src/miniature-field/*`.
   - Goal: convert public-like pages to `createMetricVisual().show*()` or mark
     them dev-only; remove or hide standalone renderer exports not used by the
     runtime path.
   - Validation: `node visual/tools/check-public-gallery-evidence.mjs`,
     `node visual/tools/check-single-render-pipeline.mjs`, and targeted `rg`
     scans showing no accepted public page imports `MetricVisualRuntime`,
     `createLayerFromDescriptor`, or direct descriptor factories.
