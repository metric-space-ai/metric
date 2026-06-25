#include <cassert>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "metric/engine.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct PositionCountingProvider {
	using distance_type = int;

	std::vector<mtrc::RecordId> ids;
	int *id_distance_calls{};
	int *position_distance_calls{};

	auto distance(mtrc::RecordId lhs, mtrc::RecordId rhs) const -> distance_type
	{
		++(*id_distance_calls);
		return distance_for_positions(position_of(lhs), position_of(rhs));
	}

	auto distance_at_position(std::size_t lhs_position, std::size_t rhs_position) const -> distance_type
	{
		++(*position_distance_calls);
		return distance_for_positions(lhs_position, rhs_position);
	}

	auto record_count() const -> std::size_t { return ids.size(); }
	auto id(std::size_t position) const -> mtrc::RecordId { return ids.at(position); }
	auto position_of(mtrc::RecordId id) const -> std::size_t
	{
		for (std::size_t index = 0; index < ids.size(); ++index) {
			if (ids[index] == id) {
				return index;
			}
		}
		throw std::out_of_range("unknown position-counting provider id");
	}
	auto contains(mtrc::RecordId id) const -> bool
	{
		for (const auto candidate : ids) {
			if (candidate == id) {
				return true;
			}
		}
		return false;
	}
	auto version() const -> std::size_t { return 13; }
	auto is_stale() const -> bool { return false; }

	static auto distance_for_positions(std::size_t lhs_position, std::size_t rhs_position) -> distance_type
	{
		return lhs_position > rhs_position
				   ? static_cast<distance_type>(lhs_position - rhs_position)
				   : static_cast<distance_type>(rhs_position - lhs_position);
	}
};

