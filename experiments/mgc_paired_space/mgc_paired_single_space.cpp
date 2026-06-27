// Native single-space MGC visual export.
//
// The visible object is one finite metric space of paired records
// p_i = (event_log_i, process_curve_i). MGC is exported as operator evidence
// over two native metrics on the paired records, never as a second visible space.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../../examples/engine/cross_space_dependency.hpp"
#include "../../visual/cpp/mtrc_visual.hpp"

namespace {

namespace visual = mtrc::visual;

constexpr std::size_t kDefaultRecords = 384;
constexpr std::uint64_t kSeed = 20240607ULL;

struct Options {
	std::filesystem::path export_dir{"/Volumes/Models/metric-demos/outputs/mgc-paired-single-space"};
	std::size_t records{kDefaultRecords};
	std::string visual_import_url{"../../src/index.js"};
};

struct MetricLawDiagnostics {
	double diagonal_max_abs{};
	double symmetry_max_abs{};
	double triangle_max_violation{};
	double minimum_nonzero_distance{std::numeric_limits<double>::infinity()};
	double maximum_distance{};
	double average_distance{};
	std::size_t pair_count{};
	std::size_t triangle_triplets{};
	bool triangle_exhaustive{true};
	bool finite{true};
};

struct PairContribution {
	std::size_t row{};
	std::size_t column{};
	double value{};
};

struct MgcEvidence {
	double statistic{};
	double compare_statistic{};
	double maximal_scale_correlation{};
	std::size_t optimal_k{};
	std::size_t optimal_l{};
	std::size_t significant_scale_cells{};
	std::size_t grid_rows{};
	std::size_t grid_columns{};
	std::vector<std::vector<double>> sampled_grid;
};

struct GeneratedDataset {
	cross_space::Dataset data;
	std::vector<double> latent_phase;
};

struct RecordCorrelationExtremes {
	std::size_t minimum_index{};
	std::size_t maximum_index{};
	double minimum_value{};
	double maximum_value{};
};

auto parse_options(int argc, char **argv) -> Options
{
	Options options;
	for (int index = 1; index < argc; ++index) {
		const std::string arg = argv[index];
		const auto require_value = [&](const char *name) -> std::string {
			if (index + 1 >= argc) {
				throw std::invalid_argument(std::string("missing value for ") + name);
			}
			return argv[++index];
		};
		if (arg == "--export-dir") {
			options.export_dir = require_value("--export-dir");
		} else if (arg == "--records") {
			options.records = static_cast<std::size_t>(std::stoull(require_value("--records")));
		} else if (arg == "--visual-import-url") {
			options.visual_import_url = require_value("--visual-import-url");
		} else if (arg == "--help" || arg == "-h") {
			std::cout << "usage: " << argv[0]
					  << " [--export-dir PATH] [--records N] [--visual-import-url URL]\n";
			std::exit(0);
		} else {
			throw std::invalid_argument("unknown argument: " + arg);
		}
	}
	if (options.records < 16) {
		throw std::invalid_argument("--records must be >= 16");
	}
	return options;
}

auto record_id(std::size_t index) -> std::string
{
	std::ostringstream out;
	out << "pair-" << std::setw(4) << std::setfill('0') << index;
	return out.str();
}

auto make_record_ids(std::size_t count) -> std::vector<std::string>
{
	std::vector<std::string> ids;
	ids.reserve(count);
	for (std::size_t index = 0; index < count; ++index) {
		ids.push_back(record_id(index));
	}
	return ids;
}

auto make_event_log_2d(double severity, double phase, int jitter) -> cross_space::EventLog
{
	static const char normal_cycle[4] = {'h', 'H', 'c', 's'};
	static const char severity_codes[4] = {'j', 'o', 'x', 'z'};
	static const char phase_codes[4] = {'a', 'b', 'd', 'f'};
	constexpr std::size_t body = 48;

	const auto phase_bin = static_cast<std::size_t>(std::min(3.0, std::floor(phase * 4.0)));
	const auto severity_bin = static_cast<std::size_t>(std::min(3.0, std::floor(severity * 4.0)));
	const auto severity_width = static_cast<std::size_t>(std::lround(4.0 + severity * 14.0));
	const double phase_center = 24.0 + phase * 17.0;
	const double phase_width = 2.5 + 3.5 * (1.0 - severity);

	cross_space::EventLog log;
	log.reserve(body + 4);
	log.push_back('S');
	for (std::size_t position = 0; position < body; ++position) {
		if (position < 18 && position < severity_width) {
			log.push_back(severity_codes[(severity_bin + position) % 4]);
			continue;
		}
		const double distance_to_phase = std::abs(static_cast<double>(position) - phase_center);
		if (position >= 18 && distance_to_phase <= phase_width) {
			log.push_back(phase_codes[phase_bin]);
		} else if (position >= 36 && ((position + phase_bin + severity_bin) % 5 == 0)) {
			log.push_back(severity_codes[(phase_bin + severity_bin) % 4]);
		} else {
			log.push_back(normal_cycle[(position + phase_bin + severity_bin) % 4]);
		}
	}
	for (int extra = 0; extra < jitter; ++extra) {
		log.push_back('H');
	}
	log.push_back('E');
	return log;
}

auto make_process_curve_2d(double severity, double phase) -> cross_space::ProcessCurve
{
	constexpr std::size_t length = 48;
	constexpr double pi = 3.14159265358979323846;
	cross_space::ProcessCurve curve(length, 0.0);
	const double phase_center = 9.0 + phase * static_cast<double>(length - 18);
	const double severity_amplitude = 0.65 + 2.8 * severity;
	const double phase_amplitude = 1.35 + 0.9 * (1.0 - std::abs(severity - 0.5));

	for (std::size_t index = 0; index < length; ++index) {
		const double t = static_cast<double>(index);
		const double ramp = 1.15 * (1.0 - std::exp(-t / 9.0));
		const double severity_bump = severity_amplitude * std::exp(-std::pow(t - 13.0, 2.0) / 18.0);
		const double severity_dip = 0.58 * severity_amplitude * std::exp(-std::pow(t - 28.0, 2.0) / 22.0);
		const double phase_bump = phase_amplitude * std::exp(-std::pow(t - phase_center, 2.0) / 10.0);
		const double periodic = 0.26 * std::sin((2.0 * pi * t / static_cast<double>(length)) + phase * 2.0 * pi);
		curve[index] = ramp + severity_bump - severity_dip + phase_bump + periodic;
	}
	return curve;
}

auto generate_paired_metric_dataset(std::size_t count, std::uint64_t seed) -> GeneratedDataset
{
	auto severity_rng = cross_space::detail::stream(seed, 41);
	auto phase_rng = cross_space::detail::stream(seed, 43);
	auto jitter_rng = cross_space::detail::stream(seed, 47);
	const auto grid = static_cast<std::size_t>(std::ceil(std::sqrt(static_cast<double>(count))));

	GeneratedDataset generated;
	generated.data.logs.reserve(count);
	generated.data.curves.reserve(count);
	generated.data.latent.reserve(count);
	generated.latent_phase.reserve(count);

	for (std::size_t index = 0; index < count; ++index) {
		const std::size_t gx = index % grid;
		const std::size_t gy = (index / grid) % grid;
		const double severity_noise = 0.62 * (cross_space::detail::next_unit(severity_rng) - 0.5);
		const double phase_noise = 0.62 * (cross_space::detail::next_unit(phase_rng) - 0.5);
		const double denominator = static_cast<double>(std::max<std::size_t>(grid - 1, 1));
		const double severity = std::clamp((static_cast<double>(gx) + severity_noise) / denominator, 0.0, 1.0);
		const double phase = std::clamp((static_cast<double>(gy) + phase_noise) / denominator, 0.0, 1.0);
		const int jitter = static_cast<int>(cross_space::detail::next_below(jitter_rng, 3));

		generated.data.logs.push_back(make_event_log_2d(severity, phase, jitter));
		generated.data.curves.push_back(make_process_curve_2d(severity, phase));
		generated.data.latent.push_back(severity);
		generated.latent_phase.push_back(phase);
	}
	return generated;
}

auto matrix_value(const mtrc::DistanceMatrix<double> &matrix, std::size_t row, std::size_t column) -> double
{
	return matrix(row, column);
}

auto max_distance(const mtrc::DistanceMatrix<double> &matrix) -> double
{
	double result = 0.0;
	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		for (std::size_t column = row + 1; column < matrix.columns(); ++column) {
			result = std::max(result, matrix_value(matrix, row, column));
		}
	}
	return result;
}

