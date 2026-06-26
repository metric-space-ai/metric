# METRIC Visual Engine Implementation Plan

This plan is about building a dependency-free browser render and data
visualization library for METRIC.

The goal is not to hand-build hero pages. The goal is a reusable engine that
lets users create impressive and useful visualizations of METRIC evidence with
simple commands.

The hero applications are proof cases for the engine. They exist to prove that
the render/data-visualization library works. If a hero can only be built by
custom page code, the engine has failed.

C++ remains the source of truth for algorithms. JavaScript imports evidence,
builds GPU buffers, renders scenes and supports interactive inspection.

## One-Sentence Product Definition

METRIC Visual is a dependency-free WebGL data visualization engine for finite
metric spaces: users export METRIC evidence, call a small visual API, and get
interactive, high-quality metric-space scenes, relation views, previews,
morphs, fields and solver/dynamics visuals.

Example target API:

```js
import { createMetricVisual } from "./visual/src/index.js";

const visual = await createMetricVisual({
  target: document.body,
  evidence: "condition-monitoring.metric.visual.json",
  view: "metric-space",
  style: "miniature",
});

visual.show({
  colorBy: "entropy",
  groundField: "density",
  labels: "regime",
  preview: "record",
});
```

That is the product. Hero pages are acceptance fixtures for this product.

## Current State

Status date: 2026-06-26

This plan is authoritative only with the following current-state constraints:

- The 60k GRAE10/MNIST view is a protected visual reference and must remain
  byte-stable against `visual/regression-baselines/grae10-metric-engine.sha256`.
- The UCR process-curve hero is real native evidence and may remain public.
- Native `metric.visual.v1` exporter assets now exist for condition
  monitoring, mixed records, cross-space dependency, mapping/dimensionality,
  dynamics/noise and relation matrix under `docs/examples/assets/*/`.
- The matching development example pages now load those native assets through
  `createMetricVisual`. Their old local `visual/examples/*/evidence.json`
  files remain synthetic renderer fixtures only.
- Synthetic fixtures may be used to develop renderer layers, but they are not
  application evidence and must not be published as real heroes.
- Native public evidence must carry explicit provenance. The native-scale gate
  requires `provenance.native_export === true`; a writer path, runtime label or
  source filename alone is not enough.
- Runtime inspection now routes record picking, relation-matrix picking,
  graph-edge picking, linked selection and record/pair preview through
  reusable engine modules. Stock point, glyph, ground-projection and heat-field
  record layers plus relation-edge, curve-ribbon and curve-tube layers expose
  GPU picking hooks, and the public browser gate verifies `gpu-picking` hits
  with record IDs or edge IDs for the native preview examples that use pickable
  record, field, graph-edge or ribbon-trajectory layers. Tube-curve picking is
  verified on the native engine probe because it is an internal engine example,
  not a public hero gate.
- Trajectory/path rendering now belongs to the semantic `TrajectoryPathView`.
  Condition monitoring, finite dynamics and process-curve previews must use
  that view for record paths, graph/transition evidence and exported timeline
  states instead of surface-local trajectory helpers.
- Scalar property-field rendering now belongs to the semantic
  `PropertyFieldView`. Metric-space, property, dynamics and condition-monitoring
  commands must use that view for exported entropy, density, anomaly, residual
  or uncertainty fields instead of surface-local field helpers.
- The relation-matrix grammar now has engine-level readability metadata and a
  WebGL tile-summary LOD texture derived from already exported matrix texture
  values. This reduces dense-matrix aliasing without adding DOM/SVG fallbacks,
  page-local canvas renderers or JavaScript relation computation.
- Relation matrices now render through the reusable `screen-readable-overlay`
  runtime phase. The matrix remains a WebGL layer and semantic relation view,
  but it is composited after the photographic 3D/post-FX pass so camera-depth
  DoF cannot blur a dense analytical matrix. This phase is an engine feature,
  not a page overlay or one-off exception.
- The mapping grammar must render residual/error evidence through
  `MappingView` descriptors. `MetricVisualSurface.showMapping()` must not add
  page-level residual fallbacks that can hide missing native residual
  properties.
- Mapping residual/error descriptors are emitted only when an explicit
  residual property is selected and finite residual values exist. A missing
  residual property must result in no residual/error layer and no preservation
  summary claim.
- `showConditionMonitoring()` is the public condition-monitoring command. It
  now composes the reusable `ProcessCurveSceneView` grammar directly; the public
  condition page must not call `showConditionMonitoring()` and then replace the
  scene with `showProcessCurves()`.
- `captureHeroFrame(visual, options)` is a top-level public API export that
  delegates to `MetricVisualSurface.captureHeroFrame(options)`. It does not
  create another capture or render pipeline.
- `MetricVisualSurface.setLayerDescriptors()` and
  `MetricVisualSurface.addLayerDescriptors()` are no longer public facade
  methods. Public code must use semantic `show*` commands or `setViews()` with
  semantic view objects; raw descriptor installation is internal-only.
- `showSolverTrace()` is now backed by native C++ `metric.visual.v1` evidence
  under `docs/examples/assets/solver-trace/metric.visual.json`, exported by
  `examples/engine/solver_trace_visual_export.cpp`.
- The old embedded Babyplots/Babylon
  `visual/examples/mnist-dimension-reduction/index.html` page has been deleted
  from active visual examples. MNIST dimension reduction remains represented by
  the protected METRIC-owned 60k GRAE10 engine page.
- The large-scene performance gate reads the public Visual-Brief manifest,
  covers all seven native public preview rows, reports minimum record targets
  and blockers per preview, and keeps `process-curve-external-hero` visibly
  scale-blocked while still testing its process-curve grammar performance.
- The next production step is to harden each native public preview into an
  accepted hero through larger evidence where required, grammar-specific
  performance budgets and screenshot review, not more hand-written hero HTML
  and not more synthetic fixture scenes.
- Browser load success means only `loads` or `renders`; it never means
  `hero-ready`.
- Every public preview must have a checked visual brief in
  `visual/hero-visual-briefs.manifest.json`; the brief states the visual claim,
  expected primary grammar, minimum evidence target, required primitives and
  explicit blockers that prevent hero acceptance. The brief also declares
  required runtime descriptor roles, and
  `visual/tools/check-hero-visual-briefs.mjs` rejects previews whose semantic
  view descriptors lose those roles.
- Every public preview must also pass
  `visual/tools/check-public-miniature-scene-contract.mjs`. This gate consumes
  the browser regression report and proves that the preview still uses the
  reusable photographic miniature render contract: camera-depth DoF, miniature
  frame, color grade, vignette, required primitives, semantic descriptor roles
  and billboard labels where label roles are declared.

The current accepted public hero set is therefore limited to:

- MNIST dimension reduction: protected 60k GRAE10 reference plus native
  integrity check.

The UCR process-curve evidence is real, native and public, but its current
Visual Engine page is still a review-pending preview rather than an accepted
hero screenshot.

The other planned hero families are currently public previews, not accepted
heroes. They load native `docs/examples/assets/*/metric.visual.json` evidence
through distinct reusable visual grammars and pass
`visual/tools/check-public-gallery-evidence.mjs` plus the browser public
regression gate, but still need screenshot acceptance and stronger public
evidence where the current native fixture is too small to prove application
value.

The integrated exporter-contract wave is tracked in:

```text
docs/visual/agent-tasks/visual-exporter-cli-contract.md
docs/visual/agent-tasks/redif-native-public-evidence.md
docs/visual/agent-tasks/relation-matrix-reproducibility-audit.md
```

These tasks are prerequisites for further hero promotion. They make exporter
output predictable, keep Redif native evidence from being labelled synthetic,
and keep public relation-matrix assets reproducible from C++.

## Goal

METRIC Visual must provide a simple, reusable API for metric-space data
visualization:

- load exported METRIC evidence without a package manager, build step or CDN
- render large record collections as metric-space scenes, not generic plots
- inspect original records through hover previews and selection panels
- show pair relations as matrices, graphs, projections and local neighborhoods
- show space properties such as density, entropy, representatives and outliers
- show mappings and dynamics as transitions between coordinate states
- show solver traces and optimization evidence without reimplementing solvers
- provide a reusable photographic miniature style for all hero applications
- expose high-level commands for common visual tasks, such as `showMetricSpace`,
  `showRelationMatrix`, `showNeighborhoodGraph`, `showSpaceProperties`,
  `showProcessCurves`, `showMapping`, `showDynamics`, `showPreview` and
  `captureHeroFrame`

Every hero application must have a hero visualization. Plan each hero
backwards from the visual proof it must produce:

1. what should a viewer understand within five seconds?
2. what visible structure proves the metric-space advantage?
3. which records, relations, properties and mappings are required as evidence?
4. which C++ exports must produce that evidence?
5. which reusable views and layers render it?
6. which screenshot or animation becomes the project-page asset?

Do not start a hero application from an algorithm name. Start from the visual
claim that will make the algorithm legible.

The visual library must be an extended METRIC library, not an ad hoc demo
folder. Renderer, material, postprocess, camera, picking and label behavior are
owned by METRIC and must be edited in the relevant METRIC modules.

## Primary Instruction: Build The Data Render Engine

Build the reusable data render engine first. Build simple public commands on
top of it. Use hero applications only to prove those commands and engine
capabilities.

The correct mental model is:

```text
METRIC evidence -> simple visual command -> reusable engine pipeline -> scene
```

The incorrect mental model is:

```text
Hero idea -> custom page script -> custom render helper -> screenshot
```

If the work produces a page but no reusable command or engine capability, it is
not progress.

An agent must implement engine capability first:

```text
metric.visual.v1 evidence
  -> data/indexing
  -> semantic view object
  -> layer descriptors
  -> MetricVisualRuntime
  -> WebGL layers/postprocess
  -> interaction/preview/selection
```

The correct deliverable is a reusable capability in `visual/src/`, proven by a
small example. The incorrect deliverable is a standalone page that renders one
case by bypassing the engine.

### Public API Target

The engine must grow toward a small, user-facing API:

```js
createMetricVisual(options)
showMetricSpace(evidence, options)
showRelationMatrix(evidence, options)
showNeighborhoodGraph(evidence, options)
showSpaceProperties(evidence, options)
showProcessCurves(evidence, options)
showMapping(evidence, options)
showDynamics(evidence, options)
showSolverTrace(evidence, options)
captureHeroFrame(visual, options)
```

These commands must be thin orchestration over:

- `visual/src/data/`
- `visual/src/views/`
- `visual/src/layers/`
- `visual/src/runtime/`
- `visual/src/interaction/`
- `visual/src/style/`

If a feature cannot be reached through such a command or a semantic view, it is
not part of the library yet.

### Data Render Engine Definition

The engine must provide these reusable capabilities:

- evidence loading and validation
- record, relation, coordinate and property indexing
- semantic view classes that produce layer descriptors
- GPU layer implementations for data primitives
- runtime composition through one `MetricVisualRuntime`
- picking and linked selection
- record and pair preview contracts
- camera/floor/focus/postprocess as engine-level features
- visual diagnostics and screenshot/performance tools

Examples may only configure and exercise these capabilities. If an example
contains unique rendering logic that belongs in `visual/src/`, the task is not
complete.

### Visual Grammar Engine

The engine is not a point-cloud renderer. It is a data visualization engine with
multiple visual grammars that share one runtime, one evidence model and one
interaction system.

Required visual grammars:

- metric-space point cloud: records as spatial states, labels and projections
- relation matrix: pair values as an ordered, readable matrix or tiled matrix
- neighborhood graph: sparse local relation structure
- property field: entropy, density, uncertainty, anomaly and other scalar
  properties as fields or surfaces
- typed glyph scene: heterogeneous records rendered with type-specific glyphs
- trajectory/path scene: record evolution, process paths and dynamics
- residual/error scene: mapping distortion, preservation error and local
  reconstruction evidence
- paired-space scene: two or more linked metric spaces with cross-space
  dependence evidence
- preview/inspection scene: original record and pair data shown from hover or
  selection

All grammars must flow through the same evidence -> semantic view -> descriptor
-> runtime -> layer pipeline. They must not collapse into one point-cloud view.

`MetricSpaceView` is the canonical grammar for dense spatial record clouds.
It is not the universal visualization for all hero applications.

### Hero Grammar Requirements

Each hero must have a primary visual grammar. Point clouds may support the hero,
but they must not be the only visual grammar except for the GRAE10 dense
metric-space baseline.

Required primary grammars:

- condition monitoring: property field + trajectory/path + record preview;
  point cloud is supporting context
- mixed records: typed glyph scene + cross-type relation edges + typed preview;
  point cloud is supporting context
- cross-space dependency: paired-space scene + dependence bridge/field +
  linked brushing; point clouds are supporting context
