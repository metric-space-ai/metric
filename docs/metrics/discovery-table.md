# Metric Discovery Table

This is the per-metric discovery table for METRIC: for each public distance
class it answers *which real metric fits which record type*, with the admission
status, the guarantees that status earns, the cost (complexity), the
minimal-recoding-cost / relation principle the metric encodes, references, and a
usage example.

It is generated from, and kept consistent with, two machine-checkable sources:

- `metric/metric/admission.hpp` — the four-status admission scheme.
- `metric/metric/discovery.hpp` — the in-code registry mapping every catalog
  class to its admission status.

The registry is verified by `tests/core_smoke/metric_discovery_smoke.cpp`, which
also proves the metric laws on small finite domains and pins the non-metrics
with concrete counterexamples.

## Admission status legend

| Status | Meaning | `metric_traits` law | Discoverable for metric-only algorithms? |
| --- | --- | --- | --- |
| **admitted** | True metric on its natural record domain; no parameter or per-fit precondition can break the law (only structural alignment is required). | `metric_law::metric` | Yes |
| **restricted metric** | True metric, but only under an enforced parameter/domain gate (e.g. `p >= 1`, nonnegative inputs, a metric ground cost, a positive fitted scale). The gate is validated on construction and on every evaluation. | `metric_law::metric` | Yes |
| **quarantine** | Not a true metric as shipped, but a metric variant exists in the same literature family (or the class has a legitimate non-metric home). Kept for a compatibility window pending admission/relocation; never deleted by family name alone. | `metric_law::distance` | No |
| **rejected** | The specific computation is not a metric, and the family's metric variant is a separate already-present class, so this computation is scheduled for removal. | `metric_law::distance` | No |

The four metric laws referenced below: **(1) non-negativity** `d >= 0`;
**(2) identity of indiscernibles** `d(a,b) = 0 ⇔ a = b`; **(3) symmetry**
`d(a,b) = d(b,a)`; **(4) triangle inequality** `d(a,c) <= d(a,b) + d(b,c)`.

A metric is the **minimal admissible recoding cost / relation value** between two
records: the cheapest cost, under the chosen cost model, to recode one record
into the other (coordinate displacement, symbol rewrites, mass transport, …).
The "recoding principle" column states that cost model for each metric.

---

## Status matrix

| Class | Family / record type | Status | Law |
| --- | --- | --- | --- |
| `Euclidean<V>` | Aligned numeric vectors (L2) | admitted | metric |
| `Manhattan<V>` | Aligned numeric vectors (L1) | admitted | metric |
| `Chebyshev<V>` | Aligned numeric vectors (L∞) | admitted | metric |
| `Edit<V>` | Strings / symbol sequences | admitted | metric |
| `P_norm<V>` | Aligned numeric vectors (Lp, `p>=1`) | restricted metric | metric |
| `WeightedMinkowski<V>` | Aligned vectors, positive weights, `p>=1` | restricted metric | metric |
| `Mahalanobis<V>` | Aligned vectors, SPD matrix | restricted metric | metric |
| `Euclidean_standardized<V>` | Aligned vectors, fitted positive scale | restricted metric | metric |
| `Manhattan_standardized<V>` | Aligned vectors, fitted positive scale | restricted metric | metric |
| `Angular<V>` | Nonzero / unit vectors (directions) | restricted metric | metric |
| `Chordal<V>` | Nonzero / unit vectors (directions) | restricted metric | metric |
| `Ruzicka<V>` | Nonnegative aligned vectors (weighted sets) | restricted metric | metric |
| `Tanimoto<V>` | Nonnegative weighted sets | restricted metric | metric |
| `DiscreteMetric<V>` | Any set with stable equality | restricted metric | metric |
| `Hamming` | Equal-length categorical/binary records | restricted metric | metric |
| `Jaccard` | Sets / sparse membership | restricted metric | metric |
| `BinaryJaccard` | Binary records | restricted metric | metric |
| `TWED<V>` | Real time series (`penalty>=0`, `elastic>0`) | restricted metric | metric |
| `ERP<V>` | Real time series, fixed gap element | restricted metric | metric |
| `Hausdorff<GroundMetric>` | Nonempty point sets, metric ground | restricted metric | metric |
| `Wasserstein<V>` | Equal-mass measures, metric ground cost | restricted metric | metric |
| `TotalVariation<V>` | Normalized measures | restricted metric | metric |
| `Hellinger<V>` | Normalized measures | restricted metric | metric |
| `SqrtJensenShannon<V>` | Normalized measures (√JS) | restricted metric | metric |
| `EmpiricalCramer<V>` | Samples → empirical CDF (root L2-CDF) | restricted metric | metric |
| `EmpiricalKolmogorovSmirnov<V>` | Samples → empirical CDF (sup-CDF) | restricted metric | metric |
| `Cosine<V>` | Vectors (angular/π, unguarded) | quarantine | distance |
| `Weierstrass<V>` | Vectors (hyperboloid distance, unguarded) | quarantine | distance |
| `Euclidean_thresholded<V>` | Vectors (`min(thres, factor·L2)`) | quarantine | distance |
| `Euclidean_hard_clipped<V>` | Vectors (`min(cap, scal·L2)`) | quarantine | distance |
| `Euclidean_soft_clipped<V>` | Vectors (saturating L2) | quarantine | distance |
| `Hassanat<V>` | Vectors (Hassanat; impl bug) | quarantine | distance |
| `EMD<V>` | Measures (permissive transport) | quarantine | distance |
| `RandomEMD<Sample,D>` | Samples (Akima-CDF transport) | quarantine | distance |
| `CramervonMises<Sample,D>` | Samples (Akima root-CDF) | quarantine | distance |
| `KolmogorovSmirnov<Sample,D>` | Samples (Akima sup-CDF) | quarantine | distance |
| `RiemannianDistance<Rec,Metric>` | Finite spaces / SPD (AIRM; impl defect) | quarantine | distance |
| `SSIM<D,V>` | Images / windows | quarantine | distance |
| `Kohonen<D,Sample,…>` | Learned SOM-graph distance | quarantine | distance |
| `CosineInverted<V>` | Vectors (`|1 − cos|`) | rejected | distance |
| `Sorensen<V>` | Vectors (Bray–Curtis) | rejected | distance |

