# Metric Discovery

Metric Discovery is the entry point for selecting the metric that defines a
finite METRIC space. Start with the record domain, then choose the cost model
whose metric-law status matches the computation.

Use this section with:

- [Metric Discovery Table](discovery-table.md) — per-metric domain, status, guarantees, complexity, recoding principle, references, and examples
- [Missing-Metrics Backlog](missing-metrics-backlog.md) — literature-driven backlog and quarantine salvage paths
- [True Metric Catalog](true-metric-catalog.md)
- [Metric Admission](metric-admission.md)
- [Rejected Non-Metrics](rejected-non-metrics.md)
- [Metric Quarantine Inventory](quarantine-inventory.md)

## Status Labels

| Status | Meaning | Routing consequence |
| --- | --- | --- |
| `stable` | Native implementation declares `metric_traits<Metric>::law == metric_law::metric` under the documented domain and parameter gates. | Available for metric-only paths such as triangle-law indexes. |
| `experimental` | The family or implementation has a metric interpretation only under unresolved or not-yet-promoted domain, parameter, proof, or result-contract conditions. | Documented for admission work, not routed as a guaranteed METRIC metric. |
| `quarantine` | The implemented computation is non-metric, unsafe, ambiguously named, or a compatibility distance rather than an admitted true metric. | Excluded from normal discovery and metric-only routing. |

Metric-only algorithms use `metric_traits`; they do not infer metric validity
from a class name, example output, or empirical checks.

## Current Stable Trait Baseline

The native C++ baseline currently promotes these true metrics:

- `mtrc::Euclidean<V>`, `mtrc::Manhattan<V>`, `mtrc::Chebyshev<V>`
- `mtrc::P_norm<V>` for finite `p >= 1`
- `mtrc::WeightedMinkowski<V>` for finite `p >= 1` and finite positive weights
- `mtrc::Mahalanobis<V>` for finite aligned vectors and an SPD precision matrix
- `mtrc::Euclidean_standardized<V>` and `mtrc::Manhattan_standardized<V>` with positive fitted scale
- `mtrc::Angular<V>` and `mtrc::Chordal<V>` for finite unit-normalized vectors
- `mtrc::DiscreteMetric<V>`, `mtrc::Hamming`, `mtrc::Jaccard`, `mtrc::BinaryJaccard`
- `mtrc::Ruzicka<V>` and `mtrc::Tanimoto<V>` for the admitted nonnegative weighted-Jaccard/Ruzicka computation
- `mtrc::Edit<V>`
- `mtrc::TWED<V>` for finite non-empty real sequences with `penalty >= 0` and `elastic > 0`
- `mtrc::ERP<V>` for finite scalar sequences whose samples are distinct from the fixed finite gap value
- `mtrc::Hausdorff<GroundMetric>` when the ground metric is admitted and inputs are non-empty finite sets
- `mtrc::Wasserstein<V>` for equal nonnegative finite measures over an admitted metric ground cost
- `mtrc::TotalVariation<V>`, `mtrc::Hellinger<V>`, and `mtrc::SqrtJensenShannon<V>` for aligned normalized probability vectors
- `mtrc::EmpiricalKolmogorovSmirnov<V>` and `mtrc::EmpiricalCramer<V>` for finite non-empty empirical distributions

## Discovery Matrix

