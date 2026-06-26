#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "metric/engine.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace py = pybind11;

namespace {

auto checked_distance(py::object metric, py::handle lhs, py::handle rhs, std::size_t lhs_index,
					  std::size_t rhs_index) -> double
{
	py::object raw = metric(lhs, rhs);
	if (PyBool_Check(raw.ptr())) {
		std::ostringstream message;
		message << "metric distance for records " << lhs_index << " and " << rhs_index << " must be a real number";
		throw std::invalid_argument(message.str());
	}

	double value = 0.0;
	try {
		value = raw.cast<double>();
	} catch (const py::cast_error &) {
		std::ostringstream message;
		message << "metric distance for records " << lhs_index << " and " << rhs_index << " must be a real number";
		throw std::invalid_argument(message.str());
	}

	if (!std::isfinite(value)) {
		std::ostringstream message;
		message << "metric distance for records " << lhs_index << " and " << rhs_index << " must be finite";
		throw std::invalid_argument(message.str());
	}
	if (value < 0.0) {
		std::ostringstream message;
		message << "metric distance for records " << lhs_index << " and " << rhs_index << " must be non-negative";
		throw std::invalid_argument(message.str());
	}
	return value;
}

auto checked_query_distance(py::object metric, py::handle query, py::handle record, std::size_t record_index)
	-> double
{
	py::object raw = metric(query, record);
	if (PyBool_Check(raw.ptr())) {
		std::ostringstream message;
		message << "metric distance for query and record " << record_index << " must be a real number";
		throw std::invalid_argument(message.str());
	}

	double value = 0.0;
	try {
		value = raw.cast<double>();
	} catch (const py::cast_error &) {
		std::ostringstream message;
		message << "metric distance for query and record " << record_index << " must be a real number";
		throw std::invalid_argument(message.str());
	}

	if (!std::isfinite(value)) {
		std::ostringstream message;
		message << "metric distance for query and record " << record_index << " must be finite";
		throw std::invalid_argument(message.str());
	}
	if (value < 0.0) {
		std::ostringstream message;
		message << "metric distance for query and record " << record_index << " must be non-negative";
		throw std::invalid_argument(message.str());
	}
	return value;
}

auto sort_neighbor_pairs(std::vector<std::pair<std::size_t, double>> &neighbors) -> void
{
	std::sort(neighbors.begin(), neighbors.end(), [](const auto &lhs, const auto &rhs) {
		if (lhs.second < rhs.second) {
			return true;
		}
		if (rhs.second < lhs.second) {
			return false;
		}
		return lhs.first < rhs.first;
	});
}

auto checked_radius(double radius, const char *name) -> double
{
	if (!std::isfinite(radius) || radius < 0.0) {
		std::ostringstream message;
		message << name << " must be a finite non-negative value";
		throw std::invalid_argument(message.str());
	}
	return radius;
}

auto native_assign_to_representatives(py::sequence records, py::object metric, const std::vector<std::size_t> &representatives)
	-> std::pair<std::vector<std::size_t>, std::vector<double>>
{
	const auto record_count = static_cast<std::size_t>(records.size());
	if (record_count > 0 && representatives.empty()) {
		throw std::invalid_argument("at least one representative is required for a non-empty record set");
	}
	for (const auto representative : representatives) {
		if (representative >= record_count) {
			throw std::out_of_range("representative index is out of range");
		}
	}

	std::vector<std::size_t> assignments(record_count, 0);
	std::vector<double> nearest_distances(record_count, 0.0);
	for (std::size_t record_index = 0; record_index < record_count; ++record_index) {
		auto best_position = std::size_t{0};
		auto best_distance = std::numeric_limits<double>::infinity();
		for (std::size_t position = 0; position < representatives.size(); ++position) {
			const auto representative = representatives[position];
			const auto distance = checked_distance(metric, records[record_index], records[representative], record_index, representative);
			if (distance < best_distance || (distance == best_distance && representative < representatives[best_position])) {
				best_distance = distance;
				best_position = position;
			}
		}
		assignments[record_index] = best_position;
		nearest_distances[record_index] = best_distance;
	}
	return {assignments, nearest_distances};
}

auto checked_matrix_value(py::handle raw, std::size_t row, std::size_t column) -> double
{
	if (PyBool_Check(raw.ptr())) {
		std::ostringstream message;
		message << "distance matrix value at (" << row << ", " << column << ") must be a real number";
		throw std::invalid_argument(message.str());
	}

	double value = 0.0;
	try {
		value = raw.cast<double>();
	} catch (const py::cast_error &) {
		std::ostringstream message;
		message << "distance matrix value at (" << row << ", " << column << ") must be a real number";
		throw std::invalid_argument(message.str());
	}

	if (!std::isfinite(value)) {
		std::ostringstream message;
		message << "distance matrix value at (" << row << ", " << column << ") must be finite";
		throw std::invalid_argument(message.str());
	}
	if (value < 0.0) {
		std::ostringstream message;
		message << "distance matrix value at (" << row << ", " << column << ") must be non-negative";
		throw std::invalid_argument(message.str());
	}
	return value;
}

auto distance_matrix_from_sequence(py::sequence distances) -> std::vector<std::vector<double>>
{
	const auto record_count = static_cast<std::size_t>(distances.size());
	std::vector<std::vector<double>> matrix(record_count, std::vector<double>(record_count, 0.0));
	for (std::size_t row_index = 0; row_index < record_count; ++row_index) {
		auto row = py::reinterpret_borrow<py::sequence>(distances[row_index]);
		if (static_cast<std::size_t>(row.size()) != record_count) {
			std::ostringstream message;
			message << "distance matrix row " << row_index << " has length " << row.size()
					<< ", expected " << record_count;
			throw std::invalid_argument(message.str());
		}
		for (std::size_t column_index = 0; column_index < record_count; ++column_index) {
			matrix[row_index][column_index] = checked_matrix_value(row[column_index], row_index, column_index);
		}
	}
	return matrix;
}

auto expansion_dimension_from_matrix(const std::vector<std::vector<double>> &distances) -> double
{
	const auto record_count = distances.size();
	auto maximum_dimension = 0.0;
	for (std::size_t row = 0; row < record_count; ++row) {
		for (std::size_t column = 0; column < record_count; ++column) {
			const auto radius = distances[row][column];
			if (radius <= 0.0) {
				continue;
			}

			const auto outer_radius = radius + radius;
			auto inner_count = std::size_t{0};
			auto outer_count = std::size_t{0};
			for (std::size_t candidate = 0; candidate < record_count; ++candidate) {
				const auto value = distances[row][candidate];
				if (value <= radius) {
					++inner_count;
				}
				if (value <= outer_radius) {
					++outer_count;
				}
			}

			if (inner_count > 0 && outer_count >= inner_count) {
				maximum_dimension =
					std::max(maximum_dimension,
							 std::log(static_cast<double>(outer_count) / static_cast<double>(inner_count)) /
								 std::log(2.0));
			}
		}
	}
	return maximum_dimension;
}

auto structure_description_from_matrix(const std::vector<std::vector<double>> &distances, const std::string &representation)
	-> py::dict
{
	const auto record_count = distances.size();
	auto pair_count = std::size_t{0};
	auto zero_distance_pair_count = std::size_t{0};
	auto minimum_nonzero_distance = 0.0;
	auto maximum_distance = 0.0;
	auto distance_sum = 0.0;
	auto has_nonzero_distances = false;
	auto has_any_pair = false;

	for (std::size_t row = 0; row < record_count; ++row) {
		for (std::size_t column = row + 1; column < record_count; ++column) {
			const auto distance = distances[row][column];
			++pair_count;
			distance_sum += distance;
			if (!has_any_pair || distance > maximum_distance) {
				maximum_distance = distance;
				has_any_pair = true;
			}
			if (distance <= 0.0) {
				++zero_distance_pair_count;
			} else if (!has_nonzero_distances || distance < minimum_nonzero_distance) {
				minimum_nonzero_distance = distance;
				has_nonzero_distances = true;
			}
		}
	}

	auto result = py::dict{};
	result["record_count"] = record_count;
	result["pair_count"] = pair_count;
	result["zero_distance_pair_count"] = zero_distance_pair_count;
	result["minimum_nonzero_distance"] = minimum_nonzero_distance;
	result["maximum_distance"] = maximum_distance;
	result["average_distance"] = pair_count == 0 ? 0.0 : distance_sum / static_cast<double>(pair_count);
	result["intrinsic_dimension"] = expansion_dimension_from_matrix(distances);
	result["has_nonzero_distances"] = has_nonzero_distances;
	result["exact"] = true;
	result["strategy"] = "exact_all_pairs";
	result["representation"] = representation;
	return result;
}

auto checked_positive_count(std::size_t value, const char *name) -> std::size_t
{
	if (value == 0) {
		std::ostringstream message;
		message << name << " must be positive";
		throw std::invalid_argument(message.str());
	}
	return value;
}

auto checked_positive_iterations(std::size_t value) -> std::size_t
{
	if (value == 0) {
		throw std::invalid_argument("max_iterations must be positive");
	}
	return value;
}

auto cluster_medoids(const std::vector<std::vector<double>> &distances, const std::vector<int> &assignments,
					 std::size_t cluster_count) -> std::vector<std::size_t>
{
	std::vector<std::size_t> medoids(cluster_count, 0);
	for (std::size_t cluster = 0; cluster < cluster_count; ++cluster) {
		auto best_index = distances.size();
		auto best_sum = std::numeric_limits<double>::infinity();
		for (std::size_t candidate = 0; candidate < distances.size(); ++candidate) {
			if (assignments[candidate] != static_cast<int>(cluster)) {
				continue;
			}
			auto sum = 0.0;
			for (std::size_t other = 0; other < distances.size(); ++other) {
				if (assignments[other] == static_cast<int>(cluster)) {
					sum += distances[candidate][other];
				}
			}
			if (sum < best_sum || (sum == best_sum && candidate < best_index)) {
				best_sum = sum;
				best_index = candidate;
			}
		}
		medoids[cluster] = best_index == distances.size() ? 0 : best_index;
	}
	return medoids;
}

auto cluster_sizes_from_assignments(const std::vector<int> &assignments, std::size_t cluster_count) -> std::vector<std::size_t>
{
	std::vector<std::size_t> sizes(cluster_count, 0);
	for (const auto label : assignments) {
		if (label >= 0 && static_cast<std::size_t>(label) < cluster_count) {
			++sizes[static_cast<std::size_t>(label)];
		}
	}
	return sizes;
}

auto clustering_payload(const std::vector<int> &assignments, const std::vector<std::size_t> &medoids,
						const std::vector<std::size_t> &core_records, const std::vector<std::size_t> &unassigned_records,
						std::size_t iterations, bool converged, const std::string &algorithm,
						const std::string &representation) -> py::dict
{
	auto result = py::dict{};
	result["assignments"] = assignments;
	result["medoids"] = medoids;
	result["core_records"] = core_records;
	result["unassigned_records"] = unassigned_records;
	result["cluster_sizes"] = cluster_sizes_from_assignments(assignments, medoids.size());
	result["record_count"] = assignments.size();
	result["cluster_count"] = medoids.size();
	result["unassigned_count"] = unassigned_records.size();
	result["iterations"] = iterations;
	result["converged"] = converged;
	result["algorithm"] = algorithm;
	result["representation"] = representation;
	return result;
}

auto assign_to_medoids(const std::vector<std::vector<double>> &distances, const std::vector<std::size_t> &medoids)
	-> std::vector<int>
{
	std::vector<int> assignments(distances.size(), -1);
	for (std::size_t record = 0; record < distances.size(); ++record) {
		auto best_cluster = std::size_t{0};
		auto best_distance = std::numeric_limits<double>::infinity();
		for (std::size_t cluster = 0; cluster < medoids.size(); ++cluster) {
			const auto distance = distances[record][medoids[cluster]];
			if (distance < best_distance || (distance == best_distance && medoids[cluster] < medoids[best_cluster])) {
				best_distance = distance;
				best_cluster = cluster;
			}
		}
		assignments[record] = static_cast<int>(best_cluster);
	}
	return assignments;
}

auto condensed_distance_profile(py::sequence records, py::object metric) -> std::vector<double>
{
	const auto record_count = static_cast<std::size_t>(records.size());
	std::vector<double> profile;
	if (record_count > 1) {
		profile.reserve(record_count * (record_count - 1) / 2);
	}
	for (std::size_t lhs = 0; lhs < record_count; ++lhs) {
		for (std::size_t rhs = lhs + 1; rhs < record_count; ++rhs) {
			profile.push_back(checked_distance(metric, records[lhs], records[rhs], lhs, rhs));
		}
	}
	return profile;
}

auto farthest_first_from_matrix(const std::vector<std::vector<double>> &distances, std::size_t count) -> std::vector<std::size_t>
{
	const auto record_count = distances.size();
	if (count == 0 || record_count == 0) {
		return {};
	}

	const auto target_count = std::min(count, record_count);
	std::vector<std::size_t> representatives;
	representatives.reserve(target_count);
	std::vector<bool> selected(record_count, false);
	std::vector<double> nearest_distances(record_count, std::numeric_limits<double>::infinity());

	representatives.push_back(0);
	selected[0] = true;
	for (std::size_t index = 0; index < record_count; ++index) {
		nearest_distances[index] = distances[index][0];
	}

	while (representatives.size() < target_count) {
		auto next_index = record_count;
		auto next_distance = -1.0;
		for (std::size_t index = 0; index < record_count; ++index) {
			if (selected[index]) {
				continue;
			}
			if (nearest_distances[index] > next_distance ||
				(nearest_distances[index] == next_distance && (next_index == record_count || index < next_index))) {
				next_distance = nearest_distances[index];
				next_index = index;
			}
		}
		if (next_index == record_count) {
			break;
		}
		representatives.push_back(next_index);
		selected[next_index] = true;
		for (std::size_t index = 0; index < record_count; ++index) {
			if (distances[index][next_index] < nearest_distances[index]) {
				nearest_distances[index] = distances[index][next_index];
			}
		}
	}
	return representatives;
}

} // namespace

