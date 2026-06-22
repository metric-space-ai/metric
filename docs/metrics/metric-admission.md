# Metric Admission

Metric admission is the process for deciding whether a distance computation can
be exposed as a METRIC-provided true metric, handled as an explicit
pseudometric with quotient semantics, kept as an admission candidate, or
quarantined for removal. The computation can be a closed formula, dynamic
program, shortest-path algorithm, transport/assignment optimization, edit-cost
solver, or another deterministic minimum-transformation-cost procedure.

The normal path is to select a catalog metric and configure its parameters.
Custom metrics are an advanced path and remain unpromoted until their law and
operating domain are documented.

## Admission Outcomes

| Outcome | Required claim | Trait policy | User-facing use |
| --- | --- | --- | --- |
| True metric | Non-negativity, identity, symmetry, and triangle inequality hold on the documented domain. | `metric_law::metric` | Allowed for metric-only algorithms. |
| Pseudometric | Non-negativity, symmetry, and triangle inequality hold, but identity can collapse distinct records. | `metric_law::pseudo_metric` | Allowed for exact pairwise workflows; not for metric-only algorithms unless quotient semantics are explicit. |
| Admission candidate | The computation may be a true metric under exact restrictions, but current implementation or documentation is incomplete. | No true-metric promotion yet. | Documented as candidate only; not a normal guaranteed metric. |
| Quarantined/rejected | The computation is known to mislead users when called a metric, lacks proof, or is unsafe under current guards. | No true-metric promotion. | Isolated, deprecated, then removed unless a true-metric variant is admitted. |

## Evidence Required

A metric candidate needs a short admission record covering:

- Record domain and equality semantics.
- Computation definition: closed formula or algorithmic optimization problem.
- Parameter constraints that preserve the claimed law.
- Return range, zero-distance behavior, and handling of invalid values.
- Determinism and uniqueness of the returned distance value for fixed inputs
  and parameters. The optimal transformation path itself may be non-unique.
- Symmetry and triangle-law basis, either by standard theorem or focused proof.
- `metric_traits` law, record kind, thread-safety, and cache-key behavior.
- Fixture tests for ordinary cases, parameter boundaries, invalid parameters, and known counterexamples.
- Documentation of whether the candidate is stable, experimental, or quarantined.

For the current native codebase, admission must start from the C++ trait and
implementation inventory. A distance class is not a true metric just because a
mathematical family has a true-metric variant. The implementation must expose
the admitted computation, reject invalid domains, and declare the correct
`metric_traits` law.

## Quarantine-To-Removal Gate

Built-in public functions that are not admitted true metrics must go through a
short quarantine path, then disappear from the public metric library unless
admission succeeds.

Quarantine requires:

- exact C++ class, exported name, and binding name
- exact computation currently implemented
- domain and parameter checks currently enforced
- reason it is not admitted as a true metric today
- known true-metric variants in the same literature family
- migration target or replacement metric
- removal scope: docs, examples, umbrella headers, bindings, tests, and
  compatibility aliases

Removal is blocked if the audit finds that the implementation is already a true
metric under a missing guard or a bad name. In that case the required action is
rename, guard, test, admit, and document, not deletion.

## Parameter Gates

Parameter validation is part of the law. The implementation rejects or demotes
configurations that do not satisfy the admitted conditions.

| Candidate family | True-metric gate | Demotion or rejection |
| --- | --- | --- |
| Minkowski | `p >= 1` | `p < 1` is quarantined or rejected for metric-only use. |
| Weighted Minkowski | Positive weights and `p >= 1` | Zero weights are pseudometric; negative weights are rejected. |
| Mahalanobis | Symmetric positive definite matrix | Positive semidefinite is pseudometric; indefinite is rejected. |
| Standardized vector metrics | Every fitted scale or sigma is positive | Zero scale is pseudometric or invalid; division by zero is rejected. |
| Hamming | Equal-length aligned records | Variable alignment requires another metric such as edit distance. |
| Edit distance | Symmetric non-negative edit costs with admitted substitution/deletion/insertion constraints | Asymmetric or negative costs are quarantined or rejected. |
| TWED (time warp edit) | Finite non-empty real sequences with `penalty >= 0` and finite `elastic > 0` | `elastic <= 0` collapses timestamp identity (pseudometric or worse) and is rejected; negative or non-finite `penalty`/`elastic`, empty inputs, and non-finite values are rejected. |
| Wasserstein / EMD | Equal mass, non-negative finite measures, and a true metric ground cost (admitted as `mtrc::Wasserstein<V>`) | Unbalanced mass, negative/non-finite mass, or nonmetric ground costs are rejected by the strict route; the permissive `mtrc::EMD<V>` route accepts them but is `metric_law::distance`, not a metric. |
| Ruzicka / weighted Jaccard | Admitted for finite nonnegative aligned vectors and fixed zero-union convention | Negative or non-finite coordinates are rejected; two all-zero records have distance 0. |
| Angular / chordal | Nonzero or normalized vectors with fixed identity convention | Raw `1 - cosine` and zero-vector ambiguity are rejected. |
| ERP | Fixed finite gap sentinel outside the sample alphabet, with a metric ground cost | Samples equal to the gap are rejected because they can collapse insertion/deletion identity. |
| Hausdorff | Non-empty finite sets over an admitted ground metric | Empty-set policy and non-metric ground costs are rejected. |
| Distribution metrics | Non-negative normalized inputs and documented support policy | Raw asymmetric divergences are rejected as metrics. |
| Cramer/root CvM | Root CDF `L2` distance with admitted CDF representation | Squared statistics are not promoted as metrics. |
| SSIM-inspired variants | Exact literature-backed metric formula, constants, and image domain | Raw SSIM and `1 - SSIM` remain quarantined. |

