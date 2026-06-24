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

Status date: 2026-06-24

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
- The next production step is reusable visual-command/gallery integration of
  the checked native exports, not more hand-written hero HTML and not more
  synthetic fixture scenes.
- Browser load success means only `loads` or `renders`; it never means
  `hero-ready`.

The current public gallery is therefore limited to:

- MNIST dimension reduction: protected 60k GRAE10 reference plus native
  integrity check.
- UCR process curves: real dataset plus native C++ export assets.

All other heroes stay planned until their native exported evidence is consumed
through a distinct reusable visual grammar and passes
`visual/tools/check-public-gallery-evidence.mjs` plus browser screenshot review.

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
  `showRelations`, `showProperties`, `showMapping`, `showDynamics`,
  `showPreview` and `captureHeroFrame`

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
- dynamics, sampling, denoise, diffusion and reverse-flow evidence
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

Noise, diffusion, denoise and reverse flow can be viewed as dynamics over a
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

- `visual/tools/check-visual-examples.mjs`
- screenshot capture tool
- performance report generator
- baseline images for hero scenes

Acceptance:

- no example ships without a browser check
- 60k-record point cloud remains interactive
- visual regressions are visible in generated artifacts

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
  `NeighborhoodGraphView`, `SpacePropertiesView`, `MappingView` and
  `DynamicsView`, composed through the same descriptor/runtime pipeline
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
- viewport composition sketch in words
- layer stack
- interaction plan
- screenshot target state
- expected failure modes

Implementation may start only after this preflight is filled in. If the agent
does not have enough evidence fields or record count, it must stop before
building the visual.

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
- one evidence fixture under `visual/examples/<work-order-id>/`
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
- `visual/examples/condition-monitoring-hero/evidence.json` as a development
  fixture only until replaced by native exported evidence
- native export target: `docs/examples/assets/condition-monitoring/metric.visual.json`
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
- `visual/examples/mixed-record-hero/evidence.json` as a development fixture
  only until replaced by native exported evidence
- native export target: `docs/examples/assets/mixed-records/metric.visual.json`
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

- at least 2,000 records
- at least three visible structure families
- residual/error values for every rendered record
- source and target states for the same record IDs

Required output paths:

- `visual/examples/mapping-dimensionality-hero/index.html`
- `visual/examples/mapping-dimensionality-hero/evidence.json` as a development
  fixture only until replaced by native exported evidence
- native export target: `docs/examples/assets/mapping-dimensionality/metric.visual.json`
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

- at least 1,000 paired observations
- two coordinate spaces with different visible geometry
- local contribution values for selected regions
- global statistic exported as evidence

Required output paths:

- `visual/examples/cross-space-dependency-hero/index.html`
- `visual/examples/cross-space-dependency-hero/evidence.json` as a development
  fixture only until replaced by native exported evidence
- native export target: `docs/examples/assets/cross-space-dependency/metric.visual.json`
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

Show diffusion, denoise or reverse-flow evidence as dynamics over a finite
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
- `visual/examples/dynamics-noise-hero/evidence.json` as a development fixture
  only until replaced by native exported evidence
- native export target: `docs/examples/assets/dynamics-noise/metric.visual.json`
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
3. Build native `metric.visual.v1` exporters for the planned heroes.
4. Wire each exporter into a distinct visual grammar through `createMetricVisual`
   or semantic views.
5. Add a browser screenshot check for each new native exporter.
6. Add a hero to the public page only after the evidence gate and screenshot
   checks pass.

## Implementation Checkpoint: Engine Command Layer

Status date: 2026-06-24

Implemented:

- Removed the parallel metric-space hero scene helper from `visual/src/views/`.
- Removed the public export for the forbidden helper.
- Added the public command surface in `visual/src/metric-visual.js`:
  `createMetricVisual`, `showMetricSpace`, `showRelationMatrix`,
  `showNeighborhoodGraph`, `showSpaceProperties`, `showMapping`,
  `showDynamics` and the `MetricVisualSurface` command object.
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

Verified:

- `node visual/tools/check-single-render-pipeline.mjs`
- `node visual/tools/check-hero-grammar-contract.mjs`
- `node visual/tools/check-views.mjs`
- `node visual/tools/check-grae10-golden.mjs`
- `node visual/tools/check-public-gallery-evidence.mjs`

Corrected status:

- The six synthetic hero fixtures load as engine development examples only.
- They are not accepted as public heroes.
- They are not evidence that METRIC has solved those application cases.
- Browser load success is not a substitute for native evidence and visual
  acceptance.

Not visually accepted yet:

- `RelationMatrixLayer` still needs a legibility pass. The current matrix can
  show high-frequency diagonal artifacts and is not acceptable as a final hero
  asset.
- `MixedRecordView` currently uses the glyph layer through the point-material
  path. It proves the grammar split but still needs stronger type-specific
  glyph geometry.
- `CrossSpaceView` now uses one runtime and a paired-space bridge, but linked
  brushing and pair preview still need to become first-class interactions.
- `DynamicsView` shows trajectory and current state, but it still lacks a
  state-controlled animation timeline and changing ground field.
- Pair preview is not yet a reusable engine component. Record preview exists,
  but needs GPU picking integration and richer payload renderers.

Next mandatory engine work:

1. Implement native visual exporters for planned heroes.
2. Implement reusable pair preview contracts.
3. Integrate GPU picking with the existing record preview CPU fallback.
4. Improve `RelationMatrixLayer` readability: block ordering, mip/LOD strategy,
   explicit row/column focus, and no moire-like artifacts.
5. Replace placeholder glyph rendering with true type-specific glyph geometry
   for mixed records.
6. Add stateful timeline controls to `DynamicsView`.
7. Add linked selection between relation matrix, graph, paired-space bridge and
   metric-space records.
8. Re-run browser screenshots and only then decide which hero is allowed to
   become the first visual quality bar.
