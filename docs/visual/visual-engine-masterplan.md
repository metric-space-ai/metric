# METRIC Visual Engine Master Plan

This document defines a standalone data and relationship visualization library
inside METRIC. The library is not an auxiliary developer tool, not a wrapper
around C++ examples, and not a set of one-off demo pages. It is a native browser
visualization engine for finite records, pair relations, metric spaces, graph
structures, properties, coordinate states, timelines, mappings, and solver
traces.

The only hard contract between METRIC algorithms and the visual library is an
import/export evidence format. C++ computes. The visual library imports and
renders.

## Product Definition

The visual engine is a data and relationship visualization library.

The implementation contract is the METRIC visual runtime itself:

```text
visual/src/runtime/metric-webgl/
visual/src/postfx/
visual/src/style/miniature/
visual/src/layers/
```

Renderer, material, camera, picking and postprocess changes must be made in
those METRIC-owned modules directly.

Executable agent work orders, stop rules and hero visual acceptance criteria
are defined in:

```text
docs/visual/metric-visual-engine-implementation-plan.md
```

## Current Implementation Status

The visual engine is no longer only a one-page proof, but it is also not yet a
complete production visualization product.

Implemented native slices:

- renderer core, render loop, scene, render targets and WebGL resources
- camera, focus-line and pointer interaction
- `metric.evidence.v1` -> `metric.visual.v1` adapter
- typed channel encoders
- point, glyph, box, heat, surface, relation-edge, ground-projection and
  ground-plane layers
- relation matrix, sparse relation graph and relation diagnostics builders
- curve/ribbon/tube path builders, `CurveRibbonLayer` and
  `CurveTubeMeshLayer`
- miniature photographic style presets, scene-fit helpers and scene bundles
- reusable `MiniaturePhotographicStyle` owner for stage, runtime options,
  descriptor styling, scene bundles, focus/postprocess updates and runtime
  application
- serializable `MiniatureStyleProfile` contract for moving the same
  photographic miniature look across examples and future visual documents
  without copying scene wiring
- reusable Hero/Capture helpers for full-frame miniature screenshots and
  project-page visuals: `createMiniatureHeroStage()`,
  `createMiniatureHeroProfile()`, `createMiniatureHeroStyle()`,
  `createMiniatureHeroSceneBundle()` and
  `createMiniatureHeroRuntimeOptions()`
- reusable style-motion presets for focus breathing, restrained camera drift
  and turntable motion over camera/focus/postprocess state
- style-motion controller diagnostics for active domains, runtime attachment,
  captured base state, before-render hook presence and last applied frame
  update
- serializable style-motion atlas for reusable focus, camera and postprocess
  motion contracts with sampled offsets for documentation and visual
  regression tooling
- reusable style-contract diagnostics for camera, lighting, focus,
  depth-of-field, materials, ground relation, post-FX, isometric staging,
  animation, post-FX pass sizing and runtime hook coverage
- `createMiniatureRuntimeOptions()` bridge for camera, controls, focus-line,
  postprocess and layer-factory setup from one stage contract
- reusable `ProcessCurveSceneView` builder shared by the native probe and the
  condition-monitoring example
- miniature look gallery for rendering the same METRIC descriptor bundle
  through every reusable photographic reference look:
  `visual/examples/miniature-look-gallery/index.html`
- reusable miniature animation presets for focus, morph, attention,
  uncertainty, surface, timeline and camera-orbit motion
- stage light rigs lowered into native shader material uniforms
- sample/glyph sprites, curve ribbons and world-space curve tubes consume the
  miniature material
  response instead of remaining flat debug marks
- point/glyph and mesh layers now expose native photographic material response:
  sphere shading, gloss, edge shade, saturation, shadow density, roughness,
  metalness, specular power, soft shadows, base lift and highlight/shadow tint
- stage focus and stage grounding contracts consumed by runtime/layers
- reusable `MiniatureCameraDofPass` consumes camera depth, focus distance,
  aperture and circle-of-confusion parameters; `MiniatureFramePass` consumes the
  same focus state for the final photographic frame response and reports the
  optical model as `camera-depth-circle-of-confusion` with raw camera depth,
  aperture-relative defocus and near/far-plane reconstruction
