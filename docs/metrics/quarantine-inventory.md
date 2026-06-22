# Metric Quarantine Inventory

This inventory tracks native C++ distance computations that are public or
discoverable but are not currently promoted true metrics.

METRIC does not expose a second-class "distance-only" metric library. Each
entry is classified by the exact computation: admitted true metric, precise
metric variant required, compatibility distance outside metric-only routing, or
removal candidate.

No class is removed by family name. Removal requires an exact
computation/domain review and a check for true metric variants in the same
literature family.

## Current Promoted Metrics

These classes currently declare `metric_traits<...>::law == metric_law::metric`
and are not quarantine targets:

| C++ class | Domain | Notes |
| --- | --- | --- |
| `mtrc::Euclidean<V>` | Aligned numeric vectors/scalars | Promoted true metric with existing trait. |
| `mtrc::Manhattan<V>` | Aligned numeric vectors | Promoted true metric with existing trait. |
| `mtrc::Chebyshev<V>` | Aligned numeric vectors | Promoted true metric with existing trait. |
| `mtrc::P_norm<V>` | Aligned numeric vectors | Promoted Minkowski metric for finite `p >= 1`; constructor rejects non-finite or sub-metric exponents, the exponent is read-only after construction, and the trait is `metric_law::metric`. |
| `mtrc::WeightedMinkowski<V>` | Aligned numeric vectors | Promoted weighted Minkowski metric for finite `p >= 1` and finite positive weights; zero weights are rejected rather than silently becoming a pseudometric. |
| `mtrc::Mahalanobis<V>` | Aligned numeric vectors | Promoted Mahalanobis metric with a finite symmetric positive definite precision matrix validated by Cholesky. |
| `mtrc::Angular<V>` | Unit-normalized aligned numeric vectors | Promoted angular metric on the unit sphere; zero/non-unit vectors are rejected. This does not promote the older `mtrc::Cosine<V>` surface. |
| `mtrc::Chordal<V>` | Unit-normalized aligned numeric vectors | Promoted chordal/Euclidean unit-sphere metric with the same unit-vector gate as `Angular`. |
| `mtrc::DiscreteMetric<V>` | Any equality-comparable record type | Promoted discrete metric with a finite positive mismatch cost. Named `DiscreteMetric` to avoid the legacy PMQ `mtrc::Discrete` distribution helper. |
| `mtrc::Hamming` | Equal-length aligned categorical records | Promoted Hamming metric; unequal lengths are rejected. |
| `mtrc::Jaccard` | Finite set records | Promoted set Jaccard metric; order and duplicates are ignored, and empty/empty is 0. |
| `mtrc::BinaryJaccard` | Aligned binary presence vectors | Promoted binary Jaccard metric; entries must be exactly 0 or 1. |
| `mtrc::Ruzicka<V>` | Nonnegative aligned numeric vectors | Promoted weighted-Jaccard/Ruzicka metric for finite nonnegative entries; both all-zero records have distance 0 and invalid negative or non-finite entries are rejected. |
| `mtrc::Tanimoto<V>` | Nonnegative aligned numeric vectors | Promoted only for the admitted weighted-Jaccard/Ruzicka computation; generic Tanimoto complements remain quarantine candidates. |
| `mtrc::Edit<V>` | Strings/sequences | Promoted Levenshtein-style edit metric with existing trait. |
| `mtrc::TWED<V>` | Finite non-empty densely indexed real sequences (read positionally) | Promoted Time Warp Edit Distance metric for `penalty >= 0` and `elastic > 0`; parameters and inputs are gated on construction and on every evaluation, the temporal anchor is symmetric, the trait is `metric_law::metric`, and the cache key encodes `penalty`, `elastic`, and the zero-padding flag. The `CompressedVector` input route reuses the same recurrence on `(value, stored-index)` sequences but is a compatibility path, not the admitted metric record domain: it ignores dropped/trailing zeros, so distinct sparse records can collapse the identity of indiscernibles. |
| `mtrc::ERP<V>` | Finite scalar sequences whose samples exclude the fixed gap value | Promoted ERP metric with absolute-value ground cost and a finite gap sentinel. Samples equal to the gap are rejected to preserve identity of indiscernibles. |
| `mtrc::Hausdorff<GroundMetric>` | Non-empty finite set records | Promoted when `GroundMetric` is itself `metric_law::metric`; default ground is Euclidean. Empty sets are rejected. |
| `mtrc::Wasserstein<V>` | Finite nonnegative measures over a fixed metric ground cost | Promoted strict 1-Wasserstein (EMD) metric. The constructor admits only a metric ground cost (square, zero diagonal, positive off-diagonal, symmetric, triangle), and every evaluation requires equal nonnegative finite mass; the object is immutable, the trait is `metric_law::metric`, and a ground-cost cache key is provided. Split out from the permissive `mtrc::EMD<V>`. |
| `mtrc::Euclidean_standardized<V>` | Aligned finite real vectors with a fitted positive scale | Promoted standardized-Euclidean true metric: `d(a,b) = ||(a-b)/sigma||_2`. The fitting constructor, the explicit `(mean, sigma)` constructor, and every evaluation reject any zero or non-finite fitted scale (the positive-scale gate that preserves identity of indiscernibles); the centering `mean` cancels out of the distance, the trait is `metric_law::metric`, and the cache key serializes `sigma`. |
| `mtrc::Manhattan_standardized<V>` | Aligned finite real vectors with a fitted positive scale | Promoted standardized-Manhattan true metric: `d(a,b) = sum_i |a_i-b_i|/sigma[i]`. Same positive-scale gate, mean-cancellation, `metric_law::metric` trait, and `sigma` cache key as `Euclidean_standardized<V>`. |
| `mtrc::TotalVariation<V>` | Aligned normalized probability vectors | Promoted total-variation metric; inputs must be finite, nonnegative, non-empty, aligned, and sum to 1 within tolerance. |
| `mtrc::Hellinger<V>` | Aligned normalized probability vectors | Promoted root Hellinger metric under the same probability-vector gate. Squared Hellinger is not promoted. |
| `mtrc::SqrtJensenShannon<V>` | Aligned normalized probability vectors | Promoted square-root Jensen-Shannon metric under the same probability-vector gate. Raw JS divergence remains rejected. |
| `mtrc::EmpiricalKolmogorovSmirnov<V>` | Finite non-empty empirical distributions | Promoted tie-safe empirical-CDF sup metric. This is separate from the quarantined Akima-CDF `mtrc::KolmogorovSmirnov`. |
| `mtrc::EmpiricalCramer<V>` | Finite non-empty empirical distributions | Promoted exact root CDF-L2/Cramer metric for empirical step CDFs. This is separate from the quarantined Akima-CDF `mtrc::CramervonMises`. |