- mapping/dimensionality: morph scene + residual/error vectors or projection;
  point cloud alone is insufficient
- dynamics/noise: trajectory/path scene + propagation field + state-history
  preview; point cloud alone is insufficient
- relation matrix/neighborhood: readable relation matrix + graph + pair
  preview; point cloud is optional context

If all hero screenshots look like colored point clouds with different labels,
the engine work has failed.

### Simple-Command Acceptance

Every new engine feature must include a minimal command-level usage example.
For example:

```js
const visual = await createMetricVisual({ target, evidence });
visual.showMetricSpace({ colorBy: "density", labels: "cluster" });
visual.captureHeroFrame({ at: "target-hold" });
```

A feature is not accepted if it can only be used by manually wiring buffers,
layers, runtime and camera in an example page.

### Engine-First Rule

Do not build or modify hero applications until the missing engine capability is
implemented in `visual/src/`.

Examples:

- need better point rendering -> edit `MetricSpaceView`, point layer, material
  or runtime, not a page helper
- need labels -> implement label layer/view behavior, not HTML overlays in one
  page
- need relation matrix -> implement `RelationMatrixView` and matrix layer, not
  draw a one-off canvas in a hero page
- need hover data -> implement preview/selection contracts, not hard-coded page
  DOM
- need camera/floor/focus -> implement runtime/style contracts, not per-page
  camera hacks

The agent must name the engine module it is improving before writing example
code. If it cannot name the module, it must stop.

### Examples Are Acceptance Fixtures

Every example must be treated as an acceptance fixture for the engine:

- the page imports semantic views/runtime from `visual/src/`
- the page loads evidence
- the page calls simple commands or configures semantic views
- the page contains minimal shell code only

The page must not own renderer architecture.

## Architecture Contract

The only hard boundary between METRIC C++ and METRIC Visual is the evidence
format.

C++ exports:

- records and record metadata
- finite metric spaces and pair relation references
- metric-law validation evidence
- coordinate states produced by mappings or diagnostics
- neighborhoods, representatives, clusters and outliers
- entropy, density, intrinsic dimension and other space properties
- cross-space dependence/correlation evidence
- dynamics, sampling, Redif inverse-dynamics, diffusion and reverse-flow evidence
- solver traces, residuals, objectives and convergence states

JavaScript owns:

- schema validation and compatibility adapters
- indexed access to records, relations and visual channels
- typed-array encoding for GPU upload
- runtime scene graph, render targets and postprocess
- camera, focus, picking, brushing and selection state
- record preview panels and arbitrary data inspectors
- layer composition and view orchestration
- animation between already exported states
- visual regression and performance measurement

JavaScript must not compute METRIC algorithms.

## Public Evidence Gate

Hero pages are not accepted by render success alone. A visual example becomes a
public gallery item only after `visual/tools/check-public-gallery-evidence.mjs`
passes and the example points to native evidence or a documented reference
asset.

The gate enforces:

- the protected GRAE10 60k reference page keeps its accepted hash
- synthetic `visual/examples/*/evidence.json` fixtures are not linked from the
  public project page as live hero results
- synthetic fixtures are not marked `done` in the visual progress report

The current synthetic development fixtures are:

- `condition-monitoring-hero`
- `mixed-record-hero`
- `cross-space-dependency-hero`
- `mapping-dimensionality-hero`
- `dynamics-noise-hero`
- `relation-matrix-neighborhood`

These fixtures are allowed for engine development only. They must not be used
to prove METRIC's application value. Replace each one with native C++ exported
evidence before moving it into the public gallery.

### How To Read W4-W8

The W4-W8 sections below are visual grammar briefs and acceptance targets. They
are not evidence that those heroes already exist.

For each W4-W8 hero, implementation order is:

1. choose or document the dataset/source
2. add a native C++ exporter that writes `metric.visual.v1`
3. validate that export through `visual/src/data/schema.js`
4. render it through the matching semantic view(s)
5. run screenshot and interaction checks
6. publish the hero only after `check-public-gallery-evidence.mjs` passes

Any existing `visual/examples/<hero>/evidence.json` file with
`provenance.synthetic: true` is only a renderer fixture. It must not be treated
as the data source for a public hero.

## Module Responsibilities

`visual/src/data/`

- define `metric.visual.v1`
- validate evidence documents
- adapt existing `metric.evidence.v1`
- build record, relation, coordinate and property indexes

`visual/src/encoders/`

- convert evidence channels into typed arrays
- normalize scalar/categorical/vector values for rendering
- provide color, size, opacity and selection channels

`visual/src/runtime/metric-webgl/`

- own the browser 3D runtime
- own camera, scene, materials, render targets and postprocess primitives
- expose only METRIC runtime concepts
- keep renderer code editable, not isolated behind wrappers

`visual/src/layers/`

- render points, labels, ground, projections, edges, matrices, fields, curves,
  surfaces and glyphs
- provide stable layer contracts and diagnostics
- keep GPU-heavy rendering in layers, not examples

`visual/src/views/`

- compose layers into reusable semantic views
- provide views for metric spaces, relation matrices, graphs, properties,
  mappings, dynamics and solver traces

`visual/src/interaction/`

- pointer normalization
- hover picking
- record previews
- brushing and linked selection
- camera controls
- focus and depth-of-field interaction

`visual/src/style/`

- own the photographic miniature style
- define stage, floor, grid, labels, light, color, material response, shadows,
  focus and motion contracts

`visual/tools/`

- run browser checks
- measure rendering performance
- generate screenshots for visual regression
- validate schema fixtures and layer contracts

## Evidence Schema Milestones

### V1.0 Minimum Shape

Deliver:

- `schema`
- `records`
- `spaces`
- `relations`
- `coordinates`
- `properties`
- `views`
- `metadata`

Acceptance:

- one schema file documents all fields
- validator rejects missing record IDs, invalid channel lengths and broken
  relation references
- examples can load the same evidence through public data APIs

### Record Preview Contract

Deliver:

- generic preview resolver
- preview payload for text, numbers, arrays, time series and image references
- stable hover picking result with record ID, screen position and world position
- panel API that accepts arbitrary DOM renderers for custom record previews

Acceptance:

- hover over a point can show original record data
- examples do not hard-code preview layout inside render loops
- preview does not break scene performance at 60k records

### Relation Contract

Deliver:

- dense relation matrix source
- sparse relation graph source
- k-neighborhood source
- relation metadata for metric-law checks

Acceptance:

- same relation can feed a matrix, graph and neighborhood view
- relation views use the same record selection state
- invalid non-metric relation can be displayed as quarantined evidence without
  being advertised as a valid metric space

## Parallel Workstreams

These streams can run in parallel after the schema shape is fixed.

The current executable workstream briefs live under
`docs/visual/agent-tasks/`:

- `visual-command-api-gallery-workstream.md` for the public command API and
  project gallery wiring
- `relation-matrix-graph-grammar-workstream.md` for relation matrix, graph,
  pair preview and linked selection grammar
- `record-glyph-field-grammar-workstream.md` for mixed records, typed glyphs
  and property fields
- `mapping-dynamics-motion-grammar-workstream.md` for mapping, dynamics,
  timelines, residuals and solver traces
- `visual-regression-performance-workstream.md` for screenshot, interaction and
  performance gates

These briefs are the unit of parallel agent execution. A worker must follow the
write scope and stop rules in its brief before editing files.

### A. Data And Schema

Scope:

- finalize `metric.visual.v1`
- write schema validator
- add fixtures for valid and invalid documents
- write compatibility adapter from current C++ evidence exports

Deliverables:

- `visual/src/data/schema.js`
- `visual/src/data/visual-space.js`
- schema fixtures under `visual/examples/fixtures/`
- validator smoke tests in `visual/tools/`

Acceptance:

- all examples load through the same data API
- no example parses evidence ad hoc
- invalid evidence produces actionable validation errors

### B. Runtime And Postprocess

Scope:

- harden `metric-webgl`
- remove stale external naming from runtime code where legally possible
- make camera-depth focus physically coherent
- expose focus distance, aperture, circle-of-confusion and depth debug probes
- ensure labels can render inside or after postprocess intentionally

Deliverables:

- runtime diagnostics API
- camera-depth DoF checks
- stable label render path
- performance counters for frame time and GPU buffer sizes

Acceptance:

- no old global names or old renderer sentinels
- focus changes follow scene depth, not screen overlays
- visual check confirms floor, labels, points and previews remain coherent

### C. Layers

Scope:

- complete reusable GPU layers
- make every layer consume the same selection and style contracts
- support large point clouds, relation edges, matrices, heat fields, curves,
  ground projections and labels

Deliverables:

- layer diagnostics
- layer examples with shared evidence fixtures
- consistent buffer lifecycle and resize handling

Acceptance:

- layers can be composed without example-specific glue
- every layer reports resource usage and draw count
- no layer depends on a one-off page script

### D. Views

Scope:

- build user-facing view classes from layers
- keep views semantic, not algorithm implementations

Deliverables:

- `MetricSpaceView`
- `RelationMatrixView`
- `NeighborhoodGraphView`
- `SpacePropertiesView`
- `MappingView`
- `DynamicsView`
- `SolverTraceView`

Acceptance:

- each view has a minimal public constructor
- each view accepts `metric.visual.v1`
- selection and hover state can be linked across views

### E. Interaction And Inspection

Scope:

- record hover previews
- point, label, edge and matrix-cell picking
- brushing and linked selection
- camera controls that do not look like debug UI

Deliverables:

- generic `RecordPreviewPanel`
- selection store
- pick result model
- compact visual controls

Acceptance:

- hovering a point can show original record data
- selecting a cluster highlights linked matrix/graph/space views
- UI controls are subtle and production-presentable

### F. Hero Applications

Scope:

- build impressive examples that demonstrate finite metric spaces beyond vector
  nearest-neighbor search
- design the hero visualization before implementing the hero application

Deliverables:

- condition monitoring with entropy and anomaly evidence
- mixed-record metric space with record previews
- cross-space dependency/correlation scene
- mapping/dimensionality scene with 2D-to-3D morph
- dynamics/noise/reverse-flow scene
- relation matrix plus neighborhood graph scene

Acceptance:

- every hero app uses real METRIC evidence, not synthetic decoration only
- each app explains a specific metric-space capability through the scene
- each app can produce a screenshot suitable for project docs

### Hero Visualization Backward Plans

Each hero must begin with a visual brief.

#### 1. Condition Monitoring Hero

Visual claim:

An evolving process curve becomes a finite metric space whose local structure,
entropy and density expose abnormal operating states before a raw time-series
plot makes the failure obvious.

Hero visualization:

- miniature 3D process-state sculpture over a bounded floor
- records colored by operating regime
- local entropy/density shown as ground fields and lifted glow intensity
- anomaly windows shown as focused record paths
- hover previews show the original time-series window and measured features
- timeline morph shows healthy baseline, drift and failure emergence

Required evidence:

- windowed records
- metric values or neighborhood graph
- entropy/density per record
- anomaly score per record
- representative healthy/failure records
- original time-series snippets for preview

Reusable components needed:

- `MetricSpaceView`
- `SpacePropertiesView`
- curve/path layer
- ground projection and density field
- record preview with mini time-series

#### 2. Mixed-Record Hero

Visual claim:

Different record types can be compared through type-appropriate metrics and
inspected in one finite metric space without forcing all data into one
hand-built vector embedding first.

Hero visualization:

- heterogeneous record cloud with icons/glyphs for text, image, curve and table
  records
- cluster labels describe semantic families, not vector dimensions
- hover preview shows original record payload by type
- relation edges reveal cross-type nearest records
- a side matrix shows relation strength between record families

Required evidence:

- typed records with preview payloads
- metric-space coordinates
- nearest-neighbor links
- family labels
- type metadata
- relation matrix by family

Reusable components needed:

- typed record preview resolver
- glyph layer
- relation edge layer
- relation matrix view
- linked selection across point cloud and matrix

#### 3. Cross-Space Dependency Hero

Visual claim:

Two different metric spaces over paired observations can be correlated as
spaces, not by pretending that both spaces share the same vector coordinate
system.

Hero visualization:

- two synchronized miniature spaces
- brushed selection in one space highlights paired records in the other
- dependence evidence displayed as a relation bridge or shared ground band
- local dependency hot spots visible as colored paired regions
- hover preview shows paired records and their local contribution

Required evidence:

- paired record IDs
- coordinates for both spaces
- local dependence/correlation contributions
- global dependence statistic
- paired-neighborhood references

Reusable components needed:

- linked selection store
- paired `MetricSpaceView`
- dependence bridge layer
- record preview for paired records
- relation/heat field overlay

#### 4. Mapping And Dimensionality Hero