- the reusable hero stage now carries the full physical camera DoF contract:
  focus distance, aperture, focal length, f-stop, sensor scale, max blur and
  focal range
- `MiniatureFramePass` applies floor/sky/focus/edge shaping, subject isolation, tabletop matte,
  floor sheen, stage spotlight, stage shadow and studio-light cues before final
  grade
- `MetricPostFxStack` now initializes and resizes every internal pass together,
  and the miniature style diagnostic flags active post-FX passes whose size
  does not match the renderer drawing buffer
- semantic ground fields and colored projection footprints in the browser probe
- reusable GroundProjection shadow controls for neutral shadow color,
  semantically tinted edges, core density and global projection density
- reusable GroundPlane shadow-catcher controls for matte floor color, subdued
  grid, horizon falloff, tabletop sheen, contact shade and stage shaping
- directed GroundProjection shadow tails and contact hardness for semantically
  tinted ground evidence that still reads as photographic contact
- surface/field shader motion for miniature field views
- raw camera-depth texture creation, camera DoF and FXAA-like postprocess
- final post-FX stack for restrained bloom, miniature frame shaping,
  photographic grade, vignette and optional grain
- browser-verified native probe at
  `visual/examples/native-engine-probe/index.html`
- browser-verified process-curve condition-monitoring example at
  `visual/examples/process-curve-condition-monitoring/index.html`
- browser-verified full-frame miniature capture example at
  `visual/examples/miniature-hero-frame/index.html`
- browser-verified miniature look gallery at
  `visual/examples/miniature-look-gallery/index.html`
- native `metric.visual.v1` exporter assets for condition monitoring, mixed
  records, cross-space dependency, dynamics/noise, relation matrix and
  mapping/dimensionality under `docs/examples/assets/*/metric.visual.json`
- native hero-scale evidence for condition monitoring, mixed records,
  cross-space dependency, finite dynamics/noise, relation matrix and
  mapping/dimensionality; the external UCR process-curve preview remains below
  the hero-scale source-record threshold because the checked-in licensed slice
  contains only 48 real source windows plus query records
- header-only C++ writer foundation under `visual/cpp/mtrc_visual.hpp`
- runtime inspection with shared picking and preview state for records,
  relation-matrix cells, graph edges and deterministic projected-record
  fallback
- reusable record/pair preview panel resolution from `metric.visual.v1`
  evidence, including runtime selection state for selected record previews,
  selected pair previews and inspection payloads
- richer record/pair preview payload resolution for exported payload families,
  record properties, linked coordinate/view memberships, relation-independent
  pair properties and symmetric native pair values
- bounded record/pair preview presentation for text, time-series,
  histogram/distribution, compact image-array, composed-record and generic
  property payloads; renderer diagnostics remain machine-readable and are not
  exposed as user-facing debug badges
- reusable `MixedRecordView` semantic grammar for heterogeneous records,
  typed glyph descriptors and cross-type relation-edge evidence without
  page-local render helpers
- reusable `CrossSpaceView` semantic grammar for paired metric spaces,
  side-by-side space descriptors, exported pair bridges and linked-selection
  metadata through the same runtime
- reusable timeline state/control descriptors for `DynamicsView`, including
  sampled exported timeline state, deterministic start/middle/end samples and
  animation-control metadata
- `MappingView` owns mapping residual/error descriptors; `showMapping()` no
  longer adds a page-level residual fallback, and public browser regression
  requires residual magnitudes from native mapping evidence
- user-facing timeline-control metadata for scrubber/playback/reset controls
  plus active ground/field state descriptors from exported timeline-step
  properties
- native solver-trace evidence under
  `docs/examples/assets/solver-trace/metric.visual.json`, exported by
  `examples/engine/solver_trace_visual_export.cpp`; `showSolverTrace()` now
  exercises native timeline-step residual/objective evidence instead of a
  synthetic JavaScript trace fixture
- reusable relation-matrix readability metadata for block/tile/LOD contracts,
  weighted dense-cell smoothing, logical tile boundaries and row/column/cell
  selection focus
- relation-matrix readability diagnostics for matrix dimensions, block count,
  tile count/source, missing-value count, selected row/column/cell state and
  graph relation identity
