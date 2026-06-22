// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cassert>
#include <type_traits>
#include <utility>
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

	auto empty_manual_space = mtrc::make_space(std::vector<int>{}, AbsoluteDistance{});
	const auto empty_manual = mtrc::core::make_mapping_result(
		std::move(empty_manual_space), std::vector<std::vector<mtrc::RecordId>>{},
		std::vector<mtrc::RecordId>{}, space.size(), false, "manual_empty_mapping", "manual_empty_strategy",
		"manual_empty_representation");
	assert(empty_manual.empty());
	assert(empty_manual.source_record_count == space.size());
	assert(!empty_manual.inverse_supported);
	assert(empty_manual.mapping == "manual_empty_mapping");
	assert(empty_manual.strategy == "manual_empty_strategy");
	assert(empty_manual.representation == "manual_empty_representation");

	const auto denoised = mtrc::denoise(space, mtrc::stats::structural_analysis::dbscan_options(2.0, 2));
	using Result = decltype(denoised);
	static_assert(std::is_same<typename Result::space_type::record_type, int>::value);

	assert(denoised.mapping == "density_denoise");
	assert(denoised.strategy == "dbscan_noise_filter");
	assert(denoised.representation == "metric_space");
	assert(!denoised.inverse_supported);
	assert(denoised.source_record_count == space.size());
	assert(denoised.space.size() == 4);
	assert(denoised.space.record(denoised.space.id(0)) == 0);
	assert(denoised.space.record(denoised.space.id(3)) == 11);
	assert(denoised.source_records.size() == denoised.space.size());
	assert(denoised.source_records[0][0] == space.id(0));
	assert(denoised.source_records[3][0] == space.id(3));
	assert(denoised.representative_records[3] == space.id(3));

	const auto direct = mtrc::denoise(space, 2.0, 2);
	assert(direct.space.size() == denoised.space.size());
	assert(direct.source_record_count == denoised.source_record_count);

	const auto materialized_policy = mtrc::space::storage::materialized(mtrc::space::storage::exact());
	const auto materialized = mtrc::denoise(space, mtrc::stats::structural_analysis::dbscan_options(2.0, 2), materialized_policy);
	assert(materialized.representation == "distance_table");
	assert(materialized.space.size() == denoised.space.size());
	assert(materialized.source_records == denoised.source_records);
	assert(materialized.representative_records == denoised.representative_records);

	const auto materialized_direct = mtrc::denoise(space, 2.0, 2, materialized_policy);
	assert(materialized_direct.representation == "distance_table");
	assert(materialized_direct.space.size() == denoised.space.size());

	const auto dense = mtrc::denoise(space, mtrc::stats::structural_analysis::dbscan_options(100.0, 2));
	assert(dense.space.size() == space.size());
	assert(dense.source_records.size() == space.size());

	bool rejected_approximate_runtime = false;
	try {
		(void)mtrc::denoise(space, mtrc::stats::structural_analysis::dbscan_options(2.0, 2), mtrc::space::storage::approximate());
	} catch (const std::invalid_argument &) {
		rejected_approximate_runtime = true;
	}
	assert(rejected_approximate_runtime);

	auto sparse = mtrc::make_space(std::vector<int>{0, 10}, AbsoluteDistance{});
	const auto empty = mtrc::denoise(sparse, mtrc::stats::structural_analysis::dbscan_options(1.0, 2));
	assert(empty.space.empty());
	assert(empty.source_records.empty());
	assert(empty.representative_records.empty());
	assert(empty.source_record_count == sparse.size());

	return 0;
}