auto native_pairwise_distance_matrix(py::sequence records, py::object metric) -> std::vector<std::vector<double>>
{
	const auto record_count = static_cast<std::size_t>(records.size());
	std::vector<std::vector<double>> matrix(record_count, std::vector<double>(record_count, 0.0));
	for (std::size_t lhs = 0; lhs < record_count; ++lhs) {
		matrix[lhs][lhs] = checked_distance(metric, records[lhs], records[lhs], lhs, lhs);
		for (std::size_t rhs = lhs + 1; rhs < record_count; ++rhs) {
			const auto value = checked_distance(metric, records[lhs], records[rhs], lhs, rhs);
			matrix[lhs][rhs] = value;
			matrix[rhs][lhs] = value;
		}
	}
	return matrix;
}

auto native_distance_profile_correlation(py::sequence left_records, py::object left_metric,
										 py::sequence right_records, py::object right_metric,
										 const std::string &left_representation = "records",
										 const std::string &right_representation = "records") -> py::dict
{
	const auto left_count = static_cast<std::size_t>(left_records.size());
	const auto right_count = static_cast<std::size_t>(right_records.size());
	if (left_count != right_count) {
		std::ostringstream message;
		message << "aligned compare requires equal record counts; left has " << left_count
				<< " records and right has " << right_count << " records";
		throw std::invalid_argument(message.str());
	}

	const auto left_profile = condensed_distance_profile(left_records, left_metric);
	const auto right_profile = condensed_distance_profile(right_records, right_metric);
	const auto pair_count = left_profile.size();

	auto left_mean = 0.0;
	auto right_mean = 0.0;
	for (std::size_t index = 0; index < pair_count; ++index) {
		left_mean += left_profile[index];
		right_mean += right_profile[index];
	}
	if (pair_count > 0) {
		left_mean /= static_cast<double>(pair_count);
		right_mean /= static_cast<double>(pair_count);
	}

	auto covariance = 0.0;
	auto left_variance = 0.0;
	auto right_variance = 0.0;
	for (std::size_t index = 0; index < pair_count; ++index) {
		const auto left_delta = left_profile[index] - left_mean;
		const auto right_delta = right_profile[index] - right_mean;
		covariance += left_delta * right_delta;
		left_variance += left_delta * left_delta;
		right_variance += right_delta * right_delta;
	}

	// Pearson correlation is undefined when either distance profile is empty
	// (fewer than two records, so no pairs) or has zero variance (all pairwise
	// distances equal). METRIC's documented deterministic behavior for these
	// degenerate aligned profiles is value=0.0 with defined=false so callers
	// can branch on the diagnostics flag instead of trapping a division error.
	const auto denominator = std::sqrt(left_variance * right_variance);
	auto value = 0.0;
	auto defined = false;
	if (pair_count > 0 && left_variance > 0.0 && right_variance > 0.0 && std::isfinite(denominator) &&
		denominator > 0.0) {
		value = covariance / denominator;
		if (value > 1.0) {
			value = 1.0;
		} else if (value < -1.0) {
			value = -1.0;
		}
		defined = true;
	}

	auto diagnostics = py::dict{};
	diagnostics["defined"] = defined;
	diagnostics["degenerate"] = !defined;
	diagnostics["left_variance"] = left_variance;
	diagnostics["right_variance"] = right_variance;
	diagnostics["left_mean"] = left_mean;
	diagnostics["right_mean"] = right_mean;
	diagnostics["covariance"] = covariance;

	auto result = py::dict{};
	result["value"] = value;
	result["left_record_count"] = left_count;
	result["right_record_count"] = right_count;
	result["pair_count"] = pair_count;
	result["exact"] = true;
	result["defined"] = defined;
	result["algorithm"] = "distance_profile_correlation";
	result["strategy"] = "distance_profile_correlation";
	result["statistic_name"] = "distance_profile_correlation";
	result["method"] = "pearson";
	result["align"] = "position";
	result["left_representation"] = left_representation;
	result["right_representation"] = right_representation;
	result["diagnostics"] = diagnostics;
	return result;
}