- stricter `metric.visual.v1` validation for dense relation shapes, relation
  value endpoints, coordinate-position dimensions and pair-property endpoints
- browser-backed public regression gate for the accepted GRAE10 reference and
  native preview examples:
  `visual/tools/check-visual-regression-public-examples.mjs`
- native hero evidence scale gate that compares public visual briefs with
  native `metric.visual.v1` assets before browser rendering and enforces
  native provenance plus record-count, relation-count and record-type blocker
  consistency:
  `visual/tools/check-native-hero-evidence-scale.mjs`
- explicit screenshot-review gate that keeps GRAE10 as the only accepted hero
  and classifies all other native public examples as review-pending previews
  until a separate acceptance decision exists:
  `visual/tools/check-hero-screenshot-review.mjs`
- large-scene performance gate for 1k, 10k and 60k point-cloud workloads plus
  current public preview grammar rows for field, glyph, relation-matrix/graph,
  dynamics, mapping, cross-space and process-curve scenes:
  `visual/tools/check-visual-performance-large-scenes.mjs`
- the large-scene performance gate now reads
  `visual/hero-visual-briefs.manifest.json`, reports Visual-Brief coverage,
  per-preview minimum record targets, current acceptance blockers and keeps
  `process-curve-external-hero` explicitly scale-blocked instead of silently
  treating a render pass as hero readiness
- command-fidelity guards that keep public examples on one semantic engine
  command, verify `showConditionMonitoring()` as the final condition grammar,
  prevent residual/error mapping layers without explicit residual evidence and
  expose `captureHeroFrame(visual, options)` as a top-level public API delegating
  to the surface method
- public descriptor-injection escape hatches on `MetricVisualSurface` are
  removed; raw descriptor installation is internal-only, while public callers
  use semantic `show*` commands or semantic view objects with
  `toLayerDescriptors()`
- the old `metric.evidence.v1` browser render facade is removed from the
  public surface; process-curve pages now load native `metric.visual.v1`
  evidence and render through semantic commands/views
- direct `MetricVisualRuntime` harnesses are quarantined to protected GRAE10
  references and explicitly internal diagnostics; accepted/public examples may
  not bypass `createMetricVisual()` and semantic `show*` commands
- direct process-curve descriptor-factory calls are removed from public/example
  pages; `ProcessCurveSceneView` owns process-curve descriptor assembly
- the old embedded Babyplots/Babylon MNIST standalone page was deleted from
  active visual examples; MNIST dimension reduction is represented by the
  protected METRIC-owned 60k GRAE10 engine page
- screenshot-review gallery generation:
  `visual/tools/check-hero-screenshot-review.mjs` writes
  `output/visual/check-hero-screenshot-review/index.html` with the real page and
  canvas screenshots, visual claims, expected grammars, native-evidence state
  and open blockers for human review without promoting previews automatically

Known gaps:

- full-frame capture API exists; no polished multi-algorithm hero
  application/gallery has been accepted yet
- the existing UCR/process-curve evidence now has a native `metric.visual.v1`
  export and command-backed preview page; it remains preview-only until
  screenshot review accepts the visual grammar and composition
- six public preview families now meet their native hero evidence scale
  thresholds: condition monitoring, mixed records, cross-space dependency,
  relation matrix, finite dynamics/noise and mapping/dimensionality. They remain
  review-pending previews until screenshot review accepts the visual grammar and
  composition. The external UCR process-curve preview remains explicitly blocked
  by source-record count and must not be inflated with synthetic windows.
- no full visual gallery for every METRIC algorithm family yet
- relation matrix has native exported evidence and a reusable WebGL readability
  pass with block bands, tile/LOD metadata, block boundaries, row/column/cell
  focus, stronger focus/boundary defaults, explicit block shader capacity and
  pair-evidence picking, but it is not yet a polished public matrix/neighborhood
  hero until screenshot review accepts it
- metric curve-body study-style curve bodies now have a native `CurveTubeMeshLayer`; more
  algorithm-family views still need to adopt it where world-space continuity is
  the main evidence
- runtime style hooks and reusable miniature scene bundles exist; more
  algorithm-family examples are still needed
