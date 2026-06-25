// Metric Discovery law verification on small finite domains.
//
// This smoke test is the executable backbone of docs/metrics/discovery-table.md.
// It checks three things that production Metric Discovery depends on:
//
//   1. The in-code admission registry (metric/metric/discovery.hpp) assigns the
//      expected four-status admission to every catalog class, and the status is
//      consistent with the declared metric_traits law (admitted/restricted <=>
//      law == metric; quarantine/rejected <=> law != metric).
//
//   2. The salvageable metric *variants* that currently sit in quarantine really
//      are true metrics on a small finite domain: the truncated/clipped
//      Euclidean transforms, the hyperboloid-model hyperbolic distance behind
//      `Weierstrass`, and the angular distance behind `Cosine`. This is the
//      evidence behind "do not delete a non-metric by family name -- a true
//      metric variant exists".
//
//   3. The genuine non-metrics are non-metrics, pinned with a concrete witness:
//      Bray-Curtis/Sorensen and the raw 1-cosine complement both violate the
//      triangle inequality, and raw `Cosine` collapses distinct positive
//      multiples (pseudometric) and is NaN at the zero vector.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <vector>

// RiemannianDistance is no longer part of the header-only public catalog.hpp
// umbrella (it needs a LAPACK eigensolver). Include it BEFORE discovery.hpp so
// the macro-guarded admission registration is active in this translation unit
// (the same include-before-registry pattern as quarantined opt-in metrics).
#include <metric/metric/catalog/space/Riemannian.hpp>

#include <metric/metric/discovery.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using mtrc::metric::admission_status;
using mtrc::metric::is_admitted_metric_v;
using mtrc::metric::is_admitted_v;
using mtrc::metric::is_discoverable_metric_v;
using mtrc::metric::is_quarantined_status_v;
using mtrc::metric::is_rejected_v;
using mtrc::metric::is_restricted_metric_v;

using Vec = std::vector<double>;

namespace {

// ---------------------------------------------------------------------------
// Generic four-law scanner over a small finite domain.
// ---------------------------------------------------------------------------

struct law_scan {
	bool self_zero = true; // d(a, a) ~ 0
	bool nonneg = true;	   // d(a, b) >= -eps
	bool finite = true;	   // every value is finite
	bool symmetry = true;  // |d(a, b) - d(b, a)| <= eps
	bool identity = true;  // d(a, b) <= eps  =>  a == b
	bool triangle = true;  // d(a, c) <= d(a, b) + d(b, c) + eps