## Current Implementation Admission State

Native promotion is tied to these implementation facts:

- `P_norm`: admitted for finite `p >= 1`; keep constructor validation,
  read-only exponent behavior, cache-key coverage, and metric-law tests.
- Standardized Euclidean and Manhattan: done. `mtrc::Euclidean_standardized<V>`
  and `mtrc::Manhattan_standardized<V>` are admitted as `metric_law::metric`
  under a positive-scale gate: the fitting constructor, the explicit
  `(mean, sigma)` constructor, and every evaluation reject a zero or non-finite
  fitted scale (so constant features and single-record fits are rejected). The
  centering mean cancels out of the distance, the cache key serializes `sigma`,
  and metric-contract/property tests live in
  `tests/core_smoke/metric_standardized_smoke.cpp`.
- Current `Cosine`: rename or document as angular distance, then add zero-vector
  and normalization policy.
- `CosineInverted`, `Sorensen`, raw `SSIM`, and fitted `Kohonen`: keep out of
  metric-only paths unless a separate metric variant is admitted.
- `EMD`/`Wasserstein`: done. The strict route `mtrc::Wasserstein<V>` enforces a
  metric ground cost (square, zero diagonal, positive off-diagonal, symmetric,
  triangle) at construction and equal nonnegative finite mass at every
  evaluation, is immutable (`thread_safe == true`), exposes a ground-cost cache
  key, and is `metric_law::metric`. The permissive `mtrc::EMD<V>` keeps the
  arbitrary-cost/extra-mass-penalty/lazy-state behavior and is explicitly
  `metric_law::distance` with `thread_safe == false`.
- `TWED`: admitted as `metric_law::metric` for finite non-empty real sequences
  with `penalty >= 0` and `elastic > 0`; the constructor and every evaluation
  enforce the parameter gate and reject empty or non-finite inputs.
- `KolmogorovSmirnov`, `CramervonMises`, and `RandomEMD`: audited and
  **kept quarantined** (`metric_law::distance`). All three build on the same
  `mtrc::PMQ` Akima-interpolated empirical CDF, which divides by zero on tied
  sample values (`RandomEMD`/`CvM` return `NaN`; `KolmogorovSmirnov` false-zeros
  distinct distributions) and whose `RandomEMD`/`CvM` integration loop hangs on
  zero-range samples (and SIGSEGV on empty inputs). The root cause is that PMQ's
  Akima-interpolated "CDF" is neither monotone nor bounded to `[0,1]`; as shipped
  all three violate the triangle inequality through that overshoot (`RandomEMD`
  and `CvM` once near-duplicate knots appear; `KolmogorovSmirnov` also returns
  values `> 1`). The root-`L2`-CDF *form* of `CramervonMises` remains the eventual
  metric candidate once the CDF is made tie-safe over a fixed support. Defects are
  pinned in `tests/core_smoke/metric_distribution_quarantine_smoke.cpp`. Route
  distribution transport to `mtrc::Wasserstein<V>`.
- Catalog additions: admitted with guard and
  property-smoke coverage in `tests/core_smoke/metric_catalog_additions_smoke.cpp`.
  The promoted classes are `WeightedMinkowski`, `Mahalanobis`, `Angular`,
  `Chordal`, `DiscreteMetric`, `Hamming`, `Jaccard`, `BinaryJaccard`,
  `Tanimoto` for the Ruzicka computation, `ERP`, `Hausdorff` over an admitted
  ground metric, `TotalVariation`, `Hellinger`, `SqrtJensenShannon`,
  `EmpiricalKolmogorovSmirnov`, and `EmpiricalCramer`. Frechet remains deferred
  until the catalog fixes continuous vs discrete curve identity, stutter policy,
  and ground-metric admission.

## Algorithm Gating

Metric-only algorithms must use `metric_traits`, not naming conventions.

- `metric_law::metric` may be used by representations or strategies that depend on triangle inequality, such as cover-tree indexing and triangle-pruning search.
- `metric_law::pseudo_metric` is weaker than a true metric unless the algorithm explicitly operates on equivalence classes.
- `metric_law::distance` uses exact pairwise, explicit pairwise-table, brute-force, or graph workflows that do not assume triangle law.
- `metric_law::unknown` is not selected for metric-only algorithms.

## Custom Metric Path

Custom metrics follow the same admission path as built-in candidates. A
project-specific callable can remain useful without being a true metric, but it
does not specialize `metric_traits<Metric>::law` to `metric_law::metric` unless
the project can state the domain and proof. Parameterized custom metrics also
provide stable cache keys so representations can be reproduced from the metric
configuration.