Visual claim:

A mapping is a transformation between finite metric-space representations. The
visual question is what structure is preserved, compressed or distorted.

Hero visualization:

- 2D-to-3D and source-to-target morph in the same scene
- preservation error shown as colored residual vectors or ground shadows
- stable clusters remain coherent during morph
- hover preview shows original record, source coordinates, target coordinates
  and local mapping error
- camera pauses in source state, quickly morphs, then pauses in target state

Required evidence:

- source coordinate state
- target coordinate state
- mapping residual/error per record
- cluster/family labels
- neighborhood preservation evidence
- original records for preview

Reusable components needed:

- coordinate-state morph timeline
- residual vector layer
- `MappingView`
- record preview
- screenshot capture with deterministic animation time

#### 5. Dynamics And Noise Hero

Visual claim:

Disorder, diffusion, Redif inverse-dynamics and reverse flow can be viewed as dynamics over a
finite metric space, not only as random perturbation in a vector coordinate
system.

Hero visualization:

- record states move over a metric graph/floor
- diffusion front shown as a soft propagation field
- reverse flow pulls records back toward representatives or low-cost states
- uncertainty shown as local spread and focus depth
- hover preview shows state history for a selected record

Required evidence:

- record states over time
- metric graph or transition graph
- diffusion/reverse-flow trajectory
- representative states
- uncertainty or transition cost per state

Reusable components needed:

- dynamics timeline view
- trajectory/path layer
- ground propagation field
- record state history preview
- animation controls with non-debug visual design

#### 6. Relation Matrix And Neighborhood Hero

Visual claim:

The finite metric space is fundamentally pairwise. The same relation can be
read as a matrix, graph, neighborhood structure and spatial scene.

Hero visualization:

- point cloud on top
- relation matrix as a tilted floor or linked side view
- selecting a point highlights its row/column and neighborhood graph
- local metric-law diagnostics visible as badges/overlays
- hover preview explains the selected pair relation

Required evidence:

- dense or tiled relation matrix
- sparse nearest-neighbor graph
- selected pair values
- metric-law diagnostics
- record previews

Reusable components needed:

- relation matrix view
- graph view
- linked point/matrix picking
- pair preview panel
- law-diagnostic overlay

### G. Visual Regression And Performance

Scope:

- browser checks for every example
- screenshot baselines
- performance matrix for record counts and layer combinations

Deliverables:

- `visual/tools/check-visual-regression-public-examples.mjs` as the
  authoritative public-example browser gate
- `visual/tools/check-visual-examples.mjs` as a targeted legacy smoke checker
  for named example folders
- screenshot capture artifacts under `output/visual/check-visual-regression-public-examples/`
- performance gates:
  `visual/tools/check-visual-performance-large-scenes.mjs` and
  `visual/tools/perf-matrix.mjs`
- baseline images for hero scenes

Acceptance:

- no public example ships without the public regression browser gate
- 60k-record point cloud remains interactive
- visual regressions are visible in generated artifacts
- loading a page is never hero acceptance; accepted heroes still require native
  evidence, grammar proof and screenshot review

### H. C++ Export Integration

Scope:

- add C++ visual export helpers without adding runtime dependencies
- ensure core C++ remains header-only/dependency-free where required

Deliverables:

- `mtrc::visual` export helpers
- examples exporting `metric.visual.v1`
- docs for exporting records, relations, properties and coordinates

Acceptance:

- C++ can export evidence without JavaScript tooling
- exported documents load directly in METRIC Visual
- examples do not require CMake-specific generated assets to be inspected

Current status:

- `visual/cpp/mtrc_visual.hpp` is the header-only C++17 writer foundation.
- `visual/tools/check-cpp-export.mjs` compiles and validates the standalone
  helper example.
- The six foundation native visual exporters are schema-valid and
  CTest-integrated.
- All six foundation native visual exporters now use the shared
  `mtrc::visual` writer path for common JSON/document/file writing.
  Domain-specific evidence is still composed explicitly with
  `visual::object(...)` where a generic helper would hide semantics.

Execution contract:

```text
docs/visual/agent-tasks/cpp-visual-export-core-workstream.md
```

## Execution Order

### Phase 1: Stabilize The Contract

1. Freeze initial `metric.visual.v1` shape.
2. Make GRAE10 and process-curve examples load through the same data API.
3. Add schema fixtures and validator checks.
4. Document the export contract for C++.

Parallelizable:

- Data/schema workstream
- Runtime diagnostics workstream
- Record preview workstream

### Phase 2: Build Reusable Views

1. Extract GRAE10 page logic into `MetricSpaceView`.
2. Extract relation matrix and neighborhood graph into reusable views.
3. Add linked selection across views.
4. Add stable preview payloads for records and relation cells.

Parallelizable:

- Layer hardening
- View construction
- Interaction and selection
- Browser checks

### Phase 3: Hero Applications

1. Condition monitoring hero.
2. Mixed-record hero.
3. Cross-space dependency hero.
4. Mapping/dimensionality hero.
5. Dynamics/noise hero.

Parallelizable:

- one implementation agent per hero
- one review agent for schema consistency
- one review agent for browser performance and screenshots

### Phase 4: Production Readiness

1. Visual regression baselines.
2. Performance benchmark matrix.
3. Documentation pages generated from real examples.
4. Public README examples updated to use real hero evidence.
5. Remove remaining one-off example logic.

Parallelizable:

- visual regression tooling
- docs integration
- example cleanup
- C++ export coverage

## Quality Gates

Every feature must pass:

- no npm, no CDN, no package-manager dependency
- no external renderer dependency
- no example-specific algorithm implementation in JavaScript
- browser check with no console errors
- screenshot or pixel-level visual evidence
- performance check for the largest relevant fixture
- schema validation for input evidence
- linked selection and hover behavior checked where applicable

## Agent Execution Contract

This section is the executable part of the plan. An agent must not treat a
render-smoke test as completion. A work item is complete only when it produces
the required files, screenshots, report and acceptance evidence.

### Engine Work Before App Work

Every work order must start by classifying itself:

```text
Work type: engine | fixture | example | documentation
Engine module changed:
Reusable API added or improved:
Example used only as acceptance fixture:
```

Hero/example work is invalid unless it points to a reusable engine module being
used or improved. A hero page that does not improve or exercise a reusable
engine path is out of scope.

### Single Render Pipeline Invariant

There is exactly one render pipeline, but it must support many visual
grammars:

```text
metric.visual.v1 evidence
  -> semantic view(s)
  -> layer descriptors
  -> MetricVisualRuntime
  -> METRIC WebGL layers
```

This means one runtime/descriptor architecture, not one view for every visual.
`MetricSpaceView` is mandatory for dense metric-space point-cloud components.
Other grammars must use their own semantic views and layers while sharing the
same `MetricVisualRuntime`.

Hero pages may configure data, properties, labels, camera, timeline and style,
but they must not create another renderer or page-local render helper.

Forbidden for metric-space heroes:

- `MetricSpaceHeroScene.js`
- `createMetricSpaceHeroScene`
- `createMetricSpaceHeroDescriptors`
- inline point/projection/label descriptor construction in example pages
- using `MetricSpaceView` as the only visual grammar for every hero
- direct `new MetricVisualRuntime()` setup in hero pages unless the page is a
  minimal shell around exported semantic views
- importing `PointCloudView`, `MorphView`, `GroundProjectionView` or
  `DenseFieldView` directly from a hero page to recreate `MetricSpaceView`
- copying GRAE10 render logic into another helper instead of moving it into
  `MetricSpaceView`

Allowed:

- `MetricVisualRuntime` as the engine
- `MetricSpaceView` as the dense point-cloud grammar
- additional semantic views such as `RelationMatrixView`,
  `NeighborhoodGraphView`, `SpacePropertiesView`, `PropertyFieldView`,
  `MappingView`, `TrajectoryPathView` and `DynamicsView`, composed through the
  same descriptor/runtime pipeline
- example pages as configuration shells only

If a hero needs a visual grammar missing from the engine, implement the grammar
as a reusable view/layer/runtime capability. Do not force it into
`MetricSpaceView`, and do not create a hero-specific render path.

### Canonical Visual Reference

Until another screenshot is explicitly accepted, the GRAE10 metric-space scene
is the visual baseline for dense point-cloud quality:

```text
visual/examples/grae10-metric-engine/index.html
```

Any new metric-space hero must be compared against the current GRAE10
screenshot for:

- frame fill
- point sharpness
- sphere/material response
- floor anchoring
- projection usefulness
- region labels
- hover preview stability
- absence of foggy alpha-disc clouds

If the new screenshot is visibly worse than GRAE10 on these dimensions, the
task is not visually accepted.

### Why This Contract Exists

Previous attempts failed in a specific pattern:

- agents built example pages instead of reusable renderer capability
- smoke tests passed while the visuals were not hero-worthy
- tiny synthetic point clusters were treated as finished hero scenes
- the same generic point-cloud template was reused for unrelated hero ideas
- parallel render helpers were created instead of improving `MetricSpaceView`
- the phrase "one pipeline" was misread as "one point-cloud view for every
  hero"; this is explicitly wrong
- existing non-point grammars such as matrix, graph, field, glyph, trajectory
  and residual layers were ignored
- GRAE10 quality was ignored instead of used as the baseline
- visual briefs were ignored after implementation started
- reports used words like `done` and `verified` for technical rendering only
- screenshots were captured but not used as acceptance evidence

This contract is written to prevent those failures. Any agent that cannot meet
the visual standard must stop and report the missing input or design decision.

### Completion Language Rules

Agents must use precise status language:

- `loads`: the page opens and resources load
- `renders`: WebGL produces non-empty pixels
- `interactive`: hover/selection/camera behavior works
- `visually accepted`: the screenshot satisfies the hero visual brief
- `complete`: all required artifacts, browser checks, screenshots and report
  are present and the screenshot is visually accepted

An agent must not write `done`, `complete`, `verified`, or `hero-ready` when it
only means `loads` or `renders`.

### Visual Acceptance Is Not A Smoke Test

Automated checks are necessary but insufficient. A browser checker may confirm:

- page status is 200
- no console errors
- WebGL context exists
- target data count is loaded
- hover preview appears
- screenshot is not blank

That still does not prove hero quality. The agent report must include a human
readable visual assessment of the screenshot against the work order's visual
claim. If the screenshot looks like a small cluster in empty space, the task is
failed even if every automated check is green.

### One-Hero Quality Bar Before Parallel Heroes

Do not assign W5-W8 in parallel until W4 or W6 has produced one visually
accepted hero screenshot. The accepted hero becomes the quality bar for later
heroes.

Parallel work before that point is allowed only for:

- schema and fixtures
- `MetricSpaceView` extraction
- deleting forbidden parallel render paths
- record preview system
- relation matrix/neighborhood view
- browser tooling
- C++ export helpers

### Evidence Density Rules

Hero pages must not use tiny placeholder datasets.

Minimums:

- point-cloud hero: at least 1,000 records
- dense flagship point-cloud hero: at least 10,000 records
- relation matrix hero: at least 128 records or a tiled/dense relation view
- graph hero: at least 500 nodes or enough edges to show structure
- time-series/process hero: at least 500 windows plus original snippets
- mixed-record hero: at least 2,000 records across at least four record types

If these minimums cannot be met with real exported evidence, the correct work
item is evidence generation/export, not a hero page.

### Composition Rules

Every hero screenshot must satisfy:

- the primary structure occupies a meaningful part of the viewport
- the scene has a deliberate camera angle, floor, labels and focus strategy
- the floor or relation surface carries information, not just decoration
- color encodes a named property, state, type or relation
- labels identify meaningful regions and are not debug overlays
- UI controls are subtle or hidden for the screenshot
- preview panels are shown only when they support the visual claim
- the screenshot can stand on the project page without an explanatory paragraph

Failure examples:

- a tiny point cluster centered in a huge empty frame
- points without labels, floor, relation field or property encoding
- one generic layout reused for unrelated hero concepts
- a side panel containing the explanation while the scene itself says nothing
- synthetic decoration that is not backed by evidence fields

### No Generic Hero Template

The shared runtime and style are reusable. The hero composition is not.

Each hero must choose its own primary visual grammar:

- condition monitoring: process-state sculpture, entropy/density fields,
  anomaly paths and time-series previews
- mixed records: heterogeneous glyphs, cross-type edges and typed previews
- cross-space dependency: paired spaces, linked brushing and dependence bridge
- mapping/dimensionality: source-target morph and residual/error evidence
- dynamics/noise: trajectory, propagation field and state-history preview
- relation matrix/neighborhood: matrix, graph and pair preview linked to a
  spatial view

If two heroes look like the same point cloud with different labels, at least one
of them is not acceptable.