int main()
{
	const auto manual = mtrc::core::make_structure_description(5, 10, 1, 1, 4, 2.0, 1.5, true,
																 "manual_description", false);
	assert(manual.record_count == 5);
	assert(manual.pair_count == 10);
	assert(manual.zero_distance_pair_count == 1);
	assert(manual.has_nonzero_distances);
	assert(manual.minimum_nonzero_distance == 1);
	assert(manual.maximum_distance == 4);
	assert(std::abs(manual.average_distance - 2.0) < 1e-12);
	assert(std::abs(manual.intrinsic_dimension - 1.5) < 1e-12);
	assert(!manual.exact);
	assert(manual.operator_name == "describe_structure");
	assert(manual.strategy == "exact_all_pairs");
	assert(manual.representation == "manual_description");

	const auto empty_manual = mtrc::core::make_structure_description(1, 0, 0, 0, 0, 0.0, 0.0, false,
																	   "empty_description");
	assert(empty_manual.record_count == 1);
	assert(empty_manual.pair_count == 0);
	assert(empty_manual.zero_distance_pair_count == 0);
	assert(!empty_manual.has_nonzero_distances);
	assert(empty_manual.exact);
	assert(empty_manual.representation == "empty_description");

	const auto line = mtrc::make_space(std::vector<int>{0, 1, 2, 3, 4}, AbsoluteDistance{});
	const auto description = mtrc::describe_structure(line);

	assert(description.record_count == 5);
	assert(description.pair_count == 10);
	assert(description.zero_distance_pair_count == 0);
	assert(description.has_nonzero_distances);
	assert(description.minimum_nonzero_distance == 1);
	assert(description.maximum_distance == 4);
	assert(std::abs(description.average_distance - 2.0) < 1e-12);
	assert(std::abs(description.intrinsic_dimension - (std::log(5.0 / 3.0) / std::log(2.0))) < 1e-12);
	assert(description.exact);
	assert(description.operator_name == "describe_structure");
	assert(description.strategy == "exact_all_pairs");
	assert(description.representation == "metric_space");

	const auto materialized_policy = mtrc::space::storage::materialized(mtrc::space::storage::exact());
	const auto materialized = mtrc::describe_structure(line, materialized_policy);
	assert(materialized.representation == "distance_table");
	assert(materialized.record_count == description.record_count);
	assert(materialized.pair_count == description.pair_count);
	assert(materialized.zero_distance_pair_count == description.zero_distance_pair_count);
	assert(materialized.minimum_nonzero_distance == description.minimum_nonzero_distance);
		assert(materialized.maximum_distance == description.maximum_distance);
		assert(std::abs(materialized.average_distance - description.average_distance) < 1e-12);
		assert(std::abs(materialized.intrinsic_dimension - description.intrinsic_dimension) < 1e-12);

		std::vector<int> larger_records;
		for (int value = 0; value < 12; ++value) {
			larger_records.push_back(value);
		}
		const auto larger_line = mtrc::make_space(larger_records, AbsoluteDistance{});
		mtrc::describe_options bounded_options;
		bounded_options.max_exact_intrinsic_records = 4;
		bounded_options.intrinsic_sample_count = 3;
		bounded_options.intrinsic_radius_sample_count = 3;
		const auto bounded_description = mtrc::describe_structure(larger_line, bounded_options);
		assert(bounded_description.record_count == larger_records.size());
		assert(bounded_description.pair_count == 66);
		assert(bounded_description.zero_distance_pair_count == 0);
		assert(bounded_description.minimum_nonzero_distance == 1);
		assert(bounded_description.maximum_distance == 11);
		assert(std::abs(bounded_description.average_distance - (13.0 / 3.0)) < 1e-12);
		assert(!bounded_description.exact);
		assert(bounded_description.strategy == "exact_pairs_sampled_intrinsic_dimension");
		assert(bounded_description.representation == "metric_space");

		mtrc::describe_options pair_bounded_options;
		pair_bounded_options.max_exact_pair_summary_records = 4;
		pair_bounded_options.pair_summary_sample_count = 6;
		pair_bounded_options.max_exact_intrinsic_records = 4;
		pair_bounded_options.intrinsic_sample_count = 3;
		pair_bounded_options.intrinsic_radius_sample_count = 3;
		const auto pair_bounded_description = mtrc::describe_structure(larger_line, pair_bounded_options);
		assert(pair_bounded_description.record_count == larger_records.size());
		assert(pair_bounded_description.pair_count == 66);
		assert(pair_bounded_description.zero_distance_pair_count == 0);
		assert(pair_bounded_description.minimum_nonzero_distance == 1);
		assert(pair_bounded_description.maximum_distance == 7);
		assert(std::abs(pair_bounded_description.average_distance - (20.0 / 6.0)) < 1e-12);
		assert(!pair_bounded_description.exact);
		assert(pair_bounded_description.strategy == "sampled_pairs_sampled_intrinsic_dimension");

		bounded_options.allow_approximate_intrinsic_dimension = false;
		const auto forced_exact_description = mtrc::describe_structure(larger_line, bounded_options);
		assert(forced_exact_description.exact);
		assert(forced_exact_description.strategy == "exact_all_pairs");

		const auto duplicate = mtrc::make_space(std::vector<int>{0, 0, 1}, AbsoluteDistance{});
		const auto duplicate_description = mtrc::describe_structure(duplicate);
	assert(duplicate_description.record_count == 3);
	assert(duplicate_description.pair_count == 3);
	assert(duplicate_description.zero_distance_pair_count == 1);
	assert(duplicate_description.minimum_nonzero_distance == 1);
	assert(duplicate_description.maximum_distance == 1);

	int exact_id_distance_calls = 0;
	int exact_position_distance_calls = 0;
	const PositionCountingProvider exact_counting_provider{
		{mtrc::RecordId::from_index(10), mtrc::RecordId::from_index(20), mtrc::RecordId::from_index(30)},
		&exact_id_distance_calls, &exact_position_distance_calls};
	const auto exact_counting_description = mtrc::describe_structure(exact_counting_provider);
	assert(exact_counting_description.record_count == 3);
	assert(exact_counting_description.pair_count == 3);
	assert(exact_counting_description.maximum_distance == 2);
	assert(exact_counting_description.representation == "pairwise_distances");
	assert(exact_id_distance_calls == 0);
	assert(exact_position_distance_calls == 6);

	int sampled_id_distance_calls = 0;
	int sampled_position_distance_calls = 0;
	const PositionCountingProvider sampled_counting_provider{
		{mtrc::RecordId::from_index(10), mtrc::RecordId::from_index(20), mtrc::RecordId::from_index(30),
		 mtrc::RecordId::from_index(40), mtrc::RecordId::from_index(50)},
		&sampled_id_distance_calls, &sampled_position_distance_calls};
	mtrc::describe_options sampled_counting_options;
	sampled_counting_options.max_exact_intrinsic_records = 2;
	sampled_counting_options.intrinsic_sample_count = 2;
	sampled_counting_options.intrinsic_radius_sample_count = 2;
	const auto sampled_counting_description =
		mtrc::describe_structure(sampled_counting_provider, sampled_counting_options);
	assert(sampled_counting_description.record_count == 5);
	assert(sampled_counting_description.pair_count == 10);
	assert(sampled_counting_description.maximum_distance == 4);
	assert(sampled_counting_description.strategy == "exact_pairs_sampled_intrinsic_dimension");
	assert(sampled_id_distance_calls == 0);
	assert(sampled_position_distance_calls == 20);

	bool rejected_approximate_runtime = false;
	try {
		(void)mtrc::describe_structure(line, mtrc::space::storage::approximate());
	} catch (const std::invalid_argument &) {
		rejected_approximate_runtime = true;
	}
	assert(rejected_approximate_runtime);

	return 0;
}
