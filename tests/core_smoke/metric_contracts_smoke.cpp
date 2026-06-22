#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "metric/concepts.hpp"
#include "metric/metric/catalog.hpp"
#include "metric/engine.hpp"

struct PaddedHamming {
	auto operator()(const std::string &lhs, const std::string &rhs) const -> std::size_t
	{
		const auto limit = std::max(lhs.size(), rhs.size());
		std::size_t distance = 0;

		for (std::size_t i = 0; i < limit; ++i) {
			const char left = i < lhs.size() ? lhs[i] : '\0';
			const char right = i < rhs.size() ? rhs[i] : '\0';
			if (left != right) {
				++distance;
			}
		}

		return distance;
	}
};

struct DirectedIntegerDistance {
	auto operator()(int lhs, int rhs) const -> int
	{
		if (lhs <= rhs) {
			return rhs - lhs;
		}
		return 2 * (lhs - rhs);
	}
};

struct TriangleViolatingDistance {
	auto operator()(int lhs, int rhs) const -> int
	{
		if (lhs == rhs) {
			return 0;
		}
		if ((lhs == 0 && rhs == 2) || (lhs == 2 && rhs == 0)) {
			return 3;
		}
		return 1;
	}
};

template <typename Record, typename Metric>
void assert_metric_contracts(const std::vector<Record> &records, Metric metric)
{
	for (const auto &lhs : records) {
		assert(metric(lhs, lhs) == 0);

		for (const auto &rhs : records) {
			const auto lhs_rhs = metric(lhs, rhs);
			const auto rhs_lhs = metric(rhs, lhs);

			assert(lhs_rhs >= 0);
			assert(lhs_rhs == rhs_lhs);

			for (const auto &through : records) {
				assert(metric(lhs, through) <= lhs_rhs + metric(rhs, through));
			}
		}
	}
}

template <typename Record, typename Metric>
void assert_declared_metric_contracts(const std::vector<Record> &records, Metric metric)
{
	for (const auto &lhs : records) {
		assert(metric(lhs, lhs) == 0);

		for (const auto &rhs : records) {
			const auto lhs_rhs = metric(lhs, rhs);
			assert(lhs_rhs >= 0);

			if constexpr (mtrc::metric_traits<Metric>::law == mtrc::metric_law::metric ||
						  mtrc::metric_traits<Metric>::law == mtrc::metric_law::pseudo_metric) {
				assert(lhs_rhs == metric(rhs, lhs));

				for (const auto &through : records) {
					assert(metric(lhs, through) <= lhs_rhs + metric(rhs, through));
				}
			}

			if constexpr (mtrc::metric_traits<Metric>::law == mtrc::metric_law::metric) {
				if (lhs_rhs == 0) {
					assert(lhs == rhs);
				}
			}
		}
	}
}

template <typename Record, typename Metric>
void assert_metric_contracts_close(const std::vector<Record> &records, Metric metric, double epsilon = 1e-10)
{
	for (const auto &lhs : records) {
		assert(std::abs(metric(lhs, lhs)) <= epsilon);

		for (const auto &rhs : records) {
			const auto lhs_rhs = metric(lhs, rhs);
			const auto rhs_lhs = metric(rhs, lhs);

			assert(lhs_rhs >= -epsilon);
			assert(std::abs(lhs_rhs - rhs_lhs) <= epsilon);

			if (lhs_rhs <= epsilon) {
				assert(lhs == rhs);
			}

			for (const auto &through : records) {
				assert(metric(lhs, through) <= lhs_rhs + metric(rhs, through) + epsilon);
			}
		}
	}
}

template <typename Callable> void assert_rejects_non_metric_cover_tree(Callable &&call)
{
	bool rejected = false;
	try {
		call();
	} catch (const mtrc::RepresentationError &error) {
		rejected = std::string(error.what()).find("metric_law::metric") != std::string::npos;
	}
	assert(rejected);
}

