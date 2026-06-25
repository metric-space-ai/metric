# Cross-Space Dependency (Hero)

**Question answered:** *Are two different kinds of record — operations event logs and
process sensor curves — statistically dependent across paired observations, without
ever forcing them into a shared embedding?*

This is METRIC's Level-1 dependence workflow on genuinely heterogeneous spaces. Each
observation `i` has two views:

| Side  | Record type            | Native metric              | Engine type           |
|-------|------------------------|----------------------------|-----------------------|
| Left  | event log (token text) | edit / Levenshtein         | `mtrc::Edit<char>`    |
| Right | process curve (series) | Time-Warp Edit Distance    | `mtrc::TWED<double>`  |

A log string and a sensor curve have no natural common vector space. Each lives in its
own finite metric space with its own distance. The dependence question is answered by
**Multiscale Graph Correlation (MGC)** through the engine intent
`mtrc::compare(left_space, right_space)`.

MGC is a **dependence test, not a metric and not a distance.** It is never used to
measure distance here.

Source:

- application — [cross_space_dependency.cpp](../../examples/engine/cross_space_dependency.cpp)
- shared logic / statistical contract — [cross_space_dependency.hpp](../../examples/engine/cross_space_dependency.hpp)
- core-gate test — [engine_cross_space_dependency_smoke.cpp](../../tests/core_smoke/engine_cross_space_dependency_smoke.cpp)

Command:

```bash
build/core/examples/engine/engine_cross_space_dependency
```

## Statistic and p-value semantics

The engine returns a single MGC **sample statistic** in `[-1, 1]` (`~1` strong, possibly
nonlinear/monotone dependence; `~0` none) and, by design, **no p-value**
(see [Correlation Between Metric Spaces](correlation-between-spaces.md)). This hero adds
the missing significance layer the correct way — a **seeded permutation test built on top
of the public `compare()` API**, not inside MGC and never as a distance:

- **statistic** — MGC on the true observation pairing (the effect size).
- **null pairing** — the same left space compared against the right space whose record
  labels have been randomly **re-paired**. Re-pairing is the textbook independence null.
- **p-value** — `(1 + #{permuted statistic ≥ observed}) / (1 + permutations)`. The
  standard add-one estimator: one-sided, monotone in the evidence, never exactly zero,
  valid for any permutation count. With 199 permutations the smallest reportable p-value
  is `1/200 = 0.005`.
- **standardized effect** — `(statistic − null_mean) / null_sd`, a z-like read on how far
  the observed dependence sits above the null cloud.

Everything is deterministic: MGC's `compare()` path is the exact computation (not the
sampling `estimate`), and every draw is seeded, so the report below is reproducible
bit-for-bit. The fast permutation loop reuses distance matrices materialised once via the
public `DistanceTable` helpers and is asserted to return the exact `compare()` value.

## Three scenarios

1. **Coupled** — logs and curves are generated from one shared hidden regime. The
   dependence is purely *structural*: the regime controls **which** event tokens appear
   and **where**, and the **shape** (not the mean) of the curve. METRIC must detect strong,
   significant dependence.
2. **Decoupled** — curves are generated from an independent regime. The two spaces are
   statistically independent; METRIC must report no significance.
3. **Permuted** — the coupled data with the curve pairing shuffled (one explicit draw from
   the independence null). Each space is unchanged in isolation, but the pairing is gone, so
   dependence must collapse to the null range.

## Baseline: naive vectorisation

The "what you would do without METRIC" path forces both modalities into numeric vectors:

- **Naive scalar Pearson** — reduce each log to its length and each curve to its mean, then
  correlate. The generator deliberately makes length and mean carry **no** regime
  information, so this generic reduction is blind to the structural dependence.
- **Forced-common-vector MGC** — run the same MGC test, but on naively vectorised records
  (zero-padded character codes vs zero-padded samples under plain Euclidean) instead of the
  native metric spaces. This isolates the contribution of the metric spaces themselves.

The naive scalar-Pearson p-value uses its own, larger permutation budget (1999 draws, so its
smallest reportable p-value is `1/2000 = 0.0005`) because shuffling two scalar columns is far
cheaper than recomputing MGC; this is independent of the 199 MGC null draws.

