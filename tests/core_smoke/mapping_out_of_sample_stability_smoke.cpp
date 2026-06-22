#include <cassert>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "metric/engine.hpp"

namespace {

auto absolute_distance(double lhs, double rhs) -> double { return std::abs(lhs - rhs); }

struct IdentityModel {
	template <typename Space> auto transform(const Space &space) const
	{
		return mtrc::map(space, [](double value) { return value; }, absolute_distance);
	}
};

struct FoldedModel {
	template <typename Space> auto transform(const Space &space) const
	{
		return mtrc::map(space, [](double value) { return std::abs(value - 7.0); }, absolute_distance);
	}
};

} // namespace

int main()
{
	const std::vector<double> source_records{0.0, 2.0, 5.0, 9.0, 14.0};
	const std::vector<double> query_records{1.0, 4.0, 13.0};
	auto source = mtrc::make_space(source_records, absolute_distance);
	auto query = mtrc::make_space(query_records, absolute_distance);

	const IdentityModel identity_model;
	static_assert(mtrc::MappingModel_v<IdentityModel, decltype(source)>);
	static_assert(mtrc::MappingModel_v<IdentityModel, decltype(query)>);

	const auto identity = mtrc::modify::map::out_of_sample_neighbor_stability(identity_model, source, query, 1);
	assert(identity.diagnostic == "out_of_sample_neighbor_stability");
	assert(identity.source_record_count == source_records.size());
	assert(identity.query_record_count == query_records.size());
	assert(identity.requested_neighbor_count == 1);
	assert(identity.evaluated_neighbor_count == 1);
	assert(identity.evaluated_queries == query_records.size());
	assert(identity.matched_anchor_neighbors == query_records.size());
	assert(identity.possible_anchor_neighbors == query_records.size());
	assert(identity.source_distance_evaluations == source_records.size() * query_records.size());
	assert(identity.mapped_distance_evaluations == source_records.size() * query_records.size());
	assert(identity.transform_supported);
	assert(identity.exact);
	assert(identity.anchor_recall == 1.0);
	assert(identity.average_query_recall == 1.0);
	assert(identity.minimum_query_recall == 1.0);
	assert(identity.maximum_query_recall == 1.0);
	assert(identity.first_anchor_matches == query_records.size());
	assert(identity.first_anchor_match_rate == 1.0);
	assert(identity.average_source_best_distance == 1.0);
	assert(identity.average_mapped_best_source_distance == 1.0);
	assert(identity.average_best_distance_penalty == 0.0);
	assert(identity.maximum_best_distance_penalty == 0.0);
	assert(identity.average_mapped_best_source_rank == 1.0);
	assert(identity.maximum_mapped_best_source_rank == 1);
	assert(identity.mapping == "deterministic_transform");
	assert(identity.strategy == "deterministic_transform");
	assert(identity.source_representation == "metric_space");
	assert(identity.mapped_source_representation == "metric_space");
	assert(identity.mapped_query_representation == "metric_space");

	const FoldedModel folded_model;
	const auto folded = mtrc::modify::map::out_of_sample_neighbor_stability(folded_model, source, query, 1);
	assert(folded.matched_anchor_neighbors < folded.possible_anchor_neighbors);
	assert(folded.anchor_recall < 1.0);
	assert(folded.minimum_query_recall == 0.0);
	assert(folded.first_anchor_matches == 2);
	assert(std::abs(folded.first_anchor_match_rate - (2.0 / 3.0)) < 1.0e-12);
	assert(folded.average_source_best_distance == 1.0);
	assert(folded.average_mapped_best_source_distance == 5.0);
	assert(folded.average_best_distance_penalty == 4.0);
	assert(folded.maximum_best_distance_penalty == 12.0);
	assert(std::abs(folded.average_mapped_best_source_rank - (7.0 / 3.0)) < 1.0e-12);
	assert(folded.maximum_mapped_best_source_rank == 5);

	const auto capped = mtrc::modify::map::out_of_sample_neighbor_stability(identity_model, source, query, 99);
	assert(capped.requested_neighbor_count == 99);
	assert(capped.evaluated_neighbor_count == source_records.size());
	assert(capped.anchor_recall == 1.0);
	assert(capped.first_anchor_match_rate == 1.0);

	bool rejected_zero_neighbors = false;
	try {
		(void)mtrc::modify::map::out_of_sample_neighbor_stability(identity_model, source, query, 0);
	} catch (const std::invalid_argument &) {
		rejected_zero_neighbors = true;
	}
	assert(rejected_zero_neighbors);

	auto empty_query = mtrc::make_space(std::vector<double>{}, absolute_distance);
	bool rejected_empty_query = false;
	try {
		(void)mtrc::modify::map::out_of_sample_neighbor_stability(identity_model, source, empty_query, 1);
	} catch (const std::invalid_argument &) {
		rejected_empty_query = true;
	}
	assert(rejected_empty_query);

	return 0;
}
