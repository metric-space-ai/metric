# METRIC Engine Overview

METRIC is organized around finite metric spaces.

```text
RecordSet + Metric -> MetricSpace -> Intent -> Strategy -> Representation -> Runtime -> Result
```

This model is the public architecture of the engine.

The first implementation slices are documented in [Engine Mental Model](mental-model.md).

## Engine Guides

- [Metric Space](metric-space.md)
- [Representations](representations.md)
- [Intent API](intent-api.md)
- [Strategies](strategies.md)
- [Operators](operators.md)
- [Mappings](mappings.md)
- [Runtime Policies](runtime.md)
- [Migration](migration.md)

## Flagship Examples

The first CI-tested engine demos live under `examples/engine/` and `python/examples/engine/`.
They start with non-vector records and use the engine vocabulary directly:

- strings with edit distance for neighbor search and grouping
- process curves with an alignment metric, representative selection, and graph construction
- histograms with cumulative transport distance, matrix materialization, and grouping
- cross-space dependency over two aligned finite metric spaces

## Core Idea

A metric space is not a vector matrix. It is a set of records with a distance function.

When the record set is finite, the metric space becomes a practical numerical object:

```text
finite records + metric = finite metric space
```

METRIC computes on that object directly. The metric defines the geometry; vectors are only one possible record type.

## System Model

### RecordSet

Records can be strings, vectors, images, histograms, time series, graphs, structured objects, or mixed domain records.

### Metric

The metric defines the geometry. It can be Euclidean distance, edit distance, time-warp distance, Earth mover distance, structural similarity converted to distance, or a custom domain metric.

### MetricSpace

The metric space owns the records, stable IDs, metric, metadata, and version state.

### Intent

The public operation is named by what the user wants:

- find neighbors
- find groups
- embed
- map
- reduce
- denoise
- find outliers
- compare spaces

### Strategy

Strategies select algorithms:

- brute force
- matrix cache
- cover tree
- kNN graph
- k-medoids
- DBSCAN
- MGC
- MDS
- diffusion embedding
- PCFA
- SOM/KOC
- PHATE-AE-style learned mapping

Algorithm names live here. A normal user can use the engine without learning the algorithm catalog first; an expert can choose strategies explicitly.

The first C++ strategy objects are available under `metric::strategies` for implemented engine paths: `brute_force`, `matrix_cache`, `cover_tree`, `knn_graph`, `k_medoids`, `dbscan`, and `farthest_first`.

### Representation

Representations are execution structures over the same metric space:

- implicit distance provider
- matrix cache
- cover-tree index
- kNN graph index
- graph topology
- learned mapping model

Representations are runtime choices and expert controls. They are not separate user concepts.

The first engine representation adapters are available in C++ under `metric::representations`. They cover implicit distance lookup, eager matrix caching, exact neighbor-index scaffolds, kNN graph adjacency, and graph topology edges over one `MetricSpace`.

### Runtime

Runtime policies make cost and approximation explicit:

- exact vs approximate
- lazy vs materialized
- dense vs sparse
- serial vs parallel
- deterministic vs stochastic
- CPU vs accelerator-backed execution

### Result

Engine APIs return named result objects, not raw tuples.

Result objects preserve:

- source record IDs
- strategy metadata
- representation metadata
- diagnostics
- warnings
- metric assumptions
- source-to-target lineage when a derived space or mapping is produced

The first engine result objects are `metric::NeighborSet<Distance>`, returned by nearest-neighbor operators over spaces, distance providers, and neighbor indexes; `metric::ClusteringResult<Distance>`, returned by k-medoids and DBSCAN grouping operators; `metric::RepresentativeSet<Distance>`, returned by representative-selection intents; `metric::OutlierResult<Score>`, returned by outlier-detection intents; `metric::StructureDescription<Distance>`, returned by exact structure diagnostics; `metric::EntropyResult<double>`, returned by entropy diagnostics; `metric::CorrelationResult<double>`, returned by MGC cross-space statistics; and `metric::MappingResult<DerivedSpace>`, returned by mapping adapters that produce derived metric spaces.

The first C++ intent helpers are `metric::find_neighbors`, `metric::find_groups`, `metric::find_representatives`, `metric::find_outliers`, `metric::describe_structure`, `metric::compare`, `metric::correlate`, deterministic `metric::map`, DBSCAN-backed `metric::denoise`, the PCFA-backed `metric::embed` helper in `<metric/intent/embed.hpp>`, and the PCFA-backed `metric::reduce` helper in `<metric/intent/reduce.hpp>`. They keep the user-facing vocabulary semantic while routing to explicit strategy objects such as `metric::strategies::cover_tree`, `metric::strategies::matrix_cache`, `metric::strategies::k_medoids`, `metric::strategies::dbscan`, `metric::strategies::farthest_first`, `metric::strategies::mgc`, and `metric::strategies::pcfa`.

The first C++ mapping adapters are `metric::mappings::make_clustered_space_mapping`, deterministic `metric::map`, PCFA-backed `metric::embed`, and the PCFA adapter in `<metric/mappings/pcfa.hpp>`. They fit or transform source spaces into derived metric spaces while preserving source `RecordId` lineage.

## Capabilities

| Capability | User question | Typical strategies |
|---|---|---|
| `neighbors` | Which records are similar to this one? | brute force, matrix cache, cover tree, kNN graph |
| `groups` | What groups or representatives exist? | k-medoids, DBSCAN, affinity propagation |
| `embed` | Show this space in coordinates. | MDS, diffusion embedding, PCFA |
| `map` | Learn or apply a transformation to another space. | deterministic transforms, PCFA, PHATE-AE-style mapping |
| `reduce` | Make the space smaller or cheaper while preserving structure. | representatives, medoids, PCFA, SOM |
| `compress` | Retain a smaller representative space with explicit loss metadata. | representatives, medoids, sparse graph |
| `denoise` | Remove local noise or manifold artifacts. | DBSCAN noise filter, graph smoothing, reverse diffusion |
| `outliers` | Which records are unusual? | local density, DBSCAN noise, KOC/manifold distance |
| `compare` | Are two metric spaces related? | MGC, distance correlation |

## Composition

METRIC exposes both low-level building blocks and high-level end-to-end capabilities.

Target shorthand:

```text
space.map(preserve="diffusion_geometry", learn_mapping=True)

MetricSpace
  -> NeighborGraph
  -> AffinityKernel
  -> DiffusionOperator
  -> DiffusionPotential
  -> MetricEmbedding
  -> NeuralMapper
  -> MappingModel
```

This is how METRIC avoids becoming a flat algorithm catalog. The public API stays simple, while the internal composition remains explicit and replaceable.

## Vector Spaces

Vector spaces remain important. They are just not the foundation.

In METRIC, a vector dataset with Euclidean distance is a finite metric space:

```text
vectors + Euclidean metric -> MetricSpace
```

The same engine model works for strings with edit distance, histograms with transport distance, time series with time-warp distance, graphs with structural distance, and mixed records with domain metrics.

## Compatibility Layer

Existing lower-level C++ names remain available as compatibility and expert APIs:

- `metric::Matrix`
- `metric::Tree`
- `metric::KNNGraph`
- mapping classes
- transform classes
- tuple-returning low-level algorithms

The engine layer is the recommended public entry point. Compatibility APIs are kept as stable adapters around the same metric-space model.