auto native_exact_scan_neighbors(py::sequence records, py::object metric, py::object query, std::size_t count)
	-> std::vector<std::pair<std::size_t, double>>
{
	const auto record_count = static_cast<std::size_t>(records.size());
	std::vector<std::pair<std::size_t, double>> neighbors;
	neighbors.reserve(record_count);
	for (std::size_t index = 0; index < record_count; ++index) {
		neighbors.emplace_back(index, checked_query_distance(metric, query, records[index], index));
	}
	sort_neighbor_pairs(neighbors);
	if (neighbors.size() > count) {
		neighbors.resize(count);
	}
	return neighbors;
}

auto native_exact_scan_radius_neighbors(py::sequence records, py::object metric, py::object query, double radius)
	-> std::vector<std::pair<std::size_t, double>>
{
	checked_radius(radius, "radius");

	const auto record_count = static_cast<std::size_t>(records.size());
	std::vector<std::pair<std::size_t, double>> neighbors;
	for (std::size_t index = 0; index < record_count; ++index) {
		const auto distance = checked_query_distance(metric, query, records[index], index);
		if (distance <= radius) {
			neighbors.emplace_back(index, distance);
		}
	}
	sort_neighbor_pairs(neighbors);
	return neighbors;
}

auto native_medoid_index(py::sequence records, py::object metric) -> std::size_t
{
	const auto record_count = static_cast<std::size_t>(records.size());
	if (record_count == 0) {
		throw std::invalid_argument("medoid_index requires at least one record");
	}

	auto best_index = std::size_t{0};
	auto best_sum = std::numeric_limits<double>::infinity();
	for (std::size_t candidate = 0; candidate < record_count; ++candidate) {
		auto sum = 0.0;
		for (std::size_t other = 0; other < record_count; ++other) {
			sum += checked_distance(metric, records[candidate], records[other], candidate, other);
		}
		if (sum < best_sum) {
			best_sum = sum;
			best_index = candidate;
		}
	}
	return best_index;
}