| Metric or computation | Domain | Meaning | Metric-law note | Example | Status |
| --- | --- | --- | --- | --- | --- |
| Euclidean | Aligned numeric vectors | Straight-line coordinate displacement. | True metric on finite aligned vectors. | `mtrc::Euclidean<std::vector<double>>{}` for sensor vectors. | `stable` |
| Manhattan | Aligned numeric vectors | Sum of absolute coordinate changes. | True metric on finite aligned vectors. | City-block distance over count vectors. | `stable` |
| Chebyshev | Aligned numeric vectors | Maximum coordinate change. | True metric on finite aligned vectors. | Worst-coordinate deviation. | `stable` |
| `P_norm` / Minkowski | Aligned numeric vectors | Lp coordinate displacement. | True metric for finite `p >= 1`; sub-metric exponents are rejected. | `mtrc::P_norm<std::vector<double>>(3.0)`. | `stable` |
| Weighted Minkowski | Aligned numeric vectors with positive feature weights | Weighted Lp displacement. | True metric for finite positive weights and finite `p >= 1`. | Reliability-weighted sensor vector comparison. | `stable` |
| Mahalanobis SPD | Aligned numeric vectors with SPD precision matrix | Covariance-normalized displacement. | True metric when the precision matrix is finite symmetric positive definite. | Distance after a validated inverse covariance fit. | `stable` |
| Standardized Euclidean / Manhattan | Aligned numeric vectors with fitted positive scale | Coordinate displacement after scale normalization. | True metric when every fitted `sigma` is finite and `> 0`. | Compare features after per-feature scale fitting. | `stable` |
| Angular | Unit finite vectors | Angle between normalized directions. | True metric on the unit sphere; zero and non-unit vectors are rejected. | Direction-only comparison of normalized embeddings. | `stable` |
| Chordal | Unit finite vectors | Euclidean chord distance on the unit sphere. | True metric under the same unit-vector gate as Angular. | Unit-vector direction distance with Euclidean chord geometry. | `stable` |
| Discrete metric | Equality-comparable records | Zero for equal records, positive constant otherwise. | True metric with positive mismatch cost. | State-label mismatch distance. | `stable` |
| Hamming | Equal-length categorical or binary records | Count of aligned mismatches. | True metric when records have fixed alignment and equal length. | Compare fixed-width event codes. | `stable` |
| Edit / Levenshtein | Strings and symbolic sequences | Minimal rewrite cost. | True metric for symmetric unit or admitted edit costs. | `mtrc::Edit<char>{}` for strings. | `stable` |
| Jaccard | Finite sets and sparse binary records | Fraction of non-overlap. | True metric with finite set semantics; empty/empty distance is 0. | Compare active-tag sets. | `stable` |
| Binary Jaccard | Aligned binary presence vectors | Jaccard distance over aligned `{0,1}` entries. | True metric when entries are exactly binary. | Compare bitset-like feature rows. | `stable` |
| Ruzicka / weighted Jaccard | Finite nonnegative aligned vectors | Fractional non-overlap for weighted sets. | True metric under finite nonnegative input gate and fixed zero-union convention. | Compare nonnegative count histograms by weighted overlap. | `stable` |
| Tanimoto, admitted Ruzicka computation | Finite nonnegative aligned vectors | Weighted-Jaccard/Ruzicka distance under the admitted implementation. | True metric for this exact computation; generic Tanimoto complements remain quarantined. | Nonnegative feature-weight distance. | `stable` |
| TWED | Finite non-empty real sequences | Elastic edit cost with temporal penalty. | True metric for `penalty >= 0` and `elastic > 0`; invalid parameters and non-finite values are rejected. | Compare process windows by time-warp edit distance. | `stable` |
| ERP | Finite scalar sequences | Edit distance with a fixed external gap value. | True metric when samples exclude the finite gap sentinel. | Compare scalar time series with gap-aware edits. | `stable` |
| Hausdorff | Non-empty finite sets over an admitted ground metric | Worst nearest-neighbor set mismatch. | True metric when the ground metric is true and sets are non-empty. | Point-cloud or feature-set comparison. | `stable` |
| Wasserstein / strict EMD | Equal nonnegative finite measures over metric ground cost | Minimum mass-transport cost. | True metric with equal mass and an admitted metric ground cost. | Histogram transport over a validated ground-distance matrix. | `stable` |
| Total Variation | Aligned normalized probability vectors | Half-L1 probability disagreement. | True metric for finite nonnegative probability vectors summing to 1. | Compare categorical distributions. | `stable` |
| Hellinger | Aligned normalized probability vectors | Euclidean distance between square-root densities. | True metric for the same probability-vector gate. | Compare normalized histograms. | `stable` |
| sqrt Jensen-Shannon | Aligned normalized probability vectors | Square root of symmetric information divergence. | True metric; raw JS divergence is not promoted. | Compare probability vectors with symmetric information geometry. | `stable` |
| Empirical Kolmogorov-Smirnov | Finite non-empty empirical distributions | Supremum distance between empirical step CDFs. | True metric for tie-safe empirical CDFs. | Compare sample distributions by maximum CDF gap. | `stable` |
| Empirical Cramer / root Cramer-von-Mises | Finite non-empty empirical distributions | Root-L2 distance between empirical step CDFs. | True metric for exact empirical step CDF representation; squared CvM is rejected. | Compare samples by integrated CDF discrepancy. | `stable` |
| Damerau-Levenshtein, unrestricted | Strings and sequences | Rewrite cost with adjacent transposition. | Metric only for exact unrestricted unit-cost formula; OSA/restricted variants are not covered. | Symbolic-sequence editing with transpositions. | `experimental` |
| Frechet | Curves | Coupled traversal cost. | Metric status depends on precise continuous/discrete model, stutter policy, and ground metric. | Curve-shape comparison after admission. | `experimental` |
| Riemannian / finite-space distance | Finite metric spaces or SPD-like matrix records | Distance between spaces or matrix manifolds. | Requires admitted record model and domain checks. | Compare two finite spaces through induced distance matrices. | `experimental` |
| Weierstrass / hyperbolic candidate | Positive-domain numeric vectors or a fixed model domain | Hyperbolic displacement. | Requires exact finite-input and `acosh` domain admission. | Hyperbolic-model vector comparison after domain validation. | `experimental` |
| Thresholded or clipped Euclidean variants | Aligned numeric vectors | Transformed Euclidean displacement. | Metric status depends on exact transform and positive parameter gates. | Bounded vector discrepancy after proof and guards. | `experimental` |
| SSIM-inspired metric variants | Image vectors or windows under exact formula | Structural image discrepancy. | Only theorem-matched metric variants can be admitted; raw SSIM is not a metric. | Image-structure metric after exact variant admission. | `experimental` |
| Raw cosine complement / `1 - cosine` | Vectors | Similarity complement. | Can collapse distinct scalar multiples and violate triangle inequality. | Use `Angular` or `Chordal` under unit-vector gates. | `quarantine` |
| Ambiguous `mtrc::Cosine<V>` | Vectors | Direction-like distance under an unclear public name. | Not promoted unless exact angular/chordal semantics and zero-vector policy are admitted. | Prefer admitted `mtrc::Angular<V>`. | `quarantine` |
| `mtrc::CosineInverted<V>` | Vectors | Raw cosine-complement style distance. | Known non-metric in general. | Replace with `Angular` or `Chordal`. | `quarantine` |
| Permissive `mtrc::EMD<V>` | Histograms with arbitrary ground cost or extra-mass penalty | Transport-like compatibility distance. | Non-metric when the ground cost is not a metric or mass is unbalanced. | Use strict `mtrc::Wasserstein<V>`. | `quarantine` |
| `mtrc::RandomEMD<Sample,D>` | Sampled distributions through Akima-interpolated empirical CDF | Wasserstein-like approximation. | Current computation is not total and can violate triangle inequality. | Use `mtrc::Wasserstein<V>` for transport. | `quarantine` |
| `mtrc::CramervonMises<Sample,D>` | Sampled distributions through Akima-interpolated empirical CDF | Root-L2 CDF approximation. | Shipped Akima-CDF computation is non-metric; exact empirical root-CDF metric is separate. | Use `mtrc::EmpiricalCramer<V>`. | `quarantine` |
| `mtrc::KolmogorovSmirnov<Sample,D>` | Sampled distributions through Akima-interpolated empirical CDF | Finite-grid CDF gap. | Shipped computation is non-metric; exact empirical sup-CDF metric is separate. | Use `mtrc::EmpiricalKolmogorovSmirnov<V>`. | `quarantine` |
| Raw SSIM / `1 - SSIM` | Images or image windows | Perceptual similarity complement. | Not an admitted true metric. | Use admitted image features, Wasserstein image distributions, or a future proven SSIM-inspired metric. | `quarantine` |
| `mtrc::Kohonen<D,Sample,...>` | Learned SOM or graph approximation | Model-dependent learned score. | A fitted score has no metric law by default. | Treat as a mapping/model artifact, not a record metric. | `quarantine` |
| `mtrc::Sorensen<V>` and raw Dice/Sorensen complements | Counts, sets, or nonnegative vectors | Similarity complement. | Common forms violate triangle inequality. | Use Jaccard, Ruzicka, Tanimoto-Ruzicka, Hamming, or Total Variation where domains match. | `quarantine` |
| `mtrc::Hassanat<V>` | Numeric vectors under branchy signed formula | Heuristic numeric dissimilarity. | No admitted domain and metric-law record. | Use an admitted vector or probability metric. | `quarantine` |
| DTW / LCSS defaults | Time series or sequences | Elastic alignment or subsequence matching. | Common defaults can collapse identity and lack triangle law. | Use TWED, ERP, Frechet, or Edit when domains match. | `quarantine` |
| KL, raw JS, raw chi-square, asymmetric divergences | Probability vectors or distributions | Divergence or score. | Asymmetry or missing triangle law excludes them as METRIC metrics; raw JS needs square root. | Use Hellinger, Total Variation, sqrt Jensen-Shannon, or Wasserstein. | `quarantine` |
| Squared metric statistics | Any domain | Squared form of a metric value. | Squaring a metric generally loses triangle inequality. | Use root metrics such as Hellinger or Empirical Cramer. | `quarantine` |

