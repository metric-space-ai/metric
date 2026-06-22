# Rejected Non-Metrics

This page records computations that are not presented as METRIC true metrics.
Rejection here means "do not promote as a true metric or route to metric-only
algorithms." For built-in public distance functions, quarantine is temporary:
the function is either replaced by an admitted true-metric variant or removed
from the public metric library.

## Rejected And Quarantined Computations

| Computation | Status | Why it is not a true metric | Safer discovery path |
| --- | --- | --- | --- |
| Raw cosine distance, usually `1 - cosine(x, y)` | Quarantine, then remove | It can give zero distance to distinct positive scalar multiples and can violate triangle inequality. | `mtrc::Angular<V>` or `mtrc::Chordal<V>` on finite unit-normalized vectors. |
| Raw SSIM or `1 - SSIM` | Quarantine, then remove | SSIM is a perceptual similarity score, and the raw complement is not admitted as a metric. | SSIM-inspired metric variants only when the exact computation, constants, and image domain are admitted; otherwise image features with an admitted metric or Wasserstein/EMD with a metric ground cost. |
| Raw Dice / Sorensen complement | Quarantine, then remove | Common Dice/Sorensen complements can violate triangle inequality. | Jaccard distance for sets, admitted Tanimoto/Jaccardized/Ruzicka variants, Hamming for aligned binary vectors, or Total Variation for normalized counts. |
| Generic Tanimoto similarity or complement | Quarantined until specified | Some Tanimoto/Jaccardized computations are metrics, but the name alone does not define a law-valid distance. | Use `mtrc::Tanimoto<V>` only for the admitted weighted-Jaccard/Ruzicka computation on finite nonnegative aligned vectors. |
| Permissive `mtrc::EMD<V>` (arbitrary ground cost, extra-mass penalty) | Quarantine (compatibility only) | A non-metric ground cost makes the transport value violate the triangle inequality (e.g. Dirac histograms over `C[0][2]=10`, `C[0][1]=C[1][2]=1` give `EMD(e0,e2)=10 > 1+1`), and the extra-mass penalty admits unbalanced mass that breaks identity. | `mtrc::Wasserstein<V>` with an admitted metric ground cost and equal nonnegative mass. |
| DTW defaults | Quarantine, then remove | Unconstrained time warping can collapse distinct sequences and commonly lacks triangle law. | TWED, ERP, or Frechet with admitted parameter constraints. |
| LCSS defaults | Quarantine, then remove | Subsequence matching can collapse distinct records and lacks a general metric guarantee under common defaults. | Edit/Levenshtein for symbolic sequences or admitted elastic time-series metrics. |
| KL divergence | Quarantine, then remove | It is asymmetric and does not satisfy metric laws. | Hellinger, Total Variation, sqrt Jensen-Shannon, or Wasserstein/EMD. |
| Raw Jensen-Shannon divergence | Quarantine, then remove | The divergence itself is not the metric candidate. | Use admitted `mtrc::SqrtJensenShannon<V>` on normalized probability vectors. |
| Squared Cramer-von-Mises statistic | Quarantine, then remove as a metric | Squared CDF `L2` distances do not inherit the metric law. | Use admitted `mtrc::EmpiricalCramer<V>` for empirical step CDFs. |
| Raw chi-square and asymmetric divergences | Quarantine, then remove | Many forms are asymmetric, unstable near zero, or lack triangle law. | Hellinger, Total Variation, or another admitted distribution metric. |
| Model-scored heuristic distances | Quarantine, then remove as metrics | A fitted score has no metric law by default and may change with training data or model version. | Expose the fitted model as a mapping or result-space view only when the output space uses an admitted metric. |

## Rules For Quarantine

- Do not assign `metric_law::metric` to a quarantined computation.
- Do not select cover-tree, triangle-pruning, or other metric-only algorithms for quarantined computations.
- Do not hide rejected computations behind metric-like names.
- Do not add new public examples that teach quarantined formulas as normal METRIC metrics.
- Keep quarantine surfaces out of Metric Discovery except on this warning page.
- Remove bindings and umbrella-header exports after the compatibility window unless admission succeeds.

## Moving Out Of Quarantine

A computation can move out of quarantine only through metric admission. The
admission record must define the record domain, parameter constraints, proof
basis, invalid-input behavior, and `metric_traits` policy. Variants are named
precisely; for example, raw Jensen-Shannon divergence remains rejected even if
square-root Jensen-Shannon is admitted as a metric.

The same precision applies to SSIM, Tanimoto, Dice/Sorensen, Cramer-von-Mises,
and cosine-family names. A rejected raw computation does not reject every metric
variant in the family; a valid variant still needs a separate admitted name and
implementation contract.

## Removal Guard

Before deleting or hiding a built-in computation, record:

- exact computation and implementation class
- record domain and parameter domain
- whether a true metric variant exists in the same family
- counterexample or proof gap for the current implementation
- migration target, such as Jaccard instead of Dice/Sorensen, sqrt-JS instead
  of raw JS, angular/chordal instead of raw cosine complement, or root Cramer
  instead of squared CvM

If a computation is a true metric under tighter conditions, it is not deleted
naively. It is renamed or guarded, admitted, tested, and documented.
