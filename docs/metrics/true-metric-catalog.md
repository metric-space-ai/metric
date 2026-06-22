# True Metric Catalog

This catalog records METRIC-provided metrics and admission candidates. A
candidate may be defined by a closed formula or by an algorithmic minimum-cost
problem. "Law target" describes the mathematical status under the listed
conditions. "Library status" uses the public discovery status: stable,
experimental, or quarantine.

Metric-only algorithms may use only rows whose admitted implementation declares `metric_traits<Metric>::law == metric_law::metric`.

The current native trait baseline declares `Euclidean`, `Manhattan`,
`Chebyshev`, `P_norm`, `WeightedMinkowski`, `Mahalanobis`, `Angular`, `Chordal`,
`DiscreteMetric`, `Hamming`, `Jaccard`, `BinaryJaccard`, `Ruzicka`, `Tanimoto`,
`Edit`, `TWED`, `ERP`, `Hausdorff` over an admitted ground metric,
`Wasserstein`, probability-vector metrics (`TotalVariation`, `Hellinger`,
`SqrtJensenShannon`), empirical-CDF metrics (`EmpiricalKolmogorovSmirnov`,
`EmpiricalCramer`), and the standardized vector metrics
(`Euclidean_standardized`, `Manhattan_standardized`) as `metric_law::metric`.
Rows marked experimental or quarantine are not promoted traits, even when the
mathematical family contains a known metric under stricter conditions.

