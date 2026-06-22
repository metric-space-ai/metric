#include <cassert>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "metric/engine.hpp"

namespace {

auto absolute_distance(double lhs, double rhs) -> double { return std::abs(lhs - rhs); }

auto close(double lhs, double rhs, double tolerance = 1.0e-12) -> bool { return std::abs(lhs - rhs) <= tolerance; }

} // namespace

int main()
{
	const std::vector<double> records{0.0, 2.0, 5.0, 9.0, 14.0};
	auto space = mtrc::make_space(records, absolute_distance);

	auto identity_mapping = mtrc::map(space, [](double value) { return value; }, absolute_distance);
	const auto identity = mtrc::modify::map::neighbor_preservation(space, identity_mapping, 1);
	assert(identity.diagnostic == "neighbor_preservation");
	assert(identity.source_record_count == records.size());
	assert(identity.mapped_record_count == records.size());
	assert(identity.requested_neighbor_count == 1);
	assert(identity.evaluated_neighbor_count == 1);
	assert(identity.evaluated_rows == records.size());
	assert(identity.matched_neighbors == records.size());
	assert(identity.possible_neighbors == records.size());
	assert(identity.source_distance_evaluations == records.size() * (records.size() - 1));
	assert(identity.mapped_distance_evaluations == records.size() * (records.size() - 1));
	assert(identity.exact);
	assert(identity.recall == 1.0);
	assert(identity.average_row_recall == 1.0);
	assert(identity.minimum_row_recall == 1.0);
	assert(identity.maximum_row_recall == 1.0);
	assert(identity.source_representation == "metric_space");
	assert(identity.mapped_representation == "metric_space");

	auto distorted_mapping = mtrc::map(
		space,
		[](double value) {
			if (value == 0.0) {
				return 100.0;
			}
			return value;
		},
		absolute_distance);
	const auto distorted = mtrc::modify::map::neighbor_preservation(space, distorted_mapping, 1);
	assert(distorted.matched_neighbors < distorted.possible_neighbors);
	assert(distorted.recall < 1.0);
	assert(distorted.minimum_row_recall == 0.0);

	const auto capped = mtrc::modify::map::neighbor_preservation(space, identity_mapping, 99);
	assert(capped.requested_neighbor_count == 99);
	assert(capped.evaluated_neighbor_count == records.size() - 1);
	assert(capped.recall == 1.0);

	bool rejected_zero_neighbors = false;
	try {
		(void)mtrc::modify::map::neighbor_preservation(space, identity_mapping, 0);
	} catch (const std::invalid_argument &) {
		rejected_zero_neighbors = true;
	}
	assert(rejected_zero_neighbors);

	auto filtered_mapping = identity_mapping;
	filtered_mapping.source_records.pop_back();
	bool rejected_bad_lineage = false;
	try {
		(void)mtrc::modify::map::neighbor_preservation(space, filtered_mapping, 1);
	} catch (const std::invalid_argument &) {
		rejected_bad_lineage = true;
	}
	assert(rejected_bad_lineage);

	// np2: multi-neighbor (k=2) recall exercises the possible>1 aggregation path
	// that every k=1 fixture skips. Distorting exactly the first record's value
	// drops its two nearest neighbours entirely while the interior rows keep both,
	// giving a hand-computable partial recall.
	//
	//   source 2-NN: r0:{r1,r2} r1:{r0,r2} r2:{r1,r3} r3:{r2,r4} r4:{r3,r2}
	//   mapped 2-NN: r0:{r4,r3} r1:{r2,r3} r2:{r1,r3} r3:{r2,r4} r4:{r3,r2}
	//   row matches:  0          1          2          2          2   => 7 of 10
	const std::vector<double> multi_records{0.0, 1.0, 2.0, 3.0, 4.0};
	auto multi_space = mtrc::make_space(multi_records, absolute_distance);
	auto multi_mapping = mtrc::map(
		multi_space, [](double value) { return value == 0.0 ? 100.0 : value; }, absolute_distance);
	const auto multi = mtrc::modify::map::neighbor_preservation(multi_space, multi_mapping, 2);
	assert(multi.evaluated_neighbor_count == 2);
	assert(multi.possible_neighbors == multi_records.size() * 2);
	assert(multi.matched_neighbors == 7);
	assert(close(multi.recall, 0.7));
	assert(close(multi.average_row_recall, 0.7));
	assert(multi.minimum_row_recall == 0.0);
	assert(multi.maximum_row_recall == 1.0);

	return 0;
}