auto native_representative_indices(py::sequence records, py::object metric, std::size_t count,
								   std::size_t seed_index = 0) -> std::vector<std::size_t>
{
	const auto record_count = static_cast<std::size_t>(records.size());
	if (count == 0 || record_count == 0) {
		return {};
	}
	if (seed_index >= record_count) {
		throw std::out_of_range("seed_index is out of range");
	}

	const auto target_count = std::min(count, record_count);
	std::vector<std::size_t> representatives;
	representatives.reserve(target_count);
	std::vector<bool> selected(record_count, false);
	std::vector<double> nearest_distances(record_count, std::numeric_limits<double>::infinity());

	representatives.push_back(seed_index);
	selected[seed_index] = true;
	for (std::size_t index = 0; index < record_count; ++index) {
		nearest_distances[index] = checked_distance(metric, records[index], records[seed_index], index, seed_index);
	}

	while (representatives.size() < target_count) {
		auto next_index = record_count;
		auto next_distance = -1.0;
		for (std::size_t index = 0; index < record_count; ++index) {
			if (selected[index]) {
				continue;
			}
			if (nearest_distances[index] > next_distance ||
				(nearest_distances[index] == next_distance && (next_index == record_count || index < next_index))) {
				next_distance = nearest_distances[index];
				next_index = index;
			}
		}
		if (next_index == record_count) {
			break;
		}
		representatives.push_back(next_index);
		selected[next_index] = true;
		for (std::size_t index = 0; index < record_count; ++index) {
			const auto distance = checked_distance(metric, records[index], records[next_index], index, next_index);
			if (distance < nearest_distances[index]) {
				nearest_distances[index] = distance;
			}
		}
	}
	return representatives;
}

auto native_separated_representative_indices(py::sequence records, py::object metric, double minimum_distance)
	-> std::vector<std::size_t>
{
	checked_radius(minimum_distance, "minimum_distance");

	const auto record_count = static_cast<std::size_t>(records.size());
	std::vector<std::size_t> representatives;
	for (std::size_t candidate = 0; candidate < record_count; ++candidate) {
		auto separated = true;
		for (const auto representative : representatives) {
			const auto distance = checked_distance(metric, records[candidate], records[representative], candidate, representative);
			if (distance < minimum_distance) {
				separated = false;
				break;
			}
		}
		if (separated) {
			representatives.push_back(candidate);
		}
	}
	return representatives;
}

auto native_coverage_representative_indices(py::sequence records, py::object metric, double radius)
	-> std::vector<std::size_t>
{
	checked_radius(radius, "radius");

	const auto record_count = static_cast<std::size_t>(records.size());
	std::vector<std::size_t> representatives;
	std::vector<bool> covered(record_count, false);
	auto covered_count = std::size_t{0};
	while (covered_count < record_count) {
		auto representative = record_count;
		for (std::size_t index = 0; index < record_count; ++index) {
			if (!covered[index]) {
				representative = index;
				break;
			}
		}
		if (representative == record_count) {
			break;
		}
		representatives.push_back(representative);
		for (std::size_t index = 0; index < record_count; ++index) {
			if (covered[index]) {
				continue;
			}
			const auto distance = checked_distance(metric, records[index], records[representative], index, representative);
			if (distance <= radius) {
				covered[index] = true;
				++covered_count;
			}
		}
	}
	return representatives;
}

auto native_intrinsic_dimension_from_distances(py::sequence distances) -> double
{
	return expansion_dimension_from_matrix(distance_matrix_from_sequence(distances));
}

auto native_intrinsic_dimension(py::sequence records, py::object metric) -> double
{
	return expansion_dimension_from_matrix(native_pairwise_distance_matrix(records, metric));
}

auto native_describe_structure(py::sequence records, py::object metric, const std::string &representation = "metric_space")
	-> py::dict
{
	return structure_description_from_matrix(native_pairwise_distance_matrix(records, metric), representation);
}

auto native_kmedoids(py::sequence records, py::object metric, std::size_t groups, std::size_t max_iterations,
					 const std::string &representation = "metric_space") -> py::dict
{
	checked_positive_count(groups, "groups");
	checked_positive_iterations(max_iterations);
	const auto distances = native_pairwise_distance_matrix(records, metric);
	const auto record_count = distances.size();
	if (record_count == 0) {
		throw std::invalid_argument("kmedoids requires at least one record");
	}

	std::vector<std::size_t> medoids = farthest_first_from_matrix(distances, std::min(groups, record_count));
	auto assignments = assign_to_medoids(distances, medoids);
	auto converged = false;
	auto iterations = std::size_t{0};
	for (; iterations < max_iterations; ++iterations) {
		const auto updated_medoids = cluster_medoids(distances, assignments, medoids.size());
		const auto updated_assignments = assign_to_medoids(distances, updated_medoids);
		converged = updated_medoids == medoids && updated_assignments == assignments;
		medoids = updated_medoids;
		assignments = updated_assignments;
		if (converged) {
			++iterations;
			break;
		}
	}

	return clustering_payload(assignments, medoids, {}, {}, iterations, converged, "kmedoids", representation);
}

