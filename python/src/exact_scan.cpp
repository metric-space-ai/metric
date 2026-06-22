#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <sstream>
#include <stdexcept>
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

void export_exact_scan(py::module &m)
{
	m.def("pairwise_distance_matrix", &native_pairwise_distance_matrix, py::arg("records"), py::arg("metric"));
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
}