auto product_pair_matrix(const mtrc::DistanceMatrix<double> &left, const mtrc::DistanceMatrix<double> &right)
	-> mtrc::DistanceMatrix<double>
{
	const std::size_t count = left.rows();
	const double left_scale = std::max(max_distance(left), 1e-12);
	const double right_scale = std::max(max_distance(right), 1e-12);
	mtrc::DistanceMatrix<double> pair(count);
	for (std::size_t row = 0; row < count; ++row) {
		pair(row, row) = 0.0;
		for (std::size_t column = row + 1; column < count; ++column) {
			const double a = matrix_value(left, row, column) / left_scale;
			const double b = matrix_value(right, row, column) / right_scale;
			pair(row, column) = std::sqrt(a * a + b * b);
		}
	}
	return pair;
}

auto metric_law_diagnostics(const mtrc::DistanceMatrix<double> &matrix) -> MetricLawDiagnostics
{
	constexpr std::size_t triangle_limit = 2'000'000;
	const std::size_t count = matrix.rows();
	MetricLawDiagnostics diagnostics;

	for (std::size_t row = 0; row < count; ++row) {
		diagnostics.diagonal_max_abs =
			std::max(diagnostics.diagonal_max_abs, std::abs(matrix_value(matrix, row, row)));
		diagnostics.finite = diagnostics.finite && std::isfinite(matrix_value(matrix, row, row));
		for (std::size_t column = 0; column < count; ++column) {
			const double lhs = matrix_value(matrix, row, column);
			const double rhs = matrix_value(matrix, column, row);
			diagnostics.symmetry_max_abs = std::max(diagnostics.symmetry_max_abs, std::abs(lhs - rhs));
			diagnostics.finite = diagnostics.finite && std::isfinite(lhs);
		}
	}

	for (std::size_t row = 0; row < count; ++row) {
		for (std::size_t column = row + 1; column < count; ++column) {
			const double distance = matrix_value(matrix, row, column);
			diagnostics.maximum_distance = std::max(diagnostics.maximum_distance, distance);
			diagnostics.average_distance += distance;
			if (distance > 0.0) {
				diagnostics.minimum_nonzero_distance = std::min(diagnostics.minimum_nonzero_distance, distance);
			}
			++diagnostics.pair_count;
		}
	}
	if (diagnostics.pair_count != 0) {
		diagnostics.average_distance /= static_cast<double>(diagnostics.pair_count);
	}
	if (!std::isfinite(diagnostics.minimum_nonzero_distance)) {
		diagnostics.minimum_nonzero_distance = 0.0;
	}

	const std::size_t exhaustive_triplets = count * count * count;
	if (exhaustive_triplets <= triangle_limit) {
		for (std::size_t i = 0; i < count; ++i) {
			for (std::size_t j = 0; j < count; ++j) {
				for (std::size_t k = 0; k < count; ++k) {
					const double violation =
						matrix_value(matrix, i, k) - (matrix_value(matrix, i, j) + matrix_value(matrix, j, k));
					diagnostics.triangle_max_violation = std::max(diagnostics.triangle_max_violation, violation);
					++diagnostics.triangle_triplets;
				}
			}
		}
	} else {
		diagnostics.triangle_exhaustive = false;
		diagnostics.triangle_triplets = triangle_limit;
		for (std::size_t sample = 0; sample < triangle_limit; ++sample) {
			const std::size_t i = (sample * 15485863ULL + 11ULL) % count;
			const std::size_t j = (sample * 32452843ULL + 23ULL) % count;
			const std::size_t k = (sample * 49979687ULL + 37ULL) % count;
			const double violation =
				matrix_value(matrix, i, k) - (matrix_value(matrix, i, j) + matrix_value(matrix, j, k));
			diagnostics.triangle_max_violation = std::max(diagnostics.triangle_max_violation, violation);
		}
	}

	return diagnostics;
}

auto select_landmarks(const mtrc::DistanceMatrix<double> &matrix) -> std::array<std::size_t, 3>
{
	const std::size_t count = matrix.rows();
	std::array<std::size_t, 3> landmarks{0, 0, 0};
	if (count == 0) {
		return landmarks;
	}
	double best = -1.0;
	for (std::size_t index = 0; index < count; ++index) {
		const double distance = matrix_value(matrix, landmarks[0], index);
		if (distance > best) {
			best = distance;
			landmarks[1] = index;
		}
	}
	best = -1.0;
	for (std::size_t index = 0; index < count; ++index) {
		const double nearest =
			std::min(matrix_value(matrix, index, landmarks[0]), matrix_value(matrix, index, landmarks[1]));
		if (nearest > best) {
			best = nearest;
			landmarks[2] = index;
		}
	}
	return landmarks;
}

auto landmark_coordinates(const mtrc::DistanceMatrix<double> &matrix) -> std::vector<std::array<double, 3>>
{
	const std::size_t count = matrix.rows();
	const auto landmarks = select_landmarks(matrix);
	std::vector<std::array<double, 3>> coordinates(count);
	const double d01 = std::max(matrix_value(matrix, landmarks[0], landmarks[1]), 1e-12);
	const double d02 = matrix_value(matrix, landmarks[0], landmarks[2]);
	const double d12 = matrix_value(matrix, landmarks[1], landmarks[2]);
	const double x2 = (d02 * d02 + d01 * d01 - d12 * d12) / (2.0 * d01);
	const double y2 = std::sqrt(std::max(0.0, d02 * d02 - x2 * x2));
	for (std::size_t row = 0; row < count; ++row) {
		const double d0 = matrix_value(matrix, row, landmarks[0]);
		const double d1 = matrix_value(matrix, row, landmarks[1]);
		const double d2 = matrix_value(matrix, row, landmarks[2]);
		const double x = (d0 * d0 + d01 * d01 - d1 * d1) / (2.0 * d01);
		double y = 0.0;
		if (y2 > 1e-12) {
			y = (d0 * d0 + d02 * d02 - d2 * d2 - 2.0 * x * x2) / (2.0 * y2);
		}
		const double z = std::sqrt(std::max(0.0, d0 * d0 - x * x - y * y));
		coordinates[row] = {x, y, z};
	}
	for (std::size_t axis = 0; axis < 3; ++axis) {
		double mean = 0.0;
		for (const auto &position : coordinates) {
			mean += position[axis];
		}
		mean /= static_cast<double>(std::max<std::size_t>(count, 1));
		double max_abs = 0.0;
		for (auto &position : coordinates) {
			position[axis] -= mean;
			max_abs = std::max(max_abs, std::abs(position[axis]));
		}
		if (max_abs > 0.0) {
			for (auto &position : coordinates) {
				position[axis] /= max_abs;
			}
		}
	}
	return coordinates;
}