| Candidate | Record fit | Cost interpretation | Law target | Library status | Admission notes |
| --- | --- | --- | --- | --- | --- |
| Euclidean | Aligned numeric vectors | Straight-line coordinate displacement | True metric | Stable | Existing trait target is `metric_law::metric` for aligned vectors. |
| Manhattan | Aligned numeric vectors | Sum of absolute coordinate changes | True metric | Stable | Existing trait target is `metric_law::metric` for aligned vectors. |
| Chebyshev | Aligned numeric vectors | Maximum coordinate change | True metric | Stable | Existing trait target is `metric_law::metric` for aligned vectors. |
| `P_norm` / Minkowski | Aligned numeric vectors | Lp coordinate displacement | True metric for finite `p >= 1` | Stable | Constructor validation rejects non-finite or sub-metric exponents, the exponent is read-only after construction, and the C++ trait is `metric_law::metric`. |
| Thresholded/clipped Euclidean variants | Aligned numeric vectors | Monotone transform of Euclidean displacement | Candidate only under exact positive-parameter computation | Experimental | Current variants need computation-level proof, positive threshold/scale/cap guards, and tests before promotion. |
| Standardized Euclidean / Manhattan | Aligned numeric vectors with fitted scale | Coordinate displacement after positive scale normalization | True metric if every fitted scale is positive | Stable (`mtrc::Euclidean_standardized<V>`, `mtrc::Manhattan_standardized<V>`) | Admitted: an injective per-coordinate rescaling of L2/L1. The fitting constructor, the explicit `(mean, sigma)` constructor, and every evaluation reject zero or non-finite scales; the centering mean cancels, the trait is `metric_law::metric`, and the cache key serializes `sigma`. Constant features and single-record fits (both zero-scale) are rejected. |
| Weighted Minkowski | Aligned numeric vectors | Lp displacement with feature weights | True metric with positive weights and `p >= 1` | Stable (`mtrc::WeightedMinkowski<V>`) | Constructor rejects empty weights, non-finite or non-positive weights, and non-finite or sub-metric exponents. Zero weights are rejected rather than promoted as a pseudometric. |
| Mahalanobis SPD | Aligned numeric vectors | Covariance-normalized displacement | True metric with SPD matrix | Stable (`mtrc::Mahalanobis<V>`) | Constructor validates a finite square symmetric positive definite precision matrix by Cholesky. PSD and indefinite matrices are rejected. |
| Weierstrass / hyperbolic candidate | Positive-domain numeric vectors or admitted model domain | Hyperbolic displacement | Candidate under documented finite-input and `acosh` domain | Experimental | Current surface needs exact model domain, invalid-input behavior, and property tests. |
| Hamming | Equal-length categorical or binary records | Count of aligned mismatches | True metric | Stable (`mtrc::Hamming`) | Requires fixed alignment and stable equality semantics per position. Variable-length or padded variants remain separate from this admission. |
| Discrete metric | Any record type with stable equality | Zero for equal records, positive constant otherwise | True metric | Stable (`mtrc::DiscreteMetric<V>`) | Positive mismatch cost required. The class name avoids colliding with the legacy PMQ `mtrc::Discrete` distribution helper. |
| Edit / Levenshtein | Strings and sequences | Rewrite cost | True metric with symmetric unit or admitted edit costs | Stable | Existing trait target is `metric_law::metric` for sequences. |
| Damerau-Levenshtein, unrestricted | Strings and sequences | Rewrite cost with adjacent transposition | True metric for admitted unrestricted unit-cost formula | Experimental | Do not admit OSA/restricted DL as this row; OSA has known triangle counterexamples. |
| Jaccard | Sets and sparse binary records | Fraction of non-overlap | True metric | Stable (`mtrc::Jaccard`, `mtrc::BinaryJaccard`) | `Jaccard` canonicalizes finite set containers and ignores order/duplicates; empty/empty distance is 0. `BinaryJaccard` requires aligned entries exactly in `{0,1}`. |
| Ruzicka / weighted Jaccard | Finite nonnegative aligned vectors with fixed zero-union convention | Fractional non-overlap for weighted sets | True metric on admitted domain | Stable | Invalid negative or non-finite entries are rejected; two all-zero records have distance 0. |
| Tanimoto / Jaccardized variants | Nonnegative real-valued aligned vectors for the admitted computation | Weighted-Jaccard/Ruzicka distance | True metric on admitted domain | Stable (`mtrc::Tanimoto<V>` for the Ruzicka computation) | Generic Tanimoto complements remain quarantined by name. The admitted class delegates to the finite nonnegative Ruzicka formula. |
| Angular | Unit finite vectors | Angle between normalized directions | True metric on the unit sphere | Stable (`mtrc::Angular<V>`) | Rejects empty, non-finite, zero, and non-unit vectors. The older `mtrc::Cosine` class remains unpromoted. |
| Chordal | Unit finite vectors | Euclidean chord between normalized directions | True metric | Stable (`mtrc::Chordal<V>`) | Same unit-vector and finite-input gate as `Angular`; raw normalization-inside-distance variants are not admitted. |
| TWED | Finite non-empty real sequences read positionally (dense) | Elastic edit cost with temporal penalty | True metric for `penalty >= 0` and `elastic > 0` | Stable | Constructor and every evaluation reject `penalty < 0`, `elastic <= 0`, non-finite parameters, empty inputs, and non-finite values; the C++ trait is `metric_law::metric`. Triangle inequality follows Marteau's TWED theory and is covered by exhaustive small-domain and randomized property tests; a closed-form proof for this boundary variant is the remaining gap. The `CompressedVector` input route shares the recurrence but is a compatibility path, not the admitted metric record domain (dropped/trailing zeros can collapse identity). Promoted into the default Python adapter: `from metric import metrics; metrics.TWED(penalty=0, elastic=1)` delegates to the native C++ binding. |
| ERP | Scalar time series | Edit distance with real penalty | True metric with fixed external gap sentinel | Stable (`mtrc::ERP<V>`) | Uses absolute-value ground cost. The finite gap value is fixed at construction and every sample equal to the gap is rejected, preserving identity. |
| Frechet | Curves | Coupled traversal cost | True metric for a precisely admitted curve model | Experimental / quarantine until domain is fixed | Continuous vs discrete, stutter/reparameterization identity, and ground-metric policy need a separate admission record. |
| Hausdorff | Sets or point clouds | Worst nearest-neighbor set mismatch | True metric on nonempty finite sets over an admitted ground metric | Stable (`mtrc::Hausdorff<GroundMetric>`) | Empty sets are rejected. The trait is promoted only when `GroundMetric` is itself `metric_law::metric`; default ground is Euclidean. |
| Wasserstein / EMD | Histograms and finite measures | Minimum transport cost | True metric with equal mass and metric ground cost | Stable (`mtrc::Wasserstein<V>`) | Strict route admitted: the ground cost is validated as a metric at construction and equal nonnegative finite mass is enforced at every evaluation; trait is `metric_law::metric` with a ground-cost cache key. The permissive `mtrc::EMD<V>` route (arbitrary cost, extra-mass penalty) stays `metric_law::distance` and is not promoted. The value type must be floating point (convert integer count histograms to `double`/`float`). |
| RandomEMD implementation | Sampled distribution records | Wasserstein-like approximation over an Akima-interpolated empirical CDF | Not promoted (non-metric as shipped) | Quarantine | Tied values return `NaN`, zero-range samples hang, and the pair-dependent integration domain breaks the triangle inequality. Use the admitted `mtrc::Wasserstein<V>` for transport. See `metric_distribution_quarantine_smoke.cpp`. |
| Total Variation | Probability vectors on common finite support | Half L1 probability disagreement | True metric | Stable (`mtrc::TotalVariation<V>`) | Inputs must be finite, nonnegative, aligned, non-empty, and normalized to sum 1 within tolerance. |
| Hellinger | Probability vectors on common finite support | Euclidean distance between square-root densities | True metric | Stable (`mtrc::Hellinger<V>`) | Same probability-vector gate as Total Variation. `H^2` is not the metric candidate. |
| sqrt Jensen-Shannon | Probability vectors on common finite support | Square root of symmetric information divergence | True metric | Stable (`mtrc::SqrtJensenShannon<V>`) | Same probability-vector gate. Uses natural logarithms; raw JS divergence remains rejected. |
| KS | Empirical CDFs or distributions | Maximum CDF separation | True metric on empirical distribution functions | Stable as `mtrc::EmpiricalKolmogorovSmirnov<V>`; shipped `mtrc::KolmogorovSmirnov` remains quarantine | New class sorts samples, groups ties, and computes the true empirical step-CDF supremum. The old Akima-CDF class remains non-metric as documented in `metric_distribution_quarantine_smoke.cpp`. |
| Cramer distance / root Cramer-von-Mises | Empirical CDFs or distributions | Root L2 CDF separation | True metric on empirical CDF representation | Stable as `mtrc::EmpiricalCramer<V>`; shipped `mtrc::CramervonMises` remains quarantine | New class computes the exact root `L2` distance between empirical step CDFs over support intervals. Squared CvM statistics and the old Akima-CDF implementation are not promoted. |
| SSIM-inspired metric variants | Image vectors or windows under exact computation/domain | Structural image discrepancy | Candidate only for proven variants | Experimental | Raw SSIM and `1 - SSIM` remain rejected; admit only literature-backed computations with constants and domain fixed. |
| Riemannian / finite-space distance | Finite metric spaces or SPD-like distance-matrix records | Distance between spaces or matrices | True metric under admitted manifold/finite-space model | Experimental | Existing Riemannian distance needs record model, SPD/domain checks, and trait policy. |