auto native_dbscan(py::sequence records, py::object metric, double radius, std::size_t min_points,
				   const std::string &representation = "metric_space") -> py::dict
{
	checked_radius(radius, "radius");
	checked_positive_count(min_points, "min_points");
	const auto distances = native_pairwise_distance_matrix(records, metric);
	const auto record_count = distances.size();

	std::vector<std::vector<std::size_t>> neighborhoods(record_count);
	std::vector<bool> core(record_count, false);
	for (std::size_t record = 0; record < record_count; ++record) {
		for (std::size_t candidate = 0; candidate < record_count; ++candidate) {
			if (distances[record][candidate] <= radius) {
				neighborhoods[record].push_back(candidate);
			}
		}
		core[record] = neighborhoods[record].size() >= min_points;
	}

	std::vector<int> assignments(record_count, -1);
	auto cluster_count = std::size_t{0};
	for (std::size_t record = 0; record < record_count; ++record) {
		if (!core[record] || assignments[record] != -1) {
			continue;
		}
		const auto label = static_cast<int>(cluster_count);
		++cluster_count;
		std::queue<std::size_t> frontier;
		assignments[record] = label;
		frontier.push(record);
		while (!frontier.empty()) {
			const auto current = frontier.front();
			frontier.pop();
			for (const auto neighbor : neighborhoods[current]) {
				if (assignments[neighbor] == -1) {
					assignments[neighbor] = label;
					if (core[neighbor]) {
						frontier.push(neighbor);
					}
				}
			}
		}
	}

	std::vector<std::size_t> core_records;
	std::vector<std::size_t> unassigned_records;
	for (std::size_t record = 0; record < record_count; ++record) {
		if (core[record]) {
			core_records.push_back(record);
		}
		if (assignments[record] == -1) {
			unassigned_records.push_back(record);
		}
	}

	const auto medoids = cluster_medoids(distances, assignments, cluster_count);
	return clustering_payload(assignments, medoids, core_records, unassigned_records, 1, true, "dbscan", representation);
}

auto native_nearest_neighbor_outliers(py::sequence records, py::object metric, std::size_t count,
									  const std::string &representation = "metric_space") -> py::dict
{
	const auto distances = native_pairwise_distance_matrix(records, metric);
	const auto record_count = distances.size();
	std::vector<std::pair<std::size_t, double>> scored;
	scored.reserve(record_count);
	for (std::size_t record = 0; record < record_count; ++record) {
		auto nearest = record_count < 2 ? 0.0 : std::numeric_limits<double>::infinity();
		for (std::size_t candidate = 0; candidate < record_count; ++candidate) {
			if (record == candidate) {
				continue;
			}
			nearest = std::min(nearest, distances[record][candidate]);
		}
		scored.emplace_back(record, nearest);
	}
	std::sort(scored.begin(), scored.end(), [](const auto &lhs, const auto &rhs) {
		if (lhs.second > rhs.second) {
			return true;
		}
		if (rhs.second > lhs.second) {
			return false;
		}
		return lhs.first < rhs.first;
	});
	if (scored.size() > count) {
		scored.resize(count);
	}

	auto result = py::dict{};
	result["outliers"] = scored;
	result["record_count"] = record_count;
	result["cluster_count"] = 0;
	result["unassigned_count"] = scored.size();
	result["exact"] = true;
	result["operator_name"] = "find_outliers";
	result["strategy"] = "nearest_neighbor_isolation";
	result["representation"] = representation;
	return result;
}

auto native_dbscan_outliers(py::sequence records, py::object metric, double radius, std::size_t min_points,
							const std::string &representation = "metric_space") -> py::dict
{
	const auto clustering = native_dbscan(records, metric, radius, min_points, representation);
	const auto assignments = clustering["assignments"].cast<std::vector<int>>();
	const auto unassigned_records = clustering["unassigned_records"].cast<std::vector<std::size_t>>();
	const auto distances = native_pairwise_distance_matrix(records, metric);

	std::vector<std::pair<std::size_t, double>> scored;
	for (const auto record : unassigned_records) {
		auto score = 0.0;
		auto has_reference = false;
		for (std::size_t candidate = 0; candidate < assignments.size(); ++candidate) {
			if (assignments[candidate] == -1) {
				continue;
			}
			if (!has_reference || distances[record][candidate] < score) {
				score = distances[record][candidate];
				has_reference = true;
			}
		}
		if (!has_reference) {
			for (std::size_t candidate = 0; candidate < assignments.size(); ++candidate) {
				if (candidate != record) {
					score = std::max(score, distances[record][candidate]);
				}
			}
		}
		scored.emplace_back(record, score);
	}
	std::sort(scored.begin(), scored.end(), [](const auto &lhs, const auto &rhs) {
		if (lhs.second > rhs.second) {
			return true;
		}
		if (rhs.second > lhs.second) {
			return false;
		}
		return lhs.first < rhs.first;
	});

	auto result = py::dict{};
	result["outliers"] = scored;
	result["record_count"] = assignments.size();
	result["cluster_count"] = clustering["cluster_count"];
	result["unassigned_count"] = scored.size();
	result["exact"] = true;
	result["operator_name"] = "find_outliers";
	result["strategy"] = "dbscan_density_outlier";
	result["representation"] = representation;
	return result;
}

auto redif_scale_policy_from_name(const std::string &name) -> mtrc::redif_scale_policy
{
	if (name == "mean_local_distance" || name == "mean") {
		return mtrc::redif_scale_policy::mean_local_distance;
	}
	if (name == "median_local_distance" || name == "median") {
		return mtrc::redif_scale_policy::median_local_distance;
	}
	if (name == "kth_local_distance" || name == "kth") {
		return mtrc::redif_scale_policy::kth_local_distance;
	}
	if (name == "global_mean_distance" || name == "global_mean") {
		return mtrc::redif_scale_policy::global_mean_distance;
	}
	throw std::invalid_argument("unknown Redif local scale policy");
}

