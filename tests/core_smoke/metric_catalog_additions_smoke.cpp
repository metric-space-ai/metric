// Smoke coverage for the Metric Catalog production-readiness track.
//
// This file covers newly admitted catalog metrics with small deterministic
// domains: identity, non-negativity, symmetry, triangle inequality, and the
// parameter/domain guards that preserve the metric law.

#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "metric/metric/admission.hpp"
#include "metric/metric/catalog.hpp"

namespace {

constexpr double kEps = 1e-9;

bool close(double a, double b, double eps = kEps) { return std::abs(a - b) <= eps; }

template <typename F> bool throws_invalid_argument(F &&fn, const std::string &needle)
{
	try {
		fn();
	} catch (const std::invalid_argument &error) {
		return needle.empty() || std::string(error.what()).find(needle) != std::string::npos;
	} catch (...) {
		return false;
	}
	return false;
}

template <typename Record, typename Metric>
void assert_metric_contracts_close(const std::vector<Record> &records, const Metric &metric, double eps = kEps)
{
	for (std::size_t i = 0; i < records.size(); ++i) {
		const auto &a = records[i];
		assert(std::abs(static_cast<double>(metric(a, a))) <= eps);
		for (std::size_t j = 0; j < records.size(); ++j) {
			const auto &b = records[j];
			const double ab = static_cast<double>(metric(a, b));
			const double ba = static_cast<double>(metric(b, a));
			assert(ab >= -eps);
			assert(std::abs(ab - ba) <= eps * (1.0 + std::abs(ab)));
			if (ab <= eps) {
				assert(i == j);
			}
			for (const auto &c : records) {
				const double ac = static_cast<double>(metric(a, c));
				const double bc = static_cast<double>(metric(b, c));
				assert(ac <= ab + bc + eps * (1.0 + std::abs(ac) + std::abs(ab) + std::abs(bc)));
			}
		}
	}
}

void check_vector_geometry_metrics()
{
	static_assert(mtrc::metric_traits<mtrc::WeightedMinkowski<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::Mahalanobis<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::Angular<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::Chordal<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric::is_admitted_metric_v<mtrc::WeightedMinkowski<double>>);

	const std::vector<std::vector<double>> records = {
		{0.0, 0.0},
		{1.0, 0.0},
		{1.0, 2.0},
		{3.0, 1.0},
	};
	const mtrc::WeightedMinkowski<double> weighted({1.0, 4.0}, 2.0);
	assert(close(weighted(std::vector<double>{0.0, 0.0}, std::vector<double>{3.0, 4.0}), std::sqrt(73.0)));
	assert_metric_contracts_close(records, weighted);
	assert(mtrc::metric_cache_key(weighted).find("WeightedMinkowski") != std::string::npos);

	assert(throws_invalid_argument([] { (void)mtrc::WeightedMinkowski<double>({1.0, 0.0}, 2.0); }, "positive"));
	assert(throws_invalid_argument([] { (void)mtrc::WeightedMinkowski<double>({1.0}, 0.5); }, "p >= 1"));

	const mtrc::Mahalanobis<double> mahalanobis({{2.0, 0.5}, {0.5, 1.0}});
	assert_metric_contracts_close(records, mahalanobis);
	assert(throws_invalid_argument([] { (void)mtrc::Mahalanobis<double>({{1.0, 2.0}, {2.0, 4.0}}); }, "positive definite"));
	assert(throws_invalid_argument([] { (void)mtrc::Mahalanobis<double>({{0.0, 1.0}, {2.0, 0.0}}); }, "symmetric"));

	const std::vector<std::vector<double>> unit_records = {
		{1.0, 0.0, 0.0},
		{0.0, 1.0, 0.0},
		{-1.0, 0.0, 0.0},
		{0.0, 0.0, 1.0},
	};
	assert(close(mtrc::Angular<double>{}(unit_records[0], unit_records[1]), std::acos(-1.0) / 2.0));
	assert(close(mtrc::Chordal<double>{}(unit_records[0], unit_records[1]), std::sqrt(2.0)));
	assert_metric_contracts_close(unit_records, mtrc::Angular<double>{}, 1e-8);
	assert_metric_contracts_close(unit_records, mtrc::Chordal<double>{});
	assert(throws_invalid_argument(
		[] { (void)mtrc::Angular<double>{}(std::vector<double>{2.0, 0.0}, std::vector<double>{1.0, 0.0}); },
		"unit-normalized"));
}

void check_categorical_and_set_metrics()
{
	static_assert(mtrc::metric_traits<mtrc::DiscreteMetric<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::Hamming>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::Jaccard>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::BinaryJaccard>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::Tanimoto<double>>::law == mtrc::metric_law::metric);

	assert_metric_contracts_close(std::vector<int>{1, 2, 3}, mtrc::DiscreteMetric<double>{2.0});
	assert(mtrc::DiscreteMetric<double>{2.5}(1, 2) == 2.5);
	assert(throws_invalid_argument([] { (void)mtrc::DiscreteMetric<double>{0.0}; }, "positive"));

	const std::vector<std::string> words = {"cat", "cot", "dog", "dig"};
	assert_metric_contracts_close(words, mtrc::Hamming{});
	assert(mtrc::Hamming{}(std::string("cat"), std::string("cot")) == 1);
	assert(throws_invalid_argument([] { (void)mtrc::Hamming{}(std::string("cat"), std::string("cats")); }, "equal size"));

	const std::vector<std::vector<int>> sets = {{}, {1}, {1, 2}, {2, 3}, {1, 2, 3}};
	assert_metric_contracts_close(sets, mtrc::Jaccard{});
	assert(close(mtrc::Jaccard{}(std::vector<int>{1, 2, 3}, std::vector<int>{2, 3, 4}), 0.5));
	assert(close(mtrc::Jaccard{}(std::vector<int>{1, 1, 2}, std::vector<int>{2, 1}), 0.0));

	const std::vector<std::vector<int>> binary = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 1}};
	assert_metric_contracts_close(binary, mtrc::BinaryJaccard{});
	assert(close(mtrc::BinaryJaccard{}(std::vector<int>{1, 1, 0}, std::vector<int>{1, 0, 1}), 2.0 / 3.0));
	assert(throws_invalid_argument(
		[] { (void)mtrc::BinaryJaccard{}(std::vector<int>{1, 2}, std::vector<int>{1, 0}); }, "binary"));

	const std::vector<std::vector<double>> weighted_sets = {
		{0.0, 0.0, 0.0},
		{1.0, 0.0, 0.0},
		{1.0, 2.0, 0.0},
		{3.0, 3.0, 1.0},
	};
	assert_metric_contracts_close(weighted_sets, mtrc::Tanimoto<double>{});
	assert(close(mtrc::Tanimoto<double>{}(std::vector<double>{1.0, 2.0}, std::vector<double>{1.0, 4.0}), 0.4));
}