## Quarantine Categories

Quarantine status is split by reason:

- **Known non-metric**: the implemented computation is not a true metric and
  is excluded from normal discovery.
- **Unresolved/proof gap**: the exact computation may or may not be metric, but
  the code lacks a law record, domain gates, or counterexample/proof evidence.
- **Metric variant required**: the family contains a true metric variant, but
  the current public class is too permissive, ambiguously named, or missing the
  guards needed to admit that variant.

## Quarantine Decisions

| C++ class | Current exposure | Quarantine category | Reason | Discovery boundary | Replacement or admission route |
| --- | --- | --- | --- | --- | --- |
| `mtrc::Euclidean_thresholded<V>` | `metric/metric/catalog.hpp`, Python alias `ThresholdedEuclidean` | Unresolved/proof gap | Current positive-parameter gates and metric proof are not documented. Threshold transforms can be metric or non-metric depending on exact transform. | Admit only if exact transform is proven metric; otherwise remove public metric exposure. | Record exact transform, add positive parameter checks, search for triangle counterexamples, decide admit/remove. |
| `mtrc::Euclidean_hard_clipped<V>` | `metric/metric/catalog.hpp` | Unresolved/proof gap | Clipped Euclidean transforms need exact proof and positive cap/scale guards. | Admit only if transform is proven metric; otherwise remove public metric exposure. | Add computation review and property tests before any trait. |
| `mtrc::Euclidean_soft_clipped<V>` | `metric/metric/catalog.hpp` | Unresolved/proof gap | Soft clipping changes the metric shape and has no admitted law record. | Admit only if transform is proven metric; otherwise remove public metric exposure. | Review transform, parameters, finite-output behavior, and triangle law. |
| `mtrc::Euclidean_standardized<V>` | `metric/metric/catalog.hpp` | Promoted (admitted) | Standardized Euclidean is a true metric exactly when every fitted scale is finite and positive. The positive-scale gate is now enforced on the fitting constructor, the explicit `(mean, sigma)` constructor, and every evaluation; zero/non-finite scale is rejected. | Admitted true metric. | Done: positive-scale guard, `sigma` cache key, `metric_law::metric` trait, and metric-contract/property tests (`tests/core_smoke/metric_standardized_smoke.cpp`). |
| `mtrc::Manhattan_standardized<V>` | `metric/metric/catalog.hpp` | Promoted (admitted) | Standardized Manhattan is a true metric exactly when every fitted scale is finite and positive, enforced by the same gate on construction and every evaluation. | Admitted true metric. | Done: positive-scale guard, `sigma` cache key, `metric_law::metric` trait, and metric-contract/property tests (`tests/core_smoke/metric_standardized_smoke.cpp`). |
| `mtrc::Cosine<V>` | `metric/metric/catalog.hpp`, Python wrapper | Metric variant required | Implementation appears to be angular-style, but naming says cosine and domain handling for zero vectors/scalar multiples is not explicit. Angular distance is metric on directions/unit vectors, not raw vector identity. | Keep quarantined; use admitted `mtrc::Angular<V>` or `mtrc::Chordal<V>` for unit-vector metrics. | Deprecate or remove the ambiguous class after compatibility review; do not route it to metric-only algorithms. |
| `mtrc::CosineInverted<V>` | `metric/metric/catalog.hpp`, internal uses | Known non-metric | Raw `1 - cosine` style complements are not true metrics in general. | Remove public metric exposure. | Replace uses with admitted angular/chordal metric or explicit internal non-metric helper outside metric-only paths. |
| `mtrc::Weierstrass<V>` | `metric/metric/catalog.hpp` | Unresolved/proof gap | Hyperbolic-distance candidate, but the record domain, finite-input handling, and `acosh` domain are not documented. | Admit only after domain proof and guards. | Define model domain, reject invalid inputs, add exact-value/property tests. |
| `mtrc::Sorensen<V>` | `metric/metric/catalog.hpp`, Python wrapper | Known non-metric | Bray-Curtis/Sorensen-style complements are not true metrics in general. | Replace with admitted Jaccard/Ruzicka/total-variation-style metrics where appropriate. | Remove normal discovery and examples; keep only temporary compatibility alias if required. |
| `mtrc::Hassanat<V>` | `metric/metric/catalog.hpp` | Unresolved/proof gap | Branchy signed formula has no local metric proof or domain admission. | Admit only if exact computation is proven metric; otherwise remove public metric exposure. | Record formula, domain restrictions, counterexample search, and decision. |
| `mtrc::EMD<V>` | `metric/metric/catalog.hpp`, Python wrapper | Known non-metric (compatibility) | Permissive Earth-Mover route: it accepts an arbitrary (possibly non-metric) ground cost, supports unbalanced mass through an extra-mass penalty, and lazily mutates its cost matrix inside a `const` call. None of these preserve the metric axioms. Now explicitly declared `metric_law::distance` with `thread_safe == false`. | Keep as compatibility distance only; route metric users to `mtrc::Wasserstein<V>`. | Done: strict metric split landed as `mtrc::Wasserstein<V>`. Keep `EMD` out of metric-only routing; deprecate then retire from the umbrella after a compatibility window. |
| `mtrc::Wasserstein<V>` | `metric/metric/catalog.hpp` | Promoted (admitted) | Strict 1-Wasserstein (EMD) true metric: equal nonnegative mass over an admitted metric ground cost. Constructor rejects any non-metric ground cost (non-square, nonzero diagonal, zero off-diagonal, asymmetric, triangle-violating); evaluation rejects misaligned, negative, non-finite, or unequal-mass measures. Immutable, `thread_safe == true`. | Admitted true metric. | Promote in discovery; reuse for `RandomEMD`/distribution transport admission where an exact equal-mass Wasserstein path is needed. |
| `mtrc::SSIM<D,V>` | `metric/metric/quarantine/SSIM.hpp` (via `metric/metric/quarantine.hpp`), Python wrapper | Metric variant required | Raw SSIM and `1 - SSIM` are not admitted true metrics. Literature contains SSIM-inspired metric variants, but this class is not admitted. | Remove raw SSIM as public metric; add separate theorem-matched SSIM-inspired metric only if implemented exactly. | Deprecate docs/examples for raw class; design admitted variant separately. |
| `mtrc::Kohonen<D,Sample,...>` | `metric/metric/quarantine/Kohonen.hpp` (via `metric/metric/quarantine.hpp`), Python wrapper | Known non-metric | Learned SOM/graph approximation is model-dependent and has no true metric guarantee by default. | Move to mapping/embedding workflow, not metric catalog. | Remove from normal distance discovery and metric-only routing; expose as model/mapping artifact if kept. |
| `mtrc::RandomEMD<Sample,D>` | `metric/metric/catalog.hpp`, Python wrapper. Audited record model: raw `std::vector<float>` observations reduced to the **Akima-spline-interpolated empirical CDF** of `mtrc::PMQ`, integrated as a Riemann sum of `\|F1-F2\|` over the **pair-dependent** range `[min,max]` with step `(max-min)*precision`. | Known non-metric (as shipped) | Approximates the 1-Wasserstein / `L1`-CDF distance, but the exact shipped computation is neither a metric nor total: (1) tied sample values divide the Akima slope by zero and return `NaN` (even `d(a,a)`); (2) zero-range (all-equal) samples make `step == 0`, so the integration loop never terminates (reproduced as SIGALRM/exit 142); (3) empty inputs SIGSEGV; (4) the pair-dependent domain plus the Akima CDF (which overshoots `[0,1]`) break the triangle inequality. Symmetry and `d(a,a)==0` hold only on the distinct-value domain. | Excluded from metric-only routing and normal discovery. | Use `mtrc::Wasserstein<V>` for transport. A real empirical-CDF metric needs a tie-safe step CDF, a fixed integration domain, and zero-range/empty guards. Defects are pinned in `tests/core_smoke/metric_distribution_quarantine_smoke.cpp`. |
| `mtrc::CramervonMises<Sample,D>` | `metric/metric/catalog.hpp`. Audited record model: same PMQ Akima-CDF as `RandomEMD`, computed as `sqrt` of the Riemann sum of `(F1-F2)^2` (the **root**-`L2`-CDF form, **not** the squared CvM statistic). | Metric variant required (shipped computation is non-metric) | The root-`L2`-CDF *form* is the correct metric candidate (the code correctly takes the `sqrt`; the squared statistic must not be promoted — see `rejected-non-metrics.md`), which keeps the *family* a "variant required". But the *shipped* computation is non-metric: the shared Akima CDF overshoots `[0,1]`, so the triangle inequality fails once near-duplicate knots appear (adversarially reproduced, e.g. `d(a,c)=35.97 > d(a,b)+d(b,c)=35.84`); it also returns `NaN` on tied sample values (so even `d(a,a)` is `NaN`) and hangs on zero-range samples (same `step == 0` loop as `RandomEMD`); empty inputs SIGSEGV. | Admit a root-CDF metric only after replacing the Akima CDF with a tie-safe empirical CDF over a fixed domain. | Root form, the `NaN`-on-ties blocker, and the root-cause CDF overshoot are pinned in `tests/core_smoke/metric_distribution_quarantine_smoke.cpp`. Do not promote the current implementation. |
| `mtrc::KolmogorovSmirnov<Sample,D>` | `metric/metric/catalog.hpp`. Audited record model: PMQ Akima-CDF; returns `max\|F1-F2\|` evaluated **only at the concatenated data points**, not the true supremum. | Known non-metric (as shipped) | The sup-CDF (`Linf`) distance is a metric on distribution functions, but the shipped computation is not: (1) a finite-point supremum on a pair-dependent grid over an **overshooting** Akima CDF violates the triangle inequality and even returns values `> 1` (impossible for a true sup-CDF distance: e.g. `d({0,1,2,10},{1,2,3,100}) ≈ 1.317 > 0.136 + 0.175`); (2) tied sample values silently collapse two distinct distributions to distance 0, breaking identity of indiscernibles. | Admit only after defining an empirical-CDF record domain and taking the true sup over a fixed support with a tie-safe CDF. | Triangle violation, `>1` overshoot, and false-zero identity pinned in `tests/core_smoke/metric_distribution_quarantine_smoke.cpp`. Do not promote the current implementation. |
| `mtrc::RiemannianDistance<RecType,Metric>` | `metric/metric/catalog/space/Riemannian.hpp` (opt-in include; NOT pulled into the header-only `catalog.hpp` umbrella because it needs a LAPACK eigensolver) | Promoted (admitted, restricted metric) | Affine-invariant Riemannian (AIRM) distance on the SPD images of two equal-size finite metric spaces. The operand-encoding defect that broke symmetry and identity of indiscernibles is fixed (both operands are built as the same regularized-Laplacian SPD matrix). It is a restricted metric whenever the base `Metric` is itself a true metric (the law is inherited, as for Hausdorff); guarded against empty/unequal-size operands. | Admitted restricted metric; route through metric-only algorithms when the base metric is a true metric. Keep opt-in (LAPACK), separate from MGC. | Declared law/status pinned in `tests/core_smoke/metric_discovery_smoke.cpp`; runtime metric-law coverage (non-negativity, identity, symmetry, triangle, domain guard) in `tests/core_smoke/metric_riemannian_law_smoke.cpp`. |