auto positions3(const std::vector<std::string> &record_ids, const std::vector<std::array<double, 3>> &coordinates)
	-> std::vector<visual::Position>
{
	std::vector<visual::Position> positions;
	positions.reserve(record_ids.size());
	for (std::size_t index = 0; index < record_ids.size(); ++index) {
		positions.push_back(visual::Position{
			record_ids[index], {coordinates[index][0], coordinates[index][1], coordinates[index][2]}});
	}
	return positions;
}

auto coordinate_metadata_json(const std::vector<std::string> &record_ids,
							  const std::array<std::size_t, 3> &landmarks,
							  const std::string &source_metric) -> std::string
{
	std::vector<std::string> landmark_ids;
	landmark_ids.reserve(landmarks.size());
	for (const std::size_t landmark : landmarks) {
		landmark_ids.push_back(record_ids[landmark]);
	}
	return visual::object({visual::string_field("method", "native_landmark_simplex_trilateration"),
						   visual::string_field("source_metric", source_metric),
						   visual::string_field("coordinate_contract",
												"coordinates are simplex trilateration values derived from distances to exported landmark records"),
						   visual::string_array_field("landmark_record_ids", landmark_ids)});
}

auto row_mean_distance(const mtrc::DistanceMatrix<double> &matrix, std::size_t row) -> double
{
	const std::size_t count = matrix.rows();
	if (count < 2) {
		return 0.0;
	}
	double total = 0.0;
	for (std::size_t column = 0; column < count; ++column) {
		if (row != column) {
			total += matrix_value(matrix, row, column);
		}
	}
	return total / static_cast<double>(count - 1);
}

auto local_profile_alignment(const mtrc::DistanceMatrix<double> &left, const mtrc::DistanceMatrix<double> &right)
	-> std::vector<double>
{
	const std::size_t count = left.rows();
	std::vector<double> values(count, 0.0);
	for (std::size_t row = 0; row < count; ++row) {
		std::vector<double> left_profile;
		std::vector<double> right_profile;
		left_profile.reserve(count - 1);
		right_profile.reserve(count - 1);
		for (std::size_t column = 0; column < count; ++column) {
			if (row == column) {
				continue;
			}
			left_profile.push_back(matrix_value(left, row, column));
			right_profile.push_back(matrix_value(right, row, column));
		}
		values[row] = cross_space::pearson(left_profile, right_profile);
	}
	return values;
}

auto pair_contributions(const mtrc::DistanceMatrix<double> &left, const mtrc::DistanceMatrix<double> &right)
	-> std::vector<PairContribution>
{
	const std::size_t count = left.rows();
	std::vector<double> left_values;
	std::vector<double> right_values;
	left_values.reserve((count * (count - 1)) / 2);
	right_values.reserve(left_values.capacity());
	for (std::size_t row = 0; row < count; ++row) {
		for (std::size_t column = row + 1; column < count; ++column) {
			left_values.push_back(matrix_value(left, row, column));
			right_values.push_back(matrix_value(right, row, column));
		}
	}
	const auto mean = [](const std::vector<double> &values) {
		if (values.empty()) {
			return 0.0;
		}
		return std::accumulate(values.begin(), values.end(), 0.0) / static_cast<double>(values.size());
	};
	const auto stddev = [](const std::vector<double> &values, double center) {
		if (values.empty()) {
			return 0.0;
		}
		double variance = 0.0;
		for (const double value : values) {
			const double delta = value - center;
			variance += delta * delta;
		}
		return std::sqrt(variance / static_cast<double>(values.size()));
	};
	const double left_mean = mean(left_values);
	const double right_mean = mean(right_values);
	const double left_sd = stddev(left_values, left_mean);
	const double right_sd = stddev(right_values, right_mean);
	std::vector<PairContribution> contributions;
	contributions.reserve(left_values.size());
	std::size_t pair_index = 0;
	for (std::size_t row = 0; row < count; ++row) {
		for (std::size_t column = row + 1; column < count; ++column) {
			double value = 0.0;
			if (left_sd > 0.0 && right_sd > 0.0) {
				value = ((left_values[pair_index] - left_mean) / left_sd) *
						((right_values[pair_index] - right_mean) / right_sd);
			}
			contributions.push_back(PairContribution{row, column, value});
			++pair_index;
		}
	}
	return contributions;
}

auto local_contribution(const std::vector<PairContribution> &pairs, std::size_t count) -> std::vector<double>
{
	std::vector<double> totals(count, 0.0);
	std::vector<std::size_t> seen(count, 0);
	for (const auto &pair : pairs) {
		totals[pair.row] += pair.value;
		totals[pair.column] += pair.value;
		++seen[pair.row];
		++seen[pair.column];
	}
	for (std::size_t index = 0; index < count; ++index) {
		if (seen[index] != 0) {
			totals[index] /= static_cast<double>(seen[index]);
		}
	}
	return totals;
}

auto scale01(const std::vector<double> &values) -> std::vector<double>
{
	std::vector<double> scaled(values.size(), 0.0);
	if (values.empty()) {
		return scaled;
	}
	auto [min_it, max_it] = std::minmax_element(values.begin(), values.end());
	const double span = std::max(1e-12, *max_it - *min_it);
	for (std::size_t index = 0; index < values.size(); ++index) {
		scaled[index] = (values[index] - *min_it) / span;
	}
	return scaled;
}

auto abs_values(const std::vector<double> &values) -> std::vector<double>
{
	std::vector<double> result(values.size(), 0.0);
	for (std::size_t index = 0; index < values.size(); ++index) {
		result[index] = std::abs(values[index]);
	}
	return result;
}

auto local_mgc_evidence(const std::vector<double> &alignment, const std::vector<double> &contribution)
	-> std::vector<double>
{
	const auto scaled_contribution = scale01(abs_values(contribution));
	std::vector<double> evidence(alignment.size(), 0.0);
	for (std::size_t index = 0; index < evidence.size(); ++index) {
		const double aligned = std::clamp(0.5 + 0.5 * alignment[index], 0.0, 1.0);
		const double contrib = index < scaled_contribution.size() ? scaled_contribution[index] : 0.0;
		evidence[index] = 0.55 * contrib + 0.45 * aligned;
	}
	return evidence;
}