int main()
{
	static_assert(mtrc::is_metric_callable_v<PaddedHamming, std::string>);
	static_assert(mtrc::is_metric_callable_v<mtrc::Manhattan<double>, std::vector<double>>);
	static_assert(mtrc::is_metric_callable_v<mtrc::Sorensen<double>, std::vector<double>>);
	static_assert(mtrc::is_metric_callable_v<mtrc::Ruzicka<double>, std::vector<double>>);
	static_assert(mtrc::metric_traits<mtrc::Euclidean<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::Edit<char>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::Euclidean<double>>::records == mtrc::record_kind::aligned_vector);
	static_assert(mtrc::metric_traits<mtrc::Manhattan<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::Manhattan<double>>::records == mtrc::record_kind::aligned_vector);
	static_assert(mtrc::metric_traits<mtrc::Chebyshev<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::Chebyshev<double>>::records == mtrc::record_kind::aligned_vector);
	static_assert(mtrc::metric_traits<mtrc::P_norm<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::P_norm<double>>::records == mtrc::record_kind::aligned_vector);
	static_assert(mtrc::metric_traits<mtrc::Ruzicka<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::Ruzicka<double>>::records == mtrc::record_kind::aligned_vector);
	static_assert(mtrc::metric_traits<mtrc::Euclidean_standardized<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::Euclidean_standardized<double>>::records ==
				  mtrc::record_kind::aligned_vector);
	static_assert(mtrc::metric_traits<mtrc::Manhattan_standardized<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::Manhattan_standardized<double>>::records ==
				  mtrc::record_kind::aligned_vector);
	static_assert(mtrc::metric_thread_safe_v<mtrc::Euclidean<double>>);
	static_assert(mtrc::metric_thread_safe_v<mtrc::P_norm<double>>);
	static_assert(mtrc::metric_thread_safe_v<mtrc::Ruzicka<double>>);
	static_assert(mtrc::metric_traits<DirectedIntegerDistance>::law == mtrc::metric_law::distance);
	static_assert(mtrc::metric_traits<TriangleViolatingDistance>::law == mtrc::metric_law::distance);

	const auto padded_hamming = mtrc::make_metric<std::string>(PaddedHamming{});
	static_assert(mtrc::is_metric_callable_v<decltype(padded_hamming), std::string>);

	assert_metric_contracts(std::vector<std::string>{"cat", "cot", "coat", "dog"}, mtrc::Edit<std::string>{});

	assert_metric_contracts(std::vector<std::vector<double>>{{0.0, 1.0}, {1.0, 1.0}, {2.0, 3.0}},
							mtrc::Manhattan<double>{});

	assert(std::abs(mtrc::Sorensen<double>{}(std::vector<double>{1.0, 2.0}, std::vector<double>{1.0, 4.0}) - 0.25) <
		   1e-12);
	assert(std::abs(mtrc::Hassanat<double>{}(std::vector<double>{1.0, 2.0}, std::vector<double>{1.0, 4.0}) - 0.4) <
		   1e-12);
	assert(std::abs(mtrc::Ruzicka<double>{}(std::vector<double>{1.0, 2.0}, std::vector<double>{1.0, 4.0}) - 0.4) <
		   1e-12);
	assert(mtrc::Ruzicka<double>{}(std::vector<double>{0.0, 0.0}, std::vector<double>{0.0, 0.0}) == 0.0);

	assert_declared_metric_contracts(std::vector<std::vector<double>>{{0.0, 1.0}, {1.0, 3.0}, {2.0, 0.0}},
									 mtrc::Chebyshev<double>{});

	const std::vector<std::vector<double>> p_norm_records{
		{0.0, 0.0, 0.0},
		{1.0, 0.0, 0.0},
		{1.0, 2.0, 0.0},
		{3.0, 3.0, 1.0},
	};
	assert_metric_contracts_close(p_norm_records, mtrc::P_norm<double>{1.0});
	assert_metric_contracts_close(p_norm_records, mtrc::P_norm<double>{2.0});
	assert_metric_contracts_close(p_norm_records, mtrc::P_norm<double>{3.0});
	assert(std::abs(mtrc::P_norm<double>{3.0}(std::vector<double>{0.0, 0.0}, std::vector<double>{1.0, 2.0}) -
					std::pow(9.0, 1.0 / 3.0)) < 1e-12);
	assert(mtrc::metric_cache_key(mtrc::P_norm<double>{3.0}).find("p=3.000000") != std::string::npos);

	auto rejects_invalid_p_norm = [](double p) {
		bool rejected = false;
		try {
			(void)mtrc::P_norm<double>{p};
		} catch (const std::invalid_argument &error) {
			rejected = std::string(error.what()).find("p >= 1") != std::string::npos;
		}
		assert(rejected);
	};
	rejects_invalid_p_norm(0.0);
	rejects_invalid_p_norm(0.999);
	rejects_invalid_p_norm(-2.0);
	rejects_invalid_p_norm(std::numeric_limits<double>::infinity());
	rejects_invalid_p_norm(std::numeric_limits<double>::quiet_NaN());

	bool rejected_unaligned_vectors = false;
	try {
		(void)mtrc::Manhattan<double>{}(std::vector<double>{0.0, 1.0}, std::vector<double>{0.0});
	} catch (const std::invalid_argument &) {
		rejected_unaligned_vectors = true;
	}
	assert(rejected_unaligned_vectors);

	bool rejected_unaligned_l1_vectors = false;
	try {
		(void)mtrc::Ruzicka<double>{}(std::vector<double>{0.0, 1.0}, std::vector<double>{0.0});
	} catch (const std::invalid_argument &) {
		rejected_unaligned_l1_vectors = true;
	}
	assert(rejected_unaligned_l1_vectors);

	bool rejected_unaligned_fit = false;
	try {
		(void)mtrc::Euclidean_standardized<double>{
			std::vector<std::vector<double>>{{0.0, 1.0}, {1.0}},
		};
	} catch (const std::invalid_argument &) {
		rejected_unaligned_fit = true;
	}
	assert(rejected_unaligned_fit);

	assert_metric_contracts(std::vector<std::string>{"red", "reed", "road", "blue"}, padded_hamming);

	assert_declared_metric_contracts(std::vector<std::vector<double>>{{0.0, 0.0}, {1.0, 0.0}, {1.0, 2.0}, {3.0, 3.0}},
									 mtrc::Euclidean<double>{});

	auto p_norm_space = mtrc::make_space(p_norm_records, mtrc::P_norm<double>{3.0});
	(void)mtrc::space::storage::cover_tree(p_norm_space);

	const std::vector<std::vector<double>> ruzicka_records{
		{0.0, 0.0, 0.0},
		{1.0, 0.0, 0.0},
		{1.0, 2.0, 0.0},
		{3.0, 3.0, 1.0},
	};
	assert_metric_contracts_close(ruzicka_records, mtrc::Ruzicka<double>{});

	auto rejects_invalid_ruzicka = [](const std::vector<double> &record) {
		bool rejected = false;
		try {
			(void)mtrc::Ruzicka<double>{}(std::vector<double>{1.0, 2.0}, record);
		} catch (const std::invalid_argument &error) {
			rejected = std::string(error.what()).find("nonnegative") != std::string::npos;
		}
		assert(rejected);
	};
	rejects_invalid_ruzicka(std::vector<double>{1.0, -1.0});
	rejects_invalid_ruzicka(std::vector<double>{1.0, std::numeric_limits<double>::infinity()});
	rejects_invalid_ruzicka(std::vector<double>{1.0, std::numeric_limits<double>::quiet_NaN()});

	auto ruzicka_space = mtrc::make_space(ruzicka_records, mtrc::Ruzicka<double>{});
	(void)mtrc::space::storage::cover_tree(ruzicka_space);

	// Standardized vector metrics are admitted true metrics under a positive
	// fitted scale, so metric-only routing (cover-tree) must accept them.
	const std::vector<std::vector<double>> standardized_records{
		{0.0, 0.0, 0.0},
		{1.0, 0.0, 2.0},
		{1.0, 2.0, 0.0},
		{3.0, 3.0, 1.0},
	};
	const mtrc::Euclidean_standardized<double> fitted_euclidean(standardized_records);
	assert_metric_contracts_close(standardized_records, fitted_euclidean);
	auto standardized_space = mtrc::make_space(standardized_records, fitted_euclidean);
	(void)mtrc::space::storage::cover_tree(standardized_space);

	const mtrc::Manhattan_standardized<double> fitted_manhattan(standardized_records);
	assert_declared_metric_contracts(standardized_records, fitted_manhattan);

	assert_declared_metric_contracts(std::vector<int>{0, 1, 3}, DirectedIntegerDistance{});

	auto non_metric_space = mtrc::make_space(std::vector<int>{0, 1, 2}, TriangleViolatingDistance{});
	assert_rejects_non_metric_cover_tree(
		[&non_metric_space]() { (void)mtrc::space::storage::cover_tree(non_metric_space); });
	assert_rejects_non_metric_cover_tree([&non_metric_space]() {
		(void)mtrc::find_neighbors(non_metric_space, 1, 2, mtrc::stats::search::cover_tree{});
	});
	assert_rejects_non_metric_cover_tree([&non_metric_space]() {
		(void)mtrc::find_neighbors(non_metric_space, non_metric_space.id(1), 2, mtrc::space::storage::using_cover_tree());
	});

	return 0;
}
