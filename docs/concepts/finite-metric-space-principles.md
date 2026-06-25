# Finite Metric-Space Principles And Special Cases

METRIC is a library for computing with finite metric spaces. Classical
statistics and ML procedures may appear as applications, special coordinate
cases, solver components, or benchmark references, but they are not the public
conceptual center.

The core object is always a finite set of records with an admitted metric:

```text
X = {x_i}, d: X x X -> R_{\ge 0}
```

Everything else is derived from that object: neighborhoods, balls, density,
transition probabilities, entropy, maps, representative spaces, quotient spaces,
and dynamics.

## Terminology Rule

Use the most general finite metric-space term first. Name a classical algorithm
only when it is the concrete implementation of that term.

| Classical wording | Finite metric-space wording in METRIC |
| --- | --- |
| feature vector | record in a finite space; vector coordinates are one possible chart |
| feature engineering | record recoding with an explicit metric or admitted coordinate chart |
| model | operator, derived map, mapping artifact, or solver component |
| fit / training | derive or calibrate an operator from `(X, d)`; use solver-specific wording only inside solver code |
| embedding / dimensionality reduction | derived coordinate space with preservation diagnostics |
| clustering | grouping, partition, quotient, or representative structure of the space |
| DBSCAN density-unassigned | density-unassigned records under one density grouping rule |
| denoising | inverse metric dynamics; for arbitrary records this is Redif measure dynamics |
| noise injection | forward metric dynamics or entropy-increasing diffusion over atoms |
| Gaussian noise | Euclidean-coordinate perturbation special case |
| anomaly detection | singular-record scoring by metric structure, density, coverage, or dynamics path length |
| kernel | affinity or transition relation derived from distances |
| probability | measure, density, or transition probability derived from distances and local volume |
| generative diffusion | parametric/vector special case of dynamics over a measure on finite-space atoms |
| classifier / regressor | downstream decision layer over metric-space results; not a core metric-space primitive |

Some finite metric-space operations have no useful simplified ML equivalent:

| METRIC operation | Why no direct simplified ML equivalent is required |
| --- | --- |
| metric-law admission and quarantine | The question is whether a proposed distance satisfies metric laws, not whether a predictor performs well. |
| pairwise metric-value execution planning | The concern is exactness, storage, and route validity for `(X, d)`. |
| lineage-preserving representative spaces | The output is a new finite metric space with source-record provenance. |
| Redif measure paths on arbitrary records | The state is a probability measure over atoms, so the record domain need not be vectorial. |
| entropy trajectory under metric dynamics | Entropy is read from measures and transitions induced by `d`, not assumed as external randomness. |
| transport path length for singular records | Outlier strength can be a dynamical transport property, not a classifier score. |

## Noise And Dynamics

In a finite metric space, disorder is a dynamical statement. A metric induces
neighborhoods, local volume, affinities, transition probabilities, and therefore
evolution of measures. Forward dynamics spreads mass and tends to increase
entropy. Inverse dynamics contracts or reconstructs structure relative to the
intrinsic metric.

`redif_add_noise` is the forward disorder operation: each atom starts as a Dirac
measure and evolves through metric-derived transitions.

`redif_remove_noise` is inverse Redif dynamics: it builds the local distance matrix,
derives the Redif weighted graph Laplacian, and applies the implicit Euler
inverse-diffusion step over measures. It is coordinate-free with respect to the
record domain.

`density_filter` is not denoising. It removes records that a concrete density
grouping rule leaves unassigned. That is useful for density filtering and
singular-record inspection, but it is not inverse dynamics.

## Specialization Boundary

Vector spaces are an important special case because coordinates allow Euclidean
perturbations, parametric coordinate maps, and concrete solver components. Those components must
stay below the metric-space abstraction:

- Public concept: finite metric space, metric, derived space, map, dynamics.
- Implementation detail: PCFA, diffusion-coordinate target, coordinate solver, DBSCAN
  density grouping, Euclidean perturbation.
- Result contract: lineage, metric-law status, preservation diagnostics, route
  exactness, and validity bounds.

The library should never make a user believe that a classical algorithm is the
primitive when the finite metric-space principle is more general.