auto optimal_scale_record_correlation(const mtrc::DistanceMatrix<double> &left,
									  const mtrc::DistanceMatrix<double> &right,
									  const MgcEvidence &mgc,
									  std::vector<double> *support = nullptr) -> std::vector<double>
{
	const std::size_t count = left.rows();
	mtrc::MGC_direct direct;
	auto A = direct.center_distance_matrix(left);
	auto B = direct.center_distance_matrix(right);
	auto RXt = direct.center_ranked_distance_matrix(left);
	auto RYt = direct.center_ranked_distance_matrix(right);
	mtrc::numeric::DynamicMatrix<double> At = mtrc::numeric::trans(A);
	mtrc::numeric::DynamicMatrix<double> Bt = mtrc::numeric::trans(B);
	mtrc::numeric::DynamicMatrix<std::size_t> RX = mtrc::numeric::trans(RXt);
	mtrc::numeric::DynamicMatrix<std::size_t> RY = mtrc::numeric::trans(RYt);
	auto varX = direct.local_covariance(A, At, RX, RXt);
	auto varY = direct.local_covariance(B, Bt, RY, RYt);

	const std::size_t k = std::min<std::size_t>(std::max<std::size_t>(mgc.optimal_k, 1) - 1, count - 1);
	const std::size_t l = std::min<std::size_t>(std::max<std::size_t>(mgc.optimal_l, 1) - 1, count - 1);
	const double denominator = std::sqrt(std::max(1e-18, varX(k, k) * varY(l, l)));

	std::vector<double> values(count, 0.0);
	std::vector<double> counts(count, 0.0);
	for (std::size_t row = 0; row < count; ++row) {
		for (std::size_t column = 0; column < count; ++column) {
			if (row == column) {
				continue;
			}
			if (RX(row, column) <= k && RYt(row, column) <= l) {
				values[row] += (A(row, column) * B(column, row)) / denominator;
				counts[row] += 1.0;
			}
		}
	}
	for (std::size_t index = 0; index < count; ++index) {
		if (counts[index] > 0.0) {
			values[index] /= counts[index];
		}
	}
	if (support != nullptr) {
		*support = counts;
	}
	return values;
}

auto local_density(const mtrc::DistanceMatrix<double> &matrix) -> std::vector<double>
{
	std::vector<double> density(matrix.rows(), 0.0);
	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		density[row] = 1.0 / (1.0 + row_mean_distance(matrix, row));
	}
	return density;
}

auto scalar_values(const std::vector<std::string> &record_ids, const std::vector<double> &values)
	-> std::vector<visual::ScalarValue>
{
	std::vector<visual::ScalarValue> scalars;
	scalars.reserve(record_ids.size());
	for (std::size_t index = 0; index < record_ids.size(); ++index) {
		scalars.push_back(visual::ScalarValue{record_ids[index], values[index]});
	}
	return scalars;
}

auto find_extremes(const std::vector<double> &values) -> RecordCorrelationExtremes
{
	RecordCorrelationExtremes extremes;
	if (values.empty()) {
		return extremes;
	}
	auto [minimum, maximum] = std::minmax_element(values.begin(), values.end());
	extremes.minimum_index = static_cast<std::size_t>(std::distance(values.begin(), minimum));
	extremes.maximum_index = static_cast<std::size_t>(std::distance(values.begin(), maximum));
	extremes.minimum_value = *minimum;
	extremes.maximum_value = *maximum;
	return extremes;
}

auto local_correlation_extreme_values(const std::vector<std::string> &record_ids,
									  const RecordCorrelationExtremes &extremes)
	-> std::vector<visual::CategoricalValue>
{
	std::vector<visual::CategoricalValue> values;
	values.reserve(record_ids.size());
	for (std::size_t index = 0; index < record_ids.size(); ++index) {
		std::string label;
		if (index == extremes.minimum_index) {
			label = "min lokale Korrelation";
		} else if (index == extremes.maximum_index) {
			label = "max lokale Korrelation";
		}
		values.push_back(visual::CategoricalValue{record_ids[index], label});
	}
	return values;
}

auto latent_region_values(const std::vector<std::string> &record_ids,
						  const std::vector<double> &severity,
						  const std::vector<double> &phase)
	-> std::vector<visual::CategoricalValue>
{
	std::vector<visual::CategoricalValue> values;
	values.reserve(record_ids.size());
	for (std::size_t index = 0; index < record_ids.size(); ++index) {
		const std::string vertical = severity[index] < 0.5 ? "low" : "high";
		const std::string horizontal = phase[index] < 0.5 ? "early" : "late";
		values.push_back(visual::CategoricalValue{record_ids[index], vertical + "/" + horizontal});
	}
	return values;
}

auto relation_json(const std::string &id, const std::string &name, const std::string &metric_name,
				   const std::string &dataset_id, const std::vector<std::string> &ids,
				   const mtrc::DistanceMatrix<double> &matrix, const MetricLawDiagnostics &diagnostics) -> std::string
{
	std::vector<std::string> rows;
	rows.reserve(matrix.rows());
	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		std::vector<double> values;
		values.reserve(matrix.columns());
		for (std::size_t column = 0; column < matrix.columns(); ++column) {
			values.push_back(matrix_value(matrix, row, column));
		}
		rows.push_back(visual::number_array(values));
	}
	const std::string metadata =
		visual::object({visual::string_field("metric", metric_name),
						visual::string_field("computed_by", "METRIC native C++ distance matrices"),
						visual::field("law_check",
									  visual::object({visual::string_field("diagonal", "checked_exhaustive"),
													  visual::string_field("symmetry", "checked_exhaustive"),
													  visual::string_field("triangle",
																		   diagnostics.triangle_exhaustive
																			   ? "checked_exhaustive"
																			   : "checked_deterministic_sample"),
													  visual::bool_field("finite", diagnostics.finite),
													  visual::bool_field("triangle_exhaustive",
																		 diagnostics.triangle_exhaustive),
													  visual::number_field("diagonal_max_abs",
																		   diagnostics.diagonal_max_abs),
													  visual::number_field("symmetry_max_abs",
																		   diagnostics.symmetry_max_abs),
													  visual::number_field("triangle_max_violation",
																		   diagnostics.triangle_max_violation),
													  visual::size_field("triangle_triplets",
																		 diagnostics.triangle_triplets)})),
						visual::size_field("pair_count", diagnostics.pair_count),
						visual::number_field("maximum_distance", diagnostics.maximum_distance),
						visual::number_field("average_distance", diagnostics.average_distance)});
	return visual::object({visual::string_field("id", id),
						   visual::string_field("dataset_id", dataset_id),
						   visual::string_field("name", name),
						   visual::string_field("relation_type", "metric"),
						   visual::string_field("value_type", "scalar"),
						   visual::string_array_field("record_ids", ids),
						   visual::string_field("storage", "dense_matrix"),
						   visual::field("values", visual::array_of(rows)),
						   visual::field("metadata", metadata)});
}