---

## Discovery records — admitted and restricted metrics

Each record gives **Domain · Guarantees · Complexity · Recoding principle ·
References · Example**. `n` = record dimension/length; `|A|`,`|B|` = set sizes.

### Aligned numeric vectors

#### `Euclidean<V>` — L2 — *admitted*
- **Domain:** aligned finite real vectors.
- **Guarantees:** all four laws (norm-induced metric). Real-arithmetic statement; extreme finite inputs may overflow under IEEE-754.
- **Complexity:** `O(n)`.
- **Recoding principle:** straight-line coordinate displacement `‖a−b‖₂`.
- **References:** Minkowski inequality (p=2); Deza & Deza, *Encyclopedia of Distances*, 4th ed. 2016.
- **Example:** `mtrc::Euclidean<double>{}(a, b)`.

#### `Manhattan<V>` — L1 — *admitted*
- **Domain:** aligned real vectors.
- **Guarantees:** all four laws (L1 norm-induced metric).
- **Complexity:** `O(n)`.
- **Recoding principle:** total absolute coordinate change `Σ|aᵢ−bᵢ|` (city-block).
- **References:** Deza & Deza 2016, §17.
- **Example:** `mtrc::Manhattan<double>{}(a, b)`.

#### `Chebyshev<V>` — L∞ — *admitted*
- **Domain:** aligned real vectors.
- **Guarantees:** all four laws (sup norm-induced metric).
- **Complexity:** `O(n)`.
- **Recoding principle:** worst single-coordinate change `maxᵢ|aᵢ−bᵢ|`.
- **References:** Deza & Deza 2016.
- **Example:** `mtrc::Chebyshev<double>{}(a, b)`.

#### `P_norm<V>` — Minkowski — *restricted metric*
- **Domain:** aligned real vectors; **gate:** finite `p >= 1` (constructor throws otherwise; exponent read-only; cache key encodes `p`).
- **Guarantees:** all four laws for `p >= 1` (Minkowski inequality). `p < 1` is **not** a metric (the unit ball is non-convex) and is rejected.
- **Complexity:** `O(n)`.
- **Recoding principle:** Lp coordinate displacement `(Σ|aᵢ−bᵢ|^p)^{1/p}`.
- **References:** Hardy, Littlewood & Pólya, *Inequalities* (1934); Deza & Deza 2016.
- **Example:** `mtrc::P_norm<double>{2.5}(a, b)`.

