// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <array>
#include <cassert>
#include <cmath>
#include <deque>
#include <stdexcept>
#include <vector>

#include "metric/metric/catalog.hpp"
#include "metric/engine.hpp"
#include "metric/stats/properties/entropy.hpp"

namespace {

auto close(double actual, double expected, double tolerance = 1e-12) -> bool
{
	return std::abs(actual - expected) < tolerance;
}

struct IntDistance {
	auto operator()(int lhs, int rhs) const -> double
	{
		return static_cast<double>(lhs > rhs ? lhs - rhs : rhs - lhs);
	}
};

} // namespace

int main()
{
	const auto manual_entropy =
		mtrc::core::make_entropy_result(1.25, 4, 3, 2, true, "manual_entropy", false);
	assert(close(manual_entropy.value, 1.25));
	assert(manual_entropy.record_count == 4);
	assert(manual_entropy.neighbor_count == 3);
	assert(manual_entropy.approximation_order == 2);
	assert(manual_entropy.exponentiated);
	assert(!manual_entropy.exact);
	assert(manual_entropy.algorithm == "entropy");
	assert(manual_entropy.representation == "manual_entropy");

	const auto manual_correlation =
		mtrc::core::make_correlation_result(0.75, 4, 4, "manual_mgc", "left_manual", "right_manual", false);
	assert(close(manual_correlation.value, 0.75));
	assert(manual_correlation.left_record_count == 4);
	assert(manual_correlation.right_record_count == 4);
	assert(!manual_correlation.exact);
	assert(manual_correlation.algorithm == "manual_mgc");
	assert(manual_correlation.left_representation == "left_manual");
	assert(manual_correlation.right_representation == "right_manual");

	std::vector<std::vector<double>> vector_records = {
		{5.0, 5.0},
		{2.0, 2.0},
		{3.0, 3.0},
		{5.0, 1.0},
	};

	auto vector_space = mtrc::make_space(vector_records, mtrc::Chebyshev<double>());
	const mtrc::Entropy<void, mtrc::Chebyshev<double>> direct_entropy(mtrc::Chebyshev<double>(), 3, 2);
	const auto expected_entropy = direct_entropy(vector_records);

	const auto space_entropy = mtrc::stats::properties::entropy(vector_space, 3, 2);
	assert(space_entropy.algorithm == "entropy");
	assert(space_entropy.representation == "metric_space");
	assert(space_entropy.record_count == vector_space.size());
	assert(space_entropy.neighbor_count == 3);
	assert(space_entropy.approximation_order == 2);
	assert(space_entropy.exact);
	assert(close(space_entropy.value, expected_entropy, 1e-12));

	const auto mapped_entropy_source = mtrc::make_space(std::vector<int>{0, 1, 2, 3}, IntDistance{});
	const auto mapped_coordinates = mtrc::map(mapped_entropy_source,
											  [](int value) {
												  return std::vector<double>{
													  static_cast<double>(value),
													  static_cast<double>(value * value),
												  };
											  },
											  mtrc::Euclidean<double>());
	const auto mapped_entropy = mtrc::stats::properties::entropy(mapped_coordinates, 3, 2);
	assert(mapped_entropy.representation == "deterministic_transform");
	assert(mapped_entropy.record_count == mapped_coordinates.space.size());

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

	const auto space_correlation = mtrc::stats::correlate::mgc(first_space, second_space);
	assert(space_correlation.algorithm == "mgc");
	assert(space_correlation.left_representation == "metric_space");
	assert(space_correlation.right_representation == "metric_space");
	assert(space_correlation.left_record_count == first_space.size());
	assert(space_correlation.right_record_count == second_space.size());
	assert(space_correlation.exact);
	assert(close(space_correlation.value, 0.28845660296530595, 1e-12));

	const auto record_correlation =
		mtrc::stats::correlate::mgc(first, mtrc::Euclidean<double>(), second, mtrc::Manhattan<float>());
	assert(record_correlation.left_representation == "records");
	assert(record_correlation.right_representation == "records");
	assert(close(record_correlation.value, space_correlation.value, 1e-12));

	bool rejected_mismatch = false;
	try {
		const auto short_second = std::deque<std::array<float, 1>>(second.begin(), second.end() - 1);
		(void)mtrc::stats::correlate::mgc(first, mtrc::Euclidean<double>(), short_second, mtrc::Manhattan<float>());
	} catch (const std::invalid_argument &) {
		rejected_mismatch = true;
	}
	assert(rejected_mismatch);

	return 0;
}