auto redif_options_from_arguments(std::size_t neighbors, std::size_t iterations, double euler_step,
								  bool adaptive_geometry, const std::string &scale_policy,
								  double stability_tolerance,
								  double marginal_stability_tolerance,
								  std::size_t max_dense_records,
								  std::size_t max_memory_bytes,
								  std::size_t max_distance_evaluations) -> mtrc::redif_options
{
	mtrc::redif_options options;
	options.neighbors = neighbors;
	options.iterations = iterations;
	options.euler_step = euler_step;
	options.adaptive_geometry = adaptive_geometry;
	options.scale_policy = redif_scale_policy_from_name(scale_policy);
	options.stability_tolerance = stability_tolerance;
	options.marginal_stability_tolerance = marginal_stability_tolerance;
	options.max_dense_records = max_dense_records;
	options.max_memory_bytes = max_memory_bytes;
	options.max_distance_evaluations = max_distance_evaluations;
	return options;
}

auto redif_operator_diagnostics_payload(const mtrc::RedifOperatorDiagnostics &diagnostics) -> py::dict
{
	auto result = py::dict{};
	result["node_count"] = diagnostics.node_count;
	result["neighbors"] = diagnostics.neighbors;
	result["local_relation_representation"] = diagnostics.local_relation_representation;
	result["local_relation_exactness"] = diagnostics.local_relation_exactness;
	result["local_relation_directed_entries"] = diagnostics.local_relation_directed_entries;
	result["local_relation_distance_evaluations"] = diagnostics.local_relation_distance_evaluations;
	result["dense_distance_evaluations"] = diagnostics.dense_distance_evaluations;
	result["local_relation_candidate_count"] = diagnostics.local_relation_candidate_count;
	result["local_relation_candidate_universe"] = diagnostics.local_relation_candidate_universe;
	result["local_relation_chunk_size"] = diagnostics.local_relation_chunk_size;
	result["local_relation_chunk_count"] = diagnostics.local_relation_chunk_count;
	result["local_relation_candidate_fraction"] = diagnostics.local_relation_candidate_fraction;
	result["local_relation_exact"] = diagnostics.local_relation_exact;
	result["local_relation_refused"] = diagnostics.local_relation_refused;
	result["local_relation_refusal_reason"] = diagnostics.local_relation_refusal_reason;
	result["minimum_degree"] = diagnostics.minimum_degree;
	result["maximum_degree"] = diagnostics.maximum_degree;
	result["degree_ratio"] = diagnostics.degree_ratio;
	result["minimum_local_scale"] = diagnostics.minimum_local_scale;
	result["maximum_local_scale"] = diagnostics.maximum_local_scale;
	result["minimum_positive_affinity"] = diagnostics.minimum_positive_affinity;
	result["maximum_affinity"] = diagnostics.maximum_affinity;
	result["minimum_transition_row_sum"] = diagnostics.minimum_transition_row_sum;
	result["maximum_transition_row_sum"] = diagnostics.maximum_transition_row_sum;
	result["maximum_self_transition_probability"] = diagnostics.maximum_self_transition_probability;
	result["minimum_transition_escape_probability"] = diagnostics.minimum_transition_escape_probability;
	result["spectral_gap_proxy"] = diagnostics.spectral_gap_proxy;
	result["spectral_gap_proxy_value"] = diagnostics.spectral_gap_proxy_value;
	result["component_count"] = diagnostics.component_count;
	result["reversible"] = diagnostics.reversible;
	return result;
}

auto redif_stability_payload(const mtrc::RedifStabilityDiagnostics &diagnostics) -> py::dict
{
	auto result = py::dict{};
	result["status"] = diagnostics.status;
	result["minimum_pivot_abs"] = diagnostics.minimum_pivot_abs;
	result["maximum_pivot_abs"] = diagnostics.maximum_pivot_abs;
	result["pivot_ratio"] = diagnostics.pivot_ratio;
	result["singularity_margin"] = diagnostics.singularity_margin;
	result["stable"] = diagnostics.stable;
	result["marginal"] = diagnostics.marginal;
	return result;
}

auto redif_step_diagnostics_payload(const mtrc::RedifStepDiagnostics &diagnostics) -> py::dict
{
	auto result = py::dict{};
	result["step"] = diagnostics.step;
	result["operator_diagnostics"] = redif_operator_diagnostics_payload(diagnostics.operator_diagnostics);
	result["stability"] = redif_stability_payload(diagnostics.stability);
	result["stationary"] = diagnostics.stationary;
	result["minimum_shannon_entropy"] = diagnostics.minimum_shannon_entropy;
	result["maximum_shannon_entropy"] = diagnostics.maximum_shannon_entropy;
	result["maximum_relative_entropy_to_current_stationary"] =
		diagnostics.maximum_relative_entropy_to_current_stationary;
	return result;
}

