#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "assets/mnist/mnist_reader.hpp"

#ifndef METRIC_MNIST_DATA_DIR
#define METRIC_MNIST_DATA_DIR "examples/engine/assets/mnist"
#endif

#ifndef METRIC_GRAE10_DATA_JSON
#define METRIC_GRAE10_DATA_JSON "../../visual/examples/grae10-metric-engine/grae10-data.json"
#endif

namespace {

constexpr int kSkipMissingMnistData = 77;

struct Options {
	std::filesystem::path mnist_dir{METRIC_MNIST_DATA_DIR};
	std::filesystem::path visual_json{METRIC_GRAE10_DATA_JSON};
};

struct NumberScan {
	std::size_t count{0};
	bool finite{true};
	double minimum{std::numeric_limits<double>::infinity()};
	double maximum{-std::numeric_limits<double>::infinity()};
};

auto read_text(const std::filesystem::path &path) -> std::string
{
	std::ifstream in(path);
	if (!in) {
		throw std::runtime_error("unable to read " + path.string());
	}
	std::ostringstream buffer;
	buffer << in.rdbuf();
	return buffer.str();
}

auto parse_options(int argc, char **argv) -> Options
{
	Options options;
	for (int index = 1; index < argc; ++index) {
		const std::string arg = argv[index];
		if (arg == "--mnist-dir" && index + 1 < argc) {
			options.mnist_dir = argv[++index];
		} else if (arg == "--visual-json" && index + 1 < argc) {
			options.visual_json = argv[++index];
		} else {
			throw std::invalid_argument("unknown or incomplete argument: " + arg);
		}
	}
	return options;
}

auto has_mnist_calibration_labels(const std::filesystem::path &folder) -> bool
{
	return std::filesystem::exists(folder / "train-labels-idx1-ubyte");
}

auto key_position(const std::string &json, const std::string &key) -> std::size_t
{
	const auto position = json.find('"' + key + '"');
	if (position == std::string::npos) {
		throw std::runtime_error("missing JSON key: " + key);
	}
	return position;
}

auto colon_after_key(const std::string &json, const std::string &key) -> std::size_t
{
	const auto colon = json.find(':', key_position(json, key));
	if (colon == std::string::npos) {
		throw std::runtime_error("missing JSON colon for key: " + key);
	}
	return colon + 1;
}

auto skip_ws(const std::string &json, std::size_t position) -> std::size_t
{
	while (position < json.size() && std::isspace(static_cast<unsigned char>(json[position])) != 0) {
		++position;
	}
	return position;
}

auto extract_string(const std::string &json, const std::string &key) -> std::string
{
	auto position = skip_ws(json, colon_after_key(json, key));
	if (position >= json.size() || json[position] != '"') {
		throw std::runtime_error("expected string for key: " + key);
	}
	++position;
	std::string value;
	while (position < json.size()) {
		const char current = json[position++];
		if (current == '"') {
			return value;
		}
		if (current == '\\') {
			if (position >= json.size()) {
				throw std::runtime_error("unterminated escape in string for key: " + key);
			}
			value.push_back(json[position++]);
		} else {
			value.push_back(current);
		}
	}
	throw std::runtime_error("unterminated string for key: " + key);
}

auto extract_size(const std::string &json, const std::string &key) -> std::size_t
{
	auto position = skip_ws(json, colon_after_key(json, key));
	std::size_t value = 0;
	bool found_digit = false;
	while (position < json.size() && std::isdigit(static_cast<unsigned char>(json[position])) != 0) {
		found_digit = true;
		value = (value * 10) + static_cast<std::size_t>(json[position] - '0');
		++position;
	}
	if (!found_digit) {
		throw std::runtime_error("expected unsigned integer for key: " + key);
	}
	return value;
}

auto array_bounds(const std::string &json, const std::string &key) -> std::pair<std::size_t, std::size_t>
{
	const auto begin = json.find('[', colon_after_key(json, key));
	if (begin == std::string::npos) {
		throw std::runtime_error("expected array for key: " + key);
	}
	std::size_t depth = 0;
	for (std::size_t position = begin; position < json.size(); ++position) {
		if (json[position] == '[') {
			++depth;
		} else if (json[position] == ']') {
			--depth;
			if (depth == 0) {
				return {begin + 1, position};
			}
		}
	}
	throw std::runtime_error("unterminated array for key: " + key);
}

auto scan_number_array(const std::string &json, const std::string &key) -> NumberScan
{
	const auto [begin, end] = array_bounds(json, key);
	NumberScan result;
	const char *cursor = json.c_str() + begin;
	const char *finish = json.c_str() + end;
	while (cursor < finish) {
		while (cursor < finish && (std::isspace(static_cast<unsigned char>(*cursor)) != 0 || *cursor == ',')) {
			++cursor;
		}
		if (cursor >= finish) {
			break;
		}
		errno = 0;
		char *next = nullptr;
		const double value = std::strtod(cursor, &next);
		if (next == cursor || errno == ERANGE) {
			throw std::runtime_error("invalid number in JSON array: " + key);
		}
		result.finite = result.finite && std::isfinite(value);
		result.minimum = std::min(result.minimum, value);
		result.maximum = std::max(result.maximum, value);
		++result.count;
		cursor = next;
	}
	return result;
}

auto extract_size_array(const std::string &json, const std::string &key) -> std::vector<std::size_t>
{
	const auto [begin, end] = array_bounds(json, key);
	std::vector<std::size_t> result;
	const char *cursor = json.c_str() + begin;
	const char *finish = json.c_str() + end;
	while (cursor < finish) {
		while (cursor < finish && (std::isspace(static_cast<unsigned char>(*cursor)) != 0 || *cursor == ',')) {
			++cursor;
		}
		if (cursor >= finish) {
			break;
		}
		errno = 0;
		char *next = nullptr;
		const auto value = std::strtoull(cursor, &next, 10);
		if (next == cursor || errno == ERANGE) {
			throw std::runtime_error("invalid unsigned integer in JSON array: " + key);
		}
		result.push_back(static_cast<std::size_t>(value));
		cursor = next;
	}
	return result;
}

auto labels_text(const std::vector<std::uint8_t> &labels) -> std::string
{
	std::string result;
	result.reserve(labels.size());
	for (const auto label : labels) {
		if (label > 9) {
			throw std::runtime_error("MNIST label outside digit range");
		}
		result.push_back(static_cast<char>('0' + label));
	}
	return result;
}

auto label_counts(const std::vector<std::uint8_t> &labels) -> std::array<std::size_t, 10>
{
	std::array<std::size_t, 10> counts{};
	for (const auto label : labels) {
		if (label > 9) {
			throw std::runtime_error("MNIST label outside digit range");
		}
		++counts[label];
	}
	return counts;
}

} // namespace

