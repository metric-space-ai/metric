// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Deterministic fixture for the "Mixed Finite Metric Records" hero application.
//
// There is no runtime randomness: every record is a closed-form function of a
// (fault family, severity) pair, plus a small set of hand-built probes that
// isolate one field at a time. The generative model is split so that
//
//   * FAULT FAMILY  is carried by the text code, the process-curve *phase*, and
//     the large-scale vital -> this is what clustering / representatives recover;
//   * SEVERITY      is carried by the spectrum *peak position*, the process-curve
//     *amplitude*, and the small-scale vital -> this is the latent outcome the
//     cross-space MGC test correlates against.
//
// The naive flattened-vector baseline mixes these axes badly: a sharp moving
// spectrum peak saturates bin-wise L2, a family-dependent curve phase swamps the
// positional L2, and an un-standardized vital on a 500x larger scale drowns the
// severity coordinate. The composite true metric keeps them separate, which is
// the whole point.

#ifndef METRIC_EXAMPLES_ENGINE_MIXED_FINITE_RECORDS_FIXTURE_HPP
#define METRIC_EXAMPLES_ENGINE_MIXED_FINITE_RECORDS_FIXTURE_HPP

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "mixed_finite_records.hpp"

namespace hero {

// ---------------------------------------------------------------------------
// Generative fleet (family x severity)
// ---------------------------------------------------------------------------

inline constexpr std::size_t kFamilyCount = 4;
inline constexpr std::size_t kSeverityLevels = 5;

struct LabeledRecord {
	MixedRecord record;
	std::string family; // ground-truth fault family
	double severity{};  // latent outcome in [0, kSeverityLevels - 1]
};

inline auto family_code(std::size_t family) -> std::string
{
	// Distinct letter multisets so the character histogram separates families,
	// but identical within a family so it carries no severity signal at all.
	static const std::array<std::string, kFamilyCount> stems{"valveflow", "bearingrun", "pumphead", "filterbed"};
	return stems[family];
}

// One process curve: a triangular bump whose CENTER encodes the family (a phase
// nuisance for severity) and whose HEIGHT encodes the severity.
inline auto family_curve(std::size_t family, std::size_t severity) -> std::vector<double>
{
	const std::size_t length = 12;
	const auto center = 2 + 2 * family;                  // family phase: 2, 4, 6, 8
	const auto amplitude = 1.0 + 0.8 * static_cast<double>(severity); // severity height
	std::vector<double> curve(length, 0.0);
	for (std::size_t i = 0; i < length; ++i) {
		const auto distance = std::abs(static_cast<double>(i) - static_cast<double>(center));
		curve[i] = std::max(0.0, amplitude * (1.0 - 0.5 * distance));
	}
	return curve;
}

// Numeric vitals: coordinate 0 is the small-scale severity signal; coordinate 1
// is a 500x-larger family signal that an un-standardized L2 fixates on.
inline auto family_vitals(std::size_t family, std::size_t severity) -> std::vector<double>
{
	const auto f = static_cast<double>(family);
	const auto k = static_cast<double>(severity);
	return {
		1.5 * k,                 // severity (small scale)
		500.0 * (f + 1.0),       // family (large scale)
		5.0 + 0.5 * f + 0.3 * k, // mixed mid-scale
		2.0 + 0.4 * k - 0.2 * f, // mixed mid-scale
	};
}

// Spectrum: a sharp moving peak at bin 3 + 2*severity. Adjacent severities sit two
// bins apart, so bin-wise L2 between them is near-saturated while the Wasserstein
// transport distance stays exactly proportional to the severity gap.
inline auto family_spectrum(std::size_t severity) -> std::vector<double>
{
	const auto peak = 3 + 2 * severity; // 3, 5, 7, 9, 11
	return peak_spectrum(peak, /*sharpness=*/2.0);
}

inline auto make_fleet() -> std::vector<LabeledRecord>
{
	std::vector<LabeledRecord> fleet;
	fleet.reserve(kFamilyCount * kSeverityLevels);
	for (std::size_t family = 0; family < kFamilyCount; ++family) {
		for (std::size_t severity = 0; severity < kSeverityLevels; ++severity) {
			MixedRecord record;
			record.code = family_code(family);
			record.spectrum = family_spectrum(severity);
			record.curve = family_curve(family, severity);
			record.vitals = family_vitals(family, severity);
			fleet.push_back({std::move(record), family_code(family), static_cast<double>(severity)});
		}
	}
	return fleet;
}

// A genuinely novel fault: an off-band twin spectrum, an out-of-phase curve, and
// vitals outside every family band. Used for the outlier / structure analysis.
inline auto make_anomaly() -> MixedRecord
{
	MixedRecord record;
	record.code = "xqzkflush";
	std::vector<double> bins(kSpectrumBins, 0.01);
	bins[1] += 1.0;
	bins[14] += 1.0; // bimodal, off every family band
	record.spectrum = unit_mass(std::move(bins));
	record.curve = {3.5, 0.0, 3.5, 0.0, 3.5, 0.0, 3.5, 0.0, 3.5, 0.0, 3.5, 0.0}; // oscillating
	record.vitals = {40.0, 50.0, 60.0, 70.0};
	return record;
}

inline auto fleet_records(const std::vector<LabeledRecord> &fleet) -> std::vector<MixedRecord>
{
	std::vector<MixedRecord> records;
	records.reserve(fleet.size());
	for (const auto &entry : fleet) {
		records.push_back(entry.record);
	}
	return records;
}

inline auto fleet_outcomes(const std::vector<LabeledRecord> &fleet) -> std::vector<std::vector<double>>
{
	std::vector<std::vector<double>> outcomes;
	outcomes.reserve(fleet.size());
	for (const auto &entry : fleet) {
		outcomes.push_back({entry.severity});
	}
	return outcomes;
}

// ---------------------------------------------------------------------------
// Per-channel "baseline loses" probes
// ---------------------------------------------------------------------------
//
// Each probe holds three of the four fields fixed (to a shared base) and varies
// exactly one field, so a single metric is on trial. The catalog contains the
// correct target, a decoy the flat baseline prefers, and unrelated distractors.

struct ChannelProbe {
	std::string channel;      // which field is on trial
	MixedRecord query;        // the probe
	std::string target_label; // catalog entry the composite metric should pick
	std::string decoy_label;  // catalog entry the flat baseline picks instead
	std::string takeaway;     // what structure the baseline loses
};

struct DemoCatalog {
	std::vector<MixedRecord> records;
	std::vector<std::string> labels;
	std::vector<ChannelProbe> probes;
};

// A neutral base used to hold "the other three fields" constant inside a probe.
inline auto demo_base() -> MixedRecord
{
	MixedRecord record;
	record.code = "steadybase";
	record.spectrum = peak_spectrum(8, 2.0);
	record.curve = family_curve(1, 2);
	record.vitals = {3.0, 800.0, 6.0, 3.0};
	return record;
}

inline auto make_demo_catalog() -> DemoCatalog
{
	DemoCatalog catalog;

	const auto base = demo_base();

	// --- Text channel (Edit vs character histogram) ---------------------------
	// target = one substitution away (true near-duplicate); decoy = an anagram
	// (identical character multiset, so the flat histogram distance is zero).
	MixedRecord text_target = base;
	text_target.code = "searing"; // Edit("bearing", "searing") = 1
	MixedRecord text_decoy = base;
	text_decoy.code = "gniraeb"; // reverse of "bearing": same letters, large edit
	MixedRecord text_query = base;
	text_query.code = "bearing";

	// --- Spectrum channel (Wasserstein vs bin-wise L2) ------------------------
	// target = the same mass shifted by one bin (tiny transport); decoy = half the
	// mass left in place (small L2) and half flung far away (large transport).
	MixedRecord spectrum_query = base;
	spectrum_query.spectrum = peak_spectrum(5, 2.5);
	MixedRecord spectrum_target = base;
	spectrum_target.spectrum = peak_spectrum(6, 2.5); // one-bin transport
	MixedRecord spectrum_decoy = base;
	{
		std::vector<double> bins(kSpectrumBins, 0.01);
		bins[5] += 0.5;  // overlaps the query peak -> shrinks L2
		bins[13] += 0.5; // far mass -> large transport cost
		spectrum_decoy.spectrum = unit_mass(std::move(bins));
	}

	// --- Curve channel (TWED vs zero-padded positional L2) --------------------
	// target = the same pulse with one extra peak sample (an elastic stretch /
	// different length) that TWED edits cheaply but the fixed-width zero padding
	// shoves out of alignment; decoy = the same length and positions as the query
	// but uniformly offset, so the positional L2 stays small while every matched
	// sample pays a value penalty under TWED.
	MixedRecord curve_query = base;
	curve_query.curve = {1, 2, 3, 2, 1};
	MixedRecord curve_target = base;
	curve_target.curve = {1, 2, 3, 3, 2, 1}; // elastic stretch (length 6)
	MixedRecord curve_decoy = base;
	curve_decoy.curve = {1.6, 2.6, 3.6, 2.6, 1.6}; // aligned, uniformly offset

	// Distractors so nearest-neighbour search is not a trivial 2-way choice.
	MixedRecord distractor_a = base;
	distractor_a.code = "pumphead";
	distractor_a.spectrum = peak_spectrum(11, 2.0);
	distractor_a.curve = family_curve(3, 4);
	distractor_a.vitals = {7.5, 1900.0, 7.0, 1.0};
	MixedRecord distractor_b = base;
	distractor_b.code = "filterbed";
	distractor_b.spectrum = peak_spectrum(3, 2.0);
	distractor_b.curve = family_curve(0, 0);
	distractor_b.vitals = {0.5, 450.0, 5.0, 2.5};

	const auto add = [&](const MixedRecord &record, const std::string &label) {
		catalog.records.push_back(record);
		catalog.labels.push_back(label);
	};

	add(text_target, "text_target");
	add(text_decoy, "text_decoy");
	add(spectrum_target, "spectrum_target");
	add(spectrum_decoy, "spectrum_decoy");
	add(curve_target, "curve_target");
	add(curve_decoy, "curve_decoy");
	add(distractor_a, "distractor_a");
	add(distractor_b, "distractor_b");

	catalog.probes.push_back({"code(text)", text_query, "text_target", "text_decoy",
							  "Edit distance keeps character order; the histogram collapses anagrams to zero."});
	catalog.probes.push_back({"spectrum(histogram)", spectrum_query, "spectrum_target", "spectrum_decoy",
							  "Wasserstein measures transport cost; bin-wise L2 rewards accidental bin overlap."});
	catalog.probes.push_back({"curve(time series)", curve_query, "curve_target", "curve_decoy",
							  "TWED edits the elastic length change cheaply; zero padding misaligns the tail, "
							  "while a uniform baseline drift looks small to positional L2."});

	return catalog;
}

inline auto label_index(const std::vector<std::string> &labels, const std::string &label) -> std::size_t
{
	for (std::size_t i = 0; i < labels.size(); ++i) {
		if (labels[i] == label) {
			return i;
		}
	}
	return labels.size();
}

} // namespace hero

#endif // METRIC_EXAMPLES_ENGINE_MIXED_FINITE_RECORDS_FIXTURE_HPP
