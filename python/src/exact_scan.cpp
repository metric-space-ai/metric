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
	if (!std::isfinite(radius) || radius < 0.0) {
		throw std::invalid_argument("radius must be a finite non-negative value");
	}

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

void export_exact_scan(py::module &m)
{
	m.def("pairwise_distance_matrix", &native_pairwise_distance_matrix, py::arg("records"), py::arg("metric"));
	m.def("exact_scan_neighbors", &native_exact_scan_neighbors, py::arg("records"), py::arg("metric"),
		  py::arg("query"), py::arg("count"));
	m.def("exact_scan_radius_neighbors", &native_exact_scan_radius_neighbors, py::arg("records"), py::arg("metric"),
		  py::arg("query"), py::arg("radius"));
}