void check_time_series_and_geometry_metrics()
{
	static_assert(mtrc::metric_traits<mtrc::ERP<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::Hausdorff<>>::law == mtrc::metric_law::metric);

	const std::vector<std::vector<double>> series = {
		{},
		{0.0},
		{1.0},
		{0.0, 1.0},
		{2.0, 1.0},
	};
	const mtrc::ERP<double> erp{-1.0};
	assert_metric_contracts_close(series, erp);
	assert(close(erp(std::vector<double>{0.0}, std::vector<double>{2.0}), 2.0));
	assert(throws_invalid_argument([] { (void)mtrc::ERP<double>{-1.0}(std::vector<double>{-1.0}, std::vector<double>{0.0}); },
								   "gap"));

	using Point = std::vector<double>;
	using Set = std::vector<Point>;
	const std::vector<Set> point_sets = {
		Set{{0.0}, {2.0}},
		Set{{1.0}},
		Set{{3.0}, {4.0}},
		Set{{0.0}, {2.0}, {4.0}},
	};
	assert_metric_contracts_close(point_sets, mtrc::Hausdorff<>{});
	assert(close(mtrc::Hausdorff<>{}(Set{{0.0}, {2.0}}, Set{{1.0}}), 1.0));
	assert(throws_invalid_argument([] { (void)mtrc::Hausdorff<>{}(Set{}, Set{{1.0}}); }, "non-empty"));
}

