#ifndef PANDA_METRIC_RIEMANNIAN_HPP
#define PANDA_METRIC_RIEMANNIAN_HPP

#include "metric/distance/k-related/Standards.hpp"

namespace metric {

template <typename RecType, typename Metric = Euclidean<typename RecType::value_type>> class RiemannianDistance {
  public:
	RiemannianDistance(Metric metric = Metric()) : metric(metric) {}

	template <typename C> double operator()(const C &Xc, const C &Yc) const;

	template <typename T> T matDistance(blaze::DynamicMatrix<T> A, blaze::DynamicMatrix<T> B) const;

	template <typename Container>
	double estimate(const Container &a, const Container &b, const size_t sampleSize = 250,
					const double threshold = 0.05, size_t maxIterations = 1000) const;

  private:
	Metric metric;
};

} // namespace metric

#include "Riemannian.cpp"
#endif // PANDA_METRIC_RIEMANNIAN_HPP
