# Embedding And Mapping Roadmap

## Purpose

This document defines the roadmap for embedding and mapping in METRIC as an
engine for finite metric spaces.

The goal is not to add a catalog of dimensionality-reduction algorithms. The
goal is to expose a small set of user-facing capabilities that are composed
from interchangeable metric-space building blocks:

- `embed`: produce a coordinate representation of a finite metric space
- `reduce`: produce a lower-complexity space or representation
- `map`: fit or apply a transformation from one space to another
- `inverse_transform`: reconstruct source-like records when a mapping supports it
- `transform`: apply a fitted mapping to out-of-sample records or spaces when supported

Algorithm names such as MDS, PHATE-style diffusion embedding, SOM, PCFA, and
PHATE-AE belong in strategy objects. They should not become the primary
user-facing vocabulary.

PHATE-AE should be treated as a strategy for a learnable structure-preserving
map. It is not the primary user concept. The primary user concept is still
`map`, selected with a strategy that preserves diffusion geometry and optionally
learns an encoder and decoder.

## Core Distinction

METRIC must distinguish two related but different operations.

| Concept | Meaning | Output | Typical API |
|---|---|---|---|
| Vector-space embedding | Represent records as coordinate vectors for inspection, indexing, downstream modeling, or visualization | A metric space whose records are vectors, usually with a vector metric | `embed(space, strategy=...)` |
| Metric-space transformation | Transform one finite metric space into another finite metric space, which may or may not use vectors | A derived metric space plus lineage, diagnostics, and optional model | `reduce(...)` or `map(...)` |

A vector embedding is one possible metric-space transformation. It is not the
foundation of METRIC. The foundation is still:

```text
RecordSet + Metric -> MetricSpace -> Representation -> Operator/Mapping -> Result
```

Examples:

- edit-distance strings can be embedded into two Euclidean coordinates for
  visualization, but the source geometry remains the edit-distance space
- a time-series space can be reduced with PCFA into feature records and decoded
  approximately
- an image or histogram space can be mapped onto a SOM graph, producing a
  structured target space rather than only coordinates
- a PHATE-style strategy can build diffusion geometry from any distance provider
  and then solve for coordinates with MDS
- a PHATE-AE-style strategy can learn an out-of-sample mapper that preserves a
  diffusion embedding objective

## Naming Rules

User-facing APIs must express intent:

- `space.embed(...)`
- `space.reduce(...)`
- `space.map(...)`
- `model.transform(...)`
- `model.inverse_transform(...)`
- C++ free functions with the same intent names, such as `metric::embed`

Strategy objects express how the intent is executed:

- `strategies::mds`
- `strategies::diffusion_embedding`
- `strategies::phate_diffusion`
- `strategies::pcfa`
- `strategies::som`
- `strategies::phate_ae`

Do not make these the primary user path:

- `space.mds(...)`
- `space.phate(...)`
- `space.phate_ae(...)`
- `space.pcfa(...)`
- top-level tuple-return APIs for new engine features

Direct algorithm APIs may remain as compatibility or expert APIs, but examples,
docs, and Python facade methods should lead with the intent names.

## Target User API

### Python

```python
from metric import Space
from metric.metrics import EditDistance
from metric.strategies import DiffusionEmbedding, MDS, PCFA, PhateAE

space = Space(["cat", "cot", "coat", "dog"], metric=EditDistance())

embedding = space.embed(dimensions=2, strategy=DiffusionEmbedding())
mds_view = space.embed(dimensions=2, strategy=MDS(kind="metric"))

reduced = space.reduce(dimensions=2, strategy=PCFA())

model = space.map(strategy=PhateAE(dimensions=2))
mapped_space = model.transform(space)
reconstructed = model.inverse_transform(mapped_space)
```

The example starts from strings and edit distance on purpose. Vector records are
supported, but they are not the conceptual starting point.

### C++

```cpp
#include <metric/engine.hpp>

auto space = metric::make_space(records, metric::Edit<char>{});

auto embedding = metric::embed(
    space,
    metric::dimensions{2},
    metric::strategies::diffusion_embedding{});

auto mds_view = metric::embed(
    space,
    metric::dimensions{2},
    metric::strategies::mds{});

auto reduced = metric::reduce(
    space,
    metric::dimensions{2},
    metric::strategies::pcfa{});

auto model = metric::map(
    space,
    metric::strategies::phate_ae{.dimensions = 2});

auto mapped_space = model.transform(space);
auto reconstructed = model.inverse_transform(mapped_space);
```

The exact spelling can evolve, but the shape should remain stable: intent first,
strategy second.

## Capability Model

`embed`, `reduce`, and `map` are composed capabilities. Each capability should
be inspectable as a pipeline of engine building blocks.

