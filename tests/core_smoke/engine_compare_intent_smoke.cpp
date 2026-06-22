#include <array>
#include <cassert>
#include <cmath>
#include <deque>
#include <stdexcept>
#include <vector>

#include "metric/metric/catalog.hpp"
#include "metric/engine.hpp"

namespace {

auto close(double actual, double expected, double tolerance = 1e-12) -> bool
{
	return std::abs(actual - expected) < tolerance;
}

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

	bool rejected_approximate_runtime = false;
	try {
		(void)mtrc::compare(first_space, second_space, mtrc::space::storage::approximate());
	} catch (const std::invalid_argument &) {
		rejected_approximate_runtime = true;
	}
	assert(rejected_approximate_runtime);

	return 0;
}
