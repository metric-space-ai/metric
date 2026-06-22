// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_METRIC_DISCOVERY_HPP
#define _METRIC_METRIC_DISCOVERY_HPP

// Metric Discovery registry.
//
// This header is the single, auditable source of truth that maps every public
// METRIC distance class to its Metric Discovery admission status
// (see metric/metric/admission.hpp for the four-status scheme and
// docs/metrics/discovery-table.md for the human-facing catalog).
//
// Including this header pulls in the metric catalog (catalog.hpp) and registers
// the 24 catalog classes. The two physically-quarantined classes that live
// under metric/metric/quarantine/ (SSIM, Kohonen) drag in heavy mapping/solver
// dependencies, so they are registered only when their headers have already
// been included (guarded on their include-guard macros below). When they are
// not included they keep the safe default status (quarantine) from the primary
// metric_admission template.
//
// Invariants enforced by tests/core_smoke/metric_discovery_smoke.cpp:
//   * admitted / restricted_metric  <=>  metric_traits law == metric
//   * quarantine / rejected         <=>  metric_traits law != metric
// i.e. only admitted and restricted metrics are discoverable for metric-only
// algorithms (cover-tree indexing, triangle-pruning search, ...).

#include <metric/metric/admission.hpp>
#include <metric/metric/catalog.hpp>

