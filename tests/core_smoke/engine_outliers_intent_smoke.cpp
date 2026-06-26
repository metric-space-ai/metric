// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cassert>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
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

struct CountingMetric {
	std::shared_ptr<std::size_t> calls;

	auto operator()(int lhs, int rhs) const -> int
	{
		++*calls;
		const auto difference = lhs - rhs;
		return difference < 0 ? -difference : difference;
	}
};

struct LargeCountingProvider {
	using distance_type = int;

	std::size_t count{};
	std::size_t *calls{};

	auto record_count() const -> std::size_t { return count; }
	auto id(std::size_t position) const -> mtrc::RecordId { return mtrc::RecordId::from_index(position); }
	auto contains(mtrc::RecordId id) const -> bool { return id.index() < count; }
	auto position_of(mtrc::RecordId id) const -> std::size_t
	{
		if (!contains(id)) {
			throw std::out_of_range("unknown large provider id");
		}
		return id.index();
	}
	auto version() const -> std::size_t { return 1; }
	auto is_stale() const -> bool { return false; }
	auto distance(mtrc::RecordId lhs, mtrc::RecordId rhs) const -> distance_type
	{
		++(*calls);
		const auto lhs_position = lhs.index();
		const auto rhs_position = rhs.index();
		return lhs_position > rhs_position ? static_cast<int>(lhs_position - rhs_position)
										   : static_cast<int>(rhs_position - lhs_position);
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
	assert(manual.unassigned_count == 2);
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
	assert(empty_manual.unassigned_count == 0);
	assert(empty_manual.exact);
	assert(empty_manual.strategy == "empty_strategy");
	assert(empty_manual.representation == "empty_representation");

	const auto outliers = mtrc::find_outliers(space, mtrc::stats::structural_analysis::dbscan_options(2.0, 2));
	using Result = decltype(outliers);
	static_assert(std::is_same<typename Result::score_type, int>::value);

	assert(outliers.operator_name == "find_outliers");
	assert(outliers.strategy == "dbscan_density_outlier");
	assert(outliers.representation == "metric_space");
	assert(outliers.record_count == space.size());
	assert(outliers.cluster_count == 2);
	assert(outliers.unassigned_count == 1);
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
	assert(dense.unassigned_count == 0);
	auto sparse = mtrc::make_space(std::vector<int>{0, 10}, AbsoluteDistance{});
	const auto unassigned_outliers = mtrc::find_outliers(sparse, mtrc::stats::structural_analysis::dbscan_options(1.0, 2));
	assert(unassigned_outliers.size() == 2);
	assert(unassigned_outliers.unassigned_count == 2);
	assert(unassigned_outliers[0].id == sparse.id(0));
	assert(unassigned_outliers[0].score == 10);
	assert(unassigned_outliers[1].id == sparse.id(1));
	assert(unassigned_outliers[1].score == 10);

	const auto approximate_outliers =
		mtrc::find_outliers(space, mtrc::stats::structural_analysis::dbscan_options(2.0, 2),
							mtrc::space::storage::approximate());
	assert(!approximate_outliers.exact);
	assert(approximate_outliers.strategy == "sampled_dbscan_density_outlier");
	assert(approximate_outliers.representation == "sampled_metric_space");
	assert(approximate_outliers.record_count == space.size());

	std::vector<int> large_records;
	large_records.reserve(4100);
	for (int value = 0; value < 4100; ++value) {
		large_records.push_back(value);
	}
	auto large_space = mtrc::make_space(large_records, AbsoluteDistance{});
	const auto default_large_outliers =
		mtrc::find_outliers(large_space, mtrc::stats::structural_analysis::dbscan_options(0.0, 2));
	assert(!default_large_outliers.exact);
	assert(default_large_outliers.strategy == "sampled_dbscan_density_outlier");
	assert(default_large_outliers.representation == "sampled_metric_space");
	assert(default_large_outliers.record_count == large_space.size());

	auto small_nearest_calls = std::make_shared<std::size_t>(0);
	auto small_nearest_space =
		mtrc::make_space(std::vector<int>{0, 1, 2, 100}, CountingMetric{small_nearest_calls});
	const auto small_nearest_outliers = mtrc::nearest_neighbor_outliers(small_nearest_space, 1);
	assert(small_nearest_outliers.exact);
	assert(small_nearest_outliers.strategy == "nearest_neighbor_distance");
	assert(small_nearest_outliers.representation == "metric_space");
	assert(small_nearest_outliers.size() == small_nearest_space.size());
	assert(small_nearest_outliers[0].id == small_nearest_space.id(3));
	assert(small_nearest_outliers[0].score == 98);
	assert(*small_nearest_calls == small_nearest_space.size() * (small_nearest_space.size() - 1));

	auto large_nearest_calls = std::make_shared<std::size_t>(0);
	auto large_nearest_space = mtrc::make_space(large_records, CountingMetric{large_nearest_calls});
	const auto sampled_nearest_outliers = mtrc::nearest_neighbor_outliers(large_nearest_space, 1);
	const auto exact_nearest_scan_calls = large_nearest_space.size() * (large_nearest_space.size() - 1);
	assert(!sampled_nearest_outliers.exact);
	assert(sampled_nearest_outliers.strategy == "sampled_nearest_neighbor_distance");
	assert(sampled_nearest_outliers.representation == "sampled_metric_space");
	assert(sampled_nearest_outliers.record_count == large_nearest_space.size());
	assert(sampled_nearest_outliers.size() == large_nearest_space.size());
	assert(sampled_nearest_outliers.approximation_quality.diagnostic == "outlier_approximation");
	assert(sampled_nearest_outliers.approximation_quality.candidate_policy == "regular_sample");
	assert(sampled_nearest_outliers.approximation_quality.candidate_count == 512);
	assert(sampled_nearest_outliers.approximation_quality.candidate_universe == large_nearest_space.size() - 1);
	assert(sampled_nearest_outliers.approximation_quality.sample_count == 512);
	assert(sampled_nearest_outliers.approximation_quality.sample_universe == large_nearest_space.size());
	assert(sampled_nearest_outliers.approximation_quality.requested_count == 1);
	assert(sampled_nearest_outliers.approximation_quality.sample_fraction > 0.0);
	assert(sampled_nearest_outliers.approximation_quality.sample_fraction < 0.13);
	assert(*large_nearest_calls <= large_nearest_space.size() * 1024);
	assert(*large_nearest_calls < exact_nearest_scan_calls / 2);
	assert(sampled_nearest_outliers.approximation_quality.distance_evaluations == *large_nearest_calls);

	auto provider_nearest_calls = std::make_shared<std::size_t>(0);
	auto provider_nearest_space =
		mtrc::make_space(std::vector<int>{0, 1, 2, 100, 101}, CountingMetric{provider_nearest_calls});
	mtrc::space::storage::LiveDistances<decltype(provider_nearest_space)> provider(provider_nearest_space);
	const auto provider_nearest_outliers = mtrc::nearest_neighbor_outliers(provider, 1);
	assert(provider_nearest_outliers.exact);
	assert(provider_nearest_outliers.strategy == "nearest_neighbor_distance");
	assert(provider_nearest_outliers.representation == "pairwise_distances");
	assert(*provider_nearest_calls == provider_nearest_space.size() * (provider_nearest_space.size() - 1));

	std::size_t provider_dbscan_calls = 0;
	const LargeCountingProvider large_dbscan_provider{
		mtrc::stats::structural_analysis::group_detail::max_default_exact_structural_records + 1,
		&provider_dbscan_calls};
	bool refused_large_provider_dbscan_outliers = false;
	try {
		(void)mtrc::find_outliers(large_dbscan_provider, mtrc::stats::structural_analysis::dbscan_options(1.0, 2));
	} catch (const mtrc::RepresentationError &error) {
		refused_large_provider_dbscan_outliers = true;
		const std::string message = error.what();
		assert(message.find("find_outliers") != std::string::npos);
		assert(message.find("max_exact_records") != std::string::npos);
	}
	assert(refused_large_provider_dbscan_outliers);
	assert(provider_dbscan_calls == 0);

	std::size_t provider_nearest_large_calls = 0;
	const LargeCountingProvider large_nearest_provider{
		mtrc::stats::structural_analysis::group_detail::max_default_exact_structural_records + 1,
		&provider_nearest_large_calls};
	bool refused_large_provider_nearest_outliers = false;
	try {
		(void)mtrc::nearest_neighbor_outliers(large_nearest_provider, 1);
	} catch (const mtrc::RepresentationError &error) {
		refused_large_provider_nearest_outliers = true;
		const std::string message = error.what();
		assert(message.find("nearest_neighbor_outliers") != std::string::npos);
		assert(message.find("max_exact_records") != std::string::npos);
	}
	assert(refused_large_provider_nearest_outliers);
	assert(provider_nearest_large_calls == 0);

	auto exact_policy_calls = std::make_shared<std::size_t>(0);
	auto exact_policy_space =
		mtrc::make_space(std::vector<int>{0, 1, 2, 100, 101}, CountingMetric{exact_policy_calls});
	const auto exact_policy_outliers =
		mtrc::nearest_neighbor_outliers(exact_policy_space, 1, mtrc::space::storage::exact());
	assert(exact_policy_outliers.exact);
	assert(exact_policy_outliers.strategy == "nearest_neighbor_distance");
	assert(exact_policy_outliers.representation == "metric_space");
	assert(*exact_policy_calls == exact_policy_space.size() * (exact_policy_space.size() - 1));

	auto refused_policy_calls = std::make_shared<std::size_t>(0);
	auto refused_policy_space = mtrc::make_space(large_records, CountingMetric{refused_policy_calls});
	const auto refused_policy = mtrc::space::storage::with_distance_evaluation_budget(
		mtrc::space::storage::exact(), refused_policy_space.size() - 1);
	bool rejected_exact_policy_budget = false;
	try {
		(void)mtrc::nearest_neighbor_outliers(refused_policy_space, 1, refused_policy);
	} catch (const mtrc::RepresentationError &error) {
		rejected_exact_policy_budget = true;
		const std::string message = error.what();
		assert(message.find("max_distance_evaluations") != std::string::npos);
		assert(message.find("fallback") != std::string::npos);
	}
	assert(rejected_exact_policy_budget);
	assert(*refused_policy_calls == 0);

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
