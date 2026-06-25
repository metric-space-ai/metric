# Finite Metric Dynamics

`mtrc::modify::dynamics` builds and evolves a process **over the structure of a
finite metric space**. It is the component home for metric-derived
probabilities, diffusion paths, forward disorder, and reverse diffusion
(reconstruction).

Its single load-bearing idea:

> **Probability is a derived interpretation of the geometry, not a primitive.**

A finite metric space gives us only pairwise distances `d(i, j)`. From those
distances — and nothing else — `dynamics` derives every stochastic object it
uses. We never start from a probability axiom; we *read probability off the
metric*.

## Level 1 — geometry becomes probability

Given the distances of a finite metric space, the component derives a heat
kernel and a Markov transition operator:

```text
W_ij = exp( -( d_ij / eps )^2 )          symmetric affinity, k-NN sparsified
P_ij = W_ij / sum_j W_ij                 row-stochastic transition matrix
```

`P_ij` is the probability of stepping from node `i` to node `j`. It exists only
because a metric exists. The bandwidth `eps` defaults to the mean k-NN distance,
so `P` is a pure function of the distances. Two consequences are worth stating as
contracts, because they are what "probability from geometry" *means*:

- **Row-stochastic.** `sum_j P_ij = 1` for every `i`. Each row is a genuine
  probability distribution that was constructed, not assumed.
- **Coordinate-free.** `P` depends on the distances alone. Any
  distance-preserving change of the records (a translation, a rotation, a
  relabelling) leaves `P` unchanged.

The invariant (stationary) distribution `pi` of the walk is read straight off the
geometry as well: for a symmetric affinity, `pi_i` is proportional to the node
degree `d_i = sum_j W_ij`, and it satisfies `pi P = pi`.

## Level 2 — the component surface

`mtrc::modify::dynamics`, re-exported into `mtrc`:

| Entry point | Role |
| --- | --- |
| `metric_transition(space, schedule)` | Derive `P`, its affinity `W`, degrees and stationary `pi` from a finite metric space. |
| `metric_random_walk(transition, start, steps, seed, walkers)` | Walk the metric graph by sampling neighbours `~ P_i`; returns visit frequencies and their total-variation gap to `pi`. |
| `redif_operator(space, options)` | Derive the inspectable Redif local-distance relation, affinity, degree measure, Laplacian, transition, and stationary measure. |
| `redif_add_noise(space, options)` | Forward metric dynamics: evolve Dirac mass at each atom into a transport path over the finite metric space. |
| `redif_remove_noise(space, options)` | Reverse Redif dynamics: apply the implicit Euler inverse-diffusion step to measures over the space atoms. |
| `redif_outliers(space, options)` | Score singular records by Wasserstein path length under Redif dynamics. |
| `metric_diffuse(space, schedule[, transition])` | Vector-record specialization: evolve a node signal with graph heat flow plus coordinate perturbation. |
| `metric_reconstruct(degraded, schedule[, transition])` | Vector-record specialization: contract a perturbed node signal back onto the structure with graph heat flow. |

`metric_transition`, `metric_random_walk`, and the Redif measure APIs work for
**any** `MetricSpaceLike` record/metric pair (strings, histograms, mixed
records): geometry -> probability needs only distances. `metric_diffuse` /
`metric_reconstruct` evolve a coordinate signal carried by the nodes, so they are
specialized to vector-valued records (a `static_assert` enforces it).

The `dynamics_schedule` carries only geometry- and reproducibility-facing knobs:
`neighbors` (graph `k`), `steps`, `diffusivity` (the per-step mixing weight
`alpha`), `perturbation_scale`, `bandwidth` (`0` ⇒ derive `eps` from the
metric), and a `seed`. None of them is a probability distribution.

## Level 1 — the dynamics

The metric structure (the graph) is the fixed substrate. A measure or signal
carried by the nodes evolves over it with operators derived from the metric.
For the vector-signal specialization, the smoothing operator is

```text
S = (1 - alpha) I + alpha P            (alpha in (0, 1])
```

- **Forward disorder (`redif_add_noise`).** Each atom starts as a Dirac measure.
  The Redif transition derived from the local distance relation spreads that
  mass through the finite space. The intrinsic signal is the path of measures,
  its transport length, and its entropy relative to the stationary distribution.

- **Reverse Redif (`redif_remove_noise`).** Redif builds the same local
  k-neighbour distance relation, derives the weighted graph Laplacian, and
  applies the implicit Euler inverse-diffusion step to measures over the
  original atoms. No coordinate chart, record recoding, or external probability
  artifact is required.

- **Vector perturbation (`metric_diffuse`).** Each step applies `S` (drift along
  the graph) and adds seeded coordinate perturbation scaled by
  `perturbation_scale`.
  This is a useful Euclidean/vector-record specialization, not the general
  definition of metric noise.

- **Vector reconstruction (`metric_reconstruct`).** The same `S` run without the
  coordinate perturbation contracts a vector signal back onto the structure. It
  is a vector-record companion to Redif's graph-Laplacian reverse diffusion.
  Because `S` is row-stochastic it obeys a **maximum principle**: a reverse step
  is a convex combination of node values, so it can never create a new
  per-coordinate extremum, and a constant signal is an exact fixed point.

Both directions are fully reproducible: a `(seed, schedule)` pair reproduces a run
bit-for-bit on any platform, because the pseudo-random draws (splitmix64 +
Box–Muller) are implemented inline rather than relying on `std::random`'s
implementation-defined distributions.

Each call returns a `DynamicsTrajectory`: the full per-step trajectory of the
signal, the graph Dirichlet energy at every frame, and a `MappingResult` for the
terminal finite metric space, carrying one-to-one lineage back to the source
records.

## Level 3 — relationship to named algorithms

The transition operator is a heat-kernel affinity row-normalised to a Markov
matrix. Redif is the implicit Euler inverse-diffusion dynamics over the weighted
graph Laplacian. Names such as parametric diffusion coordinates, diffusion maps, or Gaussian kernels are
Level-3 special cases; the Level-1 statement stands on its own: a finite metric
space already contains a diffusion, a random walk, transport paths, entropy
change, and a reverse process, and the probabilities in all of them are read off
the geometry.

## Worked example

[finite-metric-dynamics.md](../examples/finite-metric-dynamics.md) walks the hero
example end to end, and
[finite_metric_dynamics.cpp](../../examples/engine/finite_metric_dynamics.cpp)
proves every claim above with assertions that also run as a CI test.
