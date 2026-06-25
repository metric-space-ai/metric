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
		"manual_empty_representation", mtrc::metric_law::unknown, false,
		"manual empty mapping fixture with no derived records");
	mtrc::core::require_mapping_result_contract(empty_manual, "manual empty mapping smoke");
	assert(empty_manual.empty());
	assert(empty_manual.source_record_count == space.size());
	assert(!empty_manual.inverse_supported);
	assert(empty_manual.mapping == "manual_empty_mapping");
	assert(empty_manual.strategy == "manual_empty_strategy");
	assert(empty_manual.representation == "manual_empty_representation");

	const auto filtered = mtrc::density_filter(space, mtrc::stats::structural_analysis::dbscan_options(2.0, 2));
	using Result = decltype(filtered);
	static_assert(std::is_same<typename Result::space_type::record_type, int>::value);

	assert(filtered.mapping == "density_filter");
	assert(filtered.strategy == "dbscan_density_filter");
	assert(filtered.representation == "metric_space");
	assert(!filtered.inverse_supported);
	assert(filtered.source_record_count == space.size());
	assert(filtered.space.size() == 4);
	assert(filtered.space.record(filtered.space.id(0)) == 0);
	assert(filtered.space.record(filtered.space.id(3)) == 11);
	assert(filtered.source_records.size() == filtered.space.size());
	assert(filtered.source_records[0][0] == space.id(0));
	assert(filtered.source_records[3][0] == space.id(3));
	assert(filtered.representative_records[3] == space.id(3));
	mtrc::core::require_mapping_result_contract(filtered, "density filter mapping smoke");

	const auto direct = mtrc::density_filter(space, 2.0, 2);
	assert(direct.space.size() == filtered.space.size());
	assert(direct.source_record_count == filtered.source_record_count);

	const auto materialized_policy = mtrc::space::storage::materialized(mtrc::space::storage::exact());
	const auto materialized =
		mtrc::density_filter(space, mtrc::stats::structural_analysis::dbscan_options(2.0, 2), materialized_policy);
	assert(materialized.representation == "distance_table");
	assert(materialized.space.size() == filtered.space.size());
	assert(materialized.source_records == filtered.source_records);
	assert(materialized.representative_records == filtered.representative_records);

	const auto materialized_direct = mtrc::density_filter(space, 2.0, 2, materialized_policy);
	assert(materialized_direct.representation == "distance_table");
	assert(materialized_direct.space.size() == filtered.space.size());

	const auto dense = mtrc::density_filter(space, mtrc::stats::structural_analysis::dbscan_options(100.0, 2));
	assert(dense.space.size() == space.size());
	assert(dense.source_records.size() == space.size());

	const auto approximate =
		mtrc::density_filter(space, mtrc::stats::structural_analysis::dbscan_options(2.0, 2),
							 mtrc::space::storage::approximate());
	assert(approximate.strategy == "sampled_dbscan_density_filter");
	assert(approximate.representation == "sampled_metric_space");
	assert(approximate.space.size() == filtered.space.size());
	assert(approximate.source_record_count == space.size());
	assert(approximate.source_records.size() == approximate.space.size());
	mtrc::core::require_mapping_result_contract(approximate, "approximate density filter mapping smoke");

	std::vector<int> large_records;
	large_records.reserve(2048);
	for (int value = 0; value < 2048; ++value) {
		large_records.push_back(value);
	}
	const auto large_space = mtrc::make_space(large_records, AbsoluteDistance{});
	const auto bounded_approximate =
		mtrc::density_filter(large_space, mtrc::stats::structural_analysis::dbscan_options(64.0, 2),
							 mtrc::space::storage::using_knn_graph(32, mtrc::space::storage::approximate()));
	assert(bounded_approximate.strategy == "sampled_dbscan_density_filter");
	assert(bounded_approximate.representation == "sampled_metric_space");
	assert(bounded_approximate.source_record_count == large_space.size());
	assert(bounded_approximate.space.size() == large_space.size());
	mtrc::core::require_mapping_result_contract(bounded_approximate, "bounded approximate density filter mapping smoke");

	auto sparse = mtrc::make_space(std::vector<int>{0, 10}, AbsoluteDistance{});
	const auto empty = mtrc::density_filter(sparse, mtrc::stats::structural_analysis::dbscan_options(1.0, 2));
	assert(empty.space.empty());
	assert(empty.source_records.empty());
	assert(empty.representative_records.empty());
	assert(empty.source_record_count == sparse.size());

	return 0;
}