```text
MetricSpace
  -> Representation
  -> Operator pipeline
  -> Strategy-specific solver or trainer
  -> Result object
  -> Derived MetricSpace or MappingModel
```

Important building blocks:

- source `MetricSpace`
- `DistanceProvider`
- `MatrixCache`
- `KnnGraphIndex`
- `GraphTopology`
- affinity kernel
- diffusion operator
- diffusion potential or diffusion distance
- MDS or stress solver
- PCFA encoder/decoder
- SOM prototype graph
- neural encoder/decoder trainer
- runtime policy
- result object with lineage and diagnostics

The engine should let advanced users replace these pieces without changing the
top-level intent API.

## API Semantics

### `embed`

`embed` creates vector records from a finite metric space. It is appropriate
when the requested result is coordinates.

Expected output:

- `EmbeddingResult`
- coordinate records
- `MetricSpace<VectorRecord, VectorMetric>` for the embedded coordinates
- lineage from source `RecordId` to coordinate record
- solver diagnostics

`embed` does not always imply an out-of-sample `transform`. Some embedding
strategies only embed the fitted finite set. Out-of-sample support must be
explicit in the result or fitted model.

### `reduce`

`reduce` lowers complexity while preserving a declared relation. The output may
be coordinates, prototypes, representatives, sparse structure, or a compressed
space.

Expected output:

- `ReductionResult`
- reduced space or representation
- source-to-reduced lineage
- reconstruction support if available
- preservation diagnostics
- compression and loss diagnostics where meaningful

`reduce` is broader than embedding. A SOM reduction, a representative-set
reduction, and a PCFA feature reduction are all reductions, but only some of
them are vector embeddings.

### `map`

`map` fits or applies a transformation from a source space to a target space.
It is the right API when the transformation is a reusable model.

Expected output:

- `MappingModel` or `MappingResult`
- `transform` support when the model can handle out-of-sample records
- optional `inverse_transform`
- target schema and target metric
- training diagnostics
- preservation diagnostics
- source-space version and strategy provenance

`map` is the correct home for learnable structure-preserving methods, including
PHATE-AE-style models.

## Strategy Families

| Intent | Strategy family | Role |
|---|---|---|
| `embed` | MDS | Solve coordinates from a distance provider or derived distance matrix |
| `embed` | PHATE-style diffusion embedding | Build diffusion geometry, then solve coordinates |
| `embed` | spectral or Laplacian embedding | Use graph operators to produce coordinates |
| `reduce` | PCFA | Encode vector-like records into lower-dimensional features with optional decode |
| `reduce` | SOM/KOC | Quantize or structure records on a prototype graph |
| `reduce` | representative selection | Produce a smaller finite metric space with source lineage |
| `reduce` | DSPCC and transform-assisted reducers | Combine deterministic transforms with feature reduction |
| `map` | deterministic transform | Apply a known record transform and target metric |
| `map` | parametric encoder/decoder | Fit reusable transform and optional inverse |
| `map` | PHATE-AE-style strategy | Learn an encoder/decoder that preserves diffusion structure |

Strategies must declare:

- required input capabilities
- whether they need a full pairwise matrix
- whether they can use a sparse graph
- whether they produce coordinates, prototypes, or arbitrary target records
- whether they support `transform` for out-of-sample records
- whether they support `inverse_transform`
- deterministic or stochastic behavior
- runtime and memory complexity expectations

## PHATE-Style Diffusion Embedding

PHATE-style diffusion embedding is a strategy for `embed`, not a primary API
name.

Canonical pipeline:

```text
MetricSpace
  -> kNN graph or distance provider
  -> affinity kernel
  -> row-stochastic diffusion operator
  -> diffusion time selection
  -> diffusion potential or diffusion distance
  -> MDS or landmark MDS coordinate solver
  -> EmbeddingResult
```

Required design decisions:

- support source records that are not vectors
- compute from a `DistanceProvider`, not only from a dense matrix
- make graph construction and affinity kernel replaceable
- expose diffusion time and kernel parameters through strategy options
- record whether the strategy used exact, approximate, dense, sparse, or landmark mode
- return diagnostics for connectedness, diffusion time, solver stress, and warnings

The output coordinates are a vector-space view of a metric-space construction.
They should not hide the source metric or source record IDs.

## MDS

MDS is a coordinate solver strategy. It can be used directly by `embed` or as a
solver inside diffusion strategies.

Initial variants:

- classical MDS for Euclidean-compatible distance matrices
- metric MDS for stress minimization
- nonmetric MDS later, if needed
- landmark MDS for scalability and out-of-sample approximation

MDS result diagnostics should include:

- stress or residual
- eigenvalues where applicable
- convergence status
- number of iterations
- source distance provider
- whether distances were squared, centered, normalized, or clipped

MDS should not be documented as the user goal. The goal is still `embed`.

## SOM, KOC, And PCFA

SOM, KOC, and PCFA should be adapted as strategies behind `reduce` and `map`.

PCFA:

- best framed as a feature-reduction and mapping strategy for vector-like or
  structured records
- should expose `encode` through `transform`
- should expose `decode` through `inverse_transform`
- should produce a result object with weights, averages, eigenmodes, and
  reconstruction diagnostics

SOM/KOC:

- best framed as structure-preserving quantization or graph mapping
- may produce target records that are prototype IDs, prototype records, graph
  nodes, activation vectors, or distances to prototypes
- should keep the SOM graph as part of the target-space representation
- should preserve source-to-prototype assignments and quantization error
- inverse support is optional and strategy-dependent

Neither PCFA nor SOM should be the first concept in user documentation. Users
ask METRIC to reduce or map a space; PCFA and SOM are ways to do it.

## Learnable Mappings

Learnable mappings are fitted `map` strategies. They should use the same
interface as deterministic mappings.

Common contract:

```text
fit(source_space) -> MappingModel
model.transform(source_space_or_records) -> target_space_or_records
model.inverse_transform(target_space_or_records) -> reconstructed records, if supported
model.diagnostics() -> training and preservation diagnostics
```

Learnable mappings must declare the structure they preserve:

- source distances
- diffusion distances
- graph neighborhoods
- ranks
- labels or supervised targets
- reconstruction loss
- cross-space relation

PHATE-AE-style mapping belongs here:

```text
MetricSpace
  -> PHATE-style diffusion target or loss
  -> neural encoder
  -> optional neural decoder
  -> MappingModel with transform and optional inverse_transform
```

The public wording should be "learn a structure-preserving map" or "map with a
PHATE-AE strategy", not "run PHATE-AE" as the primary operation.

## Out-Of-Sample Transform

Out-of-sample behavior must be explicit. A result object must never imply that a
strategy supports new records unless it actually does.

Support levels:

| Level | Meaning | Examples |
|---|---|---|
| none | Coordinates exist only for the fitted finite set | basic dense MDS, basic diffusion embedding |
| approximate | New records can be projected through landmarks, Nystrom-style extension, or regression | landmark MDS, landmark diffusion embedding |
| parametric | A fitted function maps new records directly | PCFA, autoencoder, PHATE-AE-style encoder |
| incremental | The model can update without full refit | future work |

API requirements:

- `result.supports_transform`
- `model.supports_transform`
- clear error messages when unsupported
- diagnostics that identify approximation mode
- target-space metric and schema for transformed records

## Inverse Mappings

`inverse_transform` is optional and must be treated as reconstruction, not as a
guaranteed mathematical inverse.

Strategies should declare one of:

- no inverse
- deterministic inverse
- approximate decoder
- probabilistic or sampled reconstruction
- inverse by nearest source representative

Result diagnostics should include:

- reconstruction error when ground truth is available
- source metric error
- target metric error
- invalid or out-of-domain reconstruction warnings
- whether reconstruction returns original record type or source-like records

## Result Objects

New embedding and mapping APIs must return named result objects, not tuples.

### `EmbeddingResult`

Fields:

- `coordinates`
- `embedded_space`
- `source_ids`
- `target_metric`
- `strategy`
- `representation_used`
- `supports_transform`
- `model`, when a reusable model exists
- `diagnostics`
- `warnings`

### `ReductionResult`

Fields:

- `reduced_space`
- `source_ids`
- `assignments` or `lineage`, when applicable
- `representatives` or `prototypes`, when applicable
- `model`, when fitted
- `supports_inverse_transform`
- `compression_ratio`
- `loss`
- `diagnostics`
- `warnings`

### `MappingResult` And `MappingModel`

Fields:

- `model`
- `source_space_version`
- `source_metric_traits`
- `target_schema`
- `target_metric`
- `supports_transform`
- `supports_inverse_transform`
- `training_history`
- `preservation_scores`
- `runtime_policy`
- `diagnostics`
- `warnings`

All result objects should preserve enough provenance to reproduce the run:

- strategy type and options
- representation type and parameters
- random seed
- runtime policy
- source-space version

## Runtime Policy

Embedding and mapping strategies can be expensive. The runtime must make
materialization choices explicit.

Policies to support:

- exact vs approximate
- dense vs sparse
- lazy vs materialized
- serial vs parallel
- deterministic vs stochastic
- memory budget
- time budget

