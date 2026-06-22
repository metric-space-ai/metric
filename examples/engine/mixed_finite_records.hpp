// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Hero application: "Mixed Finite Metric Records".
//
// One heterogeneous record carries four genuinely different fields:
//
//   * code     -- a short operator status / log token (text)
//   * spectrum -- a unit-mass intensity histogram (image / vibration signal)
//   * curve    -- a variable-length process curve (time series)
//   * vitals   -- a fixed-dimension numeric sensor readout
//
// Each field is compared with a *real, admitted* METRIC true metric
// (metric_law::metric), never a hand-rolled stand-in:
//
//   * code     -> mtrc::Edit<char>              (Levenshtein, record_kind::sequence)
//   * spectrum -> mtrc::Wasserstein<double>     (1-Wasserstein / EMD on a line, structured)
//   * curve    -> mtrc::TWED<double>            (Time-Warp Edit Distance, sequence)
//   * vitals   -> mtrc::Euclidean_standardized  (fitted positive scale, aligned_vector)
//
// The composite distance is a conic (non-negative-weighted) sum of those four
// true metrics over the product space:
//
//   D(x, y) = w_code     * Edit(x.code,     y.code)
//           + w_spectrum  * W1 (x.spectrum, y.spectrum)
//           + w_curve     * TWED(x.curve,    y.curve)
//           + w_vitals    * Estd(x.vitals,   y.vitals)
//
// With strictly positive, *constant* weights and each component a true metric,
// D is itself a true metric on the product domain:
//
//   * non-negativity      -- sum of non-negative terms
//   * symmetry            -- each component is symmetric
//   * triangle inequality -- a non-negative-weighted sum of triangle-respecting
//                            terms still respects the triangle inequality
//   * identity of indisc. -- D(x, y) = 0  <=>  every weighted term is 0
//                            <=> every field distance is 0 (weights > 0)
//                            <=> every field is equal (each component separates
//                                its field) <=> x == y
//
// The weights are fixed numbers baked into the metric instance. They are NOT
// per-pair normalizers: dividing a field distance by, e.g., the longer of the
// two strings would break symmetry's scaling and the triangle inequality, so
// that "length-normalized edit distance" is deliberately avoided here. Scaling a
// true metric by a positive constant is still a true metric, which is all the
// weights do. This is why the trait below can honestly declare
// metric_law::metric, and why metric_mixed_records_smoke proves the axioms.
//
// This header is the single source of truth shared by the runnable example
// (mixed_finite_records.cpp) and the CI metric-proof (metric_mixed_records_smoke.cpp).

#ifndef METRIC_EXAMPLES_ENGINE_MIXED_FINITE_RECORDS_HPP
#define METRIC_EXAMPLES_ENGINE_MIXED_FINITE_RECORDS_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include <metric/metric/catalog.hpp>

namespace hero {

// ---------------------------------------------------------------------------
// Record domain
// ---------------------------------------------------------------------------

inline constexpr std::size_t kSpectrumBins = 16; // ground space of the histogram
inline constexpr std::size_t kVitalsDim = 4;     // numeric sensor readout width

struct MixedRecord {
	std::string code;             // text token        -> Edit
	std::vector<double> spectrum; // unit-mass, length kSpectrumBins -> Wasserstein
	std::vector<double> curve;    // variable length   -> TWED
	std::vector<double> vitals;   // length kVitalsDim  -> Euclidean_standardized

	friend auto operator==(const MixedRecord &lhs, const MixedRecord &rhs) -> bool
	{
		return lhs.code == rhs.code && lhs.spectrum == rhs.spectrum && lhs.curve == rhs.curve &&
			   lhs.vitals == rhs.vitals;
	}
	friend auto operator!=(const MixedRecord &lhs, const MixedRecord &rhs) -> bool { return !(lhs == rhs); }
};

// Per-field weighted contributions, for explainability / auditing.
struct Contributions {
	double code{};
	double spectrum{};
	double curve{};
	double vitals{};