namespace mtrc::metric {

// ---------------------------------------------------------------------------
// admitted: true metric on the natural record domain, no parameter or per-fit
// precondition can break the law (only structural alignment is required).
// ---------------------------------------------------------------------------

template <typename V> struct metric_admission<::mtrc::Euclidean<V>> {
	static constexpr admission_status status = admission_status::admitted;
};
template <typename V> struct metric_admission<::mtrc::Manhattan<V>> {
	static constexpr admission_status status = admission_status::admitted;
};
template <typename V> struct metric_admission<::mtrc::Chebyshev<V>> {
	static constexpr admission_status status = admission_status::admitted;
};
template <typename V> struct metric_admission<::mtrc::Edit<V>> {
	static constexpr admission_status status = admission_status::admitted;
};

// ---------------------------------------------------------------------------
// restricted_metric: true metric, but only under an enforced parameter/domain
// gate. The gate is validated on construction and on every evaluation, so these
// remain routable by metric-only algorithms.
// ---------------------------------------------------------------------------

template <typename V> struct metric_admission<::mtrc::P_norm<V>> {
	static constexpr admission_status status = admission_status::restricted_metric; // Minkowski, finite p >= 1
};
template <typename V> struct metric_admission<::mtrc::Ruzicka<V>> {
	static constexpr admission_status status = admission_status::restricted_metric; // Soergel / weighted Jaccard, nonnegative
};
template <typename V> struct metric_admission<::mtrc::TWED<V>> {
	static constexpr admission_status status = admission_status::restricted_metric; // penalty >= 0, elastic > 0
};
template <typename V> struct metric_admission<::mtrc::Wasserstein<V>> {
	static constexpr admission_status status = admission_status::restricted_metric; // metric ground cost, equal mass
};
template <typename V> struct metric_admission<::mtrc::Euclidean_standardized<V>> {
	static constexpr admission_status status = admission_status::restricted_metric; // every fitted sigma finite, > 0
};
template <typename V> struct metric_admission<::mtrc::Manhattan_standardized<V>> {
	static constexpr admission_status status = admission_status::restricted_metric; // every fitted sigma finite, > 0
};

// Vector-family true metrics admitted under a domain/parameter gate.
template <typename V> struct metric_admission<::mtrc::WeightedMinkowski<V>> {
	static constexpr admission_status status = admission_status::restricted_metric; // positive weights, p >= 1
};
template <typename V> struct metric_admission<::mtrc::Mahalanobis<V>> {
	static constexpr admission_status status = admission_status::restricted_metric; // symmetric positive definite
};
template <typename V> struct metric_admission<::mtrc::Angular<V>> {
	static constexpr admission_status status = admission_status::restricted_metric; // nonzero / unit vectors
};
template <typename V> struct metric_admission<::mtrc::Chordal<V>> {
	static constexpr admission_status status = admission_status::restricted_metric; // nonzero / unit vectors
};

// Categorical / set true metrics admitted under a domain gate.
template <typename V> struct metric_admission<::mtrc::DiscreteMetric<V>> {
	static constexpr admission_status status = admission_status::restricted_metric; // positive constant
};
template <> struct metric_admission<::mtrc::Hamming> {
	static constexpr admission_status status = admission_status::restricted_metric; // equal-length records
};
template <> struct metric_admission<::mtrc::Jaccard> {
	static constexpr admission_status status = admission_status::restricted_metric; // sets, fixed empty-set rule
};
template <> struct metric_admission<::mtrc::BinaryJaccard> {
	static constexpr admission_status status = admission_status::restricted_metric; // binary records
};
template <typename V> struct metric_admission<::mtrc::Tanimoto<V>> {
	static constexpr admission_status status = admission_status::restricted_metric; // nonnegative weighted sets
};

// Structured / set-geometry and time-series true metrics under a gate.
template <typename GroundMetric> struct metric_admission<::mtrc::Hausdorff<GroundMetric>> {
	static constexpr admission_status status = admission_status::restricted_metric; // nonempty sets, metric ground
};
template <typename V> struct metric_admission<::mtrc::ERP<V>> {
	static constexpr admission_status status = admission_status::restricted_metric; // fixed gap element, ground metric
};

// Distribution true metrics (admitted, tie-safe fixed-support variants) under a
// normalized-input / common-support gate. These are the admitted replacements
// for the quarantined Akima-CDF distribution distances below.
template <typename V> struct metric_admission<::mtrc::EmpiricalCramer<V>> {
	static constexpr admission_status status = admission_status::restricted_metric; // root L2-CDF (Cramer) distance
};
template <typename V> struct metric_admission<::mtrc::EmpiricalKolmogorovSmirnov<V>> {
	static constexpr admission_status status = admission_status::restricted_metric; // sup-CDF (Linf) distance
};
template <typename V> struct metric_admission<::mtrc::TotalVariation<V>> {
	static constexpr admission_status status = admission_status::restricted_metric; // normalized measures
};
template <typename V> struct metric_admission<::mtrc::Hellinger<V>> {
	static constexpr admission_status status = admission_status::restricted_metric; // normalized measures
};
template <typename V> struct metric_admission<::mtrc::SqrtJensenShannon<V>> {
	static constexpr admission_status status = admission_status::restricted_metric; // normalized measures (sqrt of JS)
};

// ---------------------------------------------------------------------------
// quarantine: not a true metric as shipped, but a metric variant exists in the
// same literature family (or the class has a legitimate non-metric home). Kept
// for a compatibility window pending admission/relocation; never deleted by
// family name alone. See docs/metrics/missing-metrics-backlog.md for the
// salvage path of each.
// ---------------------------------------------------------------------------

// acos(cos)/pi is the angular (great-circle) metric on unit vectors/directions,
// but a pseudometric on raw vectors and NaN at the zero vector -> needs an
// Angular metric with a nonzero/unit-vector domain guard.
template <typename V> struct metric_admission<::mtrc::Cosine<V>> {
	static constexpr admission_status status = admission_status::quarantine;
};
// Hyperboloid-model hyperbolic distance; a true metric on R^n but lacks the
// acosh-domain clamp guard, a metric_traits law, and admission tests.
template <typename V> struct metric_admission<::mtrc::Weierstrass<V>> {
	static constexpr admission_status status = admission_status::quarantine;
};
// min(thres, factor * L2): truncated metric (concave transform of L2) for
// positive parameters -> needs positive-parameter guards before admission.
template <typename V> struct metric_admission<::mtrc::Euclidean_thresholded<V>> {
	static constexpr admission_status status = admission_status::quarantine;
};
template <typename V> struct metric_admission<::mtrc::Euclidean_hard_clipped<V>> {
	static constexpr admission_status status = admission_status::quarantine;
};
template <typename V> struct metric_admission<::mtrc::Euclidean_soft_clipped<V>> {
	static constexpr admission_status status = admission_status::quarantine;
};
// Published Hassanat distance is a metric, but the shipped negative branch
// deviates from the published formula (uses 2*min instead of min+|min|).
template <typename V> struct metric_admission<::mtrc::Hassanat<V>> {
	static constexpr admission_status status = admission_status::quarantine;
};
// Permissive transport: arbitrary ground cost + unbalanced mass + lazy mutation.
// The admitted strict variant is Wasserstein.
template <typename V> struct metric_admission<::mtrc::EMD<V>> {
	static constexpr admission_status status = admission_status::quarantine;
};

template <typename Sample, typename D> struct metric_admission<::mtrc::RandomEMD<Sample, D>> {
	static constexpr admission_status status = admission_status::quarantine;
};
template <typename Sample, typename D> struct metric_admission<::mtrc::CramervonMises<Sample, D>> {
	static constexpr admission_status status = admission_status::quarantine; // root-CDF form ok, Akima CDF not
};
template <typename Sample, typename D> struct metric_admission<::mtrc::KolmogorovSmirnov<Sample, D>> {
	static constexpr admission_status status = admission_status::quarantine; // sup-CDF metric, Akima grid not
};

// ---------------------------------------------------------------------------
// rejected: not a metric, and the family's metric variant is a separate,
// already-present class, so this specific computation is scheduled for removal.
// ---------------------------------------------------------------------------

// |1 - cos|: raw cosine complement. Metric variant -> Cosine/Angular.
template <typename V> struct metric_admission<::mtrc::CosineInverted<V>> {
	static constexpr admission_status status = admission_status::rejected;
};
// Bray-Curtis / Sorensen: non-metric. Metric variant -> Ruzicka/Soergel.
template <typename V> struct metric_admission<::mtrc::Sorensen<V>> {
	static constexpr admission_status status = admission_status::rejected;
};

// ---------------------------------------------------------------------------
// Optional registration of heavy / external-backend classes. These are only
// declared when their headers have already been included by the translation
// unit, so this registry header (and the public catalog.hpp umbrella) stays
// free of the mapping/SOM/solver/LAPACK dependencies those classes pull in.
// ---------------------------------------------------------------------------

// conditionally restricted (law inherited from the base metric):
// Affine-invariant Riemannian metric (AIRM) on the SPD images of two equal-size
// finite metric spaces. The operand-encoding defect that broke symmetry and
// identity of indiscernibles is fixed in Riemannian.cpp; it is a restricted
// metric whenever the base Metric is itself a true metric (the law is inherited,
// as for Hausdorff). Registered only when Riemannian.hpp is included, because it
// requires a LAPACK generalized eigensolver and is therefore not part of the
// header-only public catalog by default (see catalog.hpp).
#if defined(METRIC_METRIC_CATALOG_SPACE_RIEMANNIAN_HPP)
template <typename RecType, typename Metric> struct metric_admission<::mtrc::RiemannianDistance<RecType, Metric>> {
	static constexpr admission_status status =
		core::metric_traits<::mtrc::RiemannianDistance<RecType, Metric>>::law == core::metric_law::metric
			? admission_status::restricted_metric
			: admission_status::quarantine;
};
#endif

#if defined(_METRIC_DISTANCE_K_STRUCTURED_SSIM_HPP)
// Raw SSIM / 1-SSIM are not metrics; SSIM-derived metric variants exist in the
// literature (Brunet/Vrscay/Wang 2012).
template <typename D, typename V> struct metric_admission<::mtrc::SSIM<D, V>> {
	static constexpr admission_status status = admission_status::quarantine;
};
#endif

#if defined(_METRIC_DISTANCE_K_STRUCTURED_KOHONEN_HPP)
// Learned, training-dependent SOM graph approximation -> relocate to mapping.
template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
struct metric_admission<::mtrc::Kohonen<D, Sample, Graph, Metric, Distribution>> {
	static constexpr admission_status status = admission_status::quarantine;
};
#endif

} // namespace mtrc::metric

#endif
