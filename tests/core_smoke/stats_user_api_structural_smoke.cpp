// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Structural-analysis additions: metric-space cluster validity diagnostics (silhouette
// and intra/inter-cluster distances) and a k-NN distance outlier score. Both are read-only
// investigations of an existing finite metric space and are deterministic.

#include <cassert>
#include <cstddef>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/core/errors.hpp"
#include "metric/core/metric_space.hpp"
#include "metric/space/select/representatives.hpp"
#include "metric/stats/structural_analysis/clustering.hpp"
#include "metric/stats/structural_analysis/diagnostics.hpp"
#include "metric/stats/structural_analysis/outliers.hpp"
#include "metric/stats/structural_analysis/representatives.hpp"

namespace {

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct CountingDistance {
	std::shared_ptr<std::size_t> calls;

	auto operator()(int lhs, int rhs) const -> int
	{
		++(*calls);
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
};

struct LargeCountingProvider {
	using distance_type = int;

	std::size_t count;
	std::shared_ptr<std::size_t> calls;

	auto distance(mtrc::RecordId lhs, mtrc::RecordId rhs) const -> distance_type
	{
		++(*calls);
		const auto lhs_index = lhs.index();
		const auto rhs_index = rhs.index();
		return lhs_index > rhs_index ? static_cast<int>(lhs_index - rhs_index)
									 : static_cast<int>(rhs_index - lhs_index);
	}

	auto record_count() const -> std::size_t { return count; }
	auto id(std::size_t position) const -> mtrc::RecordId { return mtrc::RecordId::from_index(position); }
	auto position_of(mtrc::RecordId id) const -> std::size_t
	{
		if (!contains(id)) {
			throw std::out_of_range("record id is outside the provider");
		}
		return id.index();
	}
	auto contains(mtrc::RecordId id) const -> bool { return id.index() < count; }
	auto version() const -> std::size_t { return 1; }
	auto is_stale() const -> bool { return false; }
};

auto close(double lhs, double rhs, double tolerance = 1.0e-9) -> bool { return std::abs(lhs - rhs) < tolerance; }

template <typename Function> auto throws_invalid_argument(Function run) -> bool
{
	try {
		run();
	} catch (const std::invalid_argument &) {
		return true;
	}
	return false;
}

void cluster_diagnostics_on_two_clean_clusters()
{
	namespace structural = mtrc::stats::structural_analysis;
	const auto space = mtrc::make_space(std::vector<int>{0, 1, 2, 100, 101, 102}, AbsoluteDistance{});

	const auto clusters = structural::kmedoids(space, 2);
	const auto diagnostics = structural::cluster_diagnostics(space, clusters);

	assert(diagnostics.algorithm == "cluster_diagnostics");
	assert(diagnostics.cluster_count == 2);
	assert(diagnostics.evaluated_record_count == 6);
	assert(diagnostics.unassigned_count == 0);
	assert((diagnostics.cluster_sizes == std::vector<std::size_t>{3, 3}));
	assert(diagnostics.silhouettes.size() == 6);
	assert(diagnostics.evaluated_ids.size() == 6);
	assert(diagnostics.per_cluster_mean_intra_distance.size() == 2);

	// Tight, well-separated clusters -> silhouette near 1, small intra and large inter.
	assert(diagnostics.mean_silhouette > 0.95);
	assert(close(diagnostics.mean_intra_cluster_distance, 4.0 / 3.0));
	assert(close(diagnostics.mean_nearest_cluster_distance, 100.0));

	// Determinism: identical on a second run.
	const auto again = structural::cluster_diagnostics(space, structural::kmedoids(space, 2));
	assert(close(again.mean_silhouette, diagnostics.mean_silhouette));
	assert(again.silhouettes == diagnostics.silhouettes);
}

void cluster_diagnostics_single_cluster_is_zero()
{
	namespace structural = mtrc::stats::structural_analysis;
	const auto space = mtrc::make_space(std::vector<int>{0, 1, 2, 3}, AbsoluteDistance{});

	const auto single = structural::kmedoids(space, 1);
	const auto diagnostics = structural::cluster_diagnostics(space, single);
	assert(diagnostics.cluster_count == 1);
	assert(diagnostics.evaluated_record_count == 4);
	// No second cluster -> silhouette and nearest-cluster distance are 0 by convention.
	assert(close(diagnostics.mean_silhouette, 0.0));
	assert(close(diagnostics.mean_nearest_cluster_distance, 0.0));
}

void cluster_diagnostics_excludes_unassigned_records()
{
	namespace structural = mtrc::stats::structural_analysis;
	// Two dense triplets plus a single far record that DBSCAN leaves unassigned.
	const auto space = mtrc::make_space(std::vector<int>{0, 1, 2, 20, 21, 22, 50}, AbsoluteDistance{});

	const auto clusters = structural::dbscan(space, 2, 2);
	const auto diagnostics = structural::cluster_diagnostics(space, clusters);
	assert(diagnostics.cluster_count == 2);
	assert(diagnostics.unassigned_count == 1);
	assert(diagnostics.evaluated_record_count == 6); // the unassigned record is not evaluated
	assert(diagnostics.mean_silhouette > 0.9);
}

void cluster_diagnostics_samples_and_refuses_before_metric_calls()
{
	namespace structural = mtrc::stats::structural_analysis;
	const std::vector<int> records{0, 1, 2, 100, 101, 102};
	const auto calls = std::make_shared<std::size_t>(0);
	const auto space = mtrc::make_space(records, CountingDistance{calls});
	const auto clusters = mtrc::core::make_clustering_result<int>(
		std::vector<std::size_t>{0, 0, 0, 1, 1, 1},
		std::vector<mtrc::RecordId>{space.id(1), space.id(4)}, {}, {},
		std::vector<std::size_t>{3, 3}, 1, true, "manual_clusters", "metric_space");

	structural::cluster_diagnostics_options sampled_options;
	sampled_options.max_exact_records = 2;
	sampled_options.sample_count = 2;
	const auto sampled = structural::cluster_diagnostics(space, clusters, sampled_options);
	assert(!sampled.exact);
	assert(sampled.algorithm == "sampled_cluster_diagnostics");
	assert(sampled.evaluated_record_count == 2);
	assert(sampled.sample_count == 2);
	assert(sampled.sample_universe == 6);
	assert(!sampled.approximation_reason.empty());
	assert(*calls == 10);

	*calls = 0;
	structural::cluster_diagnostics_options exact_only;
	exact_only.max_exact_records = 2;
	exact_only.allow_approximate = false;
	bool refused_exact_diagnostics = false;
	try {
		(void)structural::cluster_diagnostics(space, clusters, exact_only);
	} catch (const mtrc::RepresentationError &error) {
		refused_exact_diagnostics = true;
		const std::string message = error.what();
		assert(message.find("cluster_diagnostics") != std::string::npos);
		assert(message.find("max_exact_records") != std::string::npos);
	}
	assert(refused_exact_diagnostics);
	assert(*calls == 0);
}

void direct_representative_providers_refuse_before_distance_calls()
{
	namespace structural = mtrc::stats::structural_analysis;

	const auto calls = std::make_shared<std::size_t>(0);
	const LargeCountingProvider provider{2000, calls};

	bool refused_farthest_first = false;
	try {
		(void)mtrc::space::select::find_representatives(provider, 1000);
	} catch (const mtrc::RepresentationError &error) {
		refused_farthest_first = true;
		const std::string message = error.what();
		assert(message.find("find_representatives") != std::string::npos);
		assert(message.find("max_distance_evaluations") != std::string::npos);
	}
	assert(refused_farthest_first);
	assert(*calls == 0);

	bool refused_radius_coverage = false;
	try {
		(void)mtrc::space::select::find_representatives(provider, mtrc::space::select::radius_coverage{1});
	} catch (const mtrc::RepresentationError &error) {
		refused_radius_coverage = true;
		const std::string message = error.what();
		assert(message.find("radius_coverage") != std::string::npos);
		assert(message.find("max_distance_evaluations") != std::string::npos);
	}
	assert(refused_radius_coverage);
	assert(*calls == 0);

	bool refused_medoid = false;
	try {
		(void)structural::medoid_id(provider);
	} catch (const mtrc::RepresentationError &error) {
		refused_medoid = true;
		const std::string message = error.what();
		assert(message.find("medoid_id") != std::string::npos);
		assert(message.find("max_distance_evaluations") != std::string::npos);
	}
	assert(refused_medoid);
	assert(*calls == 0);
}

void representative_budget_estimates_cover_follow_on_work()
{
	namespace structural = mtrc::stats::structural_analysis;

	std::vector<int> records;
	for (std::size_t index = 0; index < 32; ++index) {
		records.push_back(static_cast<int>(index));
	}

	const auto calls = std::make_shared<std::size_t>(0);
	auto space = mtrc::make_space(records, CountingDistance{calls});
	auto policy = mtrc::space::storage::with_distance_evaluation_budget(
		mtrc::space::storage::exact(), records.size() * (records.size() - 1) / 2);

	bool refused_exact_policy = false;
	try {
		(void)mtrc::find_representatives(space, records.size(), policy);
	} catch (const mtrc::RepresentationError &error) {
		refused_exact_policy = true;
		const std::string message = error.what();
		assert(message.find("max_distance_evaluations") != std::string::npos);
	}
	assert(refused_exact_policy);
	assert(*calls == 0);

	bool refused_radius_policy = false;
	try {
		(void)mtrc::find_representatives(space, mtrc::space::select::radius_coverage{0}, policy);
	} catch (const mtrc::RepresentationError &error) {
		refused_radius_policy = true;
		const std::string message = error.what();
		assert(message.find("max_distance_evaluations") != std::string::npos);
	}
	assert(refused_radius_policy);
	assert(*calls == 0);

	bool refused_representative_indices = false;
	try {
		(void)structural::representative_indices(
			records, CountingDistance{calls}, records.size(), 0,
			structural::representatives_detail::exact_representative_work_options{records.size() * records.size()});
	} catch (const mtrc::RepresentationError &error) {
		refused_representative_indices = true;
		const std::string message = error.what();
		assert(message.find("representative_indices") != std::string::npos);
		assert(message.find("max_distance_evaluations") != std::string::npos);
	}
	assert(refused_representative_indices);
	assert(*calls == 0);
}

void nearest_neighbor_outliers_score_isolation()
{
	namespace structural = mtrc::stats::structural_analysis;
	const auto space = mtrc::make_space(std::vector<int>{0, 1, 2, 100}, AbsoluteDistance{});

	const auto outliers = structural::nearest_neighbor_outliers(space, 1);
	assert(outliers.strategy == "nearest_neighbor_distance");
	assert(outliers.representation == "metric_space");
	assert(outliers.size() == 4);
	// The far record (value 100) is the strongest outlier: its nearest neighbor is at 98.
	assert(outliers[0].id == space.id(3));
	assert(outliers[0].score == 98);
	// Sorted by score descending.
	for (std::size_t index = 1; index < outliers.size(); ++index) {
		assert(outliers[index - 1].score >= outliers[index].score);
	}

	assert(throws_invalid_argument([&] { (void)structural::nearest_neighbor_outliers(space, 0); }));
	assert(throws_invalid_argument([&] { (void)structural::nearest_neighbor_outliers(space, 4); }));
}

} // namespace

int main()
{
	cluster_diagnostics_on_two_clean_clusters();
	cluster_diagnostics_single_cluster_is_zero();
	cluster_diagnostics_excludes_unassigned_records();
	cluster_diagnostics_samples_and_refuses_before_metric_calls();
	direct_representative_providers_refuse_before_distance_calls();
	representative_budget_estimates_cover_follow_on_work();
	nearest_neighbor_outliers_score_isolation();
	return 0;
}