void check_probability_metrics()
{
	static_assert(mtrc::metric_traits<mtrc::TotalVariation<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::Hellinger<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::SqrtJensenShannon<double>>::law == mtrc::metric_law::metric);

	const std::vector<std::vector<double>> probabilities = {
		{1.0, 0.0, 0.0},
		{0.0, 1.0, 0.0},
		{0.0, 0.0, 1.0},
		{0.5, 0.5, 0.0},
		{0.2, 0.3, 0.5},
	};
	assert_metric_contracts_close(probabilities, mtrc::TotalVariation<double>{});
	assert_metric_contracts_close(probabilities, mtrc::Hellinger<double>{});
	assert_metric_contracts_close(probabilities, mtrc::SqrtJensenShannon<double>{}, 1e-8);

	assert(close(mtrc::TotalVariation<double>{}(probabilities[0], probabilities[1]), 1.0));
	assert(close(mtrc::Hellinger<double>{}(probabilities[0], probabilities[1]), 1.0));
	assert(close(mtrc::SqrtJensenShannon<double>{}(probabilities[0], probabilities[0]), 0.0));
	assert(throws_invalid_argument(
		[] { (void)mtrc::TotalVariation<double>{}(std::vector<double>{0.5, 0.4}, std::vector<double>{0.5, 0.5}); },
		"sum to 1"));
	assert(throws_invalid_argument(
		[] { (void)mtrc::Hellinger<double>{}(std::vector<double>{-0.1, 1.1}, std::vector<double>{0.5, 0.5}); },
		"nonnegative"));
}

void check_empirical_cdf_metrics()
{
	static_assert(mtrc::metric_traits<mtrc::EmpiricalKolmogorovSmirnov<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::EmpiricalCramer<double>>::law == mtrc::metric_law::metric);

	const std::vector<std::vector<double>> samples = {
		{0.0},
		{1.0},
		{0.0, 1.0},
		{0.0, 0.0, 1.0},
		{0.0, 1.0, 2.0},
	};
	assert_metric_contracts_close(samples, mtrc::EmpiricalKolmogorovSmirnov<double>{});
	assert_metric_contracts_close(samples, mtrc::EmpiricalCramer<double>{});
	assert(close(mtrc::EmpiricalKolmogorovSmirnov<double>{}(std::vector<double>{0.0, 1.0}, std::vector<double>{0.0, 2.0}),
				 0.5));
	assert(close(mtrc::EmpiricalCramer<double>{}(std::vector<double>{0.0}, std::vector<double>{2.0}), std::sqrt(2.0)));

	const auto tied = mtrc::EmpiricalKolmogorovSmirnov<double>{}(std::vector<double>{1.0, 1.0, 2.0},
																std::vector<double>{1.0, 2.0, 2.0});
	assert(std::isfinite(tied));
	assert(tied > 0.0);
	assert(throws_invalid_argument(
		[] { (void)mtrc::EmpiricalCramer<double>{}(std::vector<double>{}, std::vector<double>{1.0}); }, "non-empty"));
}

} // namespace

int main()
{
	check_vector_geometry_metrics();
	check_categorical_and_set_metrics();
	check_time_series_and_geometry_metrics();
	check_probability_metrics();
	check_empirical_cdf_metrics();
	return 0;
}
