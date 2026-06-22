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
- algorithm outputs
- search results
- entropy, density, intrinsic dimension, local volume
- clustering, groups, outliers, representatives
- MGC and paired-space dependence evidence
- diffusion, reverse diffusion, dynamics, denoise and resampling results
- mapping outputs such as PHATE, PCFA, SOM/KOC, AE latent spaces
- solver traces, DNN losses, graph solver residuals
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
- No adapter to Babyplots, Babylon, Three.js, D3, Plotly, deck.gl, or other
  visualization libraries.
- Plain JavaScript ES modules, Canvas 2D, and raw WebGL.
- METRIC-owned GLSL shaders are part of the engine, not decoration.
- GPU 3D is the primary runtime.
- Canvas 2D is allowed for record panels, small charts, labels, and fallback
  surfaces.
- SVG is only a static documentation fallback, not the main engine.
- Babyplots may be studied and code may be ported with license obligations, but
  no Babyplots names, APIs, data contracts, dependencies, or namespace artifacts
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
- DNN loss
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
- PHATE 2D/3D
- AE latent
- PCFA
- SOM/KOC grid
- graph layout
- solver scalar field lifted to 3D
- user-provided coordinates

### `timelines`

Timelines describe state changes over existing records.

Examples:

- 2D to 3D morph
- diffusion steps
- reverse diffusion steps
- denoise before/after
- resampling before/after
- mapping training epochs
- graph solver iterations
- DNN loss and latent-space evolution

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
  outlier score, residuals, and mapping loss
- coordinate morphs between 2D, 3D, PHATE, AE, PCFA, SOM/KOC, graph layouts,
  and solver layouts
- trails and temporal fades for diffusion, reverse diffusion, resampling,
  denoise, and iterative solvers
- picking passes that keep selection accurate for dense point and graph scenes
- optional postprocessing for glow, depth haze, high-contrast selection, and
  publication-quality screenshots

The default art direction is isometric miniature photography. A METRIC scene
should feel like a small physical model of a finite metric space: a stable
ground plane, orthographic/isometric camera, shallow focus, dense relation
fields, and precise linked objects. The ground is not empty decoration. It can
show a semantic projection of the same records: an interpretable 2D shadow,
PHATE/AE target state, cluster footprint, density/entropy field, or other
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

It is not a special PHATE/AE feature. It applies to any records with multiple
coordinate states.

Requirements:

- same record ids across coordinate states
- slider-controlled interpolation
- automatic loop
- linked selection stable during morph
- optional scalar overlays preserved during morph
- support for more than two states later: 2D, 3D, PHATE, AE, graph layout,
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
- training loss

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

### Mapping Views

For:

- PHATE
- AE latent spaces
- PCFA
- SOM/KOC
- Redif
- generic derived coordinate spaces
- fitted maps and out-of-sample behavior

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
- denoise
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
- DNN/autoencoder training

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

### Mapping, PHATE, AE, PCFA, SOM/KOC

Exports:

- source space
- target coordinate spaces
- preservation diagnostics
- reconstruction data where applicable
- out-of-sample evidence
- training timeline where applicable

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

- PHATE/AE, PCFA, SOM/KOC, diffusion, denoise, and resampling evidence share
  the same coordinate/timeline primitives

### Track 6: Solver And Numeric Views

Deliverables:

- sparse matrix texture view
- residual/loss timelines
- graph solver overlay
- DNN/AE training trace view
- numeric diagnostic panes

Acceptance:

- Laplacian, PCG, approximate Cholesky, sparse matrix, and DNN traces can be
  inspected without custom one-off pages

### Track 7: Hero Workspaces

Deliverables:

- process-curve condition monitoring workspace
- mixed structured records workspace
- histogram/image transport workspace
- cross-space dependency workspace
- PHATE/AE mapping workspace
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