	auto total() const -> double { return code + spectrum + curve + vitals; }
};

// ---------------------------------------------------------------------------
// Composite true metric
// ---------------------------------------------------------------------------

class MixedRecordMetric {
  public:
	struct Weights {
		double code{1.0};
		double spectrum{1.0};
		double curve{1.0};
		double vitals{1.0};
	};

	// `vitals_metric` is a *fitted* standardized Euclidean metric (positive scale)
	// so it is already an admitted true metric; the composite just rescales it.
	MixedRecordMetric(std::size_t bins, mtrc::Euclidean_standardized<double> vitals_metric, Weights weights,
					  mtrc::TWED<double> curve_metric = mtrc::TWED<double>{0.0, 1.0})
		: edit_{}, spectrum_(mtrc::Wasserstein<double>::on_line(bins)), curve_(curve_metric),
		  vitals_(std::move(vitals_metric)), weights_(weights), bins_(bins)
	{
		validate_weights();
	}

	auto operator()(const MixedRecord &lhs, const MixedRecord &rhs) const -> double
	{
		return contributions(lhs, rhs).total();
	}

	auto contributions(const MixedRecord &lhs, const MixedRecord &rhs) const -> Contributions
	{
		Contributions out;
		out.code = weights_.code * static_cast<double>(edit_(lhs.code, rhs.code));
		out.spectrum = weights_.spectrum * spectrum_(lhs.spectrum, rhs.spectrum);
		out.curve = weights_.curve * curve_(lhs.curve, rhs.curve);
		out.vitals = weights_.vitals * vitals_(lhs.vitals, rhs.vitals);
		return out;
	}

	auto weights() const -> const Weights & { return weights_; }
	auto bins() const -> std::size_t { return bins_; }

	// Exposed so the trait cache_key can fold in the full identity of the
	// parameterized field metrics (the fitted vitals scale and the TWED params);
	// two composites with the same weights but a different fitted scale are
	// different metrics and must not collide to one cache key.
	auto vitals_metric() const -> const mtrc::Euclidean_standardized<double> & { return vitals_; }
	auto curve_metric() const -> const mtrc::TWED<double> & { return curve_; }

  private:
	void validate_weights() const
	{
		const std::array<double, 4> all{weights_.code, weights_.spectrum, weights_.curve, weights_.vitals};
		for (const auto w : all) {
			if (!std::isfinite(w)) {
				throw std::invalid_argument("MixedRecordMetric requires finite weights");
			}
			// Strictly positive weights are required for identity of
			// indiscernibles; a zero weight would drop a field and demote the
			// composite to a pseudometric, which the metric trait must not claim.
			if (w <= 0.0) {
				throw std::invalid_argument("MixedRecordMetric requires strictly positive weights (w > 0)");
			}
		}
	}