- the style owner is now used by the native probe and process-curve page, but
  richer hero applications still need algorithm-specific view design
- style motion exists for the photographic layer, but hero applications still
  need curated motion direction per algorithm family
- the profile contract is now active in browser examples, the first material
  response pass is implemented, and world-space tube paths are available;
  visual quality still needs stronger floor projection, curated hero
  composition and stricter screenshot baselines against accepted photographic
  reference captures
- runtime GPU picking is active for stock point, glyph, ground-projection,
  heat-field, relation-edge, curve-ribbon and curve-tube layers; the public
  regression gate now requires real `gpu-picking` hits with record IDs and edge
  IDs for native preview examples that expose pickable records, fields, graph
  edges or ribbon trajectories. Tube-curve picking is verified on the native
  engine probe. Relation-matrix cells use the semantic matrix picker and carry
  native pair identity/properties into the same runtime preview path; remaining
  layer-specific GPU work is matrix GPU picking only if the semantic picker is
  not sufficient at larger tiled scales.
- performance gates exist for large point-cloud workloads and all seven current
  public preview grammar rows; matrix, graph, field, glyph, dynamics and mapping
  still need stricter stress-size budgets at realistic final hero evidence sizes

It must help users inspect:

- records and record collections
- pair relations between records
- finite metric spaces as a special, law-checked relation structure
- multiple relations over the same records
- scalar, categorical, vector, and tensor properties attached to records,
  pairs, spaces, graph edges, or time steps
- graph structures derived from relations
- coordinate states derived from spaces or algorithms
- morphs between coordinate states
- density, entropy, clustering, sampling, search, mapping, dynamics, and solver
  evidence as layers over the same records

It is part of the METRIC repository, but it should be usable as its own library:
a user gives it a valid visual evidence document and receives an interactive
GPU-backed inspection surface.

## Boundary To METRIC C++

C++ is the source of truth for:

- metric values
- derived states and solver outputs
- search results
- entropy, density, intrinsic dimension, local volume
- component groupings, density groupings, outliers, representatives
- MGC and paired-space dependence evidence
- diffusion, inverse diffusion, dynamics, Redif inverse-dynamics and resampling results
- derived coordinate spaces such as parametric diffusion coordinates, PCFA, parametric latent spaces, and graph layouts
- solver traces, objective trajectories, graph solver residuals
- metric-law checks and admission/quarantine status

JavaScript is responsible for:

- loading and validating exported evidence
- storing linked selection state
- camera control
- WebGL buffers and textures
- picking
- filtering and brushing
- color maps
- view layout
- interpolation between already exported coordinate states
- animation timelines over already exported states

JavaScript must not become a second implementation of METRIC algorithms.

## Non-Negotiables

- No npm dependency.
- No package manager requirement.
- No TypeScript requirement.
- No bundler.
- No external renderer.
- No adapter to the earlier point-cloud renderer, the earlier scene renderer, external renderer, D3, Plotly, deck.gl, or other
  visualization libraries.
- Plain JavaScript ES modules, Canvas 2D, and raw WebGL.
- METRIC-owned GLSL shaders are part of the engine, not decoration.
- GPU 3D is the primary runtime.
- Canvas 2D is allowed for record panels, small charts, labels, and fallback
  surfaces.
- SVG is only a static documentation fallback, not the main engine.
- the earlier point-cloud renderer may be studied and code may be ported with license obligations, but
  no the earlier point-cloud renderer names, APIs, data contracts, dependencies, or namespace artifacts
  are allowed in the public METRIC visual library.

## Library Name And Location

Repository location:

```text
visual/
```

The library should be designed as a real METRIC visual engine, not as a
throwaway demo widget.

Proposed internal module name:

```text
mtrc.visual
```

This is JavaScript-side naming only. It does not change C++ namespaces.

## Import/Export Format

The visual library consumes `metric.visual.v1`.

The current C++ example exports `metric.evidence.v1`. That first schema can be
kept as a compatibility source, but the visualization contract should become
`metric.visual.v1` once the format is stable.

The format is not algorithm-specific. It describes records and relationships.

### Top-Level Shape

