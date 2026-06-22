# Missing-Metrics Backlog

A concrete, literature-driven backlog of true metrics for Metric Discovery,
prioritized by record-type coverage. Each entry states the **record type**, the
exact **metric to admit**, the **admission criteria** (domain/parameter guards,
proof basis, required tests), a **reference**, and **priority**.

This list is split into three parts:

- **A. Recently admitted** — implemented during the catalog build-out; listed so
  the backlog is not mistaken for still-open work.
- **B. Quarantine salvage** — admit the metric *variant* of a quarantined class
  in place (the "do not delete by family name" exits).
- **C. Still missing** — net-new metrics from the literature, prioritized.

Admission for every entry must follow `docs/metrics/metric-admission.md`: a
record domain, parameter gates enforced on construction and evaluation, a
`metric_traits` law, a `metric_admission` registry entry in
`metric/metric/discovery.hpp`, metric-law tests on small finite domains, and a
counterexample/proof note.

---

## A. Recently admitted (no longer missing)

These now carry `metric_law::metric` and a `restricted metric` admission status
(see [discovery-table.md](discovery-table.md)). They close the largest coverage
gaps the earlier "Planned" rows tracked.

| Metric | Record type | Reference |
| --- | --- | --- |
| `WeightedMinkowski` | weighted aligned vectors | Deza & Deza 2016 |
| `Mahalanobis` | SPD-normalized vectors | Mahalanobis 1936 |
| `Angular`, `Chordal` | directions / unit vectors | geodesic & chordal metric on `Sⁿ⁻¹` |
| `DiscreteMetric` | any set with equality | discrete metric |
| `Hamming` | equal-length categorical/binary | Hamming 1950 |
| `Jaccard`, `BinaryJaccard` | sets / binary | Levandowsky & Winter 1971 |
| `Tanimoto` | nonnegative weighted sets | Lipkus 1999 |
| `ERP` | real time series | Chen & Ng 2004 |
| `Hausdorff` | nonempty point sets | Hausdorff 1914 |
| `TotalVariation`, `Hellinger`, `SqrtJensenShannon` | normalized measures | Endres & Schindelin 2003; Österreicher & Vajda 2003 |
| `EmpiricalCramer`, `EmpiricalKolmogorovSmirnov` | samples → empirical CDF | Cramér 1928; Kolmogorov 1933 |

---

## B. Quarantine salvage (admit the variant in place)

Each is a quarantined class whose family has a true metric variant. The work is
**rename/guard/prove/test/admit**, not deletion. Math verified by the
adversarial verdict workflow and by `tests/core_smoke/metric_discovery_smoke.cpp`.

| # | New metric | From (quarantine) | Admission criteria | Reference | Priority |
| --- | --- | --- | --- | --- | --- |
| B1 | `TruncatedEuclidean` (`min(C, k·L2)`) | `Euclidean_thresholded`, `Euclidean_hard_clipped` | Guard `C>0, k>0`; proof: `f(t)=min(C,kt)` is concave, nondecreasing, `f(0)=0`, `f(t)=0⇔t=0`, so `f∘L2` is a metric. Exhaustive small-domain triangle test with the cap **active**. | Concave transform of a metric (Deza & Deza 2016, §4) | High |
| B2 | `SaturatingEuclidean` (soft clip) | `Euclidean_soft_clipped` | Guard `max>0, scal>0, 0<thresh<1`; proof: the piecewise transform is `C¹`, concave, nondecreasing, `g(0)=0`; matched slope `scal` at the knee. Small-domain test with saturation active. | concave-transform metric | Medium |
| B3 | `Hyperbolic` (hyperboloid model) | `Weierstrass` | Clamp `acosh` argument to `≥1`; reject non-finite inputs. Proof: lift `x↦(√(1+‖x‖²), x)`; the Lorentzian product `≥1` with equality iff equal, so it is the geodesic metric of `Hⁿ`. | Reynolds 1993, *Amer. Math. Monthly*; Ratcliffe, *Foundations of Hyperbolic Manifolds* | Medium |
| B4 | `Angular` zero-vector hardening | `Cosine` | `Angular` already admitted; finish retiring raw `Cosine` (zero-vector NaN + positive-multiple collapse) from discovery after a compatibility window. | geodesic metric on `Sⁿ⁻¹` | High |
| B5 | Hassanat fix | `Hassanat` | Fix the negative branch to the published `1−1/(1+max−min)` (uses `min+|min|`, not `2·min`); per-term bounded in `[0,1)`; add triangle tests. | Hassanat 2014, *arXiv:1501.00687* | Low |
| B6 | AIRM fix | `RiemannianDistance` | **DONE (admitted, restricted metric).** Both operands now built as the same scale-normalized regularized Laplacian `L/s + I` (SPD); equal-size/non-empty guards added; law gated on the base metric. Symmetry & identity now hold (verified across 4000 random triples, 0 violations). Tests: `tests/distance_tests/riemannian_test.cpp`. | Pennec, Fillard & Ayache 2006, *IJCV* 66(1) | ✅ Done |
| B7 | SSIM-derived metric | `SSIM` | Admit `√(1−SSIM)` (or DSSIM under normalized conditions) only with the exact constants, window, and image domain fixed and proven. | Brunet, Vrscay & Wang 2012, *IEEE TIP* 21(4) | Low |

