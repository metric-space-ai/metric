# METRIC Vocabulary

METRIC is a finite metric-space framework. The public language must make that
clear before it names specific algorithms or papers.

For the broader comparison against classical statistics and ML wording, see
[Finite Metric-Space Principles And Special Cases](finite-metric-space-principles.md).

The order is:

```text
finite metric-space concept
-> METRIC namespace and engine component
-> concrete implementation or paper reference
```

## Level 1: Finite Metric-Space Concepts

Level 1 contains the mathematical object and the questions that can be asked of
it.

Core terms:

- observation
- record
- metric
- metric value
- finite metric space
- neighborhood, ball, cover, dimension, local structure
- entropy and information content of an embedded coordinate representation
- correlation or dependence between paired finite metric spaces
- sampling, search, clustering, representatives, outliers, and patterns inside
  a finite metric space
- maps, modified spaces, derived spaces, and dynamics over finite metric spaces

These terms are not owned by a paper. They are the project foundation.

## Level 2: METRIC Engine Components

Level 2 is the C++ component layer. It is not a place to invent abstract labels;
it names where code belongs.

| Level-1 area | Level-2 namespaces |
| --- | --- |
| Records | `mtrc::record::{import,export,print,group,compose,validate,convert}` |
| Spaces | `mtrc::space::{records,distances,cache,storage,index,select,split,merge}` |
| Metrics | `mtrc::metric::{catalog,custom,compose}` |
| Stats | `mtrc::stats::{search,sample,properties,correlate,structural_analysis}` |
| Modification | `mtrc::modify::{represent,reduce,expand,resample,map,dynamics,compose}` |
| Solvers | `mtrc::solve::{linear,laplacian,optimization,objective,parametric}` |
| Numerics | `mtrc::numeric::{scalar,vector,matrix,sparse,linear_algebra,graph,random,parallel,io}` |

Examples:

- Entropy is a Level-1 property/question of an embedded coordinate
  representation. The C++ implementation belongs under
  `mtrc::stats::properties`.
- Correlation between two paired spaces is a Level-1 question. MGC is one
  concrete implementation under `mtrc::stats::correlate`.
- A parametric diffusion coordinate workflow is a composition under `mtrc::modify::compose`, using
  metric-space access, mapping, and native solvers. Concrete solver mechanisms are not
  the framework vocabulary.
- A pairwise metric-value table is `mtrc::space::storage::DistanceTable`; it is
  an execution representation of a source space, not the source space itself.

## Level 3: Implementations And References

Level 3 contains concrete algorithms, estimators, metrics, papers, demos, and
presets.

Examples:

- Edit, TWED, EMD/Wasserstein, Euclidean, Manhattan, Chebyshev, Ruzicka
- cover tree, kNN graph, distance table, graph sparsification
- k-medoids, DBSCAN, affinity propagation, farthest-first representatives
- entropy estimators, MGC, distance-correlation variants
- PCFA, diffusion-coordinate geometry, diffusion maps, and parametric coordinate solver-backed maps
- application workflows and benchmark demos

These names are necessary for provenance and implementation detail. They should
not be presented as the conceptual center of METRIC.

## Wording Tests

Before adding a public method, document, or example, answer:

1. What is the source finite metric space?
2. Which metric is authoritative?
3. Which Level-1 question is being asked?
4. Which `mtrc` Level-2 namespace owns the C++ component?
5. Which concrete implementation or paper reference is being used?
6. What result, lineage, diagnostics, assumptions, and failure modes are
   reported?

Good:

```text
Map process windows from their source alignment-metric space into an explicit
coordinate representation, then compute entropy there. The implementation lives
in mtrc::stats::properties and uses a documented estimator with explicit
neighborhood parameters.
```

Good:

```text
Test dependence between two paired finite metric spaces. The component lives in
mtrc::stats::correlate; MGC is the concrete promoted implementation.
```

Good:

```text
Derive a map from a source finite metric space into a coordinate result space
and report neighbor preservation. The workflow lives under mtrc::modify::map or
mtrc::modify::compose and may use native parametric solvers.
```

Bad:

```text
AlgorithmName is the primitive.
PaperName is the framework concept.
```

## Metric Discovery

Metric Discovery documents admitted metrics by record domain, law status,
parameters, exact computation or algorithm, complexity, guarantees, and
references.

Non-metric distance functions are quarantined until they either become admitted
metric variants on a documented domain or leave the public metric surface.