int main(int argc, char **argv)
{
	const auto options = parse_options(argc, argv);
	if (!has_mnist_calibration_labels(options.mnist_dir)) {
		std::cout << "mnist grae10 integrity skipped: MNIST train label IDX file not found in "
				  << options.mnist_dir.string() << "\n";
		return kSkipMissingMnistData;
	}
	const auto json = read_text(options.visual_json);
	const auto labels = mnist::read_calibration_labels<std::vector, std::uint8_t>(options.mnist_dir.string(), 0);
	const auto mnist_labels = labels_text(labels);
	const auto mnist_counts = label_counts(labels);

	const auto schema = extract_string(json, "schema");
	const auto source = extract_string(json, "source");
	const auto record_count = extract_size(json, "recordCount");
	const auto visual_labels = extract_string(json, "labels");
	const auto visual_counts = extract_size_array(json, "counts");
	const auto p2_scan = scan_number_array(json, "p2");
	const auto p3_scan = scan_number_array(json, "p3");

	assert(schema == "metric.visual.grae10.dataset.v1");
	assert(source == "GRAE10.html");
	assert(record_count == 60000);
	assert(labels.size() == 60000);
	assert(visual_labels == mnist_labels);
	assert(visual_counts.size() == mnist_counts.size());
	for (std::size_t index = 0; index < mnist_counts.size(); ++index) {
		assert(visual_counts[index] == mnist_counts[index]);
	}
	assert(p2_scan.count == record_count * 2);
	assert(p3_scan.count == record_count * 3);
	assert(p2_scan.finite);
	assert(p3_scan.finite);
	assert(p2_scan.minimum < p2_scan.maximum);
	assert(p3_scan.minimum < p3_scan.maximum);

	std::cout << "mnist grae10 dataset = MNIST train IDX\n";
	std::cout << "mnist grae10 visual records = " << record_count << "\n";
	std::cout << "mnist grae10 labels match MNIST IDX = yes\n";
	std::cout << "mnist grae10 p2 coordinates = " << (p2_scan.count / 2) << "\n";
	std::cout << "mnist grae10 p3 coordinates = " << (p3_scan.count / 3) << "\n";
}