```json
{
  "schema": "metric.visual.v1",
  "provenance": {},
  "datasets": [],
  "records": [],
  "relations": [],
  "spaces": [],
  "properties": [],
  "graphs": [],
  "coordinates": [],
  "timelines": [],
  "events": [],
  "views": [],
  "diagnostics": []
}
```

### `datasets`

Groups records and evidence into named datasets or domains.

Required fields:

- `id`
- `title`
- `description`
- `source`
- `license`

### `records`

Records are the primary identity-bearing objects.

Required fields:

- `id`
- `dataset_id`
- `record_type`
- `label`
- `payload`

Payload may be inline for small data or referenced for large data.

Supported record payload families:

- scalar vector
- time series
- string
- categorical tuple
- histogram
- image
- audio
- composed record
- opaque external payload reference

### `relations`

Relations describe pair values between records.

This is the core abstraction for the visual library. A finite metric is one
important relation type, but the visual engine can also render baselines,
transition weights, similarities, assignment scores, residuals, or law-check
evidence as long as the producer declares their type.

Required fields:

- `id`
- `dataset_id`
- `name`
- `relation_type`
- `value_type`
- `record_ids`
- `storage`
- `values`

Relation types:

- `metric`
- `baseline_metric`
- `distance_like`
- `similarity`
- `transition`
- `assignment`
- `correlation`
- `loss`
- `law_check`
- `custom`

For `relation_type: "metric"`, the producer must also export metric-law
diagnostics. The visual library displays them; it does not prove them.

Storage forms:

- dense matrix
- symmetric dense matrix
- sparse edge list
- block matrix
- external binary table

### `spaces`

Spaces bind records and relations into an inspectable object.

Required fields:

- `id`
- `dataset_id`
- `record_ids`
- `primary_relation_id`
- `space_type`
- `metadata`

Space types:

- `finite_metric_space`
- `baseline_vector_space`
- `derived_coordinate_space`
- `graph_space`
- `paired_space`
- `custom_space`

### `properties`

Properties attach values to records, pairs, relations, spaces, edges, or time
steps.

Target types:

- `record`
- `pair`
- `relation`
- `space`
- `graph_edge`
- `timeline_step`

Value types:

- scalar
- categorical
- vector
- tensor
- boolean
- rank
- distribution

Examples:

- entropy
- density
- local volume
- intrinsic dimension
- outlier score
- cluster id
- representative assignment
- reconstruction error
- graph residual
- coordinate target error
- metric-law violation

### `graphs`

Graphs represent sparse structure over records.

Required fields:

- `id`
- `dataset_id`
- `node_record_ids`
- `edge_relation_id`
- `graph_type`
- `edges`

Graph types:

- k-nearest graph
- radius graph
- transition graph
- diffusion graph
- solver graph
- sparsified graph
- component graph
- custom graph

### `coordinates`

Coordinates are named states for records.

Required fields:

- `id`
- `dataset_id`
- `space_id`
- `name`
- `dimension`
- `record_positions`

Coordinate states are not the metric space itself. They are visual or derived
states that let users inspect relationships.

Examples:

- landmark 3D
- landmark 2D
- parametric diffusion coordinates 2D/3D
- parametric latent coordinates
- PCFA
- derived grid
- graph layout
- solver scalar field lifted to 3D
- user-provided coordinates

### `timelines`

Timelines describe state changes over existing records.

Examples:

- 2D to 3D morph
- diffusion steps
- reverse diffusion steps
- Redif inverse-dynamics before/after
- resampling before/after
- coordinate calibration steps
- graph solver iterations
- objective traces and coordinate-state evolution

The timeline contains references to coordinate states, properties, graphs, and
relations. It does not ask JavaScript to recompute them.

### `views`

Optional producer hints for useful initial views.

The visual library may ignore these hints, but examples can use them to open a
ready inspection workspace.

Examples:

- point cloud plus entropy overlay
- metric matrix plus cluster ordering
- graph plus diffusion timeline
- record gallery plus query comparison
- paired-space MGC view

## Visual Core

The engine needs reusable GPU primitives, not algorithm-specific widgets.

## Brand And Shader Direction

