# Finite Metric Dynamics

Randomness, noise, forward diffusion and reverse diffusion are usually presented
as probabilistic machinery bolted onto data. This example shows the opposite:
they are all an **evolution over the structure of a finite metric space**, and
the probabilities are *derived from the geometry*, never assumed.

> **Probability is a derived interpretation of the geometry, not a primitive.**

The promoted C++ example
[finite_metric_dynamics.cpp](../../examples/engine/finite_metric_dynamics.cpp)
proves the thesis with assertions that also run as the
`example_engine_finite_metric_dynamics` CI test. There is no Python algorithm and
no new dependency: the operator `mtrc::modify::dynamics` is header-only and uses
only the engine core and the C++ standard library. See
[engine/dynamics.md](../engine/dynamics.md) for the component reference.

## Fixture

A small, deterministic fixture
([finite_metric_dynamics_fixture.hpp](../../examples/engine/finite_metric_dynamics_fixture.hpp)):
28 points sampled along an S-curve `y = sin(x)` embedded in `R^2`. The curve is a
1-D manifold living in a 2-D space, so a k-NN graph built from the Euclidean
metric recovers it as a near-path graph — the "metric structure" the dynamics run
over. The generator is closed-form and uses no randomness.

## C++ shape

```cpp
#include <metric/engine.hpp>
#include <metric/metric/catalog/vector/Standards.hpp>

auto space = mtrc::make_space(records, mtrc::Euclidean<double>{});

mtrc::DynamicsSchedule schedule;
schedule.neighbors = 4;     // k for the metric neighbourhood graph
schedule.bandwidth = 0.0;   // derive the heat-kernel scale from the metric

// geometry -> probability
auto transition = mtrc::metric_transition(space, schedule);
auto walk       = mtrc::metric_random_walk(transition, /*start=*/0, /*steps=*/4000,
                                           /*seed=*/20240617, /*walkers=*/16);

// forward diffusion / degradation
mtrc::DynamicsSchedule forward = schedule;
forward.steps = 6; forward.diffusivity = 0.05; forward.noise_scale = 0.06; forward.seed = 1234;
auto degraded = mtrc::metric_diffuse(space, forward, transition);

// reverse diffusion / reconstruction over the fixed metric structure
mtrc::DynamicsSchedule reverse = schedule;
reverse.steps = 40; reverse.diffusivity = 0.18; reverse.noise_scale = 0.0;
auto reconstructed = mtrc::metric_reconstruct(degraded.result.space, reverse, transition);
```

## Act 1 — geometry becomes probability

`metric_transition` turns the pairwise distances into a row-stochastic Markov
transition matrix `P` and reads its stationary distribution `pi` off the node
degrees. The example asserts the contracts that make this "probability from
geometry":

- every row of `P` sums to `1` and is non-negative;
- a constant signal is a fixed point of `P` (the harmonic invariant);
- `pi P = pi`;
- translating every record leaves `P` identical to `1e-12` — `P` depends on the
  geometry, not the coordinates;
- a seeded random walk that samples neighbours `~ P_i` converges to `pi` (total
  variation `< 0.06`), and is reproducible for a fixed seed.

```text
heat-kernel bandwidth eps = 0.2680
H(stationary)            = 3.1997 nats (max 3.3322)
walk TV to stationary    = 0.0166
```

## Act 2 — forward diffusion / degradation

`metric_diffuse` evolves the node signal with graph heat flow plus seeded,
geometry-shaped noise. The signal leaves the manifold and the graph Dirichlet
energy rises. The run is reproducible bit-for-bit for a fixed `(seed, schedule)`,
and a different seed degrades the space differently. The terminal
`MappingResult` keeps the node count and dimension and carries one-to-one lineage
back to the source records.

```text
Dirichlet energy 1.1989 -> 2.7751
MSE(degraded, clean)     = 0.0176
```

## Act 3 — reverse diffusion / reconstruction

`metric_reconstruct` runs the same graph heat flow without noise. As a low-pass filter
on the metric graph it contracts the injected fluctuations back onto the
structure. The example asserts:

- the **maximum principle** — no reverse frame creates a new per-coordinate
  extremum (a guaranteed consequence of `P` being row-stochastic);
- the graph Dirichlet energy is non-increasing along the reverse trajectory;
- there is a reverse time at which the reconstruction is markedly closer to the
  clean manifold than the degraded input (≥ 15% MSE reduction);
- even when the graph is rebuilt from the **degraded observations alone** (the
  structure is not handed to us, mirroring the adaptive native `mtrc::Redif`),
  reconstruction still improves on the degraded input.

```text
best reverse step        = 5 / 40
MSE(reconstructed,clean) = 0.0078 (was 0.0176)   ~56% better
Dirichlet energy 2.7751 -> 0.4262
```

## What is promoted

The promoted behavior is the `mtrc::modify::dynamics` surface — `metric_transition`,
`metric_random_walk`, `metric_diffuse`, `metric_reconstruct` — together with the invariants
above, all covered by the core C++ CI gate. The schedule constants in the example
are tuned for the fixture; they are a demonstration, not a contract. Reverse
diffusion here is graph low-pass reconstruction; the implicit-solve variant lives
in the native `mtrc::Redif` engine.