#### `WeightedMinkowski<V>` — *restricted metric*
- **Domain:** aligned real vectors; **gate:** positive weights and `p >= 1`.
- **Guarantees:** all four laws; positive weights keep identity of indiscernibles (a zero weight degrades it to a pseudometric).
- **Complexity:** `O(n)`.
- **Recoding principle:** weighted Lp displacement `(Σ wᵢ|aᵢ−bᵢ|^p)^{1/p}`.
- **References:** Deza & Deza 2016; standard weighted-norm metric.
- **Example:** `mtrc::WeightedMinkowski<double>({1.0, 4.0}, 2.0)(a, b)`.

#### `Mahalanobis<V>` — *restricted metric*
- **Domain:** aligned real vectors; **gate:** symmetric positive-definite matrix (SPD; constructor rejects asymmetric or non-PD).
- **Guarantees:** all four laws for SPD (it is L2 after the linear map `Σ^{-1/2}`). PSD-but-singular is only a pseudometric; indefinite is rejected.
- **Complexity:** `O(n²)` per evaluation (or `O(n)` with a cached Cholesky factor).
- **Recoding principle:** covariance-normalized displacement `√((a−b)ᵀ M (a−b))`.
- **References:** Mahalanobis (1936), *Proc. Natl. Inst. Sci. India*.
- **Example:** `mtrc::Mahalanobis<double>({{2,0.5},{0.5,1}})(a, b)`.

#### `Euclidean_standardized<V>` / `Manhattan_standardized<V>` — *restricted metric*
- **Domain:** aligned finite real vectors; **gate:** every fitted scale `σᵢ` finite and `> 0` (enforced on fit, on `(mean, sigma)` construction, and on every evaluation).
- **Guarantees:** all four laws — an injective per-coordinate linear reparametrization of L2 / L1. The centering `mean` cancels. A zero/constant feature or a single-record fit (zero scale) is rejected.
- **Complexity:** `O(n)` per evaluation; `O(m·n)` to fit on `m` records.
- **Recoding principle:** coordinate displacement after positive per-axis rescaling `‖(a−b)/σ‖`.
- **References:** standardized-Euclidean / z-score metric; Deza & Deza 2016.
- **Example:** `mtrc::Euclidean_standardized<double>(training)(a, b)`.

### Directions and angles

#### `Angular<V>` — *restricted metric*
- **Domain:** nonzero / unit vectors (directions); **gate:** nonzero norms (zero vector is undefined).
- **Guarantees:** all four laws on directions — `arccos(⟨a,b⟩/(‖a‖‖b‖))` is the geodesic (great-circle) distance on the unit sphere; the implementation clamps the cosine to `[-1,1]`. On raw vectors it is a pseudometric (positive multiples collapse).
- **Complexity:** `O(n)`.
- **Recoding principle:** geodesic angle between directions.
- **References:** geodesic metric on `Sⁿ⁻¹`; Deza & Deza 2016, §19.
- **Example:** `mtrc::Angular<double>{}(unit_a, unit_b)`.

#### `Chordal<V>` — *restricted metric*
- **Domain:** nonzero / unit vectors.
- **Guarantees:** all four laws — Euclidean chord `‖â − b̂‖₂` between normalized directions (or `√(2(1−cos))`).
- **Complexity:** `O(n)`.
- **Recoding principle:** straight-line chord between normalized directions.
- **References:** chordal metric on the sphere/projective space; Deza & Deza 2016.
- **Example:** `mtrc::Chordal<double>{}(unit_a, unit_b)`.

### Sets, categories, weighted sets

#### `Ruzicka<V>` (Soergel / weighted Jaccard) — *restricted metric*
- **Domain:** finite nonnegative aligned vectors; **gate:** finite nonnegative entries (negatives / non-finite rejected); two all-zero records have distance 0.
- **Guarantees:** all four laws — `1 − Σmin(aᵢ,bᵢ)/Σmax(aᵢ,bᵢ)` is the Soergel distance, a true metric on nonnegative vectors.
- **Complexity:** `O(n)`.
- **Recoding principle:** fractional non-overlap of weighted sets.
- **References:** Soergel distance; Späth (1980); Deza & Deza 2016, §17.
- **Example:** `mtrc::Ruzicka<double>{}(a, b)`.

