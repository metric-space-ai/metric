#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "metric/modify/resample/thin.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct CountingAbsoluteDistance {
	std::shared_ptr<std::size_t> calls;

	auto operator()(int lhs, int rhs) const -> int
	{
		++*calls;
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
};

struct EditDistance {
	auto operator()(const std::string &lhs, const std::string &rhs) const -> std::size_t
	{
		std::vector<std::size_t> previous(rhs.size() + 1);
		for (std::size_t index = 0; index < previous.size(); ++index) {
			previous[index] = index;
		}
		for (std::size_t lhs_index = 0; lhs_index < lhs.size(); ++lhs_index) {
			std::vector<std::size_t> current(rhs.size() + 1);
			current[0] = lhs_index + 1;
			for (std::size_t rhs_index = 0; rhs_index < rhs.size(); ++rhs_index) {
				const auto substitute = previous[rhs_index] + (lhs[lhs_index] == rhs[rhs_index] ? 0 : 1);
				const auto insert = current[rhs_index] + 1;
				const auto erase = previous[rhs_index + 1] + 1;
				current[rhs_index + 1] = std::min(std::min(substitute, insert), erase);
			}
			previous = std::move(current);
		}
		return previous.back();
	}
};

struct HistogramWasserstein1D {
	auto operator()(const std::vector<double> &lhs, const std::vector<double> &rhs) const -> double
	{
		if (lhs.size() != rhs.size()) {
			throw std::invalid_argument("histogram fixture dimensions must match");
		}
		double cumulative = 0.0;
		double total = 0.0;
		for (std::size_t index = 0; index < lhs.size(); ++index) {
			cumulative += lhs[index] - rhs[index];
			total += std::abs(cumulative);
		}
		return total;
	}
};

struct CustomerRecord {
	std::string id;
	std::string status;
	double temperature_c{};
	std::vector<std::string> events;
};

struct CustomerDistance {
	auto operator()(const CustomerRecord &lhs, const CustomerRecord &rhs) const -> double
	{
		const auto width = lhs.events.size() > rhs.events.size() ? lhs.events.size() : rhs.events.size();
		double event_penalty = 0.0;
		for (std::size_t index = 0; index < width; ++index) {
			const auto lhs_event = index < lhs.events.size() ? lhs.events[index] : std::string{};
			const auto rhs_event = index < rhs.events.size() ? rhs.events[index] : std::string{};
			if (lhs_event != rhs_event) {
				event_penalty += 1.0;
			}
		}
		const auto status_penalty = lhs.status == rhs.status ? 0.0 : 10.0;
		const auto temperature_penalty = std::abs(lhs.temperature_c - rhs.temperature_c) / 10.0;
		return status_penalty + temperature_penalty + event_penalty;
	}
};

auto close(double lhs, double rhs) -> bool { return std::abs(lhs - rhs) < 1.0e-9; }

auto line_records(std::size_t count) -> std::vector<int>
{
	std::vector<int> records;
	records.reserve(count);
	for (std::size_t index = 0; index < count; ++index) {
		records.push_back(static_cast<int>(index));
	}
	return records;
}

