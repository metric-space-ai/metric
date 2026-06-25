# Redif Metric Dynamics

Status: production contract, with implementation gaps tracked in
`docs/engine/production-readiness-roadmap.md`.

Redif is a metric-space dynamics operator. Its input is a finite metric space,
not a vector table and not an external probability model:

```text
X = {x_1, ..., x_n}
d: X x X -> R_{\ge 0}
```

Every state used by Redif is a measure over the atoms of `X`. For record `x_i`,
the initial state is the Dirac measure `delta_i`. This is what makes Redif
usable for arbitrary records: strings, histograms, process curves, mixed records,
or vector records all enter only through the admitted metric `d`.

## Local Redif Geometry

At Redif step `t`, the implementation has a distance matrix `D^(t)`.

- With fixed geometry, `D^(t) = d`.
- With adaptive geometry, `D^(t)` is the Wasserstein transport distance between
  the current atom measures, using the original metric `d` as ground distance.

For each atom `i`, let `N_k(i)` be the deterministic set of its `k` nearest
neighbours in `D^(t)`, excluding itself. Ties are resolved by record order. The
local distance relation is

```text
ell_ij = D^(t)_ij  if j in N_k(i) or i in N_k(j)
ell_ij = 0         otherwise
```

Redif then derives a local scale and a weighted affinity from this sparse
relation. For each atom, let

```text
sigma_i = mean({ell_ij : ell_ij > 0})     fallback: 1 when the row has no edge
```

The Redif affinity is the self-tuned heat kernel on the local metric relation:

```text
K_ij  = exp(- ell_ij^2 / (sigma_i sigma_j))   when ell_ij > 0
K_ij  = 0                                      otherwise
q_i   = sum_j K_ij
Q     = diag(q_i)
L     = Q - K
```

`L` is the Redif graph Laplacian for the current finite metric geometry. It is
derived only from distances and deterministic neighbourhood order. Large
distances therefore never become large affinities; they contribute weakly to the
transition relation after the local scale is accounted for.

Degenerate isolated atoms receive an identity transition row. Algebraically this
is represented as a unit self-affinity for that atom, so the row remains a valid
probability transition and the Laplacian contribution is zero.

The same construction also defines the row-stochastic Redif transition

```text
P_ij = K_ij / q_i
```

with an identity row for a degenerate isolated atom. The C++ API exposes this
inspectable source operator as `redif_operator(space, options)`. The returned
`RedifOperator` contains `local_distances`, `local_scale`, `affinity`,
`degree`, `laplacian`, `transition`, `stationary`, and compact diagnostics for
row sums and symmetry.

## Inverse Redif Step

`redif_remove_noise(space, options)` applies the implicit Euler Redif step to the
matrix `U^(t)` whose row `i` is the current measure path for atom `x_i`:

```text
U^(0)     = I
U^(t+1)  = normalize_rows((Q + tau L)^(-1) Q U^(t))
```

Here `tau = options.euler_step`, with `0 < tau <= 1`. The linear solve is the
finite-dimensional implicit Euler equation

```text
Q (U^(t+1) - U^(t)) = - tau L U^(t+1)
```

This is the coordinate-free replacement for vector-record reconstruction. The
result is not a new record guessed in a coordinate chart; it is a path of atom
measures over the original finite metric space.

## Forward Noise Step

`redif_add_noise(space, options)` evolves Dirac atom measures by the same Redif
operator family:

```text
P_ij = A_ij / q_i                 row-stochastic Redif transition
F_tau = (1 - tau) I + tau P       explicit forward Euler
U^(t+1) = U^(t) F_tau
```

`F_tau` is a forward Euler step for the generator `P - I`. It adds disorder by
spreading atom mass through the metric relation. Forward noise and inverse Redif
share the same local distance relation, affinity, degree measure, transition,
step size, neighbourhood budget, and adaptive-geometry policy.

This does not mean every inverse request is automatically well posed. The
probability-preserving forward Euler step is bounded by `0 < tau <= 1`; an
algebraic reverse of a diffused measure must still pass the finite operator's
singularity and probability-measure checks.

## Algebraic Forward-Step Inverse

For a fixed `RedifOperator`, METRIC also exposes the bounded one-step algebra:

```text
redif_forward_noise_step(op, U, tau) = U F_tau
redif_inverse_noise_step(op, V, tau) = U such that U F_tau = V
```

The inverse step solves

```text
F_tau^T u_i^T = v_i^T
```

for each atom-measure row. It is accepted only when the finite linear system is
nonsingular and the recovered rows are valid probability measures. For example,
in the canonical two-point space the inverse is singular at `tau = 1/2`, because
the forward matrix collapses both atom measures to the same row.

## Result Diagnostics

`RedifMeasureResult` carries enough metadata to audit the dynamics without
reconstructing private implementation state:

- `initial_stationary`: the stationary measure of the first Redif operator;
- `operator_diagnostics`: one entry per step, including degree range, transition
  row-sum range, and affinity symmetry;
- `entropy_diagnostics`: Shannon entropy and relative entropy before/after for
  every atom path;
- `step_transport` and `transport_path_length`: Wasserstein movement of each
  atom measure under the original metric.

## Entropy Contract

For a measure `mu` over the atoms,

```text
H(mu) = - sum_i mu_i log(mu_i)
D(mu || pi) = sum_i mu_i log(mu_i / pi_i)
```

where `pi` is the invariant measure of the metric-derived transition. For the
forward Markov step, relative entropy to `pi` contracts under the standard
finite Markov-kernel conditions. Shannon entropy is guaranteed to increase only
under additional conditions such as a uniform invariant measure or a checked
canonical fixture. METRIC tests must state which entropy claim they are proving;
they must not treat entropy increase as unconditional for every non-uniform
finite metric space.

## Path-Length Outlier Functional

For an atom path `mu_i^(0), ..., mu_i^(T)`, Redif reports

```text
path_length(i) = sum_t W_d(mu_i^(t), mu_i^(t+1))
```

where `W_d` is the discrete Wasserstein transport distance using the original
metric `d` as ground distance. A long path means the atom's measure must travel
far through the intrinsic metric before it participates in the evolving mass
structure. This is a metric-dynamics singularity score, not a classifier score.
It is invariant under record relabelling that preserves the finite metric
distance matrix.

The expected canonical behavior is:

- In a two-point space, one Redif inverse step with `tau = 1/4` and `k = 1`
  yields terminal measures `(5/6, 1/6)` and `(1/6, 5/6)`.
- In a compact chain plus one distant point, the distant point has the longest
  Redif transport path.
- In a clique-like equidistant space with full neighbourhoods, all atom path
  lengths are equal.
- In two compact chains connected by a weak bridge, the bridge atom has the
  longest Redif transport path under the checked canonical fixture.
- In non-vector spaces, the same path functional is valid because it only uses
  measures over atoms and the admitted metric.

## Relation To Other Singularity Diagnostics

Redif path length is not the same question as local volume, nearest-neighbour
isolation, or density-unassigned records.

- Local volume asks how much metric mass is near an atom at a chosen radius or
  neighbour count.
- Nearest-neighbour isolation asks how far an atom is from its closest metric
  witnesses.
- Density-unassigned records are produced by a concrete density grouping rule.
- Redif path length asks how far an atom measure travels under metric-induced
  dynamics before it participates in the evolving mass structure.

These diagnostics can agree on an isolated point, but they need not agree on a
bridge, a thin filament, or a space with nonuniform local volume. METRIC keeps
them separate so the user can inspect which metric-space principle produced the
singularity evidence.

## Failure Modes

Redif must reject or surface these cases instead of silently producing an
uninterpretable path:

- empty spaces;
- zero-neighbour requests on spaces with more than one atom;
- non-finite or out-of-range Euler steps;
- dense construction above the configured record budget;
- singular implicit Euler systems;
- negative or non-finite measure mass after a solve;
- unstable inverse-dynamics requests once the algebraic inverse operator is
  exposed.

## Boundary To Other Procedures

`density_filter` is not Redif and is not noise removal. It filters records that
a concrete density grouping rule leaves unassigned.

Gaussian coordinate perturbation is a special case for vector records with a
particular coordinate metric. It is not the general definition of noise in a
finite metric space. In METRIC, noise is dynamics over the intrinsic metric, and
noise removal is inverse metric dynamics.