auto redif_measure_result_payload(const mtrc::RedifMeasureResult &measure_result) -> py::dict
{
	auto paths = py::list{};
	for (const auto &path : measure_result.paths) {
		auto item = py::dict{};
		item["record_id"] = path.id.index();
		item["measures"] = path.measures;
		item["step_transport"] = path.step_transport;
		item["transport_path_length"] = path.transport_path_length;
		paths.append(std::move(item));
	}

	auto entropy = py::list{};
	for (const auto &diagnostics : measure_result.entropy_diagnostics) {
		auto item = py::dict{};
		item["record_id"] = diagnostics.id.index();
		item["initial_shannon_entropy"] = diagnostics.initial_shannon_entropy;
		item["terminal_shannon_entropy"] = diagnostics.terminal_shannon_entropy;
		item["initial_relative_entropy_to_stationary"] =
			diagnostics.initial_relative_entropy_to_stationary;
		item["terminal_relative_entropy_to_stationary"] =
			diagnostics.terminal_relative_entropy_to_stationary;
		item["terminal_relative_entropy_to_terminal_stationary"] =
			diagnostics.terminal_relative_entropy_to_terminal_stationary;
		entropy.append(std::move(item));
	}

	auto operator_diagnostics = py::list{};
	for (const auto &diagnostics : measure_result.operator_diagnostics) {
		operator_diagnostics.append(redif_operator_diagnostics_payload(diagnostics));
	}

	auto step_diagnostics = py::list{};
	for (const auto &diagnostics : measure_result.step_diagnostics) {
		step_diagnostics.append(redif_step_diagnostics_payload(diagnostics));
	}

	auto summaries = py::list{};
	for (const auto &summary : measure_result.summaries()) {
		auto item = py::dict{};
		item["record_id"] = summary.id.index();
		item["transport_path_length"] = summary.transport_path_length;
		item["initial_shannon_entropy"] = summary.initial_shannon_entropy;
		item["terminal_shannon_entropy"] = summary.terminal_shannon_entropy;
		item["entropy_delta"] = summary.entropy_delta;
		item["terminal_top_record_index"] = summary.terminal_top_record_index;
		item["terminal_top_record_mass"] = summary.terminal_top_record_mass;
		summaries.append(std::move(item));
	}

	auto transport = py::dict{};
	transport["ground_metric"] = measure_result.transport_diagnostics.ground_metric;
	transport["solver"] = measure_result.transport_diagnostics.solver;
	transport["measure_count"] = measure_result.transport_diagnostics.measure_count;
	transport["pair_count"] = measure_result.transport_diagnostics.pair_count;
	transport["transport_problem_count"] = measure_result.transport_diagnostics.transport_problem_count;
	transport["max_transport_problems"] = measure_result.transport_diagnostics.max_transport_problems;
	transport["support_atom_count"] = measure_result.transport_diagnostics.support_atom_count;
	transport["max_transport_support_atoms"] =
		measure_result.transport_diagnostics.max_transport_support_atoms;
	transport["truncated_measure_count"] = measure_result.transport_diagnostics.truncated_measure_count;
	transport["discarded_mass_total"] = measure_result.transport_diagnostics.discarded_mass_total;
	transport["maximum_discarded_mass"] = measure_result.transport_diagnostics.maximum_discarded_mass;
	transport["support_mass_floor"] = measure_result.transport_diagnostics.support_mass_floor;
	transport["exact"] = measure_result.transport_diagnostics.exact;
	transport["support_truncated"] = measure_result.transport_diagnostics.support_truncated;
	transport["refused"] = measure_result.transport_diagnostics.refused;
	transport["exactness"] = measure_result.transport_diagnostics.exactness;
	transport["refusal_reason"] = measure_result.transport_diagnostics.refusal_reason;

	auto result = py::dict{};
	result["paths"] = std::move(paths);
	result["record_count"] = measure_result.record_count;
	result["neighbors"] = measure_result.neighbors;
	result["iterations"] = measure_result.iterations;
	result["euler_step"] = measure_result.euler_step;
	result["adaptive_geometry"] = measure_result.adaptive_geometry;
	result["exact"] = measure_result.exact;
	result["operator_name"] = measure_result.operator_name;
	result["strategy"] = measure_result.strategy;
	result["representation"] = measure_result.representation;
	result["initial_stationary"] = measure_result.initial_stationary;
	result["terminal_stationary"] = measure_result.terminal_stationary;
	result["entropy_diagnostics"] = std::move(entropy);
	result["operator_diagnostics"] = std::move(operator_diagnostics);
	result["step_diagnostics"] = std::move(step_diagnostics);
	result["transport_diagnostics"] = std::move(transport);
	result["summaries"] = std::move(summaries);
	return result;
}

auto native_redif_remove_noise(py::sequence records, py::object metric, std::size_t neighbors,
							   std::size_t iterations, double euler_step, bool adaptive_geometry,
							   const std::string &scale_policy, double stability_tolerance,
							   double marginal_stability_tolerance, std::size_t max_dense_records,
							   std::size_t max_memory_bytes, std::size_t max_distance_evaluations,
							   const std::string &representation = "metric_space") -> py::dict
{
	const auto distances = native_pairwise_distance_matrix(records, metric);
	auto options = redif_options_from_arguments(neighbors, iterations, euler_step, adaptive_geometry,
											   scale_policy, stability_tolerance,
											   marginal_stability_tolerance, max_dense_records,
											   max_memory_bytes, max_distance_evaluations);
	auto result = mtrc::redif_remove_noise_from_distance_matrix(distances, options);
	result.representation = representation;
	return redif_measure_result_payload(result);
}

auto native_redif_add_noise(py::sequence records, py::object metric, std::size_t neighbors,
							std::size_t iterations, double euler_step, bool adaptive_geometry,
							const std::string &scale_policy, double stability_tolerance,
							double marginal_stability_tolerance, std::size_t max_dense_records,
							std::size_t max_memory_bytes, std::size_t max_distance_evaluations,
							const std::string &representation = "metric_space") -> py::dict
{
	const auto distances = native_pairwise_distance_matrix(records, metric);
	auto options = redif_options_from_arguments(neighbors, iterations, euler_step, adaptive_geometry,
											   scale_policy, stability_tolerance,
											   marginal_stability_tolerance, max_dense_records,
											   max_memory_bytes, max_distance_evaluations);
	auto result = mtrc::redif_add_noise_from_distance_matrix(distances, options);
	result.representation = representation;
	return redif_measure_result_payload(result);
}

