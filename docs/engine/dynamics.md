# Finite Metric Dynamics

`mtrc::modify::dynamics` builds and evolves a process **over the structure of a
finite metric space**. It is the component home for diffusion, random walks,
forward degradation (noising) and reverse diffusion (reconstruction).

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
| `metric_diffuse(space, schedule[, transition])` | Forward dynamics: degrade a node signal with graph heat flow plus seeded, geometry-shaped noise. |
| `metric_reconstruct(degraded, schedule[, transition])` | Reverse dynamics: contract the degraded signal back onto the structure with noise-free graph heat flow. |

`metric_transition` and `metric_random_walk` work for **any** `MetricSpaceLike`
record/metric pair (strings, histograms, mixed records): geometry → probability
needs only distances. `metric_diffuse` / `metric_reconstruct` evolve a vector signal carried by
the nodes, so they require vector-valued records (a `static_assert` enforces it).

The `dynamics_schedule` carries only geometry- and reproducibility-facing knobs:
`neighbors` (graph `k`), `steps`, `diffusivity` (the per-step mixing weight
`alpha`), `noise_scale`, `bandwidth` (`0` ⇒ derive `eps` from the metric), and a
`seed`. None of them is a probability distribution.

## Level 1 — the dynamics

The metric structure (the graph) is the fixed substrate. A signal carried by the
nodes evolves over it with the metric Laplacian smoothing operator

```text
S = (1 - alpha) I + alpha P            (alpha in (0, 1])
```

- **Forward / degradation (`metric_diffuse`).** Each step applies `S` (drift along the
  graph) and adds seeded isotropic noise scaled by `noise_scale`. Structure
  decays: the signal leaves the low-dimensional manifold and the graph Dirichlet
  energy rises. This is the forward (noising) process of a diffusion model,
  framed as a walk over geometry.

- **Reverse / reconstruction (`metric_reconstruct`).** The same `S` run *without* noise
  is a low-pass filter on the graph: it contracts the injected high-frequency
  fluctuations back onto the structure. It is the dependency-free sibling of the
  native `mtrc::Redif` reverse-diffusion engine (graph-Laplacian backward
  diffusion). Because `S` is row-stochastic it obeys a **maximum principle** — a
  reverse step is a convex combination of node values, so it can never create a
  new per-coordinate extremum, and a constant signal is an exact fixed point.

Both directions are fully reproducible: a `(seed, schedule)` pair reproduces a run
bit-for-bit on any platform, because the pseudo-random draws (splitmix64 +
Box–Muller) are implemented inline rather than relying on `std::random`'s
implementation-defined distributions.

Each call returns a `DynamicsTrajectory`: the full per-step trajectory of the
signal, the graph Dirichlet energy at every frame, and a `MappingResult` for the
terminal finite metric space, carrying one-to-one lineage back to the source
records.

## Level 3 — relationship to named algorithms

The transition operator is the diffusion-maps / PHATE affinity (a heat kernel,
row-normalised to a Markov matrix). The reverse flow is the explicit-step form of
the graph-Laplacian backward diffusion implemented by `mtrc::Redif` (see
`metric/mapping/Redif.hpp`). Naming these is a Level-3 reference; the Level-1
statement stands on its own: a finite metric space already contains a diffusion,
a random walk and a reverse process, and the probabilities in all three are read
off the geometry.

## Worked example

[finite-metric-dynamics.md](../examples/finite-metric-dynamics.md) walks the hero
example end to end, and
[finite_metric_dynamics.cpp](../../examples/engine/finite_metric_dynamics.cpp)
proves every claim above with assertions that also run as a CI test.