## Expected output

```text
cross-space records = 48
cross-space left space = event_logs/edit_distance
cross-space right space = process_curves/twed
coupled compare() statistic = 0.947979
coupled MGC statistic = 0.947979
coupled MGC null mean = 0.002297
coupled MGC null sd = 0.030674
coupled MGC standardized effect = 30.830194
coupled MGC permutation p-value = 0.005000
coupled MGC permutations = 199
coupled MGC decision (alpha=0.050000) = dependent
coupled baseline scalar pearson r = 0.079196
coupled baseline scalar pearson p-value = 0.618500
coupled baseline scalar decision (alpha=0.050000) = independent
coupled baseline forced-vector MGC = 0.510452
decoupled MGC statistic = -0.007853
decoupled MGC null mean = 0.002456
decoupled MGC null sd = 0.033055
decoupled MGC standardized effect = -0.311873
decoupled MGC permutation p-value = 0.535000
decoupled MGC permutations = 199
decoupled MGC decision (alpha=0.050000) = independent
decoupled baseline scalar pearson r = 0.056721
decoupled baseline scalar pearson p-value = 0.732000
decoupled baseline scalar decision (alpha=0.050000) = independent
decoupled baseline forced-vector MGC = -0.003169
permuted MGC statistic = -0.018790
permuted MGC null mean = -0.003803
permuted MGC null sd = 0.023432
permuted MGC standardized effect = -0.639573
permuted MGC permutation p-value = 0.720000
permuted MGC permutations = 199
permuted MGC decision (alpha=0.050000) = independent
cross-space verdict = metric_detects_dependence_baseline_misses_it
```

The numbers above are reproducible bit-for-bit on any conforming toolchain: every random
draw is derived directly from a seeded `std::mt19937_64` (a standardised engine) rather than
from the non-portable `std::*_distribution` / `std::shuffle` adaptors.

## Interpretation

| Scenario  | MGC statistic | p-value | decision    | naive scalar p | naive decision |
|-----------|---------------|---------|-------------|----------------|----------------|
| Coupled   | **0.948**     | 0.005   | dependent   | 0.619          | independent    |
| Decoupled | −0.008        | 0.535   | independent | 0.732          | independent    |
| Permuted  | −0.019        | 0.720   | independent | —              | —              |

- **METRIC sees the dependence the baseline cannot.** In the coupled case MGC on the two
  native metric spaces is `0.948` with `p = 0.005` (standardized effect ≈ 31σ above the
  null), while the naive scalar Pearson is `r = 0.079`, `p = 0.619` — it declares the two
  modalities independent. The dependence is real and strong; it just does not live in the
  cheap scalar summaries.
- **The metric spaces, not the test, unlock the signal.** Forced-common-vector MGC drops to
  `0.510`: the same dependence test on naively vectorised records loses a large part of the
  signal that the native edit/TWED spaces preserve. Without a learned artifact for a common
  embedding, the native metrics already expose the structure.
- **The pairing carries the dependence.** Permuting the curve order collapses the statistic
  from `0.948` to `−0.019` (`p = 0.720`): each space is unchanged, only the cross-space
  alignment is destroyed. This is exactly one draw from the permutation null and confirms
  the test machinery.
- **No false positives.** With independent regimes (decoupled), MGC is `−0.008`,
  `p = 0.535` — correctly not significant.

The shared observation order is the alignment contract; the two record containers, types,
and metrics stay completely separate. No common embedding artifact is trained or required.

## Target Python shape

```python
from metric import Space, metrics

logs   = Space(event_logs,    metric=metrics.Edit())
curves = Space(process_curves, metric=metrics.TWED())

result = logs.compare(curves, method="mgc", permutations=199, seed=20240607)
print(result.statistic)  # 0.947979  (effect size)
print(result.p_value)    # 0.005     (permutation significance)
```

`result.p_value` is the significance layer this C++ application implements on top of the
public `compare()` intent. The native MGC core still computes only the sample statistic;
the permutation test is a thin, dependency-free wrapper around it.