	auto is_metric() const -> bool { return self_zero && nonneg && finite && symmetry && identity && triangle; }
};

template <typename Record, typename Metric>
auto scan_laws(const std::vector<Record> &x, Metric metric, double eps = 1e-9) -> law_scan
{
	law_scan s;
	for (std::size_t i = 0; i < x.size(); ++i) {
		const double dii = static_cast<double>(metric(x[i], x[i]));
		if (!std::isfinite(dii)) {
			s.finite = false;
		}
		if (std::abs(dii) > eps) {
			s.self_zero = false;
		}
		for (std::size_t j = 0; j < x.size(); ++j) {
			const double dij = static_cast<double>(metric(x[i], x[j]));
			const double dji = static_cast<double>(metric(x[j], x[i]));
			if (!std::isfinite(dij) || !std::isfinite(dji)) {
				s.finite = false;
				continue;
			}
			if (dij < -eps) {
				s.nonneg = false;
			}
			if (std::abs(dij - dji) > eps) {
				s.symmetry = false;
			}
			if (dij <= eps && !(x[i] == x[j])) {
				s.identity = false;
			}
			for (std::size_t k = 0; k < x.size(); ++k) {
				const double dik = static_cast<double>(metric(x[i], x[k]));
				const double dkj = static_cast<double>(metric(x[k], x[j]));
				if (std::isfinite(dik) && std::isfinite(dkj) && dij > dik + dkj + eps) {
					s.triangle = false;
				}
			}
		}
	}
	return s;
}

// True iff some triple in the domain witnesses a triangle-inequality violation.
template <typename Record, typename Metric>
auto has_triangle_violation(const std::vector<Record> &x, Metric metric, double margin = 1e-9) -> bool
{
	for (std::size_t i = 0; i < x.size(); ++i) {
		for (std::size_t j = 0; j < x.size(); ++j) {
			const double dij = static_cast<double>(metric(x[i], x[j]));
			if (!std::isfinite(dij)) {
				continue;
			}
			for (std::size_t k = 0; k < x.size(); ++k) {
				const double dik = static_cast<double>(metric(x[i], x[k]));
				const double dkj = static_cast<double>(metric(x[k], x[j]));
				if (std::isfinite(dik) && std::isfinite(dkj) && dij > dik + dkj + margin) {
					return true;
				}
			}
		}
	}
	return false;
}

// ---------------------------------------------------------------------------
// Admissible "variant" reference implementations: exactly the guarded forms
// admission would add (clamp the transcendental argument, fix the domain).
// ---------------------------------------------------------------------------

// Angular distance on directions: acos(clamp(cos)) / pi. Metric on unit vectors.
auto angular(const Vec &a, const Vec &b) -> double
{
	double dot = 0, na = 0, nb = 0;
	for (std::size_t i = 0; i < a.size(); ++i) {
		dot += a[i] * b[i];
		na += a[i] * a[i];
		nb += b[i] * b[i];
	}
	double c = dot / (std::sqrt(na) * std::sqrt(nb));
	c = std::max(-1.0, std::min(1.0, c));
	return std::acos(c) / M_PI;
}

// Hyperboloid-model hyperbolic distance: acosh(max(1, B)). Metric on all R^n.
auto hyperbolic(const Vec &a, const Vec &b) -> double
{
	double da = 0, db = 0, dab = 0;
	for (std::size_t i = 0; i < a.size(); ++i) {
		da += a[i] * a[i];
		db += b[i] * b[i];
		dab += a[i] * b[i];
	}
	const double arg = std::sqrt(1.0 + da) * std::sqrt(1.0 + db) - dab;
	return std::acosh(std::max(1.0, arg));
}

} // namespace

// ===========================================================================
// 1. Admission registry and law/status consistency.
// ===========================================================================

// admitted: unconditional true metrics.
static_assert(is_admitted_v<mtrc::Euclidean<double>>);
static_assert(is_admitted_v<mtrc::Manhattan<double>>);
static_assert(is_admitted_v<mtrc::Chebyshev<double>>);
static_assert(is_admitted_v<mtrc::Edit<char>>);

// restricted_metric: true metrics under an enforced parameter/domain gate.
static_assert(is_restricted_metric_v<mtrc::P_norm<double>>);
static_assert(is_restricted_metric_v<mtrc::Ruzicka<double>>);
static_assert(is_restricted_metric_v<mtrc::TWED<double>>);
static_assert(is_restricted_metric_v<mtrc::Wasserstein<double>>);
static_assert(is_restricted_metric_v<mtrc::Euclidean_standardized<double>>);
static_assert(is_restricted_metric_v<mtrc::Manhattan_standardized<double>>);
static_assert(is_restricted_metric_v<mtrc::WeightedMinkowski<double>>);
static_assert(is_restricted_metric_v<mtrc::Mahalanobis<double>>);
static_assert(is_restricted_metric_v<mtrc::Angular<double>>);
static_assert(is_restricted_metric_v<mtrc::Chordal<double>>);
static_assert(is_restricted_metric_v<mtrc::DiscreteMetric<double>>);
static_assert(is_restricted_metric_v<mtrc::Hamming>);
static_assert(is_restricted_metric_v<mtrc::Jaccard>);
static_assert(is_restricted_metric_v<mtrc::BinaryJaccard>);
static_assert(is_restricted_metric_v<mtrc::Tanimoto<double>>);
static_assert(is_restricted_metric_v<mtrc::Hausdorff<>>);
static_assert(is_restricted_metric_v<mtrc::ERP<double>>);
static_assert(is_restricted_metric_v<mtrc::EmpiricalCramer<double>>);
static_assert(is_restricted_metric_v<mtrc::EmpiricalKolmogorovSmirnov<double>>);
static_assert(is_restricted_metric_v<mtrc::TotalVariation<double>>);
static_assert(is_restricted_metric_v<mtrc::Hellinger<double>>);
static_assert(is_restricted_metric_v<mtrc::SqrtJensenShannon<double>>);

