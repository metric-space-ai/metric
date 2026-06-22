#include <array>
#include <cassert>
#include <cmath>
#include <deque>
#include <iostream>
#include <limits>
#include <vector>

#include <metric/metric/catalog.hpp>
#include <metric/engine.hpp>

namespace {

auto raw_vector_pairing_correct(const std::vector<std::vector<double>> &process_records,
								const std::deque<std::array<float, 1>> &quality_records) -> std::size_t
{
	std::size_t correct = 0;
	for (std::size_t process_index = 0; process_index < process_records.size(); ++process_index) {
		auto best_index = std::size_t{0};
		auto best_distance = std::numeric_limits<double>::infinity();
		for (std::size_t quality_index = 0; quality_index < quality_records.size(); ++quality_index) {
			const auto delta =
				process_records[process_index][0] - static_cast<double>(quality_records[quality_index][0]);
			const auto distance = std::abs(delta);
			if (distance < best_distance) {
				best_distance = distance;
				best_index = quality_index;
			}
		}
		if (best_index == process_index) {
			++correct;
		}
	}
	return correct;
}

} // namespace

int main()
{
	std::vector<std::vector<double>> process_space_records;
	std::deque<std::array<float, 1>> quality_space_records;

	for (int index = 0; index < 12; ++index) {
		const auto process_value = static_cast<double>(index);
		process_space_records.push_back({process_value});
		quality_space_records.push_back({static_cast<float>((process_value * process_value) / 10.0)});
	}

	auto process_space = mtrc::make_space(process_space_records, mtrc::Euclidean<double>());
	auto quality_space = mtrc::make_space(quality_space_records, mtrc::Manhattan<float>());

	const auto compared = mtrc::compare(process_space, quality_space, mtrc::stats::correlate::mgc_options{});
	assert(compared.algorithm == "mgc");
	assert(compared.left_representation == "metric_space");
	assert(compared.right_representation == "metric_space");
	assert(compared.left_record_count == process_space.size());
	assert(compared.right_record_count == quality_space.size());
	assert(compared.exact);
	assert(std::isfinite(compared.value));
	assert(compared.value >= 0.0);

	const auto raw_correct = raw_vector_pairing_correct(process_space_records, quality_space_records);
	assert(raw_correct < process_space_records.size() / 2);
	const auto raw_mismatches = process_space_records.size() - raw_correct;

	std::cout << "cross-space MGC = " << compared.value << "\n";
	std::cout << "cross-space paired records = " << process_space_records.size() << "\n";
	std::cout << "cross-space raw-vector pairing correct = " << raw_correct << "/" << process_space_records.size()
			  << "\n";
	std::cout << "cross-space raw-vector mismatches = " << raw_mismatches << "/" << process_space_records.size()
			  << "\n";

	return 0;
}
