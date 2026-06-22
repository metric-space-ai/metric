// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "metric/engine.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int
	{
		const auto difference = lhs - rhs;
		return difference < 0 ? -difference : difference;
	}
};

int main()
{
	auto space = mtrc::make_space(std::vector<int>{0, 1, 10, 11, 30}, AbsoluteDistance{});

	const auto manual = mtrc::core::make_outlier_result(
		std::vector<mtrc::Outlier<int>>{{space.id(4), 19}, {space.id(1), 2}}, space.size(), 2, 2,
		"manual_strategy", "manual_representation", false);
	assert(manual.record_count == space.size());
	assert(manual.cluster_count == 2);
	assert(manual.noise_count == 2);
	assert(manual.size() == 2);
	assert(manual[0].id == space.id(4));
	assert(manual[0].score == 19);
	assert(!manual.exact);
	assert(manual.operator_name == "find_outliers");
	assert(manual.strategy == "manual_strategy");
	assert(manual.representation == "manual_representation");

	const auto empty_manual = mtrc::core::make_outlier_result(
		std::vector<mtrc::Outlier<int>>{}, space.size(), 2, 0, "empty_strategy", "empty_representation");
	assert(empty_manual.empty());
	assert(empty_manual.record_count == space.size());
	assert(empty_manual.cluster_count == 2);
	assert(empty_manual.noise_count == 0);
	assert(empty_manual.exact);
	assert(empty_manual.strategy == "empty_strategy");
	assert(empty_manual.representation == "empty_representation");

	const auto outliers = mtrc::find_outliers(space, mtrc::stats::structural_analysis::dbscan_options(2.0, 2));
	using Result = decltype(outliers);
	static_assert(std::is_same<typename Result::score_type, int>::value);

	assert(outliers.operator_name == "find_outliers");
	assert(outliers.strategy == "dbscan_noise");
	assert(outliers.representation == "metric_space");
	assert(outliers.record_count == space.size());
	assert(outliers.cluster_count == 2);
	assert(outliers.noise_count == 1);
	assert(outliers.size() == 1);
	assert(outliers[0].id == space.id(4));
	assert(outliers[0].score == 19);

	const auto direct = mtrc::find_outliers(space, 2.0, 2);
	assert(direct.size() == outliers.size());
	assert(direct[0].id == outliers[0].id);
	assert(direct[0].score == outliers[0].score);

	const auto materialized_policy = mtrc::space::storage::materialized(mtrc::space::storage::exact());
	const auto materialized = mtrc::find_outliers(space, mtrc::stats::structural_analysis::dbscan_options(2.0, 2), materialized_policy);
	assert(materialized.representation == "distance_table");
	assert(materialized.size() == outliers.size());
	assert(materialized[0].id == outliers[0].id);
	assert(materialized[0].score == outliers[0].score);

	const auto materialized_direct = mtrc::find_outliers(space, 2.0, 2, materialized_policy);
	assert(materialized_direct.representation == "distance_table");
	assert(materialized_direct[0].id == outliers[0].id);

	const auto dense = mtrc::find_outliers(space, mtrc::stats::structural_analysis::dbscan_options(100.0, 2));
	assert(dense.empty());
	assert(dense.noise_count == 0);
	auto sparse = mtrc::make_space(std::vector<int>{0, 10}, AbsoluteDistance{});
	const auto all_noise = mtrc::find_outliers(sparse, mtrc::stats::structural_analysis::dbscan_options(1.0, 2));
	assert(all_noise.size() == 2);
	assert(all_noise.noise_count == 2);
	assert(all_noise[0].id == sparse.id(0));
	assert(all_noise[0].score == 10);
	assert(all_noise[1].id == sparse.id(1));
	assert(all_noise[1].score == 10);

	bool rejected_approximate_runtime = false;
	try {
		(void)mtrc::find_outliers(space, mtrc::stats::structural_analysis::dbscan_options(2.0, 2), mtrc::space::storage::approximate());
	} catch (const std::invalid_argument &) {
		rejected_approximate_runtime = true;
	}
	assert(rejected_approximate_runtime);

	bool rejected_empty_space = false;
	try {
		auto empty_space = mtrc::make_space(std::vector<int>{}, AbsoluteDistance{});
		(void)mtrc::find_outliers(empty_space, mtrc::stats::structural_analysis::dbscan_options(2.0, 2));
	} catch (const std::invalid_argument &) {
		rejected_empty_space = true;
	}
	assert(rejected_empty_space);

	return 0;
}