// quarantine: non-metric as shipped, metric variant exists in the family.
static_assert(is_quarantined_status_v<mtrc::Cosine<double>>);
static_assert(is_quarantined_status_v<mtrc::Weierstrass<double>>);
static_assert(is_quarantined_status_v<mtrc::Euclidean_thresholded<double>>);
static_assert(is_quarantined_status_v<mtrc::Euclidean_hard_clipped<double>>);
static_assert(is_quarantined_status_v<mtrc::Euclidean_soft_clipped<double>>);
static_assert(is_quarantined_status_v<mtrc::Hassanat<double>>);
static_assert(is_quarantined_status_v<mtrc::EMD<double>>);
static_assert(is_quarantined_status_v<mtrc::RandomEMD<std::vector<double>, double>>);
static_assert(is_quarantined_status_v<mtrc::CramervonMises<std::vector<double>, double>>);
static_assert(is_quarantined_status_v<mtrc::KolmogorovSmirnov<std::vector<double>, double>>);

// RiemannianDistance (AIRM) was promoted from quarantine to a restricted metric
// after the operand-encoding defect was fixed (symmetry + identity now hold);
// see tests/core_smoke/metric_riemannian_law_smoke.cpp for the runtime
// metric-law coverage in the active gate.
static_assert(is_restricted_metric_v<mtrc::RiemannianDistance<std::vector<double>>>);

// rejected: non-metric, metric variant is a separate already-present class.
static_assert(is_rejected_v<mtrc::CosineInverted<double>>);
static_assert(is_rejected_v<mtrc::Sorensen<double>>);

// Status <-> law consistency: discoverable-as-metric (admitted || restricted)
// must agree with the declared metric_traits law for every registered class.
template <typename M> constexpr bool consistent_v = (is_discoverable_metric_v<M> == is_admitted_metric_v<M>);
static_assert(consistent_v<mtrc::Euclidean<double>>);
static_assert(consistent_v<mtrc::Manhattan<double>>);
static_assert(consistent_v<mtrc::Chebyshev<double>>);
static_assert(consistent_v<mtrc::Edit<char>>);
static_assert(consistent_v<mtrc::P_norm<double>>);
static_assert(consistent_v<mtrc::Ruzicka<double>>);
static_assert(consistent_v<mtrc::TWED<double>>);
static_assert(consistent_v<mtrc::Wasserstein<double>>);
static_assert(consistent_v<mtrc::Euclidean_standardized<double>>);
static_assert(consistent_v<mtrc::Manhattan_standardized<double>>);
static_assert(consistent_v<mtrc::Cosine<double>>);
static_assert(consistent_v<mtrc::Weierstrass<double>>);
static_assert(consistent_v<mtrc::Euclidean_thresholded<double>>);
static_assert(consistent_v<mtrc::Euclidean_hard_clipped<double>>);
static_assert(consistent_v<mtrc::Euclidean_soft_clipped<double>>);
static_assert(consistent_v<mtrc::Hassanat<double>>);
static_assert(consistent_v<mtrc::EMD<double>>);
static_assert(consistent_v<mtrc::RandomEMD<std::vector<double>, double>>);
static_assert(consistent_v<mtrc::CramervonMises<std::vector<double>, double>>);
static_assert(consistent_v<mtrc::KolmogorovSmirnov<std::vector<double>, double>>);
static_assert(consistent_v<mtrc::RiemannianDistance<std::vector<double>>>);
static_assert(consistent_v<mtrc::CosineInverted<double>>);
static_assert(consistent_v<mtrc::Sorensen<double>>);
static_assert(consistent_v<mtrc::WeightedMinkowski<double>>);
static_assert(consistent_v<mtrc::Mahalanobis<double>>);
static_assert(consistent_v<mtrc::Angular<double>>);
static_assert(consistent_v<mtrc::Chordal<double>>);
static_assert(consistent_v<mtrc::DiscreteMetric<double>>);
static_assert(consistent_v<mtrc::Hamming>);
static_assert(consistent_v<mtrc::Jaccard>);
static_assert(consistent_v<mtrc::BinaryJaccard>);
static_assert(consistent_v<mtrc::Tanimoto<double>>);
static_assert(consistent_v<mtrc::Hausdorff<>>);
static_assert(consistent_v<mtrc::ERP<double>>);
static_assert(consistent_v<mtrc::EmpiricalCramer<double>>);
static_assert(consistent_v<mtrc::EmpiricalKolmogorovSmirnov<double>>);
static_assert(consistent_v<mtrc::TotalVariation<double>>);
static_assert(consistent_v<mtrc::Hellinger<double>>);
static_assert(consistent_v<mtrc::SqrtJensenShannon<double>>);