## Umbrella Export Policy

`metric/metric/catalog.hpp` exports promoted true metrics and explicit
admission candidates that cannot be mistaken for guaranteed metrics.
Quarantined classes are compatibility surfaces outside metric-only routing
unless admission succeeds.

Initial umbrella cleanup targets:

1. Remove or compatibility-gate raw non-metric surfaces:
   `CosineInverted`, `Sorensen`, raw `SSIM`, `Kohonen`.
2. Split ambiguous names:
   `Cosine` remains quarantined unless it becomes an admitted
   `Angular`/direction metric.
3. Split strict metric variants from permissive compatibility APIs:
   `EMD` (done: strict route is `mtrc::Wasserstein<V>`); the random distribution
   distances `RandomEMD`/`CramervonMises`/`KolmogorovSmirnov` (kept out
   of metric-only routing, defects pinned); clipped transforms.
4. Preserve admitted guard/test coverage for `P_norm`, `Ruzicka`, `TWED`,
   `Wasserstein`, and the standardized vector metrics
   (`Euclidean_standardized`, `Manhattan_standardized`). The distribution-CDF
   candidates are **not** promoted: `RandomEMD` and
   `KolmogorovSmirnov` are non-metric as shipped, and root `CramervonMises`
   remains blocked on a tie-safe CDF. Coverage lives in
   `tests/core_smoke/metric_distribution_quarantine_smoke.cpp`.

## Binding Policy

Python may expose only native C++ surfaces. It must not repair or redefine
metric behavior in Python.

For quarantined classes:

- remove from normal Python discovery docs
- add deprecation or compatibility wording if still exported
- do not route into metric-only strategies
- prefer admitted replacement names before removing aliases

## Verified Admission Notes

- `TWED` is admitted for `penalty >= 0` and `elastic > 0`; parameter gates,
  empty/non-finite input rejection, `metric_law::metric`, cache key coverage,
  and metric-contract/property tests are in
  `tests/core_smoke/metric_twed_smoke.cpp`.
- Strict `mtrc::Wasserstein<V>` is the admitted transport metric. It enforces a
  metric ground cost and equal nonnegative mass with `metric_law::metric`; the
  permissive `mtrc::EMD<V>` remains `metric_law::distance` and
  `thread_safe == false`. Coverage lives in
  `tests/core_smoke/metric_wasserstein_smoke.cpp`.
- `Euclidean_standardized<V>` and `Manhattan_standardized<V>` are admitted under
  a positive-scale gate. Coverage lives in
  `tests/core_smoke/metric_standardized_smoke.cpp` and
  `tests/core_smoke/metric_contracts_smoke.cpp`.