### Preflight Required Before Implementation

Before writing page code, the agent must add a preflight section to its report:

- visual claim in one sentence
- evidence fields available
- record count and relation count
- record-type diversity where the brief requires typed records
- viewport composition sketch in words
- layer stack
- interaction plan
- screenshot target state
- expected failure modes

Implementation may start only after this preflight is filled in. If the agent
does not have enough evidence fields or record count, it must stop before
building the visual.

For public preview work, the preflight must also be reflected in
`visual/hero-visual-briefs.manifest.json` and pass
`node visual/tools/check-native-hero-evidence-scale.mjs` and
`node visual/tools/check-hero-visual-briefs.mjs`. A preview with insufficient
record count must explicitly carry the `record-count-below-hero-minimum`
blocker. A typed-record preview with insufficient type diversity must carry
`record-type-count-below-hero-minimum`. Neither case may be presented as an
accepted hero candidate.

### Global Stop Rules

Stop and report instead of claiming completion if any of these are true:

- the code creates or keeps a second metric-space point-cloud render path
- `createMetricSpaceHeroScene`, `MetricSpaceHeroScene.js`, or equivalent helper
  exists outside a migration branch
- a hero page bypasses `MetricSpaceView` to build point/projection/label
  descriptors directly
- the scene uses fewer than 1,000 records for a hero point-cloud unless the
  work order explicitly allows a sparse visual
- the scene renders as a small cluster in an empty frame
- the scene has no intentional floor, camera, focus, label and composition
  design
- the scene has no record preview or no path to original record data
- the browser check passes but the screenshot does not communicate the visual
  claim
- the screenshot was not inspected against the visual brief
- `done` or `verified` would only mean `renders`
- the work order lacks preflight evidence and composition notes
- the implementation adds npm, CDN, package-manager, build-tool or external
  renderer dependency
- the hero uses synthetic decoration without METRIC-style evidence channels
- the work requires algorithmic computation that belongs in C++

### Required Agent Report

Every agent must write a report before marking a work order complete:

```text
visual/reports/<work-order-id>.md
```

The report must contain:

- work order ID and short summary
- work type
- engine module changed
- reusable API added or improved
- example used as acceptance fixture
- preflight section
- single-pipeline check result
- files changed
- evidence fixture path
- screenshot paths
- browser command used
- console error summary
- frame/performance notes
- visual acceptance checklist
- visual failure checklist
- honest remaining gaps

The report must include this exact status block:

```text
Status:
- loads: yes/no
- renders: yes/no
- interactive: yes/no
- visually accepted: yes/no
- complete: yes/no
```

If `visually accepted` is `no`, then `complete` must also be `no`.

### Required Visual Artifacts

Every visual work order must produce:

- one runnable HTML example under `visual/examples/<work-order-id>/`
- one checked evidence source:
  - native hero/preview work uses
    `docs/examples/assets/<work-order-id>/metric.visual.json`
  - renderer-only development work may use
    `visual/examples/<work-order-id>/evidence.json`, but that fixture must be
    labeled synthetic and must not be promoted as application evidence
- one screenshot under `visual/output/<work-order-id>/`
- one report under `visual/reports/<work-order-id>.md`
- one browser check or script under `visual/tools/` or a documented command in
  the report

Every visual work order touching metric-space point clouds must also run:

```text
node visual/tools/check-single-render-pipeline.mjs
```

If that tool fails, the work order is blocked.

Every hero work order must also run:

```text
node visual/tools/check-hero-grammar-contract.mjs
```

If that tool fails, the hero is blocked. This guard does not prove visual
quality; it only prevents the specific failure mode where every hero is reduced
to the same point-cloud grammar.

### Hero-Ready Definition

A hero visualization is not hero-ready until all conditions below are true:

- the first viewport communicates one metric-space idea without reading docs
- the scene has enough records or structure to feel dense and intentional
- the composition fills the frame without clipping important data
- labels are readable and belong to the scene
- hover preview exposes original record evidence
- color encodes a named metric-space property, relation or state
- ground projection, matrix, graph, curve or field carries information
- camera, floor and depth/focus reinforce spatial understanding
- screenshot can be used on the project page without debug UI
- browser automation confirms load, render, hover and screenshot capture
- a second reviewer or review pass explicitly confirms the screenshot matches
  the visual brief

### Work Order Template

Each agent receives exactly one work order in this form:

```text
ID:
Goal:
Visual claim:
Input evidence:
Minimum evidence density:
Required output paths:
Preflight required:
Implementation scope:
Out of scope:
Acceptance:
Browser check:
Screenshot acceptance:
Report path:
```

No agent may broaden the scope. No agent may mark completion without the report
and screenshot.

## Executable Work Orders

These work orders are suitable for parallel execution after schema V1.0 is
stable. Until then, run only W0-W3.

### W-1: Remove Parallel Metric-Space Render Paths

Goal:

Delete all metric-space point-cloud render paths that bypass `MetricSpaceView`.

Visual claim:

Not applicable. This is an architecture cleanup gate.

Input evidence:

- current source tree

Required output paths:

- `visual/src/views/MetricSpaceView.js`
- `visual/tools/check-single-render-pipeline.mjs`
- `visual/tools/check-hero-grammar-contract.mjs`
- `visual/reports/W-1-single-render-pipeline.md`

Implementation scope:

- remove `MetricSpaceHeroScene.js`
- remove exports for `createMetricSpaceHeroScene` and
  `createMetricSpaceHeroDescriptors`
- update hero examples so they do not import forbidden helpers
- ensure all metric-space point clouds are built through `MetricSpaceView`
- add or update the guard script

Out of scope:

- making bad hero visuals look good
- adding new hero pages
- changing algorithms

Acceptance:

- guard script passes
- hero grammar guard fails on known bad generic point-cloud heroes until they
  are removed, quarantined or rewritten with required grammars
- no forbidden symbol exists outside this plan file and reports that explain
  historical cleanup
- GRAE10 still loads and renders
- no hero page builds point/projection/label descriptors inline
- hero pages are either removed, quarantined as failed artifacts, or rewritten
  as minimal shells over semantic views

Browser check:

- GRAE10 loads
- hover preview appears
- screenshot captured

Report path:

```text
visual/reports/W-1-single-render-pipeline.md
```

### W0: Visual Schema And Fixture Contract

Goal:

Create the minimum executable `metric.visual.v1` schema and fixtures used by
all hero work orders.

Work type:

```text
engine
```

Input evidence:

- current GRAE10 evidence
- current process-curve evidence
- one synthetic dense relation fixture generated only for visualization
  contract testing

Required output paths:

- `visual/src/data/schema.js`
- `visual/src/data/visual-space.js`
- `visual/examples/fixtures/metric-visual-minimal.json`
- `visual/examples/fixtures/metric-visual-relation.json`
- `visual/reports/W0-visual-schema.md`

Implementation scope:

- validator
- normalized indexes
- error messages
- fixture loading helper

Out of scope:

- hero page design
- algorithm implementation

Acceptance:

- fixture validation passes
- invalid record IDs fail with actionable errors
- GRAE10 and process-curve examples can be adapted without custom parsing
- schema/index API can be called from a simple command without page-local data
  parsing

Browser check:

- not required; use node-based schema checks

### W0.5: Public Visual API Shell

Goal:

Create the small public API that users and examples must call instead of
manually wiring renderer internals.

Work type:

```text
engine
```

Required output paths:

- `visual/src/metric-visual.js`
- `visual/src/index.js`
- `visual/examples/api-smoke/index.html`
- `visual/examples/api-smoke/evidence.json`
- `visual/output/W0.5-api-smoke.png`
- `visual/reports/W0.5-public-api-shell.md`

Implementation scope:

- `createMetricVisual(options)`
- `visual.showMetricSpace(options)`
- `visual.showRelationMatrix(options)` as a stub that reports unsupported until
  W3 wires it
- `visual.captureHeroFrame(options)`
- lifecycle methods: `start`, `stop`, `destroy`
- public diagnostics: loaded record count, view count, layer count, renderer
  state

Out of scope:

- custom hero pages
- duplicating `MetricVisualRuntime`
- adding alternate render helpers

Acceptance:

- API smoke example uses only `createMetricVisual()` and `showMetricSpace()`
  for the scene
- no page-local descriptor construction
- GRAE10 can later be ported to the same API
- browser check captures a non-empty screenshot

Browser check:

- open API smoke page
- wait for diagnostics
- capture screenshot

### W1: MetricSpaceView Extraction From GRAE10

Goal:

Extract the working GRAE10 point-cloud scene into a reusable `MetricSpaceView`
without lowering visual quality.

Work type:

```text
engine
```

Visual claim:

A dense finite metric space can be inspected as a miniature 3D scene with
stable labels, floor projection, hover preview and 2D-to-3D morph.

Input evidence:

- `visual/examples/grae10-metric-engine/grae10-data.json`

Required output paths:

- `visual/src/views/MetricSpaceView.js`
- `visual/src/metric-visual.js`
- `visual/examples/grae10-metric-engine/index.html`
- `visual/output/W1-grae10-metric-space.png`
- `visual/reports/W1-metric-space-view.md`

Implementation scope:

- move reusable render, morph, label, hover and camera logic into the view
- leave page-level code as a call to `createMetricVisual()` plus
  `showMetricSpace()` configuration only
- preserve current GRAE10 density, floor, labels and record preview
- make `MetricSpaceView` the only public point-cloud metric-space scene API

Out of scope:

- changing the visual style
- adding a second hero
- adding helper scene builders outside `MetricSpaceView`

Acceptance:

- GRAE10 still loads 60,000 records
- hover preview still appears
- GRAE10 page uses the public API or `MetricSpaceView`, not page-local
  descriptor wiring
- browser check has no console errors
- screenshot is at least as visually dense as the current GRAE10 page

Browser check:

- open GRAE10 page
- wait for record count
- move pointer into the cloud
- assert preview visible
- capture screenshot

### W2: Record Preview System

Goal:

Make record previews generic enough for text, image references, time series,
numeric arrays, pair relations and custom DOM renderers.

Work type:

```text
engine
```

Visual claim:

A point in the metric-space scene is an entry point back to the original
record, not just a plotted coordinate.

Input evidence:

- GRAE10 fixture
- small mixed-record fixture with text/image/curve/table preview payloads

Required output paths:

- `visual/src/interaction/record-preview.js`
- `visual/examples/record-preview-contract/index.html`
- `visual/examples/record-preview-contract/records.json`
- `visual/output/W2-record-preview.png`
- `visual/reports/W2-record-preview.md`

Implementation scope:

- preview schema
- safe text fields
- mini time-series canvas
- image reference rendering
- custom renderer hook
- hover stability

Out of scope:

- hero page composition

Acceptance:

- previews render all supported payload types
- no unsafe HTML insertion for default renderer
- custom renderer hook works
- hover remains stable under camera motion

Browser check:

- open preview contract page
- hover at least three record types
- capture screenshot with one visible preview

### W3: Linked Point Cloud And Relation Matrix

Goal:

Build the first linked multi-view scene: point cloud plus relation matrix plus
record/pair preview.

Work type:

```text
engine + acceptance fixture
```

Visual claim:

The finite metric space is pairwise. The same relation can be read as spatial
scene, matrix and neighborhood graph.

Input evidence:

- dense relation fixture from W0

Required output paths:

- `visual/src/views/RelationMatrixView.js`
- `visual/src/metric-visual.js`
- `visual/examples/relation-matrix-neighborhood/index.html`
- `visual/examples/relation-matrix-neighborhood/evidence.json`
- `visual/output/W3-relation-matrix-neighborhood.png`
- `visual/reports/W3-relation-matrix-neighborhood.md`

Implementation scope:

- relation matrix rendering
- point selection highlights row/column
- matrix-cell hover shows pair preview
- linked selection store
- reusable `RelationMatrixView`/selection APIs first, example second
- public `visual.showRelationMatrix()` command

Out of scope:

- all hero applications

Acceptance:

- selecting a point highlights spatial point and matrix row/column
- hovering matrix cell shows pair relation value and record IDs
- scene is visually composed, not debug-grid only
- example uses public commands/semantic views, not manual runtime wiring

Browser check:

- open example
- select one point
- hover one matrix cell
- capture screenshot

### W4: Condition Monitoring Hero

Goal:

Create the first actual hero application and screenshot from process-curve
evidence.

Work type:

```text
acceptance fixture for engine capabilities
```

Visual claim:

Entropy, density and metric neighborhoods expose abnormal process states as a
3D metric-space sculpture with original time-series previews.

Input evidence:

- process-curve windows
- entropy/density/anomaly properties
- neighborhood or relation data
- original time-series snippets

Minimum evidence density:

- at least 500 windows
- at least three operating regimes
- at least one drift/failure interval
- time-series preview payloads for selected records

Required output paths:

- `visual/examples/condition-monitoring-hero/index.html`
- page reads native export target:
  `docs/examples/assets/condition-monitoring/metric.visual.json`
- `visual/output/W4-condition-monitoring-hero.png`
- `visual/reports/W4-condition-monitoring-hero.md`

Implementation scope:

- dense process-state sculpture
- entropy/density ground field
- anomaly path/focus layer
- time-series record preview
- deterministic hero screenshot state
- any missing renderer capability must be implemented in `visual/src/` first

Out of scope:

- creating fake algorithm results in JavaScript
- page-local rendering shortcuts
- new scene-builder helpers

Acceptance:

- screenshot communicates healthy, drift and failure regions
- anomaly evidence is visible without reading a side panel
- hover preview shows original time-series window
- scene contains enough records to fill the frame
- floor projection or field carries entropy/density information
- the page is not a generic point-cloud template

Browser check:

- open page
- wait for loaded evidence
- force deterministic hero timestamp
- hover anomaly record
- capture screenshot

### W5: Mixed-Record Hero

Goal:

Show heterogeneous records in one finite metric-space scene with type-specific
previews.

Work type:

```text
acceptance fixture for engine capabilities
```

Visual claim:

If two records can be compared by an appropriate metric, their relationships can
be explored in one finite metric-space workflow regardless of original type.

Input evidence:

- at least 2,000 records across text, image-like, curve and table-like payloads
- type metadata
- family labels
- nearest-neighbor edges
- original preview payloads

Required output paths:

- `visual/examples/mixed-record-hero/index.html`
- page reads native export target:
  `docs/examples/assets/mixed-records/metric.visual.json`
- `visual/output/W5-mixed-record-hero.png`
- `visual/reports/W5-mixed-record-hero.md`

Implementation scope:

- glyph/type layer
- family labels
- cross-type relation edges
- type-specific record preview
- any missing renderer capability must be implemented in `visual/src/` first

Out of scope:

- claiming real multimodal ML results
- using tiny placeholder point clouds
- page-local rendering shortcuts
- new scene-builder helpers

Acceptance:

- type families are visually distinct
- cross-type relation edges are visible
- preview proves original record heterogeneity
- screenshot is dense enough for project-page use
- at least four record types are visible in the scene
- the relation matrix or edge layer proves cross-type comparability

Browser check:

- open page
- hover one record of each type
- capture screenshot

### W6: Mapping And Dimensionality Hero

Goal:

Show source and target coordinate states as a transformation with visible
preservation error.

Work type:

```text
acceptance fixture for engine capabilities
```

Visual claim:

A mapping is a metric-space transformation; the visual question is what
structure is preserved, compressed or distorted.

Input evidence:

- source coordinate state
- target coordinate state
- mapping residual per record
- neighborhood preservation property
- original records for preview

Minimum evidence density:

- at least 1,000 records
- at least three visible structure families
- residual/error values for every rendered record
- source and target states for the same record IDs

Required output paths:

- `visual/examples/mapping-dimensionality-hero/index.html`
- page reads native export target:
  `docs/examples/assets/mapping-dimensionality/metric.visual.json`
- `visual/output/W6-mapping-dimensionality-hero.png`
- `visual/reports/W6-mapping-dimensionality-hero.md`

Implementation scope:

- timed 2D-to-3D/source-to-target morph
- residual vectors or colored ground projection
- hover preview with local error
- deterministic screenshot at source, morph and target states
- any missing renderer capability must be implemented in `visual/src/` first

Out of scope:

- implementing the mapping algorithm in JavaScript
- page-local rendering shortcuts
- new scene-builder helpers

Acceptance:

- morph timing has hold, quick transition and hold
- preservation error is visible in the scene
- screenshot shows nontrivial structure and labels
- source, transition and target screenshots tell different visual states
- the residual/error layer is visible without a side panel

Browser check:

- capture three screenshots: source hold, transition, target hold

### W7: Cross-Space Dependency Hero

Goal:

Show paired observations in two metric spaces with linked dependence evidence.

Work type:

```text
acceptance fixture for engine capabilities
```

Visual claim:

Dependence can be inspected between metric spaces without forcing both spaces
into one shared vector coordinate system.

Input evidence:

- paired record IDs
- coordinates for both spaces
- global dependence statistic
- local contribution values
- paired-neighborhood references

Minimum evidence density:

- at least 500 paired observations
- two coordinate spaces with different visible geometry
- local contribution values for selected regions
- global statistic exported as evidence

Required output paths:

- `visual/examples/cross-space-dependency-hero/index.html`
- page reads native export target:
  `docs/examples/assets/cross-space-dependency/metric.visual.json`
- `visual/output/W7-cross-space-dependency-hero.png`
- `visual/reports/W7-cross-space-dependency-hero.md`

Implementation scope:

- two linked metric-space scenes
- paired highlight behavior
- local contribution field or bridge layer
- paired record preview
- any missing renderer capability must be implemented in `visual/src/` first

Out of scope:

- recomputing dependence statistics in JavaScript
- page-local rendering shortcuts
- new scene-builder helpers

Acceptance:

- selection in one space visibly affects the other
- local dependence hot spots are visible
- global statistic is shown as evidence, not as decoration
- the two spaces do not collapse into one generic shared point cloud

Browser check:

- open page
- select one region
- assert paired highlights
- capture screenshot

### W8: Dynamics And Noise Hero

Goal:

Show diffusion, Redif inverse-dynamics or reverse-flow evidence as dynamics over a finite
metric space.

Work type:

```text
acceptance fixture for engine capabilities
```

Visual claim:

Noise-like behavior can be interpreted as movement over a metric relation
structure; reverse flow shows how states return toward lower-cost structure.

Input evidence:

- record states over time
- transition graph or metric graph
- trajectory per selected record
- uncertainty or transition cost values
- representative states

Minimum evidence density:

- at least 500 records
- at least 12 timeline states or enough keyframes to show motion
- trajectories for selected records
- transition cost or uncertainty values

Required output paths:

- `visual/examples/dynamics-noise-hero/index.html`
- page reads native export target:
  `docs/examples/assets/dynamics-noise/metric.visual.json`
- `visual/output/W8-dynamics-noise-hero.png`
- `visual/reports/W8-dynamics-noise-hero.md`

Implementation scope:

- trajectory layer
- propagation field
- reverse-flow animation
- state-history preview
- any missing renderer capability must be implemented in `visual/src/` first

Out of scope:

- simulating fake dynamics without evidence channels
- page-local rendering shortcuts
- new scene-builder helpers

Acceptance:

- motion reads as a metric-space process
- ground field changes with state
- hover preview shows record history
- screenshots at start, middle and end are visually distinct
- reverse-flow or propagation evidence is visible in the scene

Browser check:

- capture screenshots at start, middle and end state

## Immediate Next Tasks

1. Keep `visual/examples/grae10-metric-engine/index.html` byte-stable against
   `visual/regression-baselines/grae10-metric-engine.sha256`.
2. Keep the public project page limited to checked native evidence:
   60k MNIST/GRAE10 and UCR process curves.
3. Use the checked native `metric.visual.v1` assets as the only evidence source
   for the preview heroes; do not add page-local `evidence.json` fixtures.
4. Polish each native preview into a distinct visual grammar through
   `createMetricVisual` or semantic views.
5. Add or strengthen browser screenshot checks for each native preview grammar.
6. Add a hero to the public page only after the evidence gate and screenshot
   checks pass.

Current follow-up status:

- `project-site-copy-contract` is integrated. The public project page is guarded
  by `visual/tools/check-project-site-copy-contract.mjs`, which keeps visible
  copy in portfolio language and reserves internal review state for tooling
  attributes.
- `redif-preview-integration` is integrated. Redif metric-dynamics native
  evidence is rendered by
  `visual/examples/redif-metric-dynamics-preview/index.html` through
  `showDynamics()` from the native
  `docs/examples/assets/redif-metric-dynamics/metric.visual.json` asset. This
  is an engine preview, not a public hero promotion.
- `process-curve-real-data-inventory` is integrated. The local machine has only
  48 distinct real UCR-derived source windows, so
  `missing-real-source-windows-for-500-record-hero` remains the correct blocker.

## Implementation Checkpoint: Engine Command Layer

Status date: 2026-06-24

Implemented:

- Removed the parallel metric-space hero scene helper from `visual/src/views/`.
- Removed the public export for the forbidden helper.
- Added the public command surface in `visual/src/metric-visual.js`:
  `createMetricVisual`, `showMetricSpace`, `showRelationMatrix`,
  `showNeighborhoodGraph`, `showSpaceProperties`, `showMapping`,
  `showDynamics`, `showProcessCurves`, `showSolverTrace`, `showPreview` and the
  `MetricVisualSurface` command object.
- Rewired the current hero fixtures so page code calls engine commands instead
  of owning renderer construction.
- Added architecture guards:
  `visual/tools/check-single-render-pipeline.mjs` and
  `visual/tools/check-hero-grammar-contract.mjs`.
- Wired `RecordPreviewPanel` into `MetricVisualSurface` with a CPU hover
  fallback over current view positions, so examples can expose original record
  payloads without page-local preview DOM.
- Added `visual/tools/check-public-gallery-evidence.mjs` to prevent synthetic
  visual fixtures from being published as real heroes and to protect the GRAE10
  reference hash.
- Added `visual/tools/check-native-hero-evidence-scale.mjs` to compare public
  preview briefs against native assets before browser rendering. The gate
  enforces native provenance plus record-count, relation-count and record-type
  blocker consistency.

Verified:

- `node visual/tools/check-single-render-pipeline.mjs`
- `node visual/tools/check-hero-grammar-contract.mjs`
- `node visual/tools/check-views.mjs`
- `node visual/tools/check-grae10-golden.mjs`
- `node visual/tools/check-public-gallery-evidence.mjs`
- `node visual/tools/check-native-hero-evidence-scale.mjs`
- `node visual/tools/check-visual-regression-public-examples.mjs`
- `node visual/tools/check-visual-performance-large-scenes.mjs`
- `METRIC_PERF_COUNTS=1000,10000,60000 node visual/tools/perf-matrix.mjs`
- `node visual/tools/verify-miniature-rig.mjs`
- `node visual/tools/verify-postfx-depth-dof-contract.mjs`
- targeted legacy smoke checks:
  `METRIC_VISUAL_EXAMPLES=grae10-metric-engine node visual/tools/check-visual-examples.mjs`
  and
  `METRIC_VISUAL_EXAMPLES=condition-monitoring-hero node visual/tools/check-visual-examples.mjs`

Corrected status:

- The six former public preview families now have native `metric.visual.v1`
  evidence at their configured scale thresholds: condition monitoring, mixed
  records, cross-space dependency, relation-matrix/neighborhood, dynamics and
  mapping.
- They are still not accepted as public heroes until screenshot review accepts
  the visual grammar and composition.
- They prove native export coverage and scale only; they are not evidence that
  the public gallery presentation is finished.
- Browser load success is not a substitute for native evidence and visual
  acceptance.

Not visually accepted yet:

- `RelationMatrixLayer` has a reusable readability pass with block/tile
  diagnostics and selection focus. It remains pending human screenshot
  acceptance as a final hero asset.
- `MixedRecordView` is now a reusable semantic view with typed-glyph
  descriptors and cross-type relation-edge evidence. The shared glyph layer now
  carries type-specific geometry and material channels for silhouettes,
  material response and picking masks. It now has public-scale mixed-record
  evidence and still needs accepted screenshot quality.
- `CrossSpaceView` is now a reusable paired-space semantic view with side
  spaces, exported pair bridge descriptors and linked-selection metadata.
  Runtime inspection now carries linked-selection presentation payloads for
  metric-space records, matrix cells, graph edges and paired-space bridges.
  `RelationEdgeLayer` now consumes that payload to visually emphasize selected
  graph edges and paired-space bridges through the shared runtime layer path.
- `DynamicsView` now emits timeline state/control descriptors, user-facing
  scrubber/playback/reset metadata, deterministic timeline samples and active
  ground/field state from exported timeline-step properties. The browser
  surface can now attach a descriptor-driven timeline widget; screenshot and
  readability acceptance remain open.
- Record and pair preview now share the runtime inspection path and resolve
  payload families, record properties, linked memberships, relation-independent
  pair properties and symmetric native pair values. Basic bounded presentation
  is integrated for text, time-series, histograms, compact image arrays,
  composed records and generic properties. Remaining preview work is richer
  media/table renderers, not page-local preview paths.