auto record_json(const std::string &dataset_id,
				 const cross_space::Dataset &dataset,
				 const std::vector<double> &latent_phase,
				 const std::vector<double> &local_correlation,
				 const std::vector<double> &local_support,
				 const std::vector<std::string> &ids) -> std::vector<std::string>
{
	std::vector<std::string> records;
	records.reserve(dataset.size());
	for (std::size_t index = 0; index < dataset.size(); ++index) {
		const std::string payload = visual::object({
			visual::string_field("kind", "paired_metric_record"),
			visual::field("components",
						  visual::object({
							  visual::field("event_log", visual::string_payload(dataset.logs[index])),
							  visual::field("process_curve", visual::series_payload(dataset.curves[index])),
						  })),
			visual::field("metadata",
						  visual::object({visual::size_field("pair_index", index),
										  visual::number_field("latent_severity", dataset.latent[index]),
										  visual::number_field("latent_phase", latent_phase[index]),
										  visual::number_field("local_mgc_correlation", local_correlation[index]),
										  visual::number_field("optimal_scale_support", local_support[index]),
										  visual::string_field("paired_record", "p_i=(event_log_i,process_curve_i)")})),
		});
		records.push_back(visual::object({visual::string_field("id", ids[index]),
										   visual::string_field("dataset_id", dataset_id),
										   visual::string_field("record_type", "paired_event_log_process_curve"),
										   visual::string_field("label", "paired record " + std::to_string(index)),
										   visual::field("payload", payload)}));
	}
	return records;
}

auto sampled_grid_json(const std::vector<std::vector<double>> &rows) -> std::string
{
	std::vector<std::string> json_rows;
	json_rows.reserve(rows.size());
	for (const auto &row : rows) {
		json_rows.push_back(visual::number_array(row));
	}
	return visual::array_of(json_rows);
}

auto compute_mgc_evidence(const mtrc::DistanceMatrix<double> &left, const mtrc::DistanceMatrix<double> &right,
						  double compare_statistic) -> MgcEvidence
{
	MgcEvidence evidence;
	mtrc::MGC_direct direct;
	evidence.statistic = direct(left, right);
	evidence.compare_statistic = compare_statistic;
	evidence.grid_rows = left.rows();
	evidence.grid_columns = right.rows();

	auto A = direct.center_distance_matrix(left);
	auto B = direct.center_distance_matrix(right);
	auto RXt = direct.center_ranked_distance_matrix(left);
	auto RYt = direct.center_ranked_distance_matrix(right);
	mtrc::numeric::DynamicMatrix<double> At = mtrc::numeric::trans(A);
	mtrc::numeric::DynamicMatrix<double> Bt = mtrc::numeric::trans(B);
	mtrc::numeric::DynamicMatrix<std::size_t> RX = mtrc::numeric::trans(RXt);
	mtrc::numeric::DynamicMatrix<std::size_t> RY = mtrc::numeric::trans(RYt);

	auto corr = direct.local_covariance(A, Bt, RX, RYt);
	auto varX = direct.local_covariance(A, At, RX, RXt);
	auto varY = direct.local_covariance(B, Bt, RY, RYt);
	direct.normalize_generalized_correlation(corr, varX, varY);
	auto significant = direct.significant_local_correlation(corr);

	evidence.maximal_scale_correlation = corr(corr.rows() - 1, corr.columns() - 1);
	evidence.optimal_k = corr.rows();
	evidence.optimal_l = corr.columns();
	evidence.significant_scale_cells = mtrc::numeric::nonZeros(significant);
	if (evidence.significant_scale_cells >= 2 * std::min<std::size_t>(corr.rows(), corr.columns())) {
		double best = evidence.maximal_scale_correlation;
		for (std::size_t row = 0; row < corr.rows(); ++row) {
			for (std::size_t column = 0; column < corr.columns(); ++column) {
				if (significant(row, column) && corr(row, column) >= best) {
					best = corr(row, column);
					evidence.optimal_k = row + 1;
					evidence.optimal_l = column + 1;
				}
			}
		}
	}

	const std::size_t sample = 40;
	evidence.sampled_grid.assign(sample, std::vector<double>(sample, 0.0));
	for (std::size_t row = 0; row < sample; ++row) {
		const std::size_t source_row = std::min<std::size_t>(
			corr.rows() - 1, static_cast<std::size_t>(std::llround(row * (corr.rows() - 1.0) / (sample - 1.0))));
		for (std::size_t column = 0; column < sample; ++column) {
			const std::size_t source_column = std::min<std::size_t>(
				corr.columns() - 1,
				static_cast<std::size_t>(std::llround(column * (corr.columns() - 1.0) / (sample - 1.0))));
			evidence.sampled_grid[row][column] = corr(source_row, source_column);
		}
	}

	return evidence;
}

auto mgc_diagnostic_json(const std::string &dataset_id,
						 const MgcEvidence &evidence,
						 const RecordCorrelationExtremes &extremes,
						 const std::vector<std::string> &record_ids) -> std::string
{
	return visual::object({
		visual::string_field("id", "single-space-mgc-operator"),
		visual::string_field("kind", "mgc_dependence_operator"),
		visual::string_field("dataset_id", dataset_id),
		visual::string_field("visible_space_id", "paired-record-space"),
		visual::string_field("algorithm", "mtrc::compare(..., mgc_options{}) + MGC_direct local scale evidence"),
		visual::number_field("statistic", evidence.statistic),
		visual::number_field("compare_statistic", evidence.compare_statistic),
		visual::number_field("maximal_scale_correlation", evidence.maximal_scale_correlation),
		visual::size_field("optimal_log_scale_k", evidence.optimal_k),
		visual::size_field("optimal_curve_scale_l", evidence.optimal_l),
		visual::size_field("significant_scale_cells", evidence.significant_scale_cells),
		visual::size_field("grid_rows", evidence.grid_rows),
		visual::size_field("grid_columns", evidence.grid_columns),
		visual::string_field("minimum_local_correlation_record", record_ids[extremes.minimum_index]),
		visual::number_field("minimum_local_correlation", extremes.minimum_value),
		visual::string_field("maximum_local_correlation_record", record_ids[extremes.maximum_index]),
		visual::number_field("maximum_local_correlation", extremes.maximum_value),
		visual::field("sampled_local_correlation_grid", sampled_grid_json(evidence.sampled_grid)),
	});
}

auto space_metadata_json() -> std::string
{
	return visual::object({
		visual::string_field("space_contract", "single finite metric space over paired records p_i=(x_i,y_i)"),
		visual::string_field("visible_record_set", "same paired records in every coordinate state"),
		visual::string_field("morph_states", "event-log metric, process-curve metric, product pair metric"),
	});
}

