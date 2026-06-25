#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "metric/metric/catalog.hpp"
#include "metric/engine.hpp"

namespace {

auto close(double actual, double expected, double tolerance = 1e-12) -> bool
{
	return std::abs(actual - expected) < tolerance;
}

struct CountingLengthDistance {
	explicit CountingLengthDistance(std::shared_ptr<std::size_t> calls)
		: calls(std::move(calls))
	{
	}

	auto operator()(const std::string &lhs, const std::string &rhs) const -> int
	{
		++(*calls);
		const auto difference = static_cast<int>(lhs.size()) - static_cast<int>(rhs.size());
		return difference < 0 ? -difference : difference;
	}

	std::shared_ptr<std::size_t> calls;
};

struct CountingAbsoluteDistance {
	explicit CountingAbsoluteDistance(std::shared_ptr<std::size_t> calls)
		: calls(std::move(calls))
	{
	}

	auto operator()(double lhs, double rhs) const -> double
	{
		++(*calls);
		const auto difference = lhs - rhs;
		return difference < 0.0 ? -difference : difference;
	}

	std::shared_ptr<std::size_t> calls;
};

} // namespace

int main()
{
	std::vector<std::vector<double>> first = {
		{-1.08661677587398},  {-1.00699896410939},	{-0.814135753976830},  {-0.875364720432552}, {-0.659607023272462},
		{-0.798949992922930}, {-0.431585448024267}, {-0.619123703544758},  {-0.351492263653510}, {-0.394814371972061},
		{-0.309693618374598}, {-0.352009525808777}, {-0.0803413535982411}, {0.0103940699342647}, {-0.130735385695596},
		{-0.138214899507693}, {0.0279270082022143}, {0.141670765995995},   {0.112221224566625},	 {0.376767573021755},
		{0.186729429735154},  {0.597349318463320},	{0.451380104139401},   {0.639237742050564},	 {0.797420868050314},
		{0.690091614630087},  {0.921722674141222},	{0.852593762434809},   {0.954771723842945},	 {1.03297970279357}};

	std::deque<std::array<float, 1>> second = {
		{2.70625143351230F},	 {1.41259513494005F},	  {0.666086793692617F},	   {0.647856446084279F},
		{0.887764969338737F},	 {0.286220905202707F},	  {0.543682026943014F},	   {0.0402339224257120F},
		{0.105812168910424F},	 {0.0230915137205610F},	  {0.00298976085950325F},  {0.00366997150982423F},
		{0.000384825484363474F}, {7.27293780465119e-05F}, {2.50809340229209e-07F}, {0.00306636655437742F},
		{0.000456283181338950F}, {0.00801756105329616F},  {1.17238339150888e-09F}, {0.0803830108071682F},
		{0.0774478107095828F},	 {0.0474847202878941F},	  {0.0818772460512609F},   {0.486406609209630F},
		{0.197547677770060F},	 {0.628321368933714F},	  {1.02400551043736F},	   {0.552591658802459F},
		{1.52144482984914F},	 {3.43908991254968F}};

	auto first_space = mtrc::make_space(first, mtrc::Euclidean<double>());
	auto second_space = mtrc::make_space(second, mtrc::Manhattan<float>());

	const auto compared = mtrc::compare(first_space, second_space, mtrc::stats::correlate::mgc_options{});
	assert(compared.algorithm == "mgc");
	assert(compared.left_representation == "metric_space");
	assert(compared.right_representation == "metric_space");
	assert(compared.left_record_count == first_space.size());
	assert(compared.right_record_count == second_space.size());
	assert(compared.exact);
	assert(close(compared.value, 0.28845660296530595));

	const auto default_compared = mtrc::compare(first_space, second_space);
	assert(close(default_compared.value, compared.value));

	const auto correlated = mtrc::correlate(first_space, second_space);
	assert(close(correlated.value, compared.value));

	const auto materialized_policy = mtrc::space::storage::materialized(mtrc::space::storage::exact());
	const auto materialized = mtrc::compare(first_space, second_space, materialized_policy);
	assert(materialized.left_representation == "distance_table");
	assert(materialized.right_representation == "distance_table");
	assert(materialized.left_record_count == compared.left_record_count);
	assert(materialized.right_record_count == compared.right_record_count);
	assert(close(materialized.value, compared.value));

	const auto materialized_correlated =
		mtrc::correlate(first_space, second_space, mtrc::stats::correlate::mgc_options{}, materialized_policy);
	assert(materialized_correlated.left_representation == "distance_table");
	assert(materialized_correlated.right_representation == "distance_table");
	assert(close(materialized_correlated.value, compared.value));

	auto left_metric_calls = std::make_shared<std::size_t>(0);
	auto right_metric_calls = std::make_shared<std::size_t>(0);
	auto counted_left_space = mtrc::make_space(
		std::vector<std::string>{"a", "bbb", "cc", "eeeee"}, CountingLengthDistance(left_metric_calls));
	auto counted_right_space = mtrc::make_space(
		std::deque<double>{2.0, -1.0, 4.5, 8.0}, CountingAbsoluteDistance(right_metric_calls));
	const auto exact_counted = mtrc::compare(counted_left_space, counted_right_space);
	assert(exact_counted.exact);
	assert(exact_counted.left_representation == "metric_space");
	assert(exact_counted.right_representation == "metric_space");
	assert(*left_metric_calls > 0);
	assert(*right_metric_calls > 0);

	const auto refusing_materialized_policy =
		mtrc::space::storage::with_distance_table_budget(mtrc::space::storage::materialized(), 1, 0);
	auto assert_refused_before_metric_calls = [&](auto operation) {
		*left_metric_calls = 0;
		*right_metric_calls = 0;
		bool rejected = false;
		try {
			operation();
		} catch (const mtrc::RepresentationError &error) {
			rejected = true;
			const std::string message = error.what();
			assert(message.find("max_dense_records") != std::string::npos);
			assert(message.find("fallback") != std::string::npos);
		}
		assert(rejected);
		assert(*left_metric_calls == 0);
		assert(*right_metric_calls == 0);
	};
	assert_refused_before_metric_calls([&] {
		(void)mtrc::compare(counted_left_space, counted_right_space, refusing_materialized_policy);
	});
	assert_refused_before_metric_calls([&] {
		(void)mtrc::correlate(counted_left_space, counted_right_space, mtrc::stats::correlate::mgc_options{},
							  refusing_materialized_policy);
	});

	const auto blocked_materialized_policy =
		mtrc::space::storage::allow_chunking_fallback(refusing_materialized_policy);
	*left_metric_calls = 0;
	*right_metric_calls = 0;
	const auto blocked_compared = mtrc::compare(counted_left_space, counted_right_space, blocked_materialized_policy);
	assert(blocked_compared.exact);
	assert(blocked_compared.left_representation == "blocked_distance_table");
	assert(blocked_compared.right_representation == "blocked_distance_table");
	assert(close(blocked_compared.value, exact_counted.value));
	assert(*left_metric_calls > 0);
	assert(*right_metric_calls > 0);

	*left_metric_calls = 0;
	*right_metric_calls = 0;
	const auto blocked_correlated =
		mtrc::correlate(counted_left_space, counted_right_space, mtrc::stats::correlate::mgc_options{},
						blocked_materialized_policy);
	assert(blocked_correlated.exact);
	assert(blocked_correlated.left_representation == "blocked_distance_table");
	assert(blocked_correlated.right_representation == "blocked_distance_table");
	assert(close(blocked_correlated.value, exact_counted.value));
	assert(*left_metric_calls > 0);
	assert(*right_metric_calls > 0);

	std::vector<double> chunked_left_records;
	std::vector<double> chunked_right_records;
	chunked_left_records.reserve(64);
	chunked_right_records.reserve(64);
	for (std::size_t index = 0; index < 64; ++index) {
		const auto value = static_cast<double>(index);
		chunked_left_records.push_back(value);
		chunked_right_records.push_back(value * value + 3.0);
	}
	auto chunked_left_calls = std::make_shared<std::size_t>(0);
	auto chunked_right_calls = std::make_shared<std::size_t>(0);
	const auto chunked_left_space = mtrc::make_space(
		chunked_left_records, CountingAbsoluteDistance(chunked_left_calls));
	const auto chunked_right_space = mtrc::make_space(
		chunked_right_records, CountingAbsoluteDistance(chunked_right_calls));
	auto chunked_compare_policy = mtrc::space::storage::using_distance_table();
	chunked_compare_policy = mtrc::space::storage::with_distance_table_budget(chunked_compare_policy, 8, 0);
	chunked_compare_policy = mtrc::space::storage::allow_approximate_fallback(chunked_compare_policy);
	chunked_compare_policy = mtrc::space::storage::allow_chunking_fallback(chunked_compare_policy);
	const auto chunked_left_plan =
		mtrc::space::storage::estimate_cost(chunked_left_space, "compare", chunked_compare_policy);
	assert(chunked_left_plan.allowed);
	assert(chunked_left_plan.downgraded);
	assert(!chunked_left_plan.exact);
	assert(chunked_left_plan.exactness == "approximate_chunked");
	assert(chunked_left_plan.representation == "chunked_space_view");
	assert(chunked_left_plan.chunked_plan);
	assert(chunked_left_plan.chunk_size == 8);
	assert(chunked_left_plan.chunk_count == 8);
	assert(chunked_left_plan.representative_count == 8);
	assert(chunked_left_plan.estimated_distance_evaluations == 2 * ((8 * 7) / 2));
	assert(chunked_left_plan.estimated_distance_evaluations <
		   chunked_left_space.size() * (chunked_left_space.size() - 1));

	const auto chunked_compared = mtrc::compare(chunked_left_space, chunked_right_space, chunked_compare_policy);
	assert(!chunked_compared.exact);
	assert(chunked_compared.algorithm == "chunked_mgc_estimate");
	assert(chunked_compared.left_representation == "chunked_space_view");
	assert(chunked_compared.right_representation == "chunked_space_view");
	assert(chunked_compared.left_record_count == chunked_left_space.size());
	assert(chunked_compared.right_record_count == chunked_right_space.size());
	assert(chunked_compared.sample_count == 8);
	assert(chunked_compared.sample_iterations == 1);
	assert(chunked_compared.approximation_reason.find("chunk_size=8") != std::string::npos);
	assert(std::isfinite(chunked_compared.value));
	assert(chunked_compared.value >= -1.0);
	assert(chunked_compared.value <= 1.0);
	assert(*chunked_left_calls + *chunked_right_calls == chunked_left_plan.estimated_distance_evaluations);

	*chunked_left_calls = 0;
	*chunked_right_calls = 0;
	const auto chunked_correlated =
		mtrc::correlate(chunked_left_space, chunked_right_space, mtrc::stats::correlate::mgc_options{},
						chunked_compare_policy);
	assert(!chunked_correlated.exact);
	assert(chunked_correlated.algorithm == "chunked_mgc_estimate");
	assert(chunked_correlated.left_representation == "chunked_space_view");
	assert(chunked_correlated.right_representation == "chunked_space_view");
	assert(chunked_correlated.sample_count == chunked_compared.sample_count);
	assert(close(chunked_correlated.value, chunked_compared.value));
	assert(*chunked_left_calls + *chunked_right_calls == chunked_left_plan.estimated_distance_evaluations);

	mtrc::stats::correlate::mgc_options approximate_options;
	approximate_options.sample_count = 10;
	approximate_options.max_iterations = 3;
	const auto approximate =
		mtrc::compare(first_space, second_space, approximate_options, mtrc::space::storage::approximate());
	assert(approximate.algorithm == "mgc_estimate");
	assert(!approximate.exact);
	assert(approximate.left_representation == "metric_space_sample");
	assert(approximate.right_representation == "metric_space_sample");
	assert(approximate.left_record_count == first_space.size());
	assert(approximate.right_record_count == second_space.size());
	assert(approximate.sample_count == 10);
	assert(approximate.sample_iterations == 3);
	assert(!approximate.approximation_reason.empty());
	assert(std::isfinite(approximate.value));
	assert(approximate.value >= -1.0);
	assert(approximate.value <= 1.0);

	return 0;
}