#### `Tanimoto<V>` — *restricted metric*
- **Domain:** nonnegative weighted sets / vectors.
- **Guarantees:** all four laws on the admitted computation (Lipkus-style Tanimoto distance is a metric).
- **Complexity:** `O(n)`.
- **Recoding principle:** similarity-to-distance transform of weighted-set overlap.
- **References:** Lipkus (1999), *J. Math. Chem.* 26:263–265.
- **Example:** `mtrc::Tanimoto<double>{}(a, b)`.

#### `DiscreteMetric<V>` — *restricted metric*
- **Domain:** any set with stable equality; **gate:** positive constant.
- **Guarantees:** all four laws — `0` if equal, a positive constant otherwise.
- **Complexity:** `O(1)` (plus equality cost).
- **Recoding principle:** flat unit cost of "not the same record".
- **References:** discrete metric; any metric-spaces text.
- **Example:** `mtrc::DiscreteMetric<double>{2.0}(x, y)`.

#### `Hamming` — *restricted metric*
- **Domain:** equal-length categorical/binary records; **gate:** equal length.
- **Guarantees:** all four laws — count of mismatched aligned positions.
- **Complexity:** `O(n)`.
- **Recoding principle:** minimal aligned single-symbol substitutions.
- **References:** Hamming (1950), *Bell System Tech. J.* 29:147–160.
- **Example:** `mtrc::Hamming{}(std::string("cat"), std::string("cot"))`.

#### `Jaccard` / `BinaryJaccard` — *restricted metric*
- **Domain:** sets / sparse membership (`Jaccard`), binary vectors (`BinaryJaccard`).
- **Guarantees:** all four laws — `1 − |A∩B|/|A∪B|` is the Jaccard metric (empty-set rule fixed at admission).
- **Complexity:** `O(n)`–`O(n log n)`.
- **Recoding principle:** fraction of membership disagreement.
- **References:** Levandowsky & Winter (1971), *Nature* 234:34–35 (Jaccard is a metric).
- **Example:** `mtrc::Jaccard{}({1,2,3}, {2,3,4})`.

### Time series and curves

#### `TWED<V>` — Time Warp Edit Distance — *restricted metric*
- **Domain:** finite non-empty densely-indexed real sequences; **gate:** `penalty >= 0`, `elastic > 0`, finite parameters and values (enforced on construction and every evaluation).
- **Guarantees:** all four laws for `penalty >= 0`, `elastic > 0`; `elastic > 0` is what keeps timestamp identity. Verified by exhaustive small-domain and 40k randomized property trials.
- **Complexity:** `O(|a|·|b|)`.
- **Recoding principle:** minimal elastic edit cost with a temporal penalty.
- **References:** Marteau (2009), *IEEE TPAMI* 31(2):306–318.
- **Example:** `mtrc::TWED<double>{0.0, 1.0}(a, b)`.

#### `ERP<V>` — Edit distance with Real Penalty — *restricted metric*
- **Domain:** real time series; **gate:** fixed gap element and ground metric.
- **Guarantees:** all four laws (ERP is a metric, unlike DTW/LCSS).
- **Complexity:** `O(|a|·|b|)`.
- **Recoding principle:** minimal edit cost against a fixed reference gap.
- **References:** Chen & Ng (2004), *VLDB* — "On the marriage of Lp-norms and edit distance".
- **Example:** `mtrc::ERP<double>{-1.0}(a, b)`.

#### `Hausdorff<GroundMetric>` — *restricted metric*
- **Domain:** nonempty finite point sets; **gate:** a metric ground distance (the law is inherited: `metric_traits<Hausdorff<G>>::law` is `metric` iff `G` is a metric).
- **Guarantees:** all four laws on nonempty compact/finite sets under a metric ground.
- **Complexity:** `O(|A|·|B|)`.
- **Recoding principle:** worst nearest-neighbour set mismatch.
- **References:** Hausdorff (1914), *Grundzüge der Mengenlehre*; Deza & Deza 2016, §1.
- **Example:** `mtrc::Hausdorff<mtrc::Euclidean<double>>{}(A, B)`.

### Histograms, measures, distributions

#### `Wasserstein<V>` — strict 1-Wasserstein / EMD — *restricted metric*
- **Domain:** finite nonnegative measures over a fixed ground cost; **gate:** ground cost validated as a metric (square, zero diagonal, positive off-diagonal, symmetric, triangle) at construction, and equal nonnegative finite mass at every evaluation. Immutable, thread-safe.
- **Guarantees:** all four laws (Kantorovich–Rubinstein). The value type must be floating point.
- **Complexity:** `O(n³ log n)` min-cost flow; `O(n³)` ground-cost triangle check at construction.
- **Recoding principle:** minimal mass-transport cost under the ground metric.
- **References:** Villani (2009), *Optimal Transport: Old and New*; Rubner et al. (2000) EMD.
- **Example:** `mtrc::Wasserstein<double>::on_line(bins)(p, q)`.