## Quarantine And Retirement Candidates

These are not cataloged as true metrics. They are isolated
from normal Metric Discovery and then either replaced by admitted true-metric
variants or removed from the public metric library.

| Candidate | Status | Reason | Exit |
| --- | --- | --- | --- |
| Raw cosine distance | Quarantine, then remove | `1 - cosine` can collapse distinct positive scalar multiples and can violate the triangle inequality. | Admit angular/chordal variants under exact domains, then remove raw-complement discovery. |
| Raw SSIM / `1 - SSIM` | Quarantine, then remove | SSIM is a similarity score, and the raw complement is not admitted as a metric. | Admit theorem-matched SSIM-inspired metrics only; otherwise remove as public metric. |
| Raw Dice / Sorensen complement | Quarantine, then remove | Common complements can violate the triangle inequality. | Admit Jaccard, Ruzicka, or other exact metric variants; remove raw complement. |
| Generic Tanimoto names | Quarantine until computation-specific admission | Some computations are metrics and some are not. The name alone is unsafe. | Admit exact binary/weighted computations or remove ambiguous public API names. |
| DTW / LCSS defaults | Quarantine, then remove | Common unconstrained elastic alignments can violate identity or triangle law. | Admit TWED, ERP, Frechet, or exact metric edit variants; remove default non-metric routing. |
| KL / raw JS / raw chi-square / asymmetric divergences | Quarantine, then remove | Asymmetry and missing triangle law make them unsafe as metrics; raw JS requires a square root for the metric candidate. | Admit sqrt-JS, Hellinger, TV, Wasserstein, or remove raw divergences from the metric library. |
| Squared Mahalanobis, squared Hellinger, squared Cramer-von-Mises | Quarantine, then remove as metrics | Squared metric values usually lose triangle inequality. | Expose root metrics only. |
| Learned heuristic distances | Quarantine, then remove as metrics | Learned scores need an explicit proof or constraint before they can claim metric law. | Keep only as mappings or derived representations with admitted output metrics, not as METRIC metrics. |