	mtrc::Edit<char> edit_;
	mtrc::Wasserstein<double> spectrum_;
	mtrc::TWED<double> curve_;
	mtrc::Euclidean_standardized<double> vitals_;
	Weights weights_;
	std::size_t bins_;
};

// ---------------------------------------------------------------------------
// Naive vector baseline
// ---------------------------------------------------------------------------
//
// What a practitioner reaches for when they "just want a feature vector": flatten
// every field into one numeric vector and run ordinary Euclidean nearest
// neighbours. The projection is faithful to the data yet structurally blind:
//
//   * the text becomes a 27-bin character histogram -> ordering is gone
//     (anagrams collide, a single transposition looks identical),
//   * the spectrum bins are copied verbatim -> bin-wise L2 ignores the ground
//     distance, so a one-bin and a ten-bin peak shift look equally far,
//   * the curve is forced to a fixed width by zero padding/truncation -> a
//     time-warped or shorter curve is wrecked by positional misalignment,
//   * the vitals are copied verbatim -> the largest-scale coordinate dominates
//     and swamps the small-scale ones (no standardization).

inline constexpr std::size_t kFlatCurveWidth = 12;

inline auto char_histogram(const std::string &text) -> std::vector<double>
{
	std::vector<double> features(27, 0.0);
	for (const auto ch : text) {
		const auto lowered = static_cast<char>((ch >= 'A' && ch <= 'Z') ? ch - 'A' + 'a' : ch);
		if (lowered >= 'a' && lowered <= 'z') {
			features[static_cast<std::size_t>(lowered - 'a')] += 1.0;
		}
	}
	features.back() = static_cast<double>(text.size());
	return features;
}

inline auto flat_projection(const MixedRecord &record) -> std::vector<double>
{
	std::vector<double> flat;
	flat.reserve(27 + kSpectrumBins + kFlatCurveWidth + kVitalsDim);

	const auto code_features = char_histogram(record.code);
	flat.insert(flat.end(), code_features.begin(), code_features.end());

	flat.insert(flat.end(), record.spectrum.begin(), record.spectrum.end());

	for (std::size_t i = 0; i < kFlatCurveWidth; ++i) {
		flat.push_back(i < record.curve.size() ? record.curve[i] : 0.0);
	}

	flat.insert(flat.end(), record.vitals.begin(), record.vitals.end());
	return flat;
}

inline auto flat_projection(const std::vector<MixedRecord> &records) -> std::vector<std::vector<double>>
{
	std::vector<std::vector<double>> out;
	out.reserve(records.size());
	for (const auto &record : records) {
		out.push_back(flat_projection(record));
	}
	return out;
}

// Plain Euclidean over the flattened vectors -- the literal "feature vector"
// baseline. Length-guarded: the flat projection is always the same width, but the
// guard makes the comparison total instead of silently reading past the shorter
// vector.
struct FlatEuclidean {
	auto operator()(const std::vector<double> &lhs, const std::vector<double> &rhs) const -> double
	{
		const std::size_t n = std::min(lhs.size(), rhs.size());
		double squared = 0.0;
		for (std::size_t i = 0; i < n; ++i) {
			const auto delta = lhs[i] - rhs[i];
			squared += delta * delta;
		}
		for (std::size_t i = n; i < lhs.size(); ++i) {
			squared += lhs[i] * lhs[i];
		}
		for (std::size_t i = n; i < rhs.size(); ++i) {
			squared += rhs[i] * rhs[i];
		}
		return std::sqrt(squared);
	}
};

// Per-column z-scoring fitted over a set of flat vectors. This is the *fair*
// non-METRIC baseline: it gives the flat Euclidean the same per-coordinate
// standardization the composite already applies to its vitals field, so any
// remaining gap is due to genuine field structure (transport, elastic alignment,
// edit order), not a raw-scale mismatch. Constant columns are zeroed.
struct FlatStandardizer {
	std::vector<double> mean;
	std::vector<double> inv_std;