int main()
{
	// =======================================================================
	// 2. Admitted metrics satisfy all four laws on a small finite domain.
	// =======================================================================
	const std::vector<Vec> vectors{{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}, {3.0, 4.0}, {2.0, 3.0}};
	assert(scan_laws(vectors, mtrc::Euclidean<double>{}).is_metric());
	assert(scan_laws(vectors, mtrc::Manhattan<double>{}).is_metric());
	assert(scan_laws(vectors, mtrc::Chebyshev<double>{}).is_metric());
	assert(scan_laws(vectors, mtrc::P_norm<double>{2.5}).is_metric());

	const std::vector<std::vector<double>> nonneg{{0.0, 0.0}, {1.0, 0.0}, {1.0, 2.0}, {3.0, 1.0}, {2.0, 2.0}};
	assert(scan_laws(nonneg, mtrc::Ruzicka<double>{}).is_metric()); // Soergel / weighted Jaccard

	// =======================================================================
	// 3. Salvageable metric VARIANTS that currently sit in quarantine are true
	//    metrics on a small finite domain -- the "do not delete" evidence.
	// =======================================================================

	// 3a. Truncated / clipped Euclidean: concave transforms of L2 stay metrics.
	//     Domains chosen so the cap/saturation is ACTIVE on some pairs.
	const std::vector<Vec> spread{{0.0, 0.0}, {3.0, 0.0}, {0.0, 3.0}, {3.0, 3.0}, {1.0, 1.0}, {0.0, 1.0}};
	assert(scan_laws(spread, mtrc::Euclidean_hard_clipped<double>{2.0, 1.0}).is_metric());
	assert(scan_laws(spread, mtrc::Euclidean_thresholded<double>{2.0, 1.0}).is_metric());
	// soft clip: max_distance=2, scal=1, thresh=0.5 -> saturation knee at d=1.
	const std::vector<Vec> soft_domain{{0.0, 0.0}, {0.5, 0.0}, {2.0, 0.0}, {0.0, 2.0}, {2.0, 2.0}, {1.0, 1.0}};
	assert(scan_laws(soft_domain, mtrc::Euclidean_soft_clipped<double>{2.0, 1.0, 0.5}).is_metric());

	// Cap value sanity: min(2, 1 * ||(3,4)||) = min(2, 5) = 2.
	assert(std::abs(mtrc::Euclidean_hard_clipped<double>{2.0, 1.0}(Vec{0.0, 0.0}, Vec{3.0, 4.0}) - 2.0) < 1e-12);
	assert(std::abs(mtrc::Euclidean_thresholded<double>{2.0, 1.0}(Vec{0.0, 0.0}, Vec{3.0, 4.0}) - 2.0) < 1e-12);

	// 3b. Hyperboloid-model hyperbolic distance (the math behind Weierstrass).
	const std::vector<Vec> hyp_domain{{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}, {2.0, 0.0}, {-1.0, 1.0}};
	// acosh(1 + delta) ~ sqrt(2 delta) amplifies the ~1e-15 rounding in the
	// self-distance argument to ~1e-7, so the diagonal needs a looser tolerance.
	assert(scan_laws(hyp_domain, &hyperbolic, 1e-6).is_metric());
	// Shipped Weierstrass matches the hyperbolic formula where acosh is in
	// domain: B((1,0),(0,1)) = sqrt2 * sqrt2 - 0 = 2, acosh(2) = 1.31695789...
	assert(std::abs(mtrc::Weierstrass<double>{}(Vec{1.0, 0.0}, Vec{0.0, 1.0}) - std::acosh(2.0)) < 1e-9);

	// 3c. Angular distance (the math behind Cosine) is a metric on unit vectors.
	std::vector<Vec> unit;
	for (const double deg : {0.0, 30.0, 60.0, 90.0, 135.0, 180.0}) {
		const double r = deg * M_PI / 180.0;
		unit.push_back(Vec{std::cos(r), std::sin(r)});
	}
	assert(scan_laws(unit, &angular).is_metric());
	// Shipped Cosine equals angular on orthogonal unit vectors: acos(0)/pi = 0.5.
	assert(std::abs(mtrc::Cosine<double>{}(Vec{1.0, 0.0}, Vec{0.0, 1.0}) - 0.5) < 1e-12);

	// =======================================================================
	// 4. Genuine non-metrics: pin a concrete law violation.
	// =======================================================================

	// 4a. Bray-Curtis / Sorensen violates the triangle inequality.
	//     A=(0,1), B=(1,0), C=(1,1): d(A,B)=1 > d(A,C)+d(C,B)=1/3+1/3.
	mtrc::Sorensen<double> sorensen;
	const double s_ab = sorensen(Vec{0.0, 1.0}, Vec{1.0, 0.0});
	const double s_ac = sorensen(Vec{0.0, 1.0}, Vec{1.0, 1.0});
	const double s_cb = sorensen(Vec{1.0, 1.0}, Vec{1.0, 0.0});
	assert(std::abs(s_ab - 1.0) < 1e-12);
	assert(std::abs(s_ac - (1.0 / 3.0)) < 1e-12);
	assert(std::abs(s_cb - (1.0 / 3.0)) < 1e-12);
	assert(s_ab > s_ac + s_cb + 1e-9); // triangle broken
	const std::vector<Vec> bc_grid{{0.0, 1.0}, {1.0, 0.0}, {1.0, 1.0}, {2.0, 1.0}, {1.0, 2.0}};
	assert(has_triangle_violation(bc_grid, mtrc::Sorensen<double>{}));

	// 4b. Raw 1-cosine complement (CosineInverted) violates the triangle
	//     inequality: u0=(1,0), u1=(1,2), u2=(-1,2).
	mtrc::CosineInverted<double> cos_inv;
	const double c_02 = cos_inv(Vec{1.0, 0.0}, Vec{-1.0, 2.0});
	const double c_01 = cos_inv(Vec{1.0, 0.0}, Vec{1.0, 2.0});
	const double c_12 = cos_inv(Vec{1.0, 2.0}, Vec{-1.0, 2.0});
	assert(c_02 > c_01 + c_12 + 1e-9); // triangle broken
	const std::vector<Vec> ci_grid{{1.0, 0.0}, {1.0, 2.0}, {-1.0, 2.0}, {2.0, 1.0}, {0.0, 1.0}};
	assert(has_triangle_violation(ci_grid, mtrc::CosineInverted<double>{}));

	// 4c. Raw Cosine is only a pseudometric on raw vectors (distinct positive
	//     multiples collapse to 0) and is NaN at the zero vector -- the two
	//     defects that keep it in quarantine pending an Angular metric.
	const double collapse = mtrc::Cosine<double>{}(Vec{1.0, 0.0}, Vec{2.0, 0.0});
	assert(std::isfinite(collapse) && collapse < 1e-9); // distinct records, zero distance
	const double zero_vec = mtrc::Cosine<double>{}(Vec{0.0, 0.0}, Vec{1.0, 0.0});
	assert(std::isnan(zero_vec)); // zero-vector division by zero

	return 0;
}