#### `TotalVariation<V>` — *restricted metric*
- **Domain:** normalized measures / probability vectors over common support.
- **Guarantees:** all four laws — `½Σ|pᵢ−qᵢ|` (a restriction of L1).
- **Complexity:** `O(n)`.
- **Recoding principle:** half the total probability that must be moved.
- **References:** Deza & Deza 2016, §14; standard probability metric.
- **Example:** `mtrc::TotalVariation<double>{}(p, q)`.

#### `Hellinger<V>` — *restricted metric*
- **Domain:** normalized measures.
- **Guarantees:** all four laws — `‖√p − √q‖₂/√2`, the L2 distance between √-densities. (`H²`, the squared form, is **not** a metric.)
- **Complexity:** `O(n)`.
- **Recoding principle:** Euclidean displacement of √-densities.
- **References:** Hellinger (1909); Deza & Deza 2016, §14.
- **Example:** `mtrc::Hellinger<double>{}(p, q)`.

#### `SqrtJensenShannon<V>` — *restricted metric*
- **Domain:** normalized measures.
- **Guarantees:** all four laws — `√JSD(p,q)` is a metric (raw JS divergence is **not**).
- **Complexity:** `O(n)`.
- **Recoding principle:** root of the symmetric information divergence to the midpoint distribution.
- **References:** Endres & Schindelin (2003), *IEEE Trans. Inf. Theory* 49(7):1858–1860; Österreicher & Vajda (2003).
- **Example:** `mtrc::SqrtJensenShannon<double>{}(p, q)`.

#### `EmpiricalCramer<V>` / `EmpiricalKolmogorovSmirnov<V>` — *restricted metric*
- **Domain:** samples reduced to a tie-safe empirical CDF over a fixed support; **gate:** the fixed-support, monotone CDF representation (the admitted replacements for the quarantined Akima-CDF distances below).
- **Guarantees:** all four laws — `EmpiricalCramer` is the root L2-CDF (Cramér) distance, `EmpiricalKolmogorovSmirnov` is the sup-CDF (L∞) distance, both metrics on distribution functions.
- **Complexity:** `O(n log n)` (sort).
- **Recoding principle:** L2 / L∞ separation of cumulative distribution functions.
- **References:** Cramér (1928); Kolmogorov (1933); Deza & Deza 2016, §14.
- **Example:** `mtrc::EmpiricalCramer<double>{}(sample_a, sample_b)`.

---

## Quarantine — non-metric as shipped, metric variant exists

These keep `metric_law::distance` and are **not** discoverable for metric-only
algorithms. Each row has a salvage path in
[missing-metrics-backlog.md](missing-metrics-backlog.md).