- `RelationMatrixLayer` has a first WebGL readability pass: block ranges render
  as subtle same-block bands plus explicit boundaries, dense cells use
  pixel-footprint smoothing, and selection differentiates row, column and cell.
  Matrix hits carry relation id/name, native pair identity and pair properties
  into the shared runtime pair-preview path.
- `RelationMatrixLayer` now also consumes reusable block/tile/LOD readability
  metadata for weighted dense-cell smoothing, logical tile boundaries and
  explicit no-DOM fallback diagnostics. It is still not hero-accepted until
  screenshot review proves readability at the public target scale.
- Relation-matrix readability diagnostics now also carry the exported block
  labels and full block-coverage state. The public browser regression for
  `relation-matrix-neighborhood` fails if those named block diagnostics or the
  tile-summary LOD source disappear from runtime state.
- Relation-matrix descriptors and runtime layers now use stronger engine-level
  focus defaults for row, column, selected cell, block boundaries and tile
  boundaries. The WebGL layer reports explicit block shader capacity and
  truncation state through `shaderCapacity` instead of silently hiding block
  range/boundary overflow.
- The large-scene performance gate now covers the point-cloud stress rows and
  current public preview grammar rows for field, glyph, relation-matrix/graph,
  dynamics, mapping, cross-space and process-curve scenes. These rows prove
  runtime mechanics, not hero acceptance.

Next mandatory engine work:

1. Promote the native UCR/process-curve export from command-backed preview into
   a polished hero-specific visual grammar only after screenshot review proves
   the composition. The native exporter and preview page exist; the visual is
   not accepted yet.
2. Keep the explicit hero-acceptance manifest as the only path from native
   preview to accepted hero, so screenshot review distinguishes accepted heroes
   from load/render success.
3. Promote native preview exports into polished hero-specific visual grammars
   only after screenshot review.
4. Extend layer-specific matrix picking to a GPU/tiled strategy only where the
   semantic matrix picker is not sufficient at larger scales.
5. Continue `RelationMatrixLayer` readability toward hero acceptance:
   streamed/tiled large-matrix rendering if needed, screenshot review,
   row/column focus under dense matrices, no moire-like artifacts and a
   composition that makes the named blocks legible without page-local overlays.
6. Add mesh-backed relation/bridge edges only if screenshot review proves
   color/alpha emphasis is not enough under browser line-width limits.
7. Re-run browser screenshots and only then decide which hero is allowed to
   become the first visual quality bar.

## Implementation Checkpoint: Relation Edge Emphasis

Status date: 2026-06-25

Implemented:

- `RelationEdgeLayer` now implements `setSelection(selection)` and receives
  runtime selection state through the existing `applySelectionToLayers()` path.
- Graph-edge and paired-space bridge matches are resolved from selected pair
  identity, selected record endpoints, edge ids and
  `metric.visual.linked_selection_presentation.v1` features.
- The normal edge shader consumes a per-vertex `aEdgeEmphasis` attribute to
  boost selected or related edges and dim unrelated edges during an active
  selection. The picking pass stays on the existing position/pick-color path.

Verified:

- `node visual/tools/check-linked-edge-emphasis.mjs`
- `node visual/tools/check-linked-selection-presentation.mjs`
- `node visual/tools/check-runtime-picking-preview.mjs`
- `node visual/tools/check-single-render-pipeline.mjs`

Corrected status:

- This is a reusable runtime/layer capability, not a hero acceptance.
- Edge emphasis is color/alpha based. Per-edge thickness remains limited by
  browser/WebGL line-width behavior until a future mesh-backed edge layer exists.

## Implementation Checkpoint: Glyphs, Selection Presentation And Screenshot Gate

Status date: 2026-06-25

Implemented:

- Mixed-record glyphs now carry typed geometry and material semantics through
  `glyphGeometry` and `glyphMaterial` channels into `InstancedGlyphLayer`.
- Runtime record/pair inspection now exposes linked-selection presentation
  payloads for metric-space records, relation-matrix cells, graph edges and
  paired-space bridges.
- `MetricVisualSurface.showDynamics()` can attach a descriptor-driven timeline
  widget that scrubs exported timeline states without computing dynamics in
  JavaScript.
- Hero screenshot review is now an explicit gate over browser regression
  artifacts. It keeps GRAE10 accepted and all other public previews
  review-pending until manual screenshot acceptance exists.

Verified:

- `node visual/tools/check-glyph-record-grammar.mjs`
- `node visual/tools/check-mixed-glyph-geometry.mjs`
- `node visual/tools/check-runtime-picking-preview.mjs`
- `node visual/tools/check-linked-selection-presentation.mjs`
- `node visual/tools/check-dynamics-timeline-control.mjs`
- `node visual/tools/check-timeline-control-widget.mjs`
- `node visual/tools/check-visual-command-api.mjs`
- `node visual/tools/check-views.mjs`
- `node visual/tools/check-single-render-pipeline.mjs`
- `node visual/tools/check-hero-screenshot-review.mjs`

Corrected status:

- These changes are engine capability progress. They do not promote condition
  monitoring, mixed records, cross-space dependency, relation matrix, dynamics
  or mapping to hero-accepted.
- Remaining hero work is visual curation, public-scale evidence and screenshot
  acceptance, not additional one-off page rendering paths.

## Implementation Checkpoint: Inspection, Readability And Timeline Controls

Status date: 2026-06-25

Implemented:

- Shared record/pair preview now resolves exported payload families, record
  properties, coordinate/view memberships, row/column record context,
  relation-independent pair properties and symmetric native pair values from
  `metric.visual.v1` evidence.
- Relation-matrix readability now has reusable block/tile/LOD metadata,
  weighted dense-cell smoothing, logical tile boundary cues, separate
  row/column/cell selection colors and explicit no-DOM fallback diagnostics.
- Dynamics timeline control descriptors now expose scrubber/playback/reset
  metadata and active ground/field state from exported timeline-step
  properties.

Verified:

- `node visual/tools/check-record-preview-payloads.mjs`
- `node visual/tools/check-runtime-picking-preview.mjs`
- `node visual/tools/check-relation-matrix-picker.mjs`
- `node visual/tools/check-relation-matrix-readability.mjs`
- `node visual/tools/check-dynamics-timeline-control.mjs`
- `node visual/tools/check-timeline-motion-contract.mjs`
- `node visual/tools/check-dynamics-motion-grammar.mjs`
- `node visual/tools/check-visual-command-api.mjs`
- `node visual/tools/check-views.mjs`
- `node visual/tools/check-single-render-pipeline.mjs`
- `node visual/tools/check-hero-grammar-contract.mjs`
- `node visual/tools/check-public-gallery-evidence.mjs`
- `node visual/tools/check-grae10-golden.mjs`
- `node visual/tools/check-visual-regression-public-examples.mjs`
- `node visual/tools/check-visual-performance-large-scenes.mjs`

Corrected status:

- These slices are engine capability progress. They do not promote any preview
  to hero-accepted.
- The next acceptance bottleneck is visual screenshot review and public-scale
  evidence quality, not another custom page script.

## Implementation Checkpoint: Semantic View Extraction And Schema Gates

Status date: 2026-06-25

Implemented:

- `MixedRecordView` under `visual/src/views/` owns the heterogeneous-record
  grammar. `showMixedRecords()` now delegates to that semantic view instead of
  mutating a metric-space point-cloud descriptor inside the command.
- `CrossSpaceView` under `visual/src/views/` owns the paired-space grammar.
  `showCrossSpace()` now delegates to that semantic view instead of composing
  two metric-space views and a bridge inline in the command.
- `DynamicsView` now carries exported timeline state/control metadata through
  descriptors, including deterministic start/middle/end timeline samples.
- `metric.visual.v1` validation now rejects malformed dense relation shapes,
  relation values whose endpoints are outside `relation.record_ids`, invalid
  coordinate-position dimensions/values and invalid pair-property endpoints.
- `visual/examples/cross-space-dependency-hero/index.html` reads record count
  from the paired-space contract instead of assuming the first view is a
  metric-space point cloud.

Verified:

- `node visual/tools/check-schema-fixtures.mjs`
- `node visual/tools/check-visual-document.mjs docs/examples/assets/cross-space-dependency/metric.visual.json`
- `node visual/tools/check-glyph-record-grammar.mjs`
- `node visual/tools/check-cross-space-linked-selection.mjs`
- `node visual/tools/check-dynamics-motion-grammar.mjs`
- `node visual/tools/check-timeline-motion-contract.mjs`
- `node visual/tools/check-visual-command-api.mjs`
- `node visual/tools/check-views.mjs`
- `node visual/tools/check-single-render-pipeline.mjs`
- `node visual/tools/check-grae10-golden.mjs`
- `node visual/tools/check-public-gallery-evidence.mjs`
- `node visual/tools/check-hero-grammar-contract.mjs`

Corrected status:

- These changes are engine capability progress, not hero acceptance.
- `MixedRecordView`, `CrossSpaceView` and `DynamicsView` now have clearer
  semantic contracts, but their public pages remain preview-only until
  screenshot review accepts the visual result.

## Implementation Checkpoint: Preview Presentation, Matrix Diagnostics And Gate Rerun

Status date: 2026-06-25

Implemented:

- Record/pair preview presentation now covers exported text, time-series,
  histogram/distribution, compact image-array, composed-record and generic
  property payloads with bounded field counts, bounded text and compact visual
  summaries.
- Preview diagnostics expose renderer choice, truncation, summarization and
  unavailable payload states as engine data. They are not rendered as
  user-facing debug badges in the hover panel.
- Relation-matrix descriptors and runtime layers expose
  `metric.visual.relation_matrix_readability_diagnostics.v1` with matrix
  dimensions, named block ranges, block coverage, tile counts, tile-summary
  source, missing-value count, selected row/column/cell state and explicit block
  shader capacity/truncation diagnostics.
- The task registry marks preview presentation, relation-matrix readability and
  visual acceptance gates as integrated after local rerun.

Verified:

- `node visual/tools/check-record-preview-payloads.mjs`
- `node visual/tools/check-record-preview-presentation.mjs`
- `node visual/tools/check-runtime-picking-preview.mjs`
- `node visual/tools/check-relation-matrix-readability.mjs`
- `node visual/tools/check-relation-matrix-picker.mjs`
- `node visual/tools/check-visual-command-api.mjs`
- `node visual/tools/check-views.mjs`
- `node visual/tools/check-single-render-pipeline.mjs`
- `node visual/tools/check-public-gallery-evidence.mjs`
- `node visual/tools/check-grae10-golden.mjs`
- `node visual/tools/check-visual-performance-large-scenes.mjs`
- `node visual/tools/check-visual-regression-public-examples.mjs`
- `node visual/tools/check-hero-screenshot-review.mjs`

Corrected status:

- GRAE10 remains the only accepted public hero.
- Condition monitoring, mixed records, cross-space dependency,
  relation-matrix/neighborhood, dynamics and mapping remain native
  `public-preview-only` examples pending screenshot acceptance.
- The external UCR/process-curve preview has a native `metric.visual.v1` export
  and command-backed preview, but remains explicitly blocked by source-record
  count until more real licensed source windows are available. It must not be
  promoted by synthetic padding.

## Implementation Checkpoint: Command Fidelity And Residual Evidence Guards

Status date: 2026-06-26

Implemented:

- `MappingView` no longer emits residual/error vectors or preservation-summary
  claims unless an explicit residual property is selected and finite residual
  values exist for rendered records.
- `MetricVisualSurface.showMapping()` owns mapping motion progress. The mapping
  preview page no longer mutates mapping layer progress through a page-local
  animation loop; normal mode animates through the engine, screenshot frames set
  deterministic static progress through the command options.
- `MetricVisualSurface.showConditionMonitoring()` now composes the reusable
  `ProcessCurveSceneView` grammar directly. The public condition-monitoring
  preview uses a single `showConditionMonitoring()` command and no longer calls
  `showProcessCurves()` as a second render path.
- The public API now exports `captureHeroFrame(visual, options)`, a thin
  delegation layer over `MetricVisualSurface.captureHeroFrame(options)`.
- The condition-monitoring acceptance gate now checks the final
  `showConditionMonitoring()` descriptor state, not just source-code command
  presence.
- The mapping-motion gate now includes a negative evidence case: a mapping
  without residual property must not create residual/error layers.
- The hero screenshot review gate now writes
  `output/visual/check-hero-screenshot-review/index.html`, a static review
  gallery over the real browser page/canvas screenshots. The gallery includes
  visual claims, expected grammar, native-evidence state and open blockers. It
  is review evidence, not automatic hero acceptance.

Verified:

- `node visual/tools/check-mapping-motion-grammar.mjs`
- `node visual/tools/check-mapping-frame-screenshots.mjs`
- `node visual/tools/check-condition-monitoring-visual-acceptance.mjs`
- `node visual/tools/check-trajectory-path-view.mjs`
- `node visual/tools/check-process-curve-scene-view.mjs`
- `node visual/tools/check-view-reference-contract.mjs`
- `node visual/tools/check-visual-command-api.mjs`
- `node visual/tools/check-single-render-pipeline.mjs`
- `node visual/tools/check-hero-grammar-contract.mjs`
- `node visual/tools/check-public-gallery-evidence.mjs`
- `node visual/tools/check-native-hero-evidence-scale.mjs`
- `node visual/tools/check-grae10-golden.mjs`
- `node visual/tools/check-visual-regression-public-examples.mjs`
- `node visual/tools/check-hero-visual-briefs.mjs`
- `node visual/tools/check-schema-fixtures.mjs`
- `node visual/tools/check-visual-document.mjs docs/examples/assets/condition-monitoring/metric.visual.json`
- `node visual/tools/check-visual-document.mjs docs/examples/assets/mapping-dimensionality/metric.visual.json`
- `node visual/tools/check-visual-performance-large-scenes.mjs`
- `node --check visual/tools/check-hero-screenshot-review.mjs`
- `node visual/tools/check-hero-screenshot-review.mjs`
- generated gallery image-reference check for
  `output/visual/check-hero-screenshot-review/index.html`

Corrected status:

- These are engine-contract fixes and API-completeness fixes. They do not
  promote condition monitoring, mapping, mixed records, cross-space dependency,
  dynamics, relation matrix or external process curves to hero-accepted status.
- GRAE10 remains the only accepted public hero. Other native examples remain
  review-pending previews until screenshot review and public-page composition
  acceptance are explicitly granted.

## Implementation Checkpoint: Render Facade And Harness Quarantine

Status date: 2026-06-26

Implemented:

- The old `metric.evidence.v1` browser render facade is removed from the public
  METRIC Visual surface. `visual/src/metric-visual.js` no longer exports the old
  WebGL scene renderer, record gallery, heatmap, query inspector, selection
  helper or process-curve app facade.
- `visual/examples/process-curve-condition-monitoring/index.html` now loads
  native `metric.visual.v1` process-curve evidence and renders through
  `createMetricVisual(...).showProcessCurves(...)` rather than direct runtime or
  page-local renderer setup.
- `createProcessCurveMiniatureLayerDescriptors()` is quarantined behind
  `ProcessCurveSceneView`; public/example pages no longer import or call the
  descriptor factory directly.
- Direct `MetricVisualRuntime` and `createLayerFromDescriptor()` usage is
  allow-listed only for protected GRAE10 references and explicitly internal
  diagnostics. Accepted public examples may not bypass the semantic command
  surface.
- `visual/tools/check-runtime-picking-preview.mjs` now follows the current
  native cross-space evidence IDs (`obs-000`, `obs-001`) instead of stale
  two-digit IDs, so record and pair previews are tested against the checked
  native export.

Verified:

- `node visual/tools/check-runtime-picking-preview.mjs`
- `node visual/tools/check-direct-runtime-harness-quarantine.mjs`
- `node visual/tools/check-legacy-render-facade-deletion.mjs`
- `node visual/tools/check-process-curve-descriptor-factory-quarantine.mjs`
- `node visual/tools/check-process-curve-scene-view.mjs`
- `node visual/tools/check-relation-matrix-picker.mjs`
- `node visual/tools/check-visual-command-api.mjs`
- `node visual/tools/check-views.mjs`
- `node visual/tools/check-single-render-pipeline.mjs`
- `node visual/tools/check-public-gallery-evidence.mjs`
- `node visual/tools/check-visual-regression-public-examples.mjs`
- `node visual/tools/check-grae10-golden.mjs`

Corrected status:

- `legacy-render-facade-deletion`,
  `process-curve-descriptor-factory-quarantine`,
  `direct-runtime-harness-quarantine` and
  `process-curve-scene-view-consolidation` are integrated in the agent-task
  registry.
- This checkpoint narrows the public API and engine render path. It does not
  promote any preview to hero-accepted status.

## Implementation Checkpoint: Grammar Workstreams And Regression Gates

Status date: 2026-06-26

Implemented:

- Relation-matrix diagnostics now carry exported metric-law diagnostics and a
  compact reference summary (`operator`, finite/diagonal/symmetry/triangle
  flags, pair count and triangle triplets). The relation picker check protects
  the native relation-matrix fixture and verifies that matrix selection remains
  tied to the native graph and pair preview.
- Mixed-record and condition-monitoring grammar checks prove that typed glyphs,
  cross-type relation edges, property fields and trajectory context are already
  first-class engine grammar. No code changes were needed in this slice.
- `MappingView` can consume exported mapping timelines to infer source/target
  coordinates, residual properties, label properties and timeline evidence.
  Mapping animation remains an engine/runtime motion contract instead of
  page-local state mutation.
- `DynamicsView` prefers explicit exported trajectory/path collections when
  present and otherwise uses exported timeline coordinate states. Dynamics
  fields now flow through the semantic view as timeline-aware `HeatFieldLayer`
  descriptors. JavaScript still does not compute solver, diffusion, PDC or
  Redif results.
- The regression/performance workstream report now records that the browser
  regression and large-scene performance gates execute successfully with
  generated artifacts kept under `output/visual/` and no GRAE10 baseline
  regeneration.
- The public project page gallery copy was tightened to product/evidence
  language: live METRIC scene, C++ exporter, generated evidence document. It no
  longer describes non-user-facing hardening status.

Verified:

- `node visual/tools/check-glyph-record-grammar.mjs`
- `node visual/tools/check-field-property-grammar.mjs`
- `node visual/tools/check-mapping-motion-grammar.mjs`
- `node visual/tools/check-dynamics-motion-grammar.mjs`
- `node visual/tools/check-dynamics-timeline-control.mjs`
- `node visual/tools/check-timeline-motion-contract.mjs`
- `node visual/tools/check-relation-matrix-picker.mjs`
- `node visual/tools/check-views.mjs`
- `node visual/tools/check-hero-grammar-contract.mjs`
- `node visual/tools/check-single-render-pipeline.mjs`
- `node visual/tools/check-public-gallery-evidence.mjs`
- `node visual/tools/check-visual-command-api.mjs`
- `node visual/tools/check-visual-performance-large-scenes.mjs`
- `node visual/tools/check-visual-regression-public-examples.mjs`
- `node visual/tools/check-grae10-golden.mjs`

Known gate note:

- `node visual/tools/check-visual-examples.mjs` is not the public hero gate. It
  includes `record-preview-contract`, a DOM-only preview-contract page without a
  canvas. The checker now classifies that explicit contract page separately
  instead of treating it as a failed canvas render. Public example regression is
  still covered by `check-visual-regression-public-examples.mjs`, which passed
  with `total: 8`, `failed: 0`.

Corrected status:

- `relation-matrix-graph-grammar-workstream`,
  `record-glyph-field-grammar-workstream`,
  `mapping-dynamics-motion-grammar-workstream` and
  `visual-regression-performance-workstream` are integrated in the task
  registry.
- This checkpoint advances reusable engine grammar and gates. It does not
  promote condition monitoring, mixed records, cross-space dependency, mapping,
  dynamics, relation matrix or external process curves to hero-accepted status.

## Implementation Checkpoint: Gallery, Picking And First Review Candidates

Status date: 2026-06-26

Implemented:

- The project page gallery now uses explicit public status language:
  GRAE10/MNIST is the accepted reference; all other native examples are review
  pending. Public copy points to the live scene, the native source and the
  generated visual evidence without claiming hero acceptance.
- GPU/CPU picking and record/pair preview coverage is documented in
  `docs/visual/reports/gpu-picking-preview-workstream.md`. No page-local
  hover/preview logic was added.
- Condition monitoring is now a concrete review-pending visual acceptance
  candidate. The page loads only native condition evidence, renders through
  `showConditionMonitoring()`, disables derived record-skyline/time-series
  geometry for this hero and exposes original process-window time series
  through the shared preview path.
- Relation-matrix/neighborhood is now a concrete review-pending visual
  acceptance candidate with an updated report and screenshot. It remains a
  relation matrix plus graph/pair-preview scene, not a point-cloud substitute.
- The task registry marks `gpu-picking-preview-workstream` and
  `project-gallery-native-evidence` as integrated, and marks
  `condition-monitoring-visual-acceptance` plus
  `relation-matrix-visual-acceptance` as review-pending candidate integrated.

Verified:

- `node visual/tools/check-condition-monitoring-visual-acceptance.mjs`
- `METRIC_VISUAL_PORT=8789 node visual/tools/check-condition-monitoring-browser.mjs`
- `node visual/tools/check-relation-matrix-readability.mjs`
- `node visual/tools/check-relation-matrix-picker.mjs`
- `node visual/tools/check-runtime-picking-preview.mjs`
- `node visual/tools/check-views.mjs`
- `node visual/tools/check-single-render-pipeline.mjs`
- `node visual/tools/check-hero-grammar-contract.mjs`
- `node visual/tools/check-public-gallery-evidence.mjs`
- `node visual/tools/check-grae10-golden.mjs`
- `node visual/tools/check-visual-regression-public-examples.mjs`
- `node visual/tools/check-hero-screenshot-review.mjs`

Corrected status:

- GRAE10/MNIST remains the only accepted public hero.
- Condition monitoring and relation-matrix/neighborhood are screenshot-ready
  review candidates, not accepted public heroes.
- Mixed records, cross-space dependency, mapping/dimensionality and
  dynamics/noise are addressed by the later "Remaining Visual Acceptance
  Candidates" checkpoint in this file.

## Implementation Checkpoint: Render Path Inventory

Status date: 2026-06-26

Implemented:

- `docs/visual/reports/render-path-inventory-and-deletion-map.md` inventories
  remaining direct runtime, descriptor and diagnostic render paths.
- The public gallery path remains canonical: exported visual evidence flows
  through semantic commands and `MetricVisualRuntime`.
- Direct runtime construction is currently limited to protected GRAE10
  references and internal diagnostics such as the native engine probe and
  miniature look gallery.
- The task registry marks `render-path-inventory-and-deletion-map` as
  integrated.

Verified:

- `node visual/tools/check-direct-runtime-harness-quarantine.mjs`
- `node visual/tools/check-legacy-render-facade-deletion.mjs`
- `node visual/tools/check-process-curve-descriptor-factory-quarantine.mjs`
- `node visual/tools/check-process-curve-scene-view.mjs`
- `node visual/tools/check-native-hero-evidence-scale.mjs`

Corrected status:

- No new public render path was added.
- The next cleanup tickets are runtime raw-descriptor hardening, optional style
  atlas command wrapping, and DOM-only preview-contract classification.

## Implementation Checkpoint: Remaining Visual Acceptance Candidates

Status date: 2026-06-26

Implemented:

- Mixed records is now a concrete review-pending visual acceptance candidate.
  It loads only native mixed-record evidence, renders through
  `showMixedRecords()`, uses typed glyphs plus cross-type relation edges and
  resolves record/pair previews through the shared native-evidence preview path.
- Cross-space dependency is now a concrete review-pending visual acceptance
  candidate. It loads only native paired-space evidence, renders through
  `showCrossSpace()`, shows separated source/target spaces plus exported bridge
  evidence and uses shared pair selection/preview.
- Mapping/dimensionality is now a concrete review-pending visual acceptance
  candidate. It loads only native mapping evidence, renders through
  `showMapping()`, uses the exported coordinate morph timeline and residual
  vectors, and captures deterministic source/transition/target frames.
- Dynamics/noise is now a concrete review-pending visual acceptance candidate.
  It loads only native dynamics evidence, renders through `showDynamics()`, uses
  the exported reverse-reconstruction timeline, trajectory paths, lifted field
  evidence and the reusable timeline control.
- The task registry marks `mixed-records-visual-acceptance`,
  `cross-space-dependency-visual-acceptance`,
  `mapping-dimensionality-visual-acceptance` and
  `dynamics-noise-visual-acceptance` as review-pending candidate integrated.

Verified:

- `node visual/tools/check-visual-document.mjs docs/examples/assets/mixed-records/metric.visual.json`
- `node visual/tools/check-visual-document.mjs docs/examples/assets/cross-space-dependency/metric.visual.json`
- `node visual/tools/check-visual-document.mjs docs/examples/assets/mapping-dimensionality/metric.visual.json`
- `node visual/tools/check-visual-document.mjs docs/examples/assets/dynamics-noise/metric.visual.json`
- `node visual/tools/check-glyph-record-grammar.mjs`
- `node visual/tools/check-mixed-glyph-geometry.mjs`
- `node visual/tools/check-cross-space-linked-selection.mjs`
- `node visual/tools/check-linked-selection-presentation.mjs`
- `node visual/tools/check-mapping-motion-grammar.mjs`
- `node visual/tools/check-mapping-frame-screenshots.mjs`
- `node visual/tools/check-dynamics-motion-grammar.mjs`
- `node visual/tools/check-dynamics-timeline-control.mjs`
- `node visual/tools/check-timeline-motion-contract.mjs`
- `node visual/tools/check-trajectory-path-view.mjs`
- `node visual/tools/check-views.mjs`
- `node visual/tools/check-single-render-pipeline.mjs`
- `node visual/tools/check-hero-grammar-contract.mjs`
- `node visual/tools/check-public-gallery-evidence.mjs`
- `node visual/tools/check-grae10-golden.mjs`
- `node visual/tools/check-visual-regression-public-examples.mjs`
- `node visual/tools/check-hero-screenshot-review.mjs`
- `node visual/tools/check-visual-performance-large-scenes.mjs`

Corrected status:

- GRAE10/MNIST remains the only accepted public hero.
- Condition monitoring, mixed records, cross-space dependency,
  mapping/dimensionality, dynamics/noise and relation-matrix/neighborhood are
  review-pending visual candidates, not accepted public heroes.
- The full public browser regression passes in the main worktree with 8
  examples and 0 failures.
- The next acceptance bottleneck is visual curation/manual screenshot review,
  stricter grammar-specific stress budgets at final hero scale and richer
  process-curve external evidence, not more synthetic page fixtures.
- The external process-curve preview remains explicitly record-count-blocked:
  the checked-in real UCR slices contain 48 source windows plus 16 query records,
  below the 500-source-record hero target. The exact blocker is
  `missing-real-source-windows-for-500-record-hero` and is documented in
  `docs/visual/reports/process-curve-external-scale-exporter.md`.
- The follow-up local inventory in
  `docs/visual/reports/process-curve-real-data-inventory.md` confirms that no
  full UCR 2021 archive or extracted source tree is available locally, so this
  blocker cannot be cleared without external real data.

## Implementation Checkpoint: Composition Acceptance Wave

Status date: 2026-06-26

Implemented:

- Condition monitoring now separates exported anomaly evidence from density
  support evidence inside `ProcessCurveSceneView`. Its transition path uses
  `TrajectoryPathView` with exported graph/transition evidence and does not add
  page-local trajectory rendering.
- Mapping/dimensionality now renders representative residual vectors inside
  `MappingView` from the native `local-mapping-distortion` evidence. The page
  only passes command options to `showMapping()`.
- Relation-matrix/neighborhood now makes the dense matrix the primary visual
  object via `RelationMatrixView` and `RelationMatrixLayer` diagnostics:
  matrix composition metadata, block coverage, block truncation and pair-preview
  placement are exposed without a DOM/SVG/2D matrix fallback.
- The task registry marks
  `condition-monitoring-composition-acceptance`,
  `mapping-residual-composition-acceptance` and
  `relation-matrix-composition-acceptance` as integrated while keeping them
  review-pending.

Verified:

- `node visual/tools/check-condition-monitoring-composition.mjs`
- `node visual/tools/check-mapping-composition.mjs`
- `node visual/tools/check-relation-matrix-composition.mjs`
- `node visual/tools/check-relation-matrix-picker.mjs`
- `node visual/tools/check-relation-matrix-readability.mjs`
- `node visual/tools/check-mapping-motion-grammar.mjs`
- `node visual/tools/check-trajectory-path-view.mjs`
- `node visual/tools/check-views.mjs`
- `node visual/tools/check-public-gallery-evidence.mjs`
- `node visual/tools/check-project-site-copy-contract.mjs`
- `node visual/tools/check-visual-regression-public-examples.mjs`
- `node visual/tools/check-hero-screenshot-review.mjs`
- `node visual/tools/check-visual-performance-large-scenes.mjs`
- `node visual/tools/check-grae10-golden.mjs`

Corrected status:

- GRAE10/MNIST remains the only accepted public hero.
- The composition wave improves reusable engine/view modules and strengthens
  review-pending candidates. It does not accept condition monitoring,
  mapping/dimensionality or relation-matrix/neighborhood as public heroes.
- The next acceptance bottlenecks are still human visual acceptance, larger
  real evidence where needed and stricter grammar-specific stress budgets at
  final hero scale.

## Implementation Checkpoint: Grammar-Specific Performance Budgets

Status date: 2026-06-26

Implemented:

- `visual/tools/check-visual-performance-large-scenes.mjs` now reports and
  enforces grammar-specific budgets for every current public preview grammar
  row.
- The grammar rows now carry an explicit `performanceBudget` object in the JSON
  report, covering median frame time, P95 frame time, minimum frame samples,
  minimum GPU draw calls and minimum GPU buffer uploads.
- The gate still treats these rows as public previews, not accepted heroes; the
  performance budget is necessary evidence for engine health, not visual
  acceptance.

Verified:

- `node --check visual/tools/check-visual-performance-large-scenes.mjs`
- `node visual/tools/check-visual-performance-large-scenes.mjs`

Corrected status:

- This closes the missing grammar-specific budget contract for the current
  preview rows.
- It does not close the remaining final-evidence stress requirement: accepted
  heroes still need larger real evidence where the application claim requires
  it and human screenshot acceptance.

## Implementation Checkpoint: Second Composition Acceptance Wave

Status date: 2026-06-26

Implemented:

- Mixed records now renders type-specific glyph, ground projection and relation
  edge composition through `MixedRecordView`. Exported record types use stable
  type colors/labels, native relation edges carry type-pair and relation-value
  emphasis metadata, and the page only passes command options to
  `showMixedRecords()`.
- Cross-space dependency now renders separated metric-space domains and
  exported bridge relation evidence through `CrossSpaceView`. The view keeps
  all 512 paired observations in the linked selection model while showing the
  96 exported high-contribution bridge records as visible bridge geometry.
- Dynamics/noise now renders trajectory thinning, exported property color/width
  encoding, lifted field metadata and timeline state-history context through
  `DynamicsView` plus `TrajectoryPathView`. The page only passes engine options
  to `showDynamics()`.
- The task registry marks `mixed-records-composition-acceptance`,
  `cross-space-composition-acceptance` and
  `dynamics-noise-composition-acceptance` as integrated while keeping them
  review-pending.

Verified:

- `node visual/tools/check-mixed-record-composition.mjs`
- `node visual/tools/check-glyph-record-grammar.mjs`
- `node visual/tools/check-mixed-glyph-geometry.mjs`
- `node visual/tools/check-cross-space-composition.mjs`
- `node visual/tools/check-cross-space-linked-selection.mjs`
- `node visual/tools/check-linked-selection-presentation.mjs`
- `node visual/tools/check-dynamics-composition.mjs`
- `node visual/tools/check-dynamics-motion-grammar.mjs`
- `node visual/tools/check-dynamics-timeline-control.mjs`
- `node visual/tools/check-timeline-motion-contract.mjs`
- `node visual/tools/check-trajectory-path-view.mjs`
- `node visual/tools/check-visual-command-api.mjs`
- `node visual/tools/check-views.mjs`
- `node visual/tools/check-visual-agent-tasks.mjs`

Corrected status:

- GRAE10/MNIST remains the only accepted public hero.
- Mixed records, cross-space dependency and dynamics/noise are stronger
  review-pending candidates, not accepted public heroes.
- Final acceptance still requires full public browser regression, screenshot
  review and human visual acceptance.

## Implementation Checkpoint: Preview Semantic Role Contract

Status date: 2026-06-26

Implemented:

- Public preview briefs in `visual/hero-visual-briefs.manifest.json` now carry
  `requiredDescriptorRoles` in addition to required WebGL primitives.
- `visual/tools/check-hero-visual-briefs.mjs` validates those roles against the
  browser regression runtime state, so examples cannot pass their visual brief
  by emitting anonymous layers.
- `RelationMatrixView` and `NeighborhoodGraphView` now expose
  `primary-relation-matrix`, `neighborhood-graph-nodes` and
  `neighborhood-graph-edges` roles through reusable engine descriptors.

Verified:

- `node visual/tools/check-relation-matrix-composition.mjs`
- `METRIC_VISUAL_EXAMPLES=relation-matrix-neighborhood node visual/tools/check-visual-regression-public-examples.mjs`
- `node visual/tools/check-visual-regression-public-examples.mjs`
- `node visual/tools/check-hero-visual-briefs.mjs`
- `node visual/tools/check-hero-screenshot-review.mjs`
- `node visual/tools/check-public-gallery-evidence.mjs`
- `node visual/tools/check-project-site-copy-contract.mjs`
- `node visual/tools/check-grae10-golden.mjs`

Corrected status:

- This is a grammar-contract hardening step, not visual hero acceptance.
- GRAE10/MNIST remains the only accepted public hero; all other public previews
  remain review-pending until screenshot and human visual acceptance clear their
  blockers.

## Implementation Checkpoint: Public Miniature Scene Contract

Status date: 2026-06-26

Implemented:

- `visual/tools/check-public-miniature-scene-contract.mjs` validates every
  public review-pending preview from the browser regression report against the
  shared miniature render contract.
- The gate requires `postFx.cameraDof`, `postFx.miniatureFrame`,
  `postFx.colorGrade` and `postFx.vignette` for public previews, and rejects a
  preview if it falls back to a tilt-shift pass without native camera-depth DoF.
- The gate cross-checks each visual brief's required primitives and semantic
  descriptor roles against runtime state, and requires `BillboardLabelLayer`
  whenever label roles such as `region-labels`, `type-labels` or
  `paired-space-label-anchors` are present.
- `BillboardLabelLayer` is part of the miniature style descriptor role mapping,
  so labels are treated as first-class scene elements instead of page overlays.
- Miniature rig/look verifiers now exercise `ProcessCurveSceneView` through
  native `metric.visual.v1` evidence rather than a private process-curve
  descriptor factory.

Verified:

- `node --check visual/tools/check-public-miniature-scene-contract.mjs`
- `node visual/tools/check-public-miniature-scene-contract.mjs`
- `node visual/tools/verify-miniature-rig.mjs`
- `node visual/tools/verify-miniature-look-atlas.mjs`
- `node visual/tools/verify-postfx-depth-dof-contract.mjs`
- `node visual/tools/check-process-curve-descriptor-factory-quarantine.mjs`
- `node visual/tools/check-hero-visual-briefs.mjs`
- `node visual/tools/check-hero-screenshot-review.mjs`
- `node visual/tools/check-native-hero-evidence-scale.mjs`
- `node visual/tools/check-public-gallery-evidence.mjs`
- `node visual/tools/check-project-site-copy-contract.mjs`
- `node visual/tools/check-grae10-golden.mjs`

Corrected status:

- This gate proves the reusable miniature render/style contract for public
  previews. It does not prove that a preview is visually accepted.
- GRAE10/MNIST remains the only accepted public hero.
- The next production work is visual acceptance and public portfolio quality
  for each grammar, plus real-data scale-up for the external process-curve
  preview.

## Next Engine Acceptance Slices

Status date: 2026-06-26

The next parallel work is engine/view capability work, not new page HTML:

1. Relation-matrix readability:
   `RelationMatrixView`, `RelationMatrixLayer` and relation descriptors must
   make a 130x130 native matrix readable as a primary visual, with crisp block
   boundaries, row/column/cell focus and graph/matrix z-order rules. The first
   reusable engine slice is implemented: `RelationMatrixLayer` descriptors now
   run in the `screen-readable-overlay` phase after photographic post-FX, and
   browser regression reports `ScreenReadableOverlayPass`. Remaining work is
   screenshot acceptance, stronger large-matrix evidence where required, and
   any further matrix-scale tiling needed by that evidence.
2. Trajectory/field hierarchy:
   `TrajectoryPathView`, `PropertyFieldView`, `DynamicsView` and
   `ProcessCurveSceneView` must give path, scalar field, current state and
   labels predictable visual priority. Condition monitoring and finite dynamics
   currently pass grammar gates but still read too much like blended clusters.
3. Relation-edge legibility:
   `RelationEdgeLayer`, `MixedRecordView`, `CrossSpaceView` and `MappingView`
   must lane, bundle, sample or otherwise rank relation edges and residual
   vectors so they support typed records, paired spaces and mapping residuals
   instead of becoming visual noise.
4. External process-curve scale:
   `process-curve-external-hero` remains blocked until at least 500 distinct
   real licensed source windows are available. The current local inventory has
   only 48 real source windows plus 16 queries, and those queries must not be
   counted as source-scale padding.

Each slice must modify reusable engine/view/layer code, update or add a
checker/report, and keep GRAE10 golden plus public gallery evidence green.