---

## C. Still missing (net-new from literature)

Prioritized by how much new record-type coverage they add.

### High priority

| Metric | Record type | Admission criteria | Reference |
| --- | --- | --- | --- |
| **Canberra** | nonnegative vectors / counts | `Σ|aᵢ−bᵢ|/(|aᵢ|+|bᵢ|)` with the `0/0:=0` convention; it **is** a metric (unlike Bray–Curtis). Triangle test on a small grid. | Lance & Williams 1967; Deza & Deza 2016 |
| **Discrete Fréchet** | polygonal curves / ordered point sequences | coupling-cost recurrence; metric under a metric ground distance; guard nonempty curves. | Eiter & Mannila 1994 |
| **Damerau–Levenshtein (unrestricted)** | strings with transposition | the *unrestricted* DL is a metric; the OSA/"restricted" variant is **not** (pin its triangle counterexample) — admit only the unrestricted formula. | Damerau 1964; Boytsov 2011 |
| **Wasserstein-p (`p>1`)** | equal-mass measures | generalize `Wasserstein` to `Wₚ = (min Σ πᵢⱼ C^p)^{1/p}`; metric for `p≥1` under a metric ground cost. | Villani 2009 |

### Medium priority

| Metric | Record type | Admission criteria | Reference |
| --- | --- | --- | --- |
| **Continuous Fréchet** | continuous curves | free-space-diagram decision + parametric search; metric. Heavier; document complexity `O(mn log mn)`. | Alt & Godau 1995 |
| **Lévy–Prokhorov** | probability measures on a metric space | metrizes weak convergence; metric. Needs a ground metric and support model. | Prokhorov 1956 |
| **Fisher–Rao / Bhattacharyya angle** | probability vectors | `arccos(Σ√(pᵢqᵢ))`; geodesic metric on the statistical simplex (the angular form, not the Bhattacharyya *coefficient/divergence*). | Rao 1945; Deza & Deza 2016 |
| **Energy distance** | samples / distributions | the (square-root) energy distance is a metric for `0<α<2`; relate to `EmpiricalCramer` in 1-D. | Székely & Rizzo 2013 |
| **Maximum Mean Discrepancy (MMD)** | distributions via a characteristic kernel | a metric on probability measures iff the kernel is characteristic; admit with a fixed kernel. | Gretton et al. 2012, *JMLR* |
| **Aitchison** | compositional data (simplex) | Euclidean distance of centered log-ratios (clr); metric on the simplex; guard strictly positive parts. | Aitchison 1986 |
| **Haversine / great-circle** | geographic lat-lon records | spherical geodesic distance; metric on the sphere; guard valid lat-lon. | standard geodesy |

### Lower priority / specialized

| Metric | Record type | Admission criteria | Reference |
| --- | --- | --- | --- |
| **Ulam** | permutations | min element moves; metric. | Deza & Deza 2016 |
| **Cayley / Kendall-tau** | permutations / rankings | transposition / adjacent-swap counts; metrics on the symmetric group. | Diaconis 1988 |
| **Tree edit distance** | ordered/unordered trees | Zhang–Shasha recurrence; metric with unit costs. | Zhang & Shasha 1989 |
| **Graph edit distance** | attributed graphs | metric under symmetric non-negative edit costs; NP-hard exact — document approximation status. | Sanfeliu & Fu 1983 |
| **Normalized Compression Distance** | arbitrary byte records | quasi-metric in theory; only admit with documented compressor and bounds (often only approximately metric). | Li et al. 2004 |
| **Gromov–Wasserstein** | metric-measure spaces | a metric on isomorphism classes of mm-spaces; heavy; document as advanced. | Mémoli 2011 |

### Explicitly NOT to admit as metrics (record as rejected if added)

- **KL divergence**, raw **Jensen–Shannon divergence**, raw **χ²**, **Bregman divergences** — asymmetric / no triangle. (`SqrtJensenShannon` is the JS metric.)
- **DTW**, **LCSS** defaults — collapse distinct sequences / no triangle. (`TWED`, `ERP`, Fréchet are the metric alternatives.)
- **OSA / restricted Damerau–Levenshtein** — known triangle counterexample; admit only *unrestricted* DL.
- **Squared** Mahalanobis / Hellinger² / squared Cramér–von-Mises — squaring loses the triangle inequality; expose only the root forms.
- **Bray–Curtis (`Sorensen`)**, raw **cosine complement (`CosineInverted`)** — already rejected; metric variants are `Ruzicka` and `Angular`.