auto native_redif_transport_path_outliers(py::sequence records, py::object metric, std::size_t neighbors,
										  std::size_t iterations, double euler_step,
										  bool adaptive_geometry, const std::string &scale_policy,
										  double stability_tolerance,
										  double marginal_stability_tolerance,
										  std::size_t max_dense_records,
										  std::size_t max_memory_bytes,
										  std::size_t max_distance_evaluations,
										  const std::string &representation = "metric_space") -> py::dict
{
	const auto dynamics = native_redif_remove_noise(records, metric, neighbors, iterations, euler_step,
												   adaptive_geometry, scale_policy, stability_tolerance,
												   marginal_stability_tolerance, max_dense_records,
												   max_memory_bytes, max_distance_evaluations,
												   representation);
	auto summaries = dynamics["summaries"].cast<py::list>();
	std::vector<std::pair<std::size_t, double>> scored;
	scored.reserve(static_cast<std::size_t>(summaries.size()));
	for (auto item : summaries) {
		auto summary = py::reinterpret_borrow<py::dict>(item);
		scored.emplace_back(summary["record_id"].cast<std::size_t>(),
							summary["transport_path_length"].cast<double>());
	}
	std::sort(scored.begin(), scored.end(), [](const auto &lhs, const auto &rhs) {
		if (lhs.second > rhs.second) {
			return true;
		}
		if (rhs.second > lhs.second) {
			return false;
		}
		return lhs.first < rhs.first;
	});

	auto result = py::dict{};
	result["outliers"] = scored;
	result["record_count"] = dynamics["record_count"];
	result["cluster_count"] = 0;
	result["unassigned_count"] = scored.size();
	result["exact"] = true;
	result["operator_name"] = "find_outliers";
	result["strategy"] = "redif_transport_path_length";
	result["representation"] = representation;
	result["dynamics"] = dynamics;
	return result;
}

void export_exact_scan(py::module &m)
{
	m.def("pairwise_distance_matrix", &native_pairwise_distance_matrix, py::arg("records"), py::arg("metric"));
	m.def("distance_profile_correlation", &native_distance_profile_correlation, py::arg("left_records"),
		  py::arg("left_metric"), py::arg("right_records"), py::arg("right_metric"),
		  py::arg("left_representation") = "records", py::arg("right_representation") = "records");
	m.def("exact_scan_neighbors", &native_exact_scan_neighbors, py::arg("records"), py::arg("metric"),
		  py::arg("query"), py::arg("count"));
	m.def("exact_scan_radius_neighbors", &native_exact_scan_radius_neighbors, py::arg("records"), py::arg("metric"),
		  py::arg("query"), py::arg("radius"));
	m.def("medoid_index", &native_medoid_index, py::arg("records"), py::arg("metric"));
	m.def("representative_indices", &native_representative_indices, py::arg("records"), py::arg("metric"),
		  py::arg("count"), py::arg("seed_index") = 0);
	m.def("separated_representative_indices", &native_separated_representative_indices, py::arg("records"),
		  py::arg("metric"), py::arg("minimum_distance"));
	m.def("coverage_representative_indices", &native_coverage_representative_indices, py::arg("records"),
		  py::arg("metric"), py::arg("radius"));
	m.def("assign_to_representatives", &native_assign_to_representatives, py::arg("records"), py::arg("metric"),
		  py::arg("representatives"));
	m.def("intrinsic_dimension_from_distances", &native_intrinsic_dimension_from_distances, py::arg("distances"));
	m.def("intrinsic_dimension", &native_intrinsic_dimension, py::arg("records"), py::arg("metric"));
	m.def("describe_structure", &native_describe_structure, py::arg("records"), py::arg("metric"),
		  py::arg("representation") = "metric_space");
	m.def("kmedoids", &native_kmedoids, py::arg("records"), py::arg("metric"), py::arg("groups"),
		  py::arg("max_iterations") = 100, py::arg("representation") = "metric_space");
	m.def("dbscan", &native_dbscan, py::arg("records"), py::arg("metric"), py::arg("radius"),
		  py::arg("min_points"), py::arg("representation") = "metric_space");
	m.def("nearest_neighbor_outliers", &native_nearest_neighbor_outliers, py::arg("records"), py::arg("metric"),
		  py::arg("count"), py::arg("representation") = "metric_space");
	m.def("dbscan_outliers", &native_dbscan_outliers, py::arg("records"), py::arg("metric"), py::arg("radius"),
		  py::arg("min_points"), py::arg("representation") = "metric_space");
	m.def("redif_remove_noise", &native_redif_remove_noise, py::arg("records"), py::arg("metric"),
		  py::arg("neighbors") = 10, py::arg("iterations") = 15, py::arg("euler_step") = 0.25,
		  py::arg("adaptive_geometry") = true, py::arg("scale_policy") = "mean_local_distance",
		  py::arg("stability_tolerance") = 1.0e-12,
		  py::arg("marginal_stability_tolerance") = 1.0e-8,
		  py::arg("max_dense_records") = mtrc::modify::dynamics::default_metric_transition_max_dense_records,
		  py::arg("max_memory_bytes") = mtrc::modify::dynamics::default_metric_transition_max_memory_bytes,
		  py::arg("max_distance_evaluations") =
			  mtrc::modify::dynamics::default_metric_transition_max_distance_evaluations,
		  py::arg("representation") = "metric_space");
	m.def("redif_add_noise", &native_redif_add_noise, py::arg("records"), py::arg("metric"),
		  py::arg("neighbors") = 10, py::arg("iterations") = 15, py::arg("euler_step") = 0.25,
		  py::arg("adaptive_geometry") = true, py::arg("scale_policy") = "mean_local_distance",
		  py::arg("stability_tolerance") = 1.0e-12,
		  py::arg("marginal_stability_tolerance") = 1.0e-8,
		  py::arg("max_dense_records") = mtrc::modify::dynamics::default_metric_transition_max_dense_records,
		  py::arg("max_memory_bytes") = mtrc::modify::dynamics::default_metric_transition_max_memory_bytes,
		  py::arg("max_distance_evaluations") =
			  mtrc::modify::dynamics::default_metric_transition_max_distance_evaluations,
		  py::arg("representation") = "metric_space");
	m.def("redif_transport_path_outliers", &native_redif_transport_path_outliers,
		  py::arg("records"), py::arg("metric"), py::arg("neighbors") = 10,
		  py::arg("iterations") = 15, py::arg("euler_step") = 0.25,
		  py::arg("adaptive_geometry") = true, py::arg("scale_policy") = "mean_local_distance",
		  py::arg("stability_tolerance") = 1.0e-12,
		  py::arg("marginal_stability_tolerance") = 1.0e-8,
		  py::arg("max_dense_records") = mtrc::modify::dynamics::default_metric_transition_max_dense_records,
		  py::arg("max_memory_bytes") = mtrc::modify::dynamics::default_metric_transition_max_memory_bytes,
		  py::arg("max_distance_evaluations") =
			  mtrc::modify::dynamics::default_metric_transition_max_distance_evaluations,
		  py::arg("representation") = "metric_space");
}