## Discover By Record Type

| Record type | Primary stable choices | Notes |
| --- | --- | --- |
| Aligned numeric vectors | Euclidean, Manhattan, Chebyshev, `P_norm`, Weighted Minkowski, Mahalanobis SPD, standardized vector metrics | Match feature scale, weights, and SPD assumptions before selecting the metric. |
| Fixed categorical or binary vectors | Hamming, Discrete, Binary Jaccard | Use Hamming for aligned positions and Jaccard for set presence. |
| Strings and symbol sequences | Edit/Levenshtein, Hamming, Discrete | Edit distance models rewrite cost; Hamming requires equal-length alignment. |
| Time series and curves | TWED, ERP; Frechet after admission | Prefer admitted elastic metrics over unconstrained DTW or LCSS defaults. |
| Sets, point clouds, shapes | Hausdorff, Jaccard | Select by geometry, membership, and ground-metric semantics. |
| Histograms and distributions | Wasserstein, Total Variation, Hellinger, sqrt Jensen-Shannon, empirical KS, empirical Cramer | Match mass constraints, normalization, and support policy. |
| Images and feature fields | Wasserstein image distributions, Hausdorff feature sets, admitted feature metrics | Raw SSIM and `1 - SSIM` remain quarantine entries. |
| Finite metric spaces | Riemannian/finite-space distance after admission | Use only when the record itself is a finite space or admitted matrix-domain object. |

## Custom Metrics

Custom callables remain useful when a domain is not covered by the catalog, but
they are not METRIC-promoted metrics until the project provides an admission
record and truthful `metric_traits` specialization. A custom admission record
states the record kind, metric law, parameter constraints, invalid-input
behavior, thread-safety, and cache key.