#define REQUIRE(condition)                                                                                              \
	do {                                                                                                                \
		if (!(condition)) {                                                                                            \
			std::fprintf(stderr, "%s:%d: requirement failed: %s\n", __FILE__, __LINE__, #condition);                  \
			return 1;                                                                                                  \
		}                                                                                                             \
	} while (false)

template <typename Function> auto rejects(Function &&function) -> bool
{
	try {
		function();
	} catch (const std::invalid_argument &) {
		return true;
	}
	return false;
}

int main()
{
	const auto space = mtrc::make_space(std::vector<int>{0, 1, 2, 8, 9, 30}, AbsoluteDistance{});

	const auto thinned = mtrc::modify::resample::thin(space, 3);
	REQUIRE(thinned.mapping == "thin");
	REQUIRE(thinned.strategy == "preserve_distribution_regular");
	REQUIRE(thinned.source_record_count == space.size());
	REQUIRE(thinned.space.size() == 3);
	REQUIRE(thinned.space.record(thinned.space.id(0)) == 0);
	REQUIRE(thinned.space.record(thinned.space.id(1)) == 2);
	REQUIRE(thinned.space.record(thinned.space.id(2)) == 9);
	REQUIRE(thinned.source_records[0][0] == space.id(0));
	REQUIRE(thinned.source_records[1][0] == space.id(2));
	REQUIRE(thinned.source_records[2][0] == space.id(4));
	REQUIRE(thinned.representative_records[0] == space.id(0));
	REQUIRE(thinned.representative_records[1] == space.id(2));
	REQUIRE(thinned.representative_records[2] == space.id(4));
	REQUIRE(!thinned.has_assignment_summary);
	REQUIRE(thinned.assignments.empty());
	REQUIRE(thinned.nearest_representative_distances.empty());
	REQUIRE(thinned.representative_multiplicities.empty());
	REQUIRE(thinned.representative_weights.empty());
	REQUIRE(thinned.metric_status == mtrc::core::metric_traits<AbsoluteDistance>::law);
	REQUIRE(thinned.validity.find("distribution-preserving deterministic thinning") != std::string::npos);
	REQUIRE(thinned.validity.find("no full-source assignment map") != std::string::npos);

	const auto offset_thinned = mtrc::distribution_sample(space, 3, mtrc::preserve_distribution(1));
	REQUIRE(offset_thinned.space.record(offset_thinned.space.id(0)) == 1);
	REQUIRE(offset_thinned.space.record(offset_thinned.space.id(1)) == 8);
	REQUIRE(offset_thinned.space.record(offset_thinned.space.id(2)) == 30);
	REQUIRE(offset_thinned.source_records[0][0] == space.id(1));
	REQUIRE(offset_thinned.source_records[1][0] == space.id(3));
	REQUIRE(offset_thinned.source_records[2][0] == space.id(5));

	const auto uniform = mtrc::thin(space, mtrc::uniform_density(1));
	REQUIRE(uniform.mapping == "thin");
	REQUIRE(uniform.strategy == "uniform_density_radius_net");
	REQUIRE(uniform.space.size() == 4);
	REQUIRE(uniform.space.record(uniform.space.id(0)) == 0);
	REQUIRE(uniform.space.record(uniform.space.id(1)) == 2);
	REQUIRE(uniform.space.record(uniform.space.id(2)) == 8);
	REQUIRE(uniform.space.record(uniform.space.id(3)) == 30);
	REQUIRE(uniform.validity.find("uniform-density deterministic thinning") != std::string::npos);
	REQUIRE(uniform.has_assignment_summary);
	REQUIRE((uniform.assignments == std::vector<std::size_t>{0, 0, 1, 2, 2, 3}));
	REQUIRE(uniform.nearest_representative_distances.size() == space.size());
	REQUIRE(close(static_cast<double>(uniform.nearest_representative_distances[1]), 1.0));
	REQUIRE((uniform.representative_multiplicities == std::vector<std::size_t>{2, 1, 2, 1}));
	REQUIRE(uniform.representative_weights.size() == 4);
	REQUIRE(close(uniform.representative_weights[0], 2.0 / 6.0));
	REQUIRE(close(uniform.representative_weights[1], 1.0 / 6.0));
	REQUIRE(close(static_cast<double>(uniform.coverage_radius), 1.0));
	REQUIRE(close(uniform.average_assignment_distance, 2.0 / 6.0));
	REQUIRE(uniform.diagnostics.populated);
	REQUIRE(uniform.diagnostics.diagnostic == "uniform_density_thinning");
	REQUIRE(uniform.diagnostics.policy == "maximal_radius_net");
	REQUIRE(uniform.diagnostics.source_record_count == space.size());
	REQUIRE(uniform.diagnostics.target_record_count == uniform.space.size());
	REQUIRE(uniform.diagnostics.coverage_radius == 1.0);
	REQUIRE(uniform.diagnostics.average_assignment_distance >= 0.0);
	REQUIRE(uniform.diagnostics.local_volume_radius == 1.0);
	REQUIRE(close(uniform.diagnostics.source_average_local_volume_count, 2.0));
	REQUIRE(close(uniform.diagnostics.target_average_local_volume_count, 1.0));
	REQUIRE(close(uniform.diagnostics.local_volume_count_drift, -1.0));
	REQUIRE(close(uniform.diagnostics.source_average_local_volume_density, 1.0 / 3.0));
	REQUIRE(close(uniform.diagnostics.target_average_local_volume_density, 0.25));
	REQUIRE(close(uniform.diagnostics.local_volume_density_drift, -1.0 / 12.0));
	REQUIRE(!uniform.diagnostics.empirical_density_preserved);

	const auto equalized = mtrc::equalize(space, mtrc::uniform_density(1));
	REQUIRE(equalized.mapping == "equalize");
	REQUIRE(equalized.strategy == "uniform_density_radius_net");
	REQUIRE(equalized.space.size() == uniform.space.size());
	REQUIRE(equalized.space.record(equalized.space.id(0)) == 0);
	REQUIRE(equalized.space.record(equalized.space.id(1)) == 2);
	REQUIRE(equalized.space.record(equalized.space.id(2)) == 8);
	REQUIRE(equalized.space.record(equalized.space.id(3)) == 30);
	REQUIRE(equalized.validity.find("density-equalizing deterministic thinning") != std::string::npos);
	REQUIRE(equalized.has_assignment_summary);
	REQUIRE(equalized.assignments == uniform.assignments);
	REQUIRE(equalized.representative_multiplicities == uniform.representative_multiplicities);
	REQUIRE(equalized.representative_weights == uniform.representative_weights);
	REQUIRE(equalized.diagnostics.populated);
	REQUIRE(equalized.diagnostics.target_record_count == equalized.space.size());
	REQUIRE(close(equalized.diagnostics.local_volume_density_drift, -1.0 / 12.0));

	const auto chunked_records = line_records(48);
	auto chunked_calls = std::make_shared<std::size_t>(0);
	const auto chunked_space = mtrc::make_space(chunked_records, CountingAbsoluteDistance{chunked_calls});
	auto chunked_policy = mtrc::space::storage::using_distance_table();
	chunked_policy = mtrc::space::storage::with_distance_table_budget(chunked_policy, 8, 0);
	chunked_policy = mtrc::space::storage::allow_approximate_fallback(chunked_policy);
	chunked_policy = mtrc::space::storage::allow_chunking_fallback(chunked_policy);
	const auto chunked_plan = mtrc::space::storage::estimate_cost(chunked_space, "resample", chunked_policy);
	REQUIRE(chunked_plan.allowed);
	REQUIRE(chunked_plan.downgraded);
	REQUIRE(!chunked_plan.exact);
	REQUIRE(chunked_plan.representation == "chunked_space_view");
	const auto chunked_uniform = mtrc::thin(chunked_space, mtrc::uniform_density(8), chunked_policy);
	REQUIRE(chunked_uniform.mapping == "thin");
	REQUIRE(chunked_uniform.strategy == "chunked_uniform_density_radius_net");
	REQUIRE(chunked_uniform.representation == "chunked_space_view");
	REQUIRE(chunked_uniform.source_record_count == chunked_space.size());
	REQUIRE(chunked_uniform.has_assignment_summary);
	REQUIRE(chunked_uniform.assignments.size() == chunked_space.size());
	REQUIRE(chunked_uniform.diagnostics.populated);
	REQUIRE(chunked_uniform.diagnostics.policy == "chunked_radius_net");
	REQUIRE(*chunked_calls < chunked_space.size() * (chunked_space.size() - 1) / 2);

	const auto sampled_records = line_records(80);
	auto sampled_calls = std::make_shared<std::size_t>(0);
	const auto sampled_space = mtrc::make_space(sampled_records, CountingAbsoluteDistance{sampled_calls});
	auto sampled_policy = mtrc::space::storage::using_knn_graph(16);
	sampled_policy = mtrc::space::storage::with_distance_table_budget(sampled_policy, 8, 0);
	sampled_policy = mtrc::space::storage::allow_approximate_fallback(sampled_policy);
	const auto sampled_plan = mtrc::space::storage::estimate_cost(
		sampled_space, "resample", mtrc::space::storage::using_distance_table(sampled_policy));
	REQUIRE(sampled_plan.allowed);
	REQUIRE(sampled_plan.downgraded);
	REQUIRE(!sampled_plan.exact);
	REQUIRE(sampled_plan.representation == "sampled_metric_space");
	const auto sampled_uniform = mtrc::equalize(sampled_space, mtrc::uniform_density(8), sampled_policy);
	REQUIRE(sampled_uniform.mapping == "equalize");
	REQUIRE(sampled_uniform.strategy == "sampled_uniform_density_radius_net");
	REQUIRE(sampled_uniform.representation == "sampled_metric_space");
	REQUIRE(sampled_uniform.source_record_count == sampled_space.size());
	REQUIRE(sampled_uniform.has_assignment_summary);
	REQUIRE(sampled_uniform.assignments.size() == sampled_space.size());
	REQUIRE(sampled_uniform.diagnostics.populated);
	REQUIRE(sampled_uniform.diagnostics.policy == "sampled_radius_net");
	REQUIRE(*sampled_calls < sampled_space.size() * (sampled_space.size() - 1) / 2);

	const auto large_records = line_records(4097);
	auto large_calls = std::make_shared<std::size_t>(0);
	const auto large_space = mtrc::make_space(large_records, CountingAbsoluteDistance{large_calls});
	const auto large_uniform = mtrc::uniform_density_sample(large_space, mtrc::uniform_density(128));
	REQUIRE(large_uniform.strategy == "sampled_uniform_density_radius_net");
	REQUIRE(large_uniform.representation == "sampled_metric_space");
	REQUIRE(large_uniform.source_record_count == large_space.size());
	REQUIRE(large_uniform.has_assignment_summary);
	REQUIRE(large_uniform.assignments.size() == large_space.size());
	REQUIRE(large_uniform.diagnostics.policy == "sampled_radius_net");
	REQUIRE(*large_calls < large_space.size() * (large_space.size() - 1) / 2);

	const auto empty_thinned = mtrc::thin(space, 0);
	REQUIRE(empty_thinned.empty());
	REQUIRE(empty_thinned.source_record_count == space.size());
	REQUIRE(empty_thinned.metric_status == mtrc::core::metric_traits<AbsoluteDistance>::law);

	const auto empty_space = mtrc::make_space(std::vector<int>{}, AbsoluteDistance{});
	const auto empty_uniform = mtrc::thin(empty_space, mtrc::uniform_density(1));
	REQUIRE(empty_uniform.mapping == "thin");
	REQUIRE(empty_uniform.strategy == "uniform_density_radius_net");
	REQUIRE(empty_uniform.empty());
	REQUIRE(empty_uniform.source_record_count == 0);
	REQUIRE(empty_uniform.has_assignment_summary);
	REQUIRE(empty_uniform.assignments.empty());
	REQUIRE(empty_uniform.nearest_representative_distances.empty());
	REQUIRE(empty_uniform.representative_multiplicities.empty());
	REQUIRE(empty_uniform.representative_weights.empty());
	REQUIRE(close(static_cast<double>(empty_uniform.coverage_radius), 0.0));
	REQUIRE(close(empty_uniform.average_assignment_distance, 0.0));
	REQUIRE(empty_uniform.diagnostics.populated);
	REQUIRE(empty_uniform.diagnostics.policy == "maximal_radius_net");
	REQUIRE(empty_uniform.diagnostics.source_record_count == 0);
	REQUIRE(empty_uniform.diagnostics.target_record_count == 0);
	REQUIRE(close(empty_uniform.diagnostics.coverage_radius, 0.0));
	REQUIRE(close(empty_uniform.diagnostics.average_assignment_distance, 0.0));

	const auto empty_equalized = mtrc::equalize(empty_space, mtrc::uniform_density(1));
	REQUIRE(empty_equalized.mapping == "equalize");
	REQUIRE(empty_equalized.empty());
	REQUIRE(empty_equalized.has_assignment_summary);
	REQUIRE(empty_equalized.diagnostics.populated);
	REQUIRE(empty_equalized.diagnostics.target_record_count == 0);

	const auto empty_sampled =
		mtrc::uniform_density_sample(empty_space, mtrc::uniform_density(1), mtrc::space::storage::approximate());
	REQUIRE(empty_sampled.mapping == "thin");
	REQUIRE(empty_sampled.strategy == "sampled_uniform_density_radius_net");
	REQUIRE(empty_sampled.empty());
	REQUIRE(empty_sampled.has_assignment_summary);
	REQUIRE(empty_sampled.diagnostics.policy == "sampled_radius_net");

	REQUIRE(rejects([&] { (void)mtrc::thin(space, space.size() + 1); }));
	REQUIRE(rejects([&] { (void)mtrc::thin(space, 1, mtrc::preserve_distribution(space.size())); }));

	const auto duplicated_tail = mtrc::make_space(std::vector<int>{0, 0, 0, 1, 2, 50}, AbsoluteDistance{});
	const auto preserve_duplicates = mtrc::thin(duplicated_tail, 3);
	REQUIRE(preserve_duplicates.strategy == "preserve_distribution_regular");
	REQUIRE(preserve_duplicates.space.size() == 3);
	REQUIRE(preserve_duplicates.space.record(preserve_duplicates.space.id(0)) == 0);
	REQUIRE(preserve_duplicates.space.record(preserve_duplicates.space.id(1)) == 0);
	REQUIRE(preserve_duplicates.space.record(preserve_duplicates.space.id(2)) == 2);
	REQUIRE(preserve_duplicates.source_records[0][0] == duplicated_tail.id(0));
	REQUIRE(preserve_duplicates.source_records[1][0] == duplicated_tail.id(2));
	REQUIRE(preserve_duplicates.source_records[2][0] == duplicated_tail.id(4));
	REQUIRE(!preserve_duplicates.diagnostics.populated);

	const auto uniform_duplicates = mtrc::thin(duplicated_tail, mtrc::uniform_density(0));
	REQUIRE(uniform_duplicates.strategy == "uniform_density_radius_net");
	REQUIRE(uniform_duplicates.space.size() == 4);
	REQUIRE(uniform_duplicates.space.record(uniform_duplicates.space.id(0)) == 0);
	REQUIRE(uniform_duplicates.space.record(uniform_duplicates.space.id(1)) == 1);
	REQUIRE(uniform_duplicates.space.record(uniform_duplicates.space.id(2)) == 2);
	REQUIRE(uniform_duplicates.space.record(uniform_duplicates.space.id(3)) == 50);
	REQUIRE(uniform_duplicates.source_records[0][0] == duplicated_tail.id(0));
	REQUIRE(uniform_duplicates.source_records[1][0] == duplicated_tail.id(3));
	REQUIRE(uniform_duplicates.source_records[2][0] == duplicated_tail.id(4));
	REQUIRE(uniform_duplicates.source_records[3][0] == duplicated_tail.id(5));
	REQUIRE(uniform_duplicates.has_assignment_summary);
	REQUIRE((uniform_duplicates.assignments == std::vector<std::size_t>{0, 0, 0, 1, 2, 3}));
	REQUIRE((uniform_duplicates.representative_multiplicities == std::vector<std::size_t>{3, 1, 1, 1}));
	REQUIRE(close(uniform_duplicates.representative_weights[0], 0.5));
	REQUIRE(uniform_duplicates.diagnostics.populated);
	REQUIRE(uniform_duplicates.diagnostics.coverage_radius == 0.0);
	REQUIRE(close(uniform_duplicates.diagnostics.average_assignment_distance, 0.0));
	REQUIRE(close(uniform_duplicates.diagnostics.source_average_local_volume_count, 2.0));
	REQUIRE(close(uniform_duplicates.diagnostics.target_average_local_volume_count, 1.0));
	REQUIRE(close(uniform_duplicates.diagnostics.local_volume_count_drift, -1.0));
	REQUIRE(close(uniform_duplicates.diagnostics.source_average_local_volume_density, 1.0 / 3.0));
	REQUIRE(close(uniform_duplicates.diagnostics.target_average_local_volume_density, 0.25));
	REQUIRE(close(uniform_duplicates.diagnostics.local_volume_density_drift, -1.0 / 12.0));
	REQUIRE(!uniform_duplicates.diagnostics.empirical_density_preserved);

	const auto equalized_duplicates = mtrc::equalize(duplicated_tail, mtrc::uniform_density(0));
	REQUIRE(equalized_duplicates.mapping == "equalize");
	REQUIRE(equalized_duplicates.space.size() == uniform_duplicates.space.size());
	REQUIRE(equalized_duplicates.source_records == uniform_duplicates.source_records);
	REQUIRE(close(equalized_duplicates.diagnostics.local_volume_density_drift, -1.0 / 12.0));

	const auto words = mtrc::make_space(std::vector<std::string>{"cat", "cot", "coat", "dog"}, EditDistance{});
	const auto word_uniform = mtrc::thin(words, mtrc::uniform_density(1));
	REQUIRE(word_uniform.strategy == "uniform_density_radius_net");
	REQUIRE(word_uniform.space.size() == 2);
	REQUIRE(word_uniform.space.record(word_uniform.space.id(0)) == "cat");
	REQUIRE(word_uniform.space.record(word_uniform.space.id(1)) == "dog");
	REQUIRE(word_uniform.source_records[0][0] == words.id(0));
	REQUIRE(word_uniform.source_records[1][0] == words.id(3));
	REQUIRE(word_uniform.has_assignment_summary);
	REQUIRE((word_uniform.assignments == std::vector<std::size_t>{0, 0, 0, 1}));
	REQUIRE((word_uniform.representative_multiplicities == std::vector<std::size_t>{3, 1}));
	REQUIRE(close(word_uniform.representative_weights[0], 0.75));
	REQUIRE(close(word_uniform.representative_weights[1], 0.25));
	REQUIRE(word_uniform.diagnostics.populated);
	REQUIRE(word_uniform.diagnostics.coverage_radius == 1.0);
	REQUIRE(close(word_uniform.diagnostics.average_assignment_distance, 0.5));
	REQUIRE(close(word_uniform.diagnostics.source_average_local_volume_count, 2.5));
	REQUIRE(close(word_uniform.diagnostics.target_average_local_volume_count, 1.0));
	REQUIRE(close(word_uniform.diagnostics.local_volume_count_drift, -1.5));
	REQUIRE(close(word_uniform.diagnostics.source_average_local_volume_density, 0.625));
	REQUIRE(close(word_uniform.diagnostics.target_average_local_volume_density, 0.5));
	REQUIRE(close(word_uniform.diagnostics.local_volume_density_drift, -0.125));
	REQUIRE(!word_uniform.diagnostics.empirical_density_preserved);

	const auto histograms = mtrc::make_space(
		std::vector<std::vector<double>>{{1.0, 0.0, 0.0, 0.0}, {0.0, 1.0, 0.0, 0.0},
										 {0.0, 0.0, 0.0, 1.0}, {0.5, 0.5, 0.0, 0.0},
										 {0.0, 0.5, 0.5, 0.0}},
		HistogramWasserstein1D{});
	const auto histogram_uniform = mtrc::uniform_density_sample(histograms, mtrc::uniform_density(1.0));
	REQUIRE(histogram_uniform.space.size() == 3);
	REQUIRE(histogram_uniform.source_records[0][0] == histograms.id(0));
	REQUIRE(histogram_uniform.source_records[1][0] == histograms.id(2));
	REQUIRE(histogram_uniform.source_records[2][0] == histograms.id(4));
	REQUIRE(close(histogram_uniform.diagnostics.coverage_radius, 0.5));
	REQUIRE(close(histogram_uniform.diagnostics.average_assignment_distance, 0.2));
	REQUIRE(close(histogram_uniform.diagnostics.source_average_local_volume_count, 3.0));
	REQUIRE(close(histogram_uniform.diagnostics.target_average_local_volume_count, 1.0));
	REQUIRE(close(histogram_uniform.diagnostics.local_volume_count_drift, -2.0));
	REQUIRE(close(histogram_uniform.diagnostics.source_average_local_volume_density, 0.6));
	REQUIRE(close(histogram_uniform.diagnostics.target_average_local_volume_density, 1.0 / 3.0));
	REQUIRE(close(histogram_uniform.diagnostics.local_volume_density_drift, (1.0 / 3.0) - 0.6));

	const auto customers = mtrc::make_space(
		std::vector<CustomerRecord>{{"pump-a", "ok", 62.0, {"start", "load", "idle"}},
									{"pump-b", "ok", 64.5, {"start", "load", "idle"}},
									{"valve-c", "warn", 82.0, {"start", "alarm", "manual"}},
									{"pump-d", "ok", 61.0, {"start", "load", "stop"}}},
		CustomerDistance{});
	const auto customer_equalized = mtrc::equalize(customers, mtrc::uniform_density(1.0));
	REQUIRE(customer_equalized.mapping == "equalize");
	REQUIRE(customer_equalized.space.size() == 3);
	REQUIRE(customer_equalized.space.record(customer_equalized.space.id(0)).id == "pump-a");
	REQUIRE(customer_equalized.space.record(customer_equalized.space.id(1)).id == "valve-c");
	REQUIRE(customer_equalized.space.record(customer_equalized.space.id(2)).id == "pump-d");
	REQUIRE(close(customer_equalized.diagnostics.coverage_radius, 0.25));
	REQUIRE(close(customer_equalized.diagnostics.average_assignment_distance, 0.0625));
	REQUIRE(close(customer_equalized.diagnostics.source_average_local_volume_count, 1.5));
	REQUIRE(close(customer_equalized.diagnostics.target_average_local_volume_count, 1.0));
	REQUIRE(close(customer_equalized.diagnostics.local_volume_count_drift, -0.5));
	REQUIRE(close(customer_equalized.diagnostics.source_average_local_volume_density, 0.375));
	REQUIRE(close(customer_equalized.diagnostics.target_average_local_volume_density, 1.0 / 3.0));
	REQUIRE(close(customer_equalized.diagnostics.local_volume_density_drift, (1.0 / 3.0) - 0.375));

	return 0;
}