	auto apply(const std::vector<double> &v) const -> std::vector<double>
	{
		std::vector<double> out(v.size(), 0.0);
		const std::size_t n = std::min(v.size(), mean.size());
		for (std::size_t i = 0; i < n; ++i) {
			out[i] = (v[i] - mean[i]) * inv_std[i];
		}
		return out;
	}
};

inline auto fit_flat_standardizer(const std::vector<std::vector<double>> &flats) -> FlatStandardizer
{
	FlatStandardizer scaler;
	if (flats.empty()) {
		return scaler;
	}
	const std::size_t dim = flats.front().size();
	scaler.mean.assign(dim, 0.0);
	scaler.inv_std.assign(dim, 0.0);
	for (const auto &flat : flats) {
		for (std::size_t i = 0; i < dim; ++i) {
			scaler.mean[i] += flat[i];
		}
	}
	for (auto &m : scaler.mean) {
		m /= static_cast<double>(flats.size());
	}
	std::vector<double> variance(dim, 0.0);
	for (const auto &flat : flats) {
		for (std::size_t i = 0; i < dim; ++i) {
			const auto delta = flat[i] - scaler.mean[i];
			variance[i] += delta * delta;
		}
	}
	for (std::size_t i = 0; i < dim; ++i) {
		const auto sigma = std::sqrt(variance[i] / static_cast<double>(flats.size()));
		scaler.inv_std[i] = sigma > 0.0 ? 1.0 / sigma : 0.0; // zero out constant columns
	}
	return scaler;
}

inline auto standardized_flat_projection(const std::vector<MixedRecord> &records, const FlatStandardizer &scaler)
	-> std::vector<std::vector<double>>
{
	std::vector<std::vector<double>> out;
	out.reserve(records.size());
	for (const auto &record : records) {
		out.push_back(scaler.apply(flat_projection(record)));
	}
	return out;
}

// ---------------------------------------------------------------------------
// Helpers to build valid records
// ---------------------------------------------------------------------------

// Normalize a non-negative histogram to unit total mass so every spectrum lives
// on the same equal-mass simplex that the strict Wasserstein metric requires.
inline auto unit_mass(std::vector<double> bins) -> std::vector<double>
{
	double total = 0.0;
	for (const auto value : bins) {
		total += value;
	}
	if (total <= 0.0) {
		throw std::invalid_argument("spectrum must carry positive mass");
	}
	for (auto &value : bins) {
		value /= total;
	}
	return bins;
}

// A single-peak spectrum at `peak`, with a small triangular skirt and a flat
// background so the distribution is non-degenerate, then normalized to unit mass.
inline auto peak_spectrum(std::size_t peak, double sharpness = 1.0) -> std::vector<double>
{
	std::vector<double> bins(kSpectrumBins, 0.02); // flat background
	for (std::size_t i = 0; i < kSpectrumBins; ++i) {
		const auto distance = std::abs(static_cast<double>(i) - static_cast<double>(peak));
		bins[i] += std::max(0.0, 1.0 - sharpness * distance);
	}
	return unit_mass(std::move(bins));
}

// ---------------------------------------------------------------------------
// Fitted vitals metric
// ---------------------------------------------------------------------------

inline auto fit_vitals_metric(const std::vector<MixedRecord> &records) -> mtrc::Euclidean_standardized<double>
{
	std::vector<std::vector<double>> vitals;
	vitals.reserve(records.size());
	for (const auto &record : records) {
		vitals.push_back(record.vitals);
	}
	return mtrc::Euclidean_standardized<double>(vitals);
}

} // namespace hero

// Declare the composite as an admitted true metric. The proof lives in
// tests/core_smoke/metric_mixed_records_smoke.cpp (axiom battery + randomized
// property search). The type lives in a *named* namespace, so this single
// specialization is consistent across every translation unit that includes it.
namespace mtrc::core {

template <> struct metric_traits<::hero::MixedRecordMetric> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::structured;
	static constexpr bool thread_safe = true;

	static auto cache_key(const ::hero::MixedRecordMetric &metric) -> std::string
	{
		const auto &w = metric.weights();
		// Fold in the full identity of every field metric, including the fitted
		// vitals scale (sigma) and the TWED parameters, so distinct composites
		// never alias to one persisted/diagnostic representation key.
		return std::string("hero::MixedRecordMetric:bins=") + std::to_string(metric.bins()) +
			   ":w_code=" + std::to_string(w.code) + ":w_spectrum=" + std::to_string(w.spectrum) +
			   ":w_curve=" + std::to_string(w.curve) + ":w_vitals=" + std::to_string(w.vitals) +
			   ":vitals=" + metric_cache_key(metric.vitals_metric()) +
			   ":curve=" + metric_cache_key(metric.curve_metric());
	}
};

} // namespace mtrc::core

#endif // METRIC_EXAMPLES_ENGINE_MIXED_FINITE_RECORDS_HPP
