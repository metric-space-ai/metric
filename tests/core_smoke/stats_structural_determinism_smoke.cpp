// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Determinism and degenerate-space fixtures for mtrc::stats::structural_analysis (Level-1).
//
// Structural analysis investigates an existing finite metric space and must be deterministic
// (no hidden randomness) and total (no surprise exceptions on valid degenerate spaces). This
// test pins: (1) affinity_propagation on a zero-diameter space falls back to a single global
// medoid cluster instead of throwing; (2) k-medoids with cluster_count == record_count yields
// one singleton cluster per record; (3) DBSCAN over a density-connected line collapses to one
// cluster with every record a core record; and (4) repeated runs are bitwise-identical.

#include <cassert>
#include <vector>

#include "metric/engine.hpp"
#include "metric/space/storage/distance_table.hpp"

namespace {

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct DoubleAbsoluteDistance {
	auto operator()(double lhs, double rhs) const -> double
	{
		const auto difference = lhs - rhs;
		return difference < 0.0 ? -difference : difference;
	}
};

void affinity_propagation_degenerate_space_falls_back_to_single_cluster()
{
	namespace structural = mtrc::stats::structural_analysis;

	// Four identical records -> every pairwise distance is zero, so the message-passing fixed
	// point exposes no positive self-availability and produces no exemplars. The hardened
	// implementation must return a deterministic single-cluster result, not throw.
	auto space = mtrc::make_space(std::vector<double>{5.0, 5.0, 5.0, 5.0}, DoubleAbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(space)> table(space);

	const auto degenerate = structural::affinity_propagation(table, 0.01, 5, 1.0e-6, 0.99);
	assert(degenerate.algorithm == "affinity_propagation");
	assert(degenerate.cluster_count == 1);
	assert((degenerate.assignments == std::vector<std::size_t>{0, 0, 0, 0}));
	assert((degenerate.cluster_sizes == std::vector<std::size_t>{4}));
	assert((degenerate.medoids == std::vector<mtrc::RecordId>{space.id(0)}));

	// Live and materialized paths agree.
	const auto degenerate_live = structural::affinity_propagation(space, 0.01, 5, 1.0e-6, 0.99);
	assert(degenerate_live.assignments == degenerate.assignments);
	assert(degenerate_live.medoids == degenerate.medoids);
}

void kmedoids_one_cluster_per_record()
{
	namespace structural = mtrc::stats::structural_analysis;

	auto space = mtrc::make_space(std::vector<int>{0, 1, 10, 11}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(space)> table(space);

	const auto each_own = structural::kmedoids(table, 4);
	assert(each_own.cluster_count == 4);
	assert((each_own.cluster_sizes == std::vector<std::size_t>{1, 1, 1, 1}));
	assert((each_own.assignments == std::vector<std::size_t>{0, 1, 2, 3}));
	assert((each_own.medoids ==
			std::vector<mtrc::RecordId>{space.id(0), space.id(1), space.id(2), space.id(3)}));
	assert(each_own.converged);
}

void dbscan_density_connected_line_is_one_cluster()
{
	namespace structural = mtrc::stats::structural_analysis;
	using IntGroups = mtrc::ClusteringResult<int>;

	auto space = mtrc::make_space(std::vector<int>{0, 1, 2, 3}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(space)> table(space);

	const auto single = structural::dbscan(table, 2, 1);
	assert(single.cluster_count == 1);
	assert(single.noise_count == 0);
	assert(single.core_records.size() == 4);
	assert((single.assignments == std::vector<std::size_t>{0, 0, 0, 0}));
	(void)IntGroups::noise_label;
}

void repeated_runs_are_identical()
{
	namespace structural = mtrc::stats::structural_analysis;

	auto space = mtrc::make_space(std::vector<int>{0, 1, 10, 11}, AbsoluteDistance{});
	mtrc::space::storage::DistanceTable<decltype(space)> table(space);

	const auto medoids_a = structural::kmedoids(table, 2);
	const auto medoids_b = structural::kmedoids(table, 2);
	assert(medoids_a.assignments == medoids_b.assignments);
	assert(medoids_a.medoids == medoids_b.medoids);

	const auto dbscan_a = structural::dbscan(table, 2, 2);
	const auto dbscan_b = structural::dbscan(table, 2, 2);
	assert(dbscan_a.assignments == dbscan_b.assignments);
	assert(dbscan_a.medoids == dbscan_b.medoids);
	assert(dbscan_a.noise_records == dbscan_b.noise_records);
}

} // namespace

int main()
{
	affinity_propagation_degenerate_space_falls_back_to_single_cluster();
	kmedoids_one_cluster_per_record();
	dbscan_density_connected_line_is_one_cluster();
	repeated_runs_are_identical();
	return 0;
}