METRIC visual should give the project a recognizable brand. The brand should
come from the behavior of finite metric spaces on screen: dense relation fields,
linked records, metric graph motion, morphing coordinate states, scalar overlays,
and precise interaction. It should not look like a generic plotting dashboard.

WebGL is shader-based. The engine should use METRIC-owned shaders for:

- a permanent ground/stage layer so metric spaces read as miniature models
  rather than arbitrary floating scatterplots
- semantic ground projections that act like shadows but carry data: a 2D
  coordinate state, low-dimensional embedding, density map, cluster footprint,
  relation field, or solver/property projection
- colored projection shadows where point colors and scalar overlays remain
  semantically linked to records on the ground plane
- gradient density projection shaders for entropy, density, local volume,
  cluster concentration, residual fields, transition probability, and other
  exported scalar fields
- point glyphs with depth, selected-record halos, scalar color ramps, and
  density-aware alpha
- graph edges with weight-based opacity, path highlights, and transition flow
- matrix textures for relation fields, law checks, MGC grids, transition
  matrices, assignment matrices, and solver matrices
- scalar fields for entropy, density, local volume, intrinsic dimension,
  outlier score, residuals, and derivation residuals
- coordinate morphs between 2D, 3D, parametric diffusion coordinates, parametric latent coordinates, PCFA, graph layouts,
  and solver layouts
- trails and temporal fades for diffusion, inverse diffusion, resampling,
  Redif inverse-dynamics, and iterative solvers
- picking passes that keep selection accurate for dense point and graph scenes
- optional postprocessing for glow, depth haze, high-contrast selection, and
  publication-quality screenshots

The default art direction is isometric miniature photography. A METRIC scene
should feel like a small physical model of a finite metric space: a stable
ground plane, orthographic/isometric camera, shallow focus, dense relation
fields, and precise linked objects. The ground is not empty decoration. It can
show a semantic projection of the same records: an interpretable 2D shadow,
diffusion-coordinate target state, parametric coordinate state, cluster footprint,
density/entropy field, or other
low-dimensional view exported by C++. Projection shadows should inherit the
record or property colors and may form gradients when many projected records
accumulate. Free camera movement can exist, but the brand default is a composed
miniature view.

The style target is technical and alive: precise enough for analysis, visual
enough to be memorable. The scene should make users feel that records and
relations are tangible objects, not rows in a table.

Shader work must remain data-faithful. Shaders may encode, reveal, emphasize,
interpolate, and animate exported evidence. They must not fabricate algorithmic
results.

### Scene Core

Responsibilities:

- WebGL context
- device-pixel-ratio resizing
- isometric camera as the default, with optional free camera
- ground/stage layer
- semantic ground-projection layer
- point buffers
- line/edge buffers
- matrix textures
- scalar color ramps
- picking buffers or projected picking
- shared selection state
- timeline state
- layer composition

### Layers

Required first-class layers:

- point cloud layer
- ground/stage layer
- semantic ground-projection layer
- graph edge layer
- matrix texture layer
- scalar field layer
- path layer
- record glyph layer
- label layer
- image/texture layer
- curve strip layer
- histogram layer
- timeline layer

### Linked Selection

Selection must use record identity, relation identity, graph edge identity, and
timeline step identity.

Selecting a point must be able to update:

- record detail
- pair matrix row/column
- nearest-neighbor list
- graph neighborhood
- entropy/density scalar readout
- cluster/representative assignment
- mapping reconstruction
- solver residual

Selecting a matrix cell must be able to update:

- the two records
- the relation value
- graph edge if present
- pair comparison
- any derived coordinate links

### 2D/3D Morph

2D-to-3D morph is a core engine capability.

It is not a special parametric diffusion coordinates or parametric-coordinate feature. It applies to any records with multiple
coordinate states.

Requirements:

- same record ids across coordinate states
- slider-controlled interpolation
- automatic loop
- linked selection stable during morph
- optional scalar overlays preserved during morph
- support for more than two states later: 2D, 3D, parametric diffusion coordinates, AE, graph layout,
  solver layout, and user-provided coordinates

## View Families

The visual library exposes view families. METRIC algorithms feed these views by
exporting the right relation/property/coordinate/timeline evidence.

### Record Views

