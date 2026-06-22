#ifndef METRIC_METRIC_CATALOG_SPACE_RIEMANNIAN_HPP
#define METRIC_METRIC_CATALOG_SPACE_RIEMANNIAN_HPP

#include "metric/metric/catalog/vector/Standards.hpp"

namespace mtrc {

template <typename RecType, typename Metric = Euclidean<typename RecType::value_type>> class RiemannianDistance {
  public:
	RiemannianDistance(Metric metric = Metric()) : metric(metric) {}

	template <typename C> double operator()(const C &Xc, const C &Yc) const;

	template <typename T> T matDistance(mtrc::numeric::DynamicMatrix<T> A, mtrc::numeric::DynamicMatrix<T> B) const;

	template <typename Container>
	double estimate(const Container &a, const Container &b, const size_t sampleSize = 250,
					const double threshold = 0.05, size_t maxIterations = 1000) const;

  private:
	Metric metric;
};

} // namespace mtrc

namespace mtrc::core {

// Affine-invariant Riemannian metric (AIRM) on the SPD images of two equal-size
// finite metric spaces (Pennec, Fillard & Ayache 2006). It is a true metric when
// the base Metric used to build each space's Laplacian is itself a true metric;
// the law is inherited from the base exactly as for Hausdorff. The record domain
// is an ordered, equal-cardinality finite metric space (its distance matrix); the
// distance is invariant to the global metric scale and to isometry of the
// configuration.
template <typename RecType, typename Metric> struct metric_traits<::mtrc::RiemannianDistance<RecType, Metric>> {
	static constexpr auto law =
		metric_traits<Metric>::law == metric_law::metric ? metric_law::metric : metric_law::distance;
	static constexpr auto records = record_kind::structured;
	static constexpr bool thread_safe = true;
};

} // namespace mtrc::core

#include "Riemannian.cpp"
#endif // METRIC_METRIC_CATALOG_SPACE_RIEMANNIAN_HPP
