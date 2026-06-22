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

#include "Riemannian.cpp"
#endif // METRIC_METRIC_CATALOG_SPACE_RIEMANNIAN_HPP