For:

- strings
- curves
- histograms
- images
- vectors
- categorical tuples
- composed records

Views:

- dense gallery
- selected record detail
- pair comparison
- query/winner/baseline comparison
- validation issue overlay

### Relation Views

For:

- metric values
- baseline values
- similarities
- transition weights
- assignment values
- law-check values
- losses

Views:

- dense matrix
- sparse matrix
- relation histogram
- pair inspector
- relation comparison
- law diagnostics overlay

### Space Views

For:

- finite metric spaces
- baseline vector spaces
- derived coordinate spaces
- paired spaces
- graph spaces

Views:

- space overview
- pair matrix
- graph topology
- coordinate states
- storage/representation metadata
- lineage and before/after comparison

### Property Views

For:

- entropy
- density
- local volume
- intrinsic dimension
- outlier score
- cluster id
- representative assignment
- reconstruction error
- solver residual
- calibration loss

Views:

- scalar overlay on point cloud
- scalar histogram
- threshold brush
- ranked record list
- property over time

### Graph Views

For:

- kNN graphs
- radius graphs
- transition graphs
- diffusion graphs
- solver graphs
- sparsified graphs
- connected components

Views:

- node/edge scene
- path highlight
- neighborhood brush
- component overlay
- edge-weight matrix
- graph timeline

### Derived Coordinate Views

For:

- parametric diffusion coordinates
- parametric latent spaces
- PCFA
- Redif
- generic derived coordinate spaces
- derived maps and extension behavior

Views:

- source/target coordinate morph
- preservation matrix comparison
- reconstruction view
- residual scalar field
- out-of-sample query view
- training timeline if available

### Dynamics Views

For:

- diffusion
- reverse diffusion
- finite metric dynamics
- Redif inverse-dynamics
- resampling
- transition processes

Views:

- transition graph
- state timeline
- path animation
- before/after space comparison
- scalar change overlay
- density correction view

### Solver Views

For:

- Laplacian solver
- PCG
- approximate Cholesky
- sparse CSC
- parametric coordinate solver calibration

Views:

- residual timeline
- loss timeline
- sparse pattern
- solution scalar field
- graph solver scene
- layer shape and parameter norm view

## Algorithm Coverage Through Evidence

Algorithms are not visual primitives. They become visible by exporting records,
relations, spaces, properties, graphs, coordinates, and timelines.

### Search

Exports:

- query record
- result records
- relation values
- margins
- optional frontier/neighborhood graph

Views:

- query comparison
- nearest result list
- pair matrix row
- point cloud highlight

### Sampling

Exports:

- selected record ids
- sample order
- coverage property
- assignment to sampled records

Views:

- sample animation
- coverage overlay
- assignment matrix

### Entropy, Density, Local Volume, Intrinsic Dimension

Exports:

- per-record scalar properties
- estimator metadata
- optional convergence timeline

Views:

- scalar overlay
- histogram
- ranked record list
- local neighborhood inspector

### Clustering, Groups, Outliers, Representatives

Exports:

- group ids
- cluster ids
- outlier scores
- representative ids
- assignment relations

Views:

- cluster color overlay
- representative coverage
- outlier ranking
- component/cluster graph

### Correlation And MGC

Exports:

- paired spaces
- aligned record ids
- relation matrices for both spaces
- MGC local grid
- statistic
- permutation/null distribution

Views:

- paired-space inspection
- MGC grid texture
- null distribution chart
- linked pair matrix comparison

### Metric Catalog And Admission

Exports:

- metric id
- law diagnostics
- pair relation
- admission status
- quarantine reason if rejected

Views:

- metric discovery page
- metric law overlay
- pair distribution
- quarantine report

### Derived Coordinates, parametric diffusion coordinates, Parametric Latent Spaces, PCFA

Exports:

- source space
- target coordinate spaces
- preservation diagnostics
- reconstruction data where applicable
- extension evidence
- calibration timeline where applicable

Views:

- coordinate scene
- 2D/3D/state morph
- preservation matrix comparison
- reconstruction residual overlay
- out-of-sample query inspector

### Diffusion, Reverse Diffusion, Denoise, Resample

Exports:

- transition graph
- timeline states
- before/after properties
- generated or retained record ids
- density correction evidence

Views:

- graph timeline
- state morph
- density/entropy overlay
- before/after record comparison

### Graph And Numeric Solvers

Exports:

- graph or sparse matrix structure
- iteration timeline
- residuals
- solution values
- convergence diagnostics

Views:

- graph scene
- sparse matrix texture
- residual chart
- scalar solution overlay

## Parallel Implementation Plan

The engine can be built in parallel because the import/export format is the
contract.

### Track 1: Visual Format

Deliverables:

- `metric.visual.v1` schema document
- JSON validator in plain JavaScript
- C++ writer helper plan
- compatibility importer for current `metric.evidence.v1`

Acceptance:

- process-curve evidence loads through the compatibility importer
- new fixtures can be authored directly in `metric.visual.v1`

### Track 2: GPU Core

Deliverables:

- scene core
- camera
- point layer
- graph edge layer
- matrix texture layer
- scalar color ramp
- picking
- linked selection
- timeline
- 2D/3D morph

Acceptance:

- synthetic 100k point cloud fixture is interactive
- synthetic 1M-cell matrix fixture uses a texture, not DOM/SVG cells
- selection links points, matrix cells, graph edges, and record details

### Track 3: Record And Relation Views

Deliverables:

- record gallery framework
- curve renderer
- string renderer
- histogram renderer
- image renderer
- composed record renderer
- relation matrix view
- pair inspector

Acceptance:

- strings, process curves, histograms, images, and mixed records render from the
  same format

### Track 4: Property And Graph Views

Deliverables:

- scalar overlays
- scalar histograms
- threshold brush
- graph topology view
- path highlight
- component/cluster overlay

Acceptance:

- entropy, density, clustering, outlier, and graph evidence render as layers on
  the same record scene

### Track 5: Mapping And Dynamics Views

Deliverables:

- coordinate-state manager
- multi-state morph
- preservation matrix comparison
- reconstruction/residual panels
- diffusion timeline
- reverse diffusion timeline
- resampling/density correction view

Acceptance:

- parametric diffusion coordinates, parametric latent spaces, PCFA, diffusion, Redif inverse-dynamics, and resampling evidence share
  the same coordinate/timeline primitives

### Track 6: Solver And Numeric Views

Deliverables:

- sparse matrix texture view
- residual/loss timelines
- graph solver overlay
- coordinate-solver calibration trace view
- numeric diagnostic panes

Acceptance:

- Laplacian, PCG, approximate Cholesky, sparse matrix, and coordinate-solver traces can be
  inspected without custom one-off pages

### Track 7: Hero Workspaces

Deliverables:

- process-curve condition monitoring workspace
- mixed structured records workspace
- histogram/image transport workspace
- cross-space dependency workspace
- parametric diffusion coordinates and parametric-coordinate workspace
- finite dynamics workspace
- graph solver workspace

Acceptance:

- each workspace is a configuration of the same visual library
- each workspace has a C++ exporter and static fallback
- no workspace contains hidden algorithm logic in JavaScript

## Implementation Order

1. Stabilize the format boundary with `metric.visual.v1`.
2. Keep a compatibility loader for the current process-curve
   `metric.evidence.v1` export.
3. Build GPU core with point cloud, matrix texture, graph layer, selection, and
   morph timeline.
4. Convert process-curve evidence into the new generic model.
5. Add record and relation views.
6. Add property overlays for entropy, density, clustering, and outliers.
7. Add graph and dynamics timelines.
8. Add mapping and solver traces.
9. Build hero workspaces as configurations, not as one-off pages.

## Definition Of Done

The visual engine is successful when:

- it is a standalone native browser library inside the METRIC repo
- it accepts a stable import/export format
- it can visualize arbitrary records and pair relations
- finite metric spaces are one central use case, not a hardcoded demo path
- dense point clouds, dense matrices, graphs, scalar fields, and timelines are
  GPU-backed
- 2D/3D morph is a generic coordinate-state feature
- entropy, density, clustering, mapping, dynamics, and solvers are visible as
  layers and timelines
- all algorithmic values come from C++ evidence
- hero applications are built as workspaces over the same visual primitives