No promoted API should silently materialize an O(n^2) distance matrix without a
documented policy or diagnostic. If a strategy requires dense distances, the
strategy should say so before execution when possible.

## Documentation Requirements

Docs and examples must reinforce the engine model:

- start with arbitrary records and metrics, not only Euclidean vectors
- show `embed`, `reduce`, and `map` before algorithm names
- put MDS, PHATE-style diffusion, PCFA, SOM, and PHATE-AE in strategy sections
- explain when an embedding is only a coordinate view
- explain when a mapping is reusable out-of-sample
- explain when inverse transformation is unsupported or approximate
- show result objects and diagnostics

Example documentation titles:

- "Embedding A Finite Metric Space"
- "Reducing A Metric Space"
- "Mapping Between Metric Spaces"
- "Strategies For Embedding And Mapping"
- "Out-Of-Sample Transform"
- "Inverse Transform And Reconstruction"

Avoid titles such as "PHATE-AE API" or "Run MDS" for the primary docs.

## Implementation Roadmap

### Phase 1: Concepts And Results

Deliverables:

- `EmbeddingResult`
- `ReductionResult`
- `MappingResult`
- `MappingModel` concept
- strategy capability traits
- target-space schema metadata
- source-to-target lineage primitives

Acceptance criteria:

- no new tuple-return API
- every result carries source `RecordId` lineage
- unsupported `transform` and `inverse_transform` states are explicit

### Phase 2: First `embed`

Deliverables:

- `metric::embed`
- `Space.embed`
- MDS strategy adapter
- PHATE-style diffusion embedding strategy adapter
- dense and graph-backed distance providers
- diagnostics for stress, graph connectedness, and runtime policy

Acceptance criteria:

- non-vector records can be embedded from a metric
- embedded coordinates are returned as a derived metric space
- strategy names appear only as strategy objects in public examples
- no hidden dense matrix allocation without diagnostics

### Phase 3: First `reduce`

Deliverables:

- `metric::reduce`
- `Space.reduce`
- PCFA strategy adapter
- SOM/KOC strategy adapter
- representative-space reduction adapter
- reconstruction diagnostics for PCFA
- quantization diagnostics for SOM/KOC

Acceptance criteria:

- at least one reduction returns vector features
- at least one reduction returns a non-vector derived space
- result lineage maps source records to reduced records or prototypes
- inverse support is optional and visible

### Phase 4: First `map`

Deliverables:

- `metric::map`
- `Space.map`
- reusable mapping model interface
- deterministic transform adapter
- PCFA mapping adapter
- out-of-sample transform contract
- inverse-transform contract

Acceptance criteria:

- fitted mappings can transform the training space
- supported mappings can transform new records
- unsupported mappings fail with clear errors
- mapping results identify target record type and target metric

### Phase 5: Learnable Structure-Preserving Maps

Deliverables:

- PHATE-AE-style strategy object
- diffusion-geometry loss adapter
- neural encoder adapter
- optional decoder adapter
- training diagnostics
- preservation metrics

Acceptance criteria:

- public API remains `map(..., strategy=PhateAE(...))`
- docs describe the user goal as learnable structure-preserving mapping
- the strategy can report transform and inverse-transform support separately
- the result records both reconstruction loss and structure-preservation loss

### Phase 6: Python Facade And Examples

Deliverables:

- `metric.strategies` exports for MDS, DiffusionEmbedding, PCFA, SOM, PhateAE
- `Space.embed`, `Space.reduce`, `Space.map`
- typed Python result objects
- examples with strings, time series, histograms, and vectors
- compatibility notes for legacy mapping APIs

Acceptance criteria:

- examples do not require users to import algorithm modules directly
- examples show how to inspect result diagnostics
- examples show an unsupported out-of-sample transform case
- examples show one inverse-transform workflow

### Phase 7: Runtime And Scale

Deliverables:

- runtime policy options for embedding and mapping
- dense/sparse/landmark selection
- memory diagnostics
- approximate mode diagnostics
- reusable representation cache

Acceptance criteria:

- strategy diagnostics explain representation choice
- dense matrix requirements are surfaced early
- repeated embedding or mapping workflows can reuse representations
- large-space examples use sparse or landmark modes where appropriate

## Stable Public Message

METRIC embeds, reduces, and maps finite metric spaces. A vector embedding is one
possible output, not the foundation. PHATE-style diffusion, MDS, SOM, PCFA, and
PHATE-AE are strategies that execute user intent through the engine.

The stable mental model is:

```text
User intent: embed, reduce, map
Strategy: MDS, diffusion embedding, PCFA, SOM, PHATE-AE, ...
Engine: MetricSpace, Representation, Operator, Runtime
Result: named object with space, model, lineage, diagnostics, and capabilities
```