| Class | Why not a metric (as shipped) | Metric variant / exit |
| --- | --- | --- |
| `Cosine<V>` | `arccos(cos)/π` with **no** zero-vector guard: NaN at the zero vector, and a pseudometric on raw vectors (distinct positive multiples → 0, e.g. `d((1,0),(2,0))=0`). | The guarded **`Angular`** metric (admitted) — directions/unit vectors. |
| `Weierstrass<V>` | Hyperboloid-model hyperbolic distance `acosh(√(1+‖a‖²)√(1+‖b‖²) − ⟨a,b⟩)`; a **true metric on Rⁿ**, but the `acosh` argument is not clamped to `≥1`, so rounding can produce NaN. | Admit a **`Hyperbolic`** metric with the `acosh` clamp + finite-input guard (the math is verified). |
| `Euclidean_thresholded<V>` | `min(thres, factor·L2)` — a **truncated metric** (concave transform of L2) for positive parameters, but no positive-parameter guard. | Admit a **`TruncatedEuclidean`** metric with `thres>0, factor>0` guards. |
| `Euclidean_hard_clipped<V>` | `min(cap, scal·L2)` — same truncated-metric argument, no guard. | Admit with `cap>0, scal>0` guards. |
| `Euclidean_soft_clipped<V>` | Continuous, nondecreasing, **concave** saturating transform of L2 — a metric for valid params (`max>0, scal>0, 0<thresh<1`), no guard. | Admit with the parameter guard. |
| `Hassanat<V>` | The shipped negative branch computes `1−(1+2·min)/(1+max+min)` instead of the published `1−1/(1+max−min)` (uses `2·min` not `min+|min|`), so the shipped form is not the proven metric. | Fix the negative branch to the published Hassanat (2014) formula, then admit. |
| `EMD<V>` | Permissive transport: arbitrary (possibly non-metric) ground cost, unbalanced mass via extra-mass penalty, and lazy mutation in a `const` call. Counterexample pinned in `metric_wasserstein_smoke.cpp` (`d(e0,e2)=10 > 1+1`). | Use the admitted **`Wasserstein`** (strict). Keep `EMD` as a compatibility distance. |
| `RandomEMD<Sample,D>` | Integrates `|F1−F2|` over a PMQ Akima-interpolated CDF on a pair-dependent grid; the CDF overshoots `[0,1]` (triangle fails), NaN on ties, infinite loop on zero range. | Use **`Wasserstein`** / the new **`EmpiricalCramer`**. |
| `CramervonMises<Sample,D>` | Correct **root**-L2-CDF *form*, but the same Akima CDF breaks triangle and returns NaN on ties. | Admitted replacement: **`EmpiricalCramer`** (tie-safe fixed support). |
| `KolmogorovSmirnov<Sample,D>` | Sup over a finite grid of the same Akima CDF: returns values `>1`, violates triangle, false-zero on ties. | Admitted replacement: **`EmpiricalKolmogorovSmirnov`**. |
| `RiemannianDistance<Rec,Metric>` | **Confirmed non-metric defect**: the two operands are encoded with different formulas (`Riemannian.cpp:422` off-diagonal `−d` vs `:432` `+d`), so **symmetry fails** (`d(ds1,ds2)=0.784 ≠ d(ds2,ds1)=1.894`) and **identity fails** (`d(ds2,ds2)=1.41 ≠ 0`) on the repo's own `riemannian_test.cpp` data. | The AIRM `√(Σ log²λᵢ)` *is* a metric on the SPD cone (Pennec et al. 2006); fix the encoding to use the same SPD operator for both operands, then admit. |
| `SSIM<D,V>` | Raw SSIM-based aggregate; raw SSIM and `1−SSIM` are not metrics. | Admit a theorem-matched SSIM-derived metric (e.g. `√(1−SSIM)` under normalized conditions, Brunet/Vrscay/Wang 2012). |
| `Kohonen<D,Sample,…>` | `min(m(a,b), m(a,w_Ba)+S(Ba,Bb)+m(w_Bb,b))` with `S` = Dijkstra all-pairs on the SOM graph and **exact** base-metric edges. With a metric base `m`, `S(i,j) >= m(w_i,w_j)` so the detour `>= m(a,b)` and the `min` collapses to `m` — it **is** a true metric (verified on 200k random graphs). Quarantine is defensible only because it is a stateful *trained-model artifact* that adds no metric structure of its own and breaks if templated with a non-metric base (e.g. squared-Euclidean creates a shortcut). The "BMU/residual lift breaks identity/triangle" rationale is false. | Relocate to the mapping/embedding workflow; not a catalog metric (it merely inherits the base metric). |

## Rejected — non-metric, metric variant is a separate class

| Class | Why not a metric | Counterexample (executed in `metric_discovery_smoke.cpp`) | Metric variant (present) |
| --- | --- | --- | --- |
| `CosineInverted<V>` | Raw `|1 − cos|` complement violates the triangle inequality. | `u0=(1,0), u1=(1,2), u2=(−1,2)`: `d(u0,u2) > d(u0,u1) + d(u1,u2)`. | `Angular` / `Chordal`. |
| `Sorensen<V>` | Bray–Curtis `Σ|aᵢ−bᵢ|/Σ|aᵢ+bᵢ|` violates the triangle inequality. | `A=(0,1), B=(1,0), C=(1,1)`: `d(A,B)=1 > d(A,C)+d(C,B)=1/3+1/3`. | `Ruzicka` (Soergel). |

References note: the umbrella reference for most families is M. M. Deza & E.
Deza, *Encyclopedia of Distances*, 4th ed., Springer 2016. Family-specific
citations are listed per record above and re-stated in the backlog.