auto build_document(std::size_t record_count) -> visual::Document
{
	const std::string dataset_id = "mgc-paired-records";
	const auto generated = generate_paired_metric_dataset(record_count, kSeed);
	const auto &dataset = generated.data;
	const auto ids = make_record_ids(dataset.size());
	const auto matrices = cross_space::build_distance_matrices(dataset);
	const auto pair_matrix = product_pair_matrix(matrices.left, matrices.right);

	auto log_space = mtrc::make_space(dataset.logs, cross_space::LogMetric{});
	auto curve_space = mtrc::make_space(dataset.curves, cross_space::curve_metric());
	const double compare_statistic = mtrc::compare(log_space, curve_space, mtrc::stats::correlate::mgc_options{}).value;
	const auto mgc = compute_mgc_evidence(matrices.left, matrices.right, compare_statistic);

	const auto alignment = local_profile_alignment(matrices.left, matrices.right);
	const auto contributions = local_contribution(pair_contributions(matrices.left, matrices.right), dataset.size());
	std::vector<double> optimal_scale_support;
	const auto optimal_scale_correlation =
		optimal_scale_record_correlation(matrices.left, matrices.right, mgc, &optimal_scale_support);
	const auto correlation_extremes = find_extremes(alignment);
	const auto evidence = local_mgc_evidence(alignment, contributions);
	const auto disagreement = scale01(abs_values(alignment));
	const auto log_density = local_density(matrices.left);
	const auto curve_density = local_density(matrices.right);

	const auto log_coordinates = landmark_coordinates(matrices.left);
	const auto curve_coordinates = landmark_coordinates(matrices.right);
	const auto pair_coordinates = landmark_coordinates(pair_matrix);
	const auto log_landmarks = select_landmarks(matrices.left);
	const auto curve_landmarks = select_landmarks(matrices.right);
	const auto pair_landmarks = select_landmarks(pair_matrix);

	visual::Document document;
	document.provenance_json(visual::object({
		visual::string_field("writer", "experiments/mgc_paired_space/mgc_paired_single_space.cpp"),
		visual::string_field("writer_language", "C++"),
		visual::string_field("computation", "native METRIC C++"),
		visual::bool_field("native_export", true),
		visual::bool_field("synthetic_js", false),
		visual::string_field("source_example", "experiments/mgc_paired_space/mgc_paired_single_space.cpp"),
		visual::string_field("dataset_geometry", "two shared latent factors: severity and phase"),
		visual::size_field("record_count", record_count),
		visual::size_field("seed", kSeed),
	}));
	document.dataset(dataset_id, "MGC paired record space",
					 "One finite metric space whose records are paired event logs and process curves; MGC is "
					 "operator evidence over native Edit and TWED metrics.",
					 "METRIC deterministic native generator", "MPL-2.0");
	for (const auto &record :
		 record_json(dataset_id, dataset, generated.latent_phase, alignment, optimal_scale_support, ids)) {
		document.record_json(record);
	}
	document.relation_json(relation_json("event-log-edit-distance", "event-log metric state", "mtrc::Edit<char>",
										 dataset_id, ids, matrices.left, metric_law_diagnostics(matrices.left)));
	document.relation_json(relation_json("process-curve-twed-distance", "process-curve metric state",
										 "mtrc::TWED<double>", dataset_id, ids, matrices.right,
										 metric_law_diagnostics(matrices.right)));
	document.relation_json(relation_json("paired-product-distance", "product pair metric state",
										 "sqrt(normalized_edit^2 + normalized_twed^2)", dataset_id, ids,
										 pair_matrix, metric_law_diagnostics(pair_matrix)));
	document.space("paired-record-space", dataset_id, ids, "paired-product-distance", "finite_metric_space",
				   space_metadata_json());
	document.scalar_property("record-local-correlation", dataset_id, "record-local distance-profile correlation",
							 scalar_values(ids, alignment),
							 visual::object({visual::string_field(
								 "meaning",
								 "per-record Pearson correlation between the two native distance profiles d_X(q,*) and d_Y(q,*)")}));
	document.scalar_property("signed-optimal-scale-local-correlation", dataset_id,
							 "signed local correlation at MGC optimal scale",
							 scalar_values(ids, optimal_scale_correlation),
							 visual::object({visual::string_field(
								 "meaning",
								 "signed centered-distance product contribution restricted to the MGC optimal k,l scale")}));
	document.scalar_property("optimal-scale-support", dataset_id, "records inside MGC optimal local window",
							 scalar_values(ids, optimal_scale_support));
	document.scalar_property("local-mgc-evidence", dataset_id, "local MGC evidence", scalar_values(ids, evidence),
							 visual::object({visual::string_field("meaning",
																  "absolute local contribution blended with local distance-profile alignment")}));
	document.scalar_property("local-profile-alignment", dataset_id, "local distance-profile alignment",
							 scalar_values(ids, alignment));
	document.scalar_property("local-dependence-contribution", dataset_id, "local dependence contribution",
							 scalar_values(ids, contributions));
	document.scalar_property("metric-view-disagreement", dataset_id, "metric-view disagreement",
							 scalar_values(ids, disagreement));
	document.scalar_property("event-log-local-density", dataset_id, "event-log local density",
							 scalar_values(ids, log_density));
	document.scalar_property("process-curve-local-density", dataset_id, "process-curve local density",
							 scalar_values(ids, curve_density));
	document.scalar_property("latent-severity", dataset_id, "latent severity", scalar_values(ids, dataset.latent));
	document.scalar_property("latent-phase", dataset_id, "latent phase", scalar_values(ids, generated.latent_phase));
	document.categorical_property("local-correlation-extreme", dataset_id, "local correlation extrema",
								  local_correlation_extreme_values(ids, correlation_extremes));
	document.categorical_property("latent-region", dataset_id, "latent 2D region",
								  latent_region_values(ids, dataset.latent, generated.latent_phase));
	document.coordinates3("event-log-metric-3d", dataset_id, "paired-record-space",
						  "paired records under event-log Edit metric", positions3(ids, log_coordinates),
						  coordinate_metadata_json(ids, log_landmarks, "mtrc::Edit<char> on event_log component"));
	document.coordinates3("process-curve-metric-3d", dataset_id, "paired-record-space",
						  "paired records under process-curve TWED metric", positions3(ids, curve_coordinates),
						  coordinate_metadata_json(ids, curve_landmarks, "mtrc::TWED<double> on process_curve component"));
	document.coordinates3("paired-product-metric-3d", dataset_id, "paired-record-space",
						  "paired records under product pair metric", positions3(ids, pair_coordinates),
						  coordinate_metadata_json(ids, pair_landmarks,
												   "sqrt(normalized Edit^2 + normalized TWED^2)"));
	document.view_json(visual::object({visual::string_field("id", "mgc-single-space-view"),
									   visual::string_field("kind", "metric-space"),
									   visual::string_field("name", "MGC paired record space"),
									   visual::string_field("spaceId", "paired-record-space"),
									   visual::string_field("coordinateId", "paired-product-metric-3d"),
									   visual::string_field("propertyId", "record-local-correlation"),
									   visual::string_field("diagnosticId", "single-space-mgc-operator")}));
	document.diagnostic_json(mgc_diagnostic_json(dataset_id, mgc, correlation_extremes, ids));
	return document;
}

auto write_html(const std::filesystem::path &path, const std::string &visual_import_url) -> void
{
	std::ofstream out(path);
	if (!out) {
		throw std::runtime_error("failed to write " + path.string());
	}
	out << R"HTML(<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <title>METRIC — Single-space MGC Morph</title>
  <style>
    :root {
      color-scheme: light;
      --ink:#172326;
      --muted:#556568;
      --panel:rgba(248,250,246,.82);
      --line:rgba(24,35,37,.16);
      --accent:#1b7583;
    }
    * { box-sizing:border-box; }
    html, body { width:100%; height:100%; margin:0; overflow:hidden; }
    body {
      background:radial-gradient(circle at 48% 42%, rgba(250,252,246,.92), rgba(229,237,232,.96) 62%, #dbe5e1 100%);
      color:var(--ink);
      font-family:Inter, ui-sans-serif, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
    }
    main { position:fixed; inset:0; }
    #scene { position:absolute; inset:0; width:100%; height:100%; display:block; touch-action:none; cursor:grab; }
    #scene:active { cursor:grabbing; }
    .analysis-tag {
      position:absolute; top:18px; left:18px; z-index:4;
      max-width:min(560px, calc(100vw - 36px));
      padding:10px 12px;
      border:1px solid var(--line);
      border-radius:12px;
      background:var(--panel);
      backdrop-filter:blur(18px) saturate(1.08);
      box-shadow:0 18px 48px rgba(28,42,40,.11);
    }
    .analysis-tag strong { display:block; margin-bottom:4px; font-size:13px; line-height:1.18; font-weight:820; }
    .analysis-tag strong b { font-size:19px; letter-spacing:0; margin-left:3px; }
    .analysis-tag span { display:block; color:var(--muted); font-size:11px; line-height:1.35; }
    .analysis-tag code { display:block; margin-top:4px; color:#223236; font:650 10.5px/1.35 ui-monospace,SFMono-Regular,Menlo,monospace; }
    .controls {
      position:absolute; left:50%; bottom:18px; transform:translateX(-50%); z-index:4;
      width:min(820px, calc(100vw - 28px));
      display:grid;
      grid-template-columns:1fr 210px;
      gap:9px 14px;
      align-items:center;
      padding:11px 12px;
      border:1px solid var(--line);
      border-radius:14px;
      background:rgba(249,250,246,.78);
      backdrop-filter:blur(18px) saturate(1.08);
      box-shadow:0 18px 56px rgba(28,42,40,.13);
    }
    .state-buttons { display:flex; gap:7px; flex-wrap:wrap; }
    button {
      min-height:34px; padding:0 11px;
      border:1px solid rgba(24,35,37,.16);
      border-radius:999px;
      background:rgba(255,255,252,.68);
      color:#223236;
      font:760 12px/1 ui-sans-serif,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;
    }
    button.active { background:#172326; color:#fff; }
    .readout { color:#27383b; font:720 12px/1.35 ui-monospace,SFMono-Regular,Menlo,monospace; text-align:right; white-space:pre; }
    input[type=range] { grid-column:1 / -1; width:100%; height:18px; margin:0; appearance:none; background:transparent; }
    input[type=range]::-webkit-slider-runnable-track { height:3px; border-radius:999px; background:rgba(32,48,52,.22); }
    input[type=range]::-webkit-slider-thumb {
      appearance:none; width:14px; height:14px; margin-top:-5.5px; border-radius:999px;
      background:var(--accent); border:2px solid rgba(249,250,246,.95); box-shadow:0 3px 10px rgba(22,48,54,.22);
    }
    body .mtrc-record-preview {
      width:min(430px, calc(100vw - 28px));
      padding:0;
      overflow:hidden auto;
      border-radius:10px;
      background:rgba(251,251,244,.98);
    }
    .paired-card { display:block; }
    .paired-card__log {
      padding:10px 11px 8px;
      border-bottom:1px solid rgba(24,35,37,.12);
      font:700 12px/1.45 ui-monospace,SFMono-Regular,Menlo,monospace;
      color:#16282b;
      overflow-wrap:anywhere;
      background:rgba(255,255,250,.96);
    }
    .paired-card__curve { display:block; width:100%; height:116px; background:#07100f; }
    .paired-card__caption {
      display:flex; justify-content:space-between; gap:10px;
      padding:7px 9px;
      color:#172326;
      font:750 11px/1.2 ui-sans-serif,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;
    }
    .paired-card__caption span:last-child { color:#526166; font-family:ui-monospace,SFMono-Regular,Menlo,monospace; font-size:10px; }
    .loading, .error { position:absolute; inset:0; display:grid; place-items:center; z-index:6; color:#243236; }
    .error { color:#842525; }
    @media (max-width: 760px) {
      .analysis-tag { top:12px; left:12px; right:12px; }
      .controls { grid-template-columns:1fr; }
      .readout { text-align:left; }
    }
  </style>
</head>
<body>
  <main>
    <canvas id="scene" aria-label="One finite metric space of paired records with MGC evidence"></canvas>
    <section class="analysis-tag" aria-label="single-space MGC analysis">
      <strong>Gesamt-MGC-Korrelation <b id="global-mgc">...</b></strong>
      <span>Ein sichtbarer Metric Space aller gepaarten Records. Farbe = lokale Korrelation der beiden Distanzprofile d_X(q,*) und d_Y(q,*); beschriftet sind Minimum und Maximum.</span>
      <code id="metric-code">mtrc::compare(log_space, curve_space, mgc_options{})</code>
    </section>
    <div class="controls" aria-label="metric-state morph controls">
      <div class="state-buttons" id="state-buttons"></div>
      <div class="readout" id="readout">loading</div>
      <input id="morph" type="range" min="0" max="1" step="0.001" value="0" aria-label="metric morph progress">
    </div>
    <div class="loading" id="loading">Loading native MGC evidence...</div>
  </main>
  <script type="module">
)HTML";
	out << "    import { createMetricVisual } from \"" << visual::escape(visual_import_url) << "\";\n";
	out << R"HTML(

    const canvas = document.getElementById("scene");
    const loading = document.getElementById("loading");
    const readout = document.getElementById("readout");
    const morph = document.getElementById("morph");
    const buttons = document.getElementById("state-buttons");
    const code = document.getElementById("metric-code");

    const globalMgc = document.getElementById("global-mgc");

    const STATES = [
      { id: "log", label: "Log-Nachbarschaften", coordinateId: "event-log-metric-3d", metric: "Event-Logs mit Edit-Distanz" },
      { id: "curve", label: "Kurven-Nachbarschaften", coordinateId: "process-curve-metric-3d", metric: "Prozesskurven mit TWED" },
      { id: "pair", label: "Gemeinsame Pair-Metrik", coordinateId: "paired-product-metric-3d", metric: "Produktmetrik aus Edit und TWED" },
    ];

    let visual = null;
    let document_ = null;
    let fromIndex = 2;
    let toIndex = 2;
    let animationId = 0;

    try {
      const response = await fetch("metric.visual.json", { cache: "no-store" });
      if (!response.ok) throw new Error(`metric.visual.json ${response.status}`);
      document_ = await response.json();
      const diagnostic = document_.diagnostics?.find((entry) => entry.id === "single-space-mgc-operator");
      const statistic = Number(diagnostic?.statistic);
      const minValue = Number(diagnostic?.minimum_local_correlation);
      const maxValue = Number(diagnostic?.maximum_local_correlation);
      globalMgc.textContent = Number.isFinite(statistic) ? statistic.toFixed(4) : "n/a";
      code.textContent = `optimal k=${diagnostic?.optimal_log_scale_k ?? "?"}, l=${diagnostic?.optimal_curve_scale_l ?? "?"} · min ${diagnostic?.minimum_local_correlation_record ?? "?"} ${Number.isFinite(minValue) ? minValue.toExponential(2) : "n/a"} · max ${diagnostic?.maximum_local_correlation_record ?? "?"} ${Number.isFinite(maxValue) ? maxValue.toExponential(2) : "n/a"}`;

      visual = await createMetricVisual({
        canvas,
        evidence: document_,
        profileId: "mgc-paired-single-space-profile",
        runtime: {
          camera: { fov: 36, target: [0, -0.08, 0], yaw: 0.64, pitch: 0.46, radius: 5.8 },
          controls: { rotateSpeed: 0.0038 },
        },
      });
      buildButtons();
      renderState(2, 2, 0);
      visual.enableRecordPreview({ mode: "record", panel: { render: renderPairedRecord } });
      visual.setMotion("studio-drift", { start: true });
      loading.remove();
      document.documentElement.dataset.metricMgcSingleSpace = "ready";
      document.documentElement.dataset.metricRecordCount = String(document_.records?.length || 0);
      document.documentElement.dataset.metricMgcStatistic = Number.isFinite(statistic) ? statistic.toFixed(6) : "";
      window.metricMgcSingleSpace = { visual, document: document_, getState: () => visual.getState() };
    } catch (error) {
      console.error(error);
      loading.className = "loading error";
      loading.textContent = error instanceof Error ? error.message : String(error);
    }

    morph.addEventListener("input", () => {
      cancelAnimationFrame(animationId);
      const progress = Number(morph.value);
      applyProgress(progress);
    });

    function buildButtons() {
      buttons.replaceChildren();
      STATES.forEach((state, index) => {
        const button = document.createElement("button");
        button.type = "button";
        button.textContent = state.label;
        button.addEventListener("click", () => morphTo(index));
        buttons.appendChild(button);
      });
      updateButtonState();
    }

    function morphTo(nextIndex) {
      if (!visual || nextIndex === toIndex) return;
      cancelAnimationFrame(animationId);
      fromIndex = toIndex;
      toIndex = nextIndex;
      renderState(fromIndex, toIndex, 0);
      const started = performance.now();
      const duration = 940;
      const tick = (now) => {
        const raw = Math.min(1, (now - started) / duration);
        const eased = raw * raw * (3 - 2 * raw);
        morph.value = String(eased);
        applyProgress(eased);
        if (raw < 1) animationId = requestAnimationFrame(tick);
      };
      animationId = requestAnimationFrame(tick);
    }

    function renderState(sourceIndex, targetIndex, progress) {
      const source = STATES[sourceIndex];
      const target = STATES[targetIndex];
      visual.showMetricSpace({
        spaceId: "paired-record-space",
        coordinateId: source.coordinateId,
        targetCoordinateId: target.coordinateId,
        scalarProperty: "record-local-correlation",
        colorProperty: "record-local-correlation",
        labelProperty: "local-correlation-extreme",
        targetRadius: 1.42,
        pointSize: 1.55,
        pointAlpha: 0.9,
        groundScale: 2.35,
        groundProjectionAlpha: 0.28,
        loop: false,
        progress,
        preview: "record",
        previewOptions: { panel: { render: renderPairedRecord } },
      });
      morph.value = String(progress);
      updateReadout(progress);
      updateButtonState();
    }

    function applyProgress(progress) {
      visual?.applyMappingMotionProgress(progress, { render: true, phase: `${STATES[fromIndex].id}->${STATES[toIndex].id}` });
      updateReadout(progress);
      updateButtonState();
    }

    function updateButtonState() {
      [...buttons.children].forEach((button, index) => {
        button.classList.toggle("active", index === toIndex);
      });
    }

    function updateReadout(progress) {
      const target = STATES[toIndex];
      readout.textContent = `${target.label}\nMorph ${progress.toFixed(3)}`;
    }

    function renderPairedRecord(preview, input = {}) {
      const recordId = String(input.recordId || input.focusTarget?.recordId || "");
      const record = document_.records?.find((item) => String(item.id) === recordId);
      if (!record) return null;
      const components = record.payload?.components || {};
      const eventLog = components.event_log?.text || "";
      const curve = components.process_curve?.series || [];
      const article = document.createElement("article");
      article.className = "paired-card";
      const log = document.createElement("div");
      log.className = "paired-card__log";
      log.textContent = eventLog;
      const curveCanvas = document.createElement("canvas");
      curveCanvas.className = "paired-card__curve";
      curveCanvas.width = 430;
      curveCanvas.height = 116;
      drawCurve(curveCanvas, curve);
      const caption = document.createElement("div");
      caption.className = "paired-card__caption";
      const left = document.createElement("span");
      left.textContent = record.id;
      const right = document.createElement("span");
      const idx = record.payload?.metadata?.pair_index;
      const local = Number(record.payload?.metadata?.local_mgc_correlation);
      const support = Number(record.payload?.metadata?.optimal_scale_support);
      right.textContent = `p_${idx ?? "?"} · lokal ${Number.isFinite(local) ? local.toExponential(2) : "n/a"} · n=${Number.isFinite(support) ? support.toFixed(0) : "?"}`;
      caption.append(left, right);
      article.append(log, curveCanvas, caption);
      return article;
    }

    function drawCurve(canvas, values) {
      const ctx = canvas.getContext("2d");
      const width = canvas.width;
      const height = canvas.height;
      ctx.clearRect(0, 0, width, height);
      ctx.fillStyle = "#07100f";
      ctx.fillRect(0, 0, width, height);
      const numeric = values.map(Number).filter(Number.isFinite);
      if (numeric.length < 2) return;
      const min = Math.min(...numeric);
      const max = Math.max(...numeric);
      const span = Math.max(1e-9, max - min);
      ctx.strokeStyle = "rgba(255,255,255,.12)";
      ctx.lineWidth = 1;
      for (let y = 22; y < height; y += 24) {
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(width, y);
        ctx.stroke();
      }
      ctx.strokeStyle = "#6bd0dc";
      ctx.lineWidth = 3;
      ctx.lineJoin = "round";
      ctx.lineCap = "round";
      ctx.beginPath();
      numeric.forEach((value, index) => {
        const x = 12 + (index / (numeric.length - 1)) * (width - 24);
        const y = height - 14 - ((value - min) / span) * (height - 28);
        if (index === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
      });
      ctx.stroke();
    }
  </script>
</body>
</html>
)HTML";
}

} // namespace

int main(int argc, char **argv)
{
	try {
		const auto options = parse_options(argc, argv);
		std::filesystem::create_directories(options.export_dir);
		const auto document = build_document(options.records);
		if (!document.write_file(options.export_dir / "metric.visual.json")) {
			throw std::runtime_error("failed to write metric.visual.json");
		}
		write_html(options.export_dir / "index.html", options.visual_import_url);
		std::cout << "records=" << options.records << "\n";
		std::cout << "wrote " << (options.export_dir / "index.html") << "\n";
		std::cout << "pipeline paired records -> Edit/TWED distance matrices -> MGC -> single-space metric morph\n";
		return 0;
	} catch (const std::exception &error) {
		std::cerr << "error: " << error.what() << "\n";
		return 1;
	}
}
