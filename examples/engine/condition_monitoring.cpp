// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// =====================================================================================
//  Finite Metric Condition Monitoring  --  METRIC engine hero application
// =====================================================================================
//
//  Industrial story
//  ----------------
//  A reciprocating machine emits one sensor curve per operating cycle (think a
//  per-stroke pressure or per-batch temperature trace). The plant historian keeps a
//  rolling window of recent cycle curves. We turn that rolling window into a FINITE
//  METRIC SPACE and read its condition straight off the geometry of the space.
//
//  Why a finite metric space (and not a feature vector)
//  ----------------------------------------------------
//  Healthy cycles differ from each other mostly by TIMING -- when the ramp starts, how
//  long the plateau holds, how many samples the cycle ran for. They are time-warped
//  versions of one shape. The Time Warp Edit Distance (TWED, Marteau 2009) is a true
//  metric on such sequences: warping in time is cheap, changing the shape is expensive.
//  A naive fixed-grid Euclidean "window vector" instead pays for every phase shift and
//  length change, so it confuses a time-shifted recurring signature with a clean plateau
//  and misses the recurrence (Scenario 4). This example contrasts the two head to head.
//
//  What the space tells us (every signal below stays explicit about its route:
//  distance-only diagnostics read metric values; coordinate diagnostics first map/embed):
//    * describe_structure(): diameter / mean distance / intrinsic dimension of the space
//    * entropy():            kpN differential entropy -- the information content / local
//                            freedom of an explicitly mapped coordinate view
//    * find_neighbors():     nearest-neighbour distance to the healthy reference set
//                            -> drift score and abrupt-change score
//    * find_outliers():      DBSCAN density-unassigned records -> abrupt regime change
//    * compare(..., mgc):    MGC DEPENDENCE between the process space and an independent
//                            condition-index space. MGC is used ONLY as a dependence test
//                            between two spaces; it is never used as a metric.
//
//  Four monitored conditions are exercised, each with assertions:
//    1. Normal state          2. Slow drift (early warning)
//    3. Abrupt regime change  4. Recurring pattern (recognised across time)
//
//  The program is fully deterministic (hashed coordinate jitter, no <random> locale or
//  implementation drift) and depends only on the header-only METRIC core. It doubles as
//  a regression test: every diagnosis is asserted, and it returns 0 only if all four
//  conditions are correctly identified and the TWED metric space out-diagnoses the
//  Euclidean window-vector baseline.
// =====================================================================================

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include <metric/metric/catalog.hpp>
#include <metric/engine.hpp>

#if defined(METRIC_HERO_WITH_ENTROPY)
#include <metric/stats/properties/entropy.hpp>
#endif

namespace {

// ------------------------------------------------------------------------------------
// Deterministic, platform-independent noise.
// splitmix64 finalizer -> a reproducible pseudo-random value in [-1, 1). We deliberately
// avoid <random> distributions, whose output is not portable across standard libraries.
// ------------------------------------------------------------------------------------
auto hash_unit(std::uint64_t seed) -> double
{
	std::uint64_t z = seed + 0x9E3779B97F4A7C15ULL;
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	z = z ^ (z >> 31);
	return static_cast<double>(z >> 11) / 9007199254740992.0 * 2.0 - 1.0;
}

auto jitter(std::uint64_t stream, int position, double scale) -> double
{
	return scale * hash_unit(stream * 1000003ULL + static_cast<std::uint64_t>(position + 1));
}

using Curve = std::vector<double>;

// The four operating regimes the monitor must tell apart.
enum class Regime { Normal, Drift, Fault, Signature };

auto regime_name(Regime regime) -> std::string
{
	switch (regime) {
	case Regime::Normal:
		return "normal";
	case Regime::Drift:
		return "drift";
	case Regime::Fault:
		return "fault";
	case Regime::Signature:
		return "signature";
	}
	return "unknown";
}

// ------------------------------------------------------------------------------------
// Cycle-curve generators. The healthy shape is a trapezoid (baseline -> ramp up ->
// plateau -> ramp down -> baseline). Healthy variation lives in the TIMING parameters
// (rise_start, ramp, plateau, tail) and in tiny amplitude jitter, i.e. healthy cycles
// are time-warped / length-varied versions of one another.
// ------------------------------------------------------------------------------------
auto trapezoid(int rise_start, int ramp, int plateau, int tail, double amplitude, double baseline,
			   std::uint64_t seed) -> Curve
{
	Curve curve;
	int pos = 0;
	for (int i = 0; i < rise_start; ++i, ++pos) {
		curve.push_back(baseline + jitter(seed, pos, 0.02));
	}
	for (int i = 1; i <= ramp; ++i, ++pos) {
		curve.push_back(baseline + amplitude * static_cast<double>(i) / ramp + jitter(seed, pos, 0.02));
	}
	for (int i = 0; i < plateau; ++i, ++pos) {
		curve.push_back(baseline + amplitude + jitter(seed, pos, 0.03));
	}
	for (int i = ramp - 1; i >= 1; --i, ++pos) {
		curve.push_back(baseline + amplitude * static_cast<double>(i) / ramp + jitter(seed, pos, 0.02));
	}
	for (int i = 0; i < tail; ++i, ++pos) {
		curve.push_back(baseline + jitter(seed, pos, 0.02));
	}
	return curve;
}

// Healthy cycle: nominal trapezoid with timing variation but no baseline shift.
auto healthy_cycle(int rise_start, int ramp, int plateau, int tail, std::uint64_t seed) -> Curve
{
	return trapezoid(rise_start, ramp, plateau, tail, 1.0, 0.0, seed);
}

// Drift cycle: the same healthy morphology but slowly fouling -- the baseline creeps up
// and the plateau grows with the drift step. step 0 is essentially healthy; later steps
// pull progressively away from the reference set.
auto drift_cycle(int step, std::uint64_t seed) -> Curve
{
	const double creep = 0.16 * static_cast<double>(step);
	const double amplitude = 1.0 + 0.14 * static_cast<double>(step);
	return trapezoid(3, 3, 3, 3, amplitude, creep, seed);
}

// Fault cycle: an abrupt, qualitatively different morphology -- a high double-peak with
// overshoot (e.g. valve knock). No healthy time-warp can reach this shape.
auto fault_cycle(std::uint64_t seed) -> Curve
{
	Curve curve;
	int pos = 0;
	for (int i = 0; i < 2; ++i, ++pos) {
		curve.push_back(jitter(seed, pos, 0.02));
	}
	const double peaks[] = {1.8, 3.6, 2.0, 3.2, 1.5}; // sharp double peak with overshoot
	for (double value : peaks) {
		curve.push_back(value + jitter(seed, pos++, 0.04));
	}
	for (int i = 0; i < 3; ++i, ++pos) {
		curve.push_back(jitter(seed, pos, 0.02));
	}
	return curve;
}

// Recurring signature cycle: a distinctive but recurring operating event. Its amplitude
// band is the SAME as a healthy plateau (~1.0), but the plateau is interrupted by a short
// notch -- a momentary dropout (e.g. a recurring valve closure). The notch TIMING wanders
// between occurrences. That is the crux: a rigid pointwise baseline confuses a
// notch-shifted recurrence with a clean normal plateau (they differ by only a couple of
// samples), whereas the elastic metric warps the two notches into alignment and links the
// recurrences. notch_offset selects where inside the plateau the dropout sits.
auto signature_cycle(int rise_start, int notch_offset, std::uint64_t seed) -> Curve
{
	const int ramp = 3;
	const int plateau = 6;
	Curve curve = trapezoid(rise_start, ramp, plateau, 3, 1.0, 0.0, seed);
	const int notch_start = rise_start + ramp + notch_offset;
	for (int i = 0; i < 3; ++i) {
		const int index = notch_start + i;
		if (index >= 0 && index < static_cast<int>(curve.size())) {
			curve[index] = 0.0 + jitter(seed, 500 + index, 0.03);
		}
	}
	return curve;
}

// ------------------------------------------------------------------------------------
// Baseline distance: the naive fixed-grid Euclidean "window vector". Curves are zero
// padded to a common length and compared pointwise. This is the standard strawman a
// finite metric space is meant to beat on warped / variable-length sequences.
// ------------------------------------------------------------------------------------
struct WindowVectorEuclidean {
	auto operator()(const Curve &lhs, const Curve &rhs) const -> double
	{
		const std::size_t n = std::max(lhs.size(), rhs.size());
		double sum = 0.0;
		for (std::size_t i = 0; i < n; ++i) {
			const double a = i < lhs.size() ? lhs[i] : 0.0;
			const double b = i < rhs.size() ? rhs[i] : 0.0;
			const double d = a - b;
			sum += d * d;
		}
		return std::sqrt(sum);
	}
};

// TWED parameters used throughout: a small temporal stiffness (nu) makes warping cheap,
// a moderate deletion penalty (lambda) keeps the metric discriminating on shape.
auto process_metric() -> mtrc::TWED<double> { return mtrc::TWED<double>(/*penalty=*/0.5, /*elastic=*/0.001); }

// ------------------------------------------------------------------------------------
// A labelled exemplar catalog spanning all four regimes, with timing variation inside
// each regime so the catalog is realistic (not one perfect template per class).
// ------------------------------------------------------------------------------------
struct Catalog {
	std::vector<Curve> curves;
	std::vector<Regime> families;
};

auto build_reference_catalog() -> Catalog
{
	Catalog catalog;
	auto add = [&](Curve curve, Regime regime) {
		catalog.curves.push_back(std::move(curve));
		catalog.families.push_back(regime);
	};

	// Healthy cycles -- varied phase (rise_start), ramp and internal plateau split, but a
	// COMMON sample count (length = rise_start + 2*ramp + plateau + tail - 1 = 13 for all
	// six). The healthy set can be mapped to a coordinate view for kpN differential entropy,
	// whose estimator treats data[0].size() as the space dimension and requires equal-length
	// records (see the assert before the entropy() call). The wider monitored streams stay
	// ragged on purpose, to exercise TWED's warp-/length-invariance against the baseline.
	add(healthy_cycle(2, 3, 3, 3, 11), Regime::Normal);
	add(healthy_cycle(3, 3, 2, 3, 12), Regime::Normal);
	add(healthy_cycle(1, 4, 3, 2, 13), Regime::Normal);
	add(healthy_cycle(4, 3, 2, 2, 14), Regime::Normal);
	add(healthy_cycle(2, 4, 2, 2, 15), Regime::Normal);
	add(healthy_cycle(3, 2, 4, 3, 16), Regime::Normal);

	// Drift exemplars at moderate-to-strong fouling.
	add(drift_cycle(4, 21), Regime::Drift);
	add(drift_cycle(5, 22), Regime::Drift);
	add(drift_cycle(6, 23), Regime::Drift);

	// Fault exemplars (the abrupt morphology).
	add(fault_cycle(31), Regime::Fault);
	add(fault_cycle(32), Regime::Fault);

	// Recurring-signature exemplars (the notched plateau), varied phase / notch position.
	add(signature_cycle(2, 0, 41), Regime::Signature);
	add(signature_cycle(2, 3, 42), Regime::Signature);
	add(signature_cycle(3, 1, 43), Regime::Signature);

	return catalog;
}

// ------------------------------------------------------------------------------------
// Small helpers over the engine API.
// ------------------------------------------------------------------------------------
// Largest "nearest healthy neighbour" distance inside the healthy reference set: the
// radius of the healthy cloud, used as a data-driven normal band (no magic constant).
template <typename Space>
auto healthy_radius(const Space &space, const std::vector<Regime> &families) -> double
{
	double radius = 0.0;
	for (std::size_t i = 0; i < space.size(); ++i) {
		if (families[i] != Regime::Normal) {
			continue;
		}
		double best = std::numeric_limits<double>::infinity();
		for (std::size_t j = 0; j < space.size(); ++j) {
			if (i == j || families[j] != Regime::Normal) {
				continue;
			}
			best = std::min(best, space.metric()(space[space.id(i)], space[space.id(j)]));
		}
		if (std::isfinite(best)) {
			radius = std::max(radius, best);
		}
	}
	return radius;
}

// Distance from an arbitrary query curve to the nearest HEALTHY reference cycle. This is
// the core monitoring score: small => normal, growing => drift, large => abrupt change.
template <typename Space>
auto distance_to_healthy(const Space &space, const std::vector<Regime> &families, const Curve &query) -> double
{
	double best = std::numeric_limits<double>::infinity();
	for (std::size_t i = 0; i < space.size(); ++i) {
		if (families[i] != Regime::Normal) {
			continue;
		}
		best = std::min(best, space.metric()(query, space[space.id(i)]));
	}
	return best;
}

// 1-NN family vote of a query against the labelled catalog.
template <typename Space>
auto classify(const Space &space, const std::vector<Regime> &families, const Curve &query)
	-> std::pair<Regime, double>
{
	const auto neighbors = mtrc::find_neighbors(space, query, 1);
	assert(!neighbors.empty());
	return {families[neighbors[0].id.index()], neighbors[0].distance};
}

auto rule(const std::string &title) -> void
{
	std::cout << "\n--- " << title << " ---\n";
}

} // namespace

int main()
{
	std::cout << std::fixed << std::setprecision(4);
	std::cout << "=====================================================================\n";
	std::cout << " Finite Metric Condition Monitoring  (TWED finite metric space)\n";
	std::cout << "=====================================================================\n";

	const Catalog catalog = build_reference_catalog();
	auto process_space = mtrc::make_space(catalog.curves, process_metric());
	auto baseline_space = mtrc::make_space(catalog.curves, WindowVectorEuclidean{});
	const auto &families = catalog.families;

	// ---- Space-level properties of the healthy reference -----------------------------
	std::vector<Curve> healthy_only;
	for (std::size_t i = 0; i < catalog.curves.size(); ++i) {
		if (families[i] == Regime::Normal) {
			healthy_only.push_back(catalog.curves[i]);
		}
	}
	auto healthy_space = mtrc::make_space(healthy_only, process_metric());

	const auto structure = mtrc::describe_structure(healthy_space);
	const double radius = healthy_radius(process_space, families);
	const double warn_threshold = 2.0 * radius;  // drift / early-warning band
	const double alarm_threshold = 4.0 * radius;  // abrupt-change band

	assert(structure.record_count == healthy_only.size());
	assert(structure.has_nonzero_distances);
	assert(radius > 0.0);

	std::cout << "[reference] healthy cycles            = " << healthy_only.size() << "\n";
	std::cout << "[reference] catalog cycles (4 regimes)= " << catalog.curves.size() << "\n";
	std::cout << "[reference] describe_structure: mean  = " << structure.average_distance
			  << "  max = " << structure.maximum_distance
			  << "  intrinsic_dim = " << structure.intrinsic_dimension << "\n";
	std::cout << "[reference] healthy radius (max NN)   = " << radius << "\n";
	std::cout << "[reference] warn / alarm thresholds   = " << warn_threshold << " / " << alarm_threshold << "\n";

#if defined(METRIC_HERO_WITH_ENTROPY)
	// Entropy is a coordinate-space operator (kpN differential entropy in nats).
	// The source diagnostic still uses TWED; this branch makes the coordinate view
	// explicit before entropy instead of treating TWED values as local coordinates.
	for (const auto &curve : healthy_only) {
		assert(curve.size() == healthy_only.front().size());
	}
	const auto healthy_coordinate_view =
		mtrc::map(healthy_space, [](const Curve &curve) { return curve; }, mtrc::Euclidean<double>{});
	const auto healthy_entropy = mtrc::stats::properties::entropy(healthy_coordinate_view, /*k=*/3, /*p=*/2);
	assert(healthy_entropy.algorithm == "entropy");
	assert(healthy_entropy.representation == healthy_coordinate_view.representation);
	assert(!std::isnan(healthy_entropy.value));
	std::cout << "[reference] entropy (kpN, nats)       = " << healthy_entropy.value
			  << "   (information content / local freedom of the mapped coordinate view)\n";
#else
	std::cout << "[reference] entropy property          = skipped (LAPACK-free build)\n";
#endif

	// The healthy reference must be internally coherent: with the warn-band radius, the
	// dense healthy cloud yields no DBSCAN-unassigned records.
	const auto healthy_outliers = mtrc::find_outliers(healthy_space, warn_threshold, 2);
	std::cout << "[reference] DBSCAN density-unassigned records   = " << healthy_outliers.unassigned_count
			  << "  -> reference cloud is coherent\n";
	assert(healthy_outliers.unassigned_count == 0);

	// =================================================================================
	//  Scenario 1: NORMAL STATE
	// =================================================================================
	rule("Scenario 1: NORMAL STATE");
	const Curve normal_query = healthy_cycle(2, 3, 3, 3, 1001);
	const double normal_score = distance_to_healthy(process_space, families, normal_query);
	const auto [normal_class, normal_nn] = classify(process_space, families, normal_query);
	const auto [normal_base_class, normal_base_nn] = classify(baseline_space, families, normal_query);

	std::cout << "query: fresh healthy cycle (unseen timing)\n";
	std::cout << "  TWED distance to healthy set  = " << normal_score << "   (band <= " << warn_threshold << ")\n";
	std::cout << "  TWED 1-NN class               = " << regime_name(normal_class) << " @ " << normal_nn << "\n";
	std::cout << "  baseline 1-NN class           = " << regime_name(normal_base_class) << " @ " << normal_base_nn
			  << "\n";
	std::cout << "  diagnosis                     = "
			  << (normal_score <= warn_threshold ? "NORMAL (within healthy band)" : "??") << "\n";
	assert(normal_score <= warn_threshold);
	assert(normal_class == Regime::Normal);

	// =================================================================================
	//  Scenario 2: SLOW DRIFT  (early warning before alarm)
	// =================================================================================
	rule("Scenario 2: SLOW DRIFT (early warning)");
	std::cout << "  step :  TWED-to-healthy   state\n";
	std::vector<double> drift_scores;
	for (int step = 0; step <= 6; ++step) {
		const Curve drift_query = drift_cycle(step, 5000 + step);
		const double score = distance_to_healthy(process_space, families, drift_query);
		drift_scores.push_back(score);
		const char *state = score <= warn_threshold ? "NORMAL"
						   : score <= alarm_threshold ? "DRIFT/WARN"
													  : "ALARM";
		std::cout << "  " << std::setw(4) << step << " :  " << std::setw(13) << score << "   " << state << "\n";
	}
	// The drift score must rise monotonically -- a usable early-warning trend -- and must
	// cross from the normal band, through the warning band, before hitting alarm.
	bool monotone = true;
	for (std::size_t i = 1; i < drift_scores.size(); ++i) {
		monotone = monotone && (drift_scores[i] > drift_scores[i - 1]);
	}
	std::cout << "  drift score strictly increasing = " << (monotone ? "yes" : "no")
			  << "  -> early warning is actionable before alarm\n";
	assert(monotone);
	assert(drift_scores.front() <= warn_threshold);          // earliest drift still looks normal
	assert(drift_scores.back() > warn_threshold);            // latest drift has clearly departed

	// =================================================================================
	//  Scenario 3: ABRUPT REGIME CHANGE  (structural outlier)
	// =================================================================================
	rule("Scenario 3: ABRUPT REGIME CHANGE");
	std::vector<Curve> stream;            // healthy run with one fault cycle spliced in
	std::vector<Regime> stream_truth;
	for (int i = 0; i < 5; ++i) {
		stream.push_back(healthy_cycle(2 + (i % 3), 3, 3, 3, 7000 + i));
		stream_truth.push_back(Regime::Normal);
	}
	const std::size_t fault_position = stream.size();
	stream.push_back(fault_cycle(7100));
	stream_truth.push_back(Regime::Fault);
	for (int i = 0; i < 4; ++i) {
		stream.push_back(healthy_cycle(1 + (i % 3), 3, 3, 3, 7200 + i));
		stream_truth.push_back(Regime::Normal);
	}
	auto stream_space = mtrc::make_space(stream, process_metric());

	const Curve fault_query = stream[fault_position];
	const double fault_score = distance_to_healthy(process_space, families, fault_query);
	const auto [fault_class, fault_nn] = classify(process_space, families, fault_query);

	// DBSCAN over the live stream: the fault cycle is density-isolated.
	const auto stream_outliers = mtrc::find_outliers(stream_space, warn_threshold, 2);
	assert(stream_outliers.strategy == "dbscan_density_outlier");
	bool fault_flagged = false;
	for (const auto &outlier : stream_outliers) {
		if (outlier.id.index() == fault_position) {
			fault_flagged = true;
		}
	}

	std::cout << "  stream cycles                 = " << stream.size() << " (fault spliced at index "
			  << fault_position << ")\n";
	std::cout << "  fault TWED distance to healthy= " << fault_score << "   (alarm > " << alarm_threshold << ")\n";
	std::cout << "  fault TWED 1-NN class         = " << regime_name(fault_class) << " @ " << fault_nn << "\n";
	std::cout << "  DBSCAN density-unassigned records       = " << stream_outliers.unassigned_count << "\n";
	std::cout << "  fault flagged as outlier      = " << (fault_flagged ? "yes" : "no") << "\n";
	std::cout << "  diagnosis                     = "
			  << (fault_score > alarm_threshold ? "ABRUPT REGIME CHANGE" : "??") << "\n";
	assert(fault_score > alarm_threshold);                   // a clear jump, not gradual drift
	assert(fault_score > drift_scores.back());               // and beyond the worst drift
	assert(fault_class == Regime::Fault);
	assert(fault_flagged);

	// =================================================================================
	//  Scenario 4: RECURRING PATTERN  (metric recognises it, baseline does not)
	// =================================================================================
	rule("Scenario 4: RECURRING PATTERN");
	std::vector<Curve> timeline;          // normal run with the signature event recurring
	std::vector<Regime> timeline_truth;
	auto push = [&](Curve curve, Regime regime) {
		timeline.push_back(std::move(curve));
		timeline_truth.push_back(regime);
	};
	push(healthy_cycle(2, 3, 3, 3, 8000), Regime::Normal);
	push(healthy_cycle(3, 3, 2, 3, 8001), Regime::Normal);
	const std::size_t sig_first = timeline.size();
	push(signature_cycle(2, 1, 8100), Regime::Signature);  // first occurrence
	push(healthy_cycle(1, 4, 3, 2, 8002), Regime::Normal);
	// A CLEAN long-plateau normal that overlaps the first signature's plateau pointwise: it
	// differs from the signature by only the notch samples, so the rigid baseline finds it
	// nearer than the time-shifted recurrence -- exactly the trap an elastic metric avoids.
	push(healthy_cycle(2, 3, 6, 3, 8003), Regime::Normal);
	push(healthy_cycle(2, 4, 2, 4, 8004), Regime::Normal);
	push(healthy_cycle(3, 2, 4, 3, 8005), Regime::Normal);
	const std::size_t sig_second = timeline.size();
	// Recurrence: the SAME signature translated later in the cycle (onset shifted by 4
	// samples). Time-warp absorbs the translation, so TWED links it to the first
	// occurrence; the fixed-grid baseline sees a large pointwise misalignment instead.
	push(signature_cycle(6, 1, 8101), Regime::Signature);
	push(healthy_cycle(2, 3, 3, 3, 8006), Regime::Normal);

	auto timeline_twed = mtrc::make_space(timeline, process_metric());
	auto timeline_base = mtrc::make_space(timeline, WindowVectorEuclidean{});

	const auto twed_nn = mtrc::find_neighbors(timeline_twed, timeline_twed.id(sig_first), 1);
	const auto base_nn = mtrc::find_neighbors(timeline_base, timeline_base.id(sig_first), 1);
	const std::size_t twed_match = twed_nn[0].id.index();
	const std::size_t base_match = base_nn[0].id.index();

#if defined(METRIC_HERO_DEBUG)
	std::cout << "  [debug] distances from signature#1 (index " << sig_first << ")\n";
	for (std::size_t i = 0; i < timeline.size(); ++i) {
		std::cout << "    idx " << std::setw(2) << i << " (" << regime_name(timeline_truth[i]) << ")"
				  << "  TWED=" << std::setw(9) << timeline_twed.metric()(timeline[sig_first], timeline[i])
				  << "  EUC=" << std::setw(9) << timeline_base.metric()(timeline[sig_first], timeline[i]) << "\n";
	}
#endif

	// Metric margin: how much closer the recurring twin is under TWED than the baseline's
	// (phase-fooled) pick would be, measured on the shared TWED ruler.
	const double base_pick_under_twed = timeline_twed.metric()(timeline[sig_first], timeline[base_match]);
	const double metric_margin = base_pick_under_twed - twed_nn[0].distance;

	std::cout << "  timeline cycles               = " << timeline.size() << "\n";
	std::cout << "  signature recurs at indices   = " << sig_first << " and " << sig_second << "  (dt = "
			  << (sig_second - sig_first) << " cycles)\n";
	std::cout << "  TWED  NN of signature#1       = index " << twed_match << " ("
			  << regime_name(timeline_truth[twed_match]) << ")\n";
	std::cout << "  baseline NN of signature#1    = index " << base_match << " ("
			  << regime_name(timeline_truth[base_match]) << ")\n";
	std::cout << "  metric margin (TWED ruler)    = " << metric_margin << "\n";
	std::cout << "  diagnosis                     = "
			  << (timeline_truth[twed_match] == Regime::Signature ? "RECURRENCE RECOGNISED by metric" : "??")
			  << ((timeline_truth[base_match] != Regime::Signature) ? " | baseline MISSED it" : "") << "\n";
	assert(twed_match == sig_second);                         // metric links the two signatures
	assert(timeline_truth[twed_match] == Regime::Signature);
	assert(timeline_truth[base_match] != Regime::Signature);  // baseline is fooled by notch timing
	assert(metric_margin > 0.0);

	// =================================================================================
	//  Metric vs. baseline: 1-NN regime classification over a held-out query set
	// =================================================================================
	rule("Benchmark: TWED metric space vs. Euclidean window-vector baseline");
	struct Query {
		Curve curve;
		Regime truth;
	};
	std::vector<Query> queries;
	queries.push_back({healthy_cycle(2, 3, 3, 3, 9001), Regime::Normal});
	queries.push_back({healthy_cycle(4, 3, 4, 2, 9002), Regime::Normal});
	queries.push_back({healthy_cycle(1, 4, 2, 4, 9003), Regime::Normal});
	queries.push_back({drift_cycle(5, 9101), Regime::Drift});
	queries.push_back({drift_cycle(6, 9102), Regime::Drift});
	queries.push_back({fault_cycle(9201), Regime::Fault});
	queries.push_back({fault_cycle(9202), Regime::Fault});
	queries.push_back({signature_cycle(2, 1, 9301), Regime::Signature});
	queries.push_back({signature_cycle(3, 4, 9302), Regime::Signature});

	std::size_t twed_correct = 0;
	std::size_t base_correct = 0;
	double margin_sum = 0.0;
	for (const auto &query : queries) {
		const auto [twed_class, twed_dist] = classify(process_space, families, query.curve);
		const auto [base_class, base_dist] = classify(baseline_space, families, query.curve);
		if (twed_class == query.truth) {
			++twed_correct;
		}
		if (base_class == query.truth) {
			++base_correct;
		}
		const auto base_nn_neighbors = mtrc::find_neighbors(baseline_space, query.curve, 1);
		const double base_pick_twed_dist =
			process_space.metric()(query.curve, catalog.curves[base_nn_neighbors[0].id.index()]);
		margin_sum += base_pick_twed_dist - twed_dist;
	}

	std::cout << "  queries                       = " << queries.size() << "\n";
	std::cout << "  TWED metric-space accuracy    = " << twed_correct << "/" << queries.size() << "\n";
	std::cout << "  Euclidean baseline accuracy   = " << base_correct << "/" << queries.size() << "\n";
	std::cout << "  mean metric margin (TWED)     = " << margin_sum / static_cast<double>(queries.size()) << "\n";
	assert(twed_correct == queries.size());                  // metric space is perfect here
	assert(twed_correct > base_correct);                     // and strictly beats the baseline

	// =================================================================================
	//  Cross-space dependence with MGC  (a DEPENDENCE test, never a metric)
	// =================================================================================
	rule("Cross-space dependence: MGC(process space, condition index)");
	// An independent maintenance system records one scalar condition/severity index per
	// monitored cycle. We test whether the geometry of the PROCESS space is statistically
	// dependent on that independent signal. MGC is the dependence statistic between two
	// finite metric spaces -- it is never used to build a space.
	std::vector<Curve> monitored;
	std::vector<std::array<double, 1>> condition; // severity index, one per cycle
	auto observe = [&](Curve curve, double severity) {
		monitored.push_back(std::move(curve));
		condition.push_back({severity});
	};
	for (int i = 0; i < 6; ++i) {
		observe(healthy_cycle(2 + (i % 3), 3, 3, 3, 12000 + i), 0.05 + jitter(13000 + i, 0, 0.01));
	}
	for (int step = 1; step <= 5; ++step) {
		observe(drift_cycle(step, 12100 + step), 0.12 + 0.14 * static_cast<double>(step));
	}
	observe(fault_cycle(12200), 0.95);
	observe(fault_cycle(12201), 0.93);
	observe(signature_cycle(2, 0, 12300), 0.50);
	observe(signature_cycle(2, 3, 12301), 0.52);

	auto monitored_twed = mtrc::make_space(monitored, process_metric());
	auto monitored_base = mtrc::make_space(monitored, WindowVectorEuclidean{});
	auto condition_space = mtrc::make_space(condition, mtrc::Euclidean<double>());

	const auto mgc_twed = mtrc::compare(monitored_twed, condition_space, mtrc::stats::correlate::mgc_options{});
	const auto mgc_base = mtrc::compare(monitored_base, condition_space, mtrc::stats::correlate::mgc_options{});

	assert(mgc_twed.algorithm == "mgc");
	assert(mgc_twed.left_representation == "metric_space");
	assert(mgc_twed.right_representation == "metric_space");
	assert(mgc_twed.left_record_count == monitored.size());
	assert(mgc_twed.right_record_count == condition.size());
	assert(mgc_twed.exact);
	assert(std::isfinite(mgc_twed.value));
	assert(std::isfinite(mgc_base.value));

	std::cout << "  monitored cycles              = " << monitored.size() << "\n";
	std::cout << "  MGC(TWED space,     condition)= " << mgc_twed.value << "\n";
	std::cout << "  MGC(Euclidean win., condition)= " << mgc_base.value << "\n";
	std::cout << "  interpretation                = TWED geometry tracks the independent\n";
	std::cout << "                                  condition index more strongly than the\n";
	std::cout << "                                  phase-sensitive window-vector space\n";
	assert(mgc_twed.value > 0.0);
	assert(mgc_twed.value > mgc_base.value);

	// =================================================================================
	//  Summary
	// =================================================================================
	std::cout << "\n=====================================================================\n";
	std::cout << " SUMMARY: all four conditions identified from finite-metric geometry\n";
	std::cout << "   1. normal   -> within healthy band (score " << normal_score << ")\n";
	std::cout << "   2. drift    -> monotone early-warning trend, crosses warn band\n";
	std::cout << "   3. abrupt   -> DBSCAN outlier + score past alarm (" << fault_score << ")\n";
	std::cout << "   4. recurring-> metric links recurrence, baseline misses it\n";
	std::cout << " metric vs baseline 1-NN accuracy = " << twed_correct << "/" << queries.size() << "  vs  "
			  << base_correct << "/" << queries.size() << "\n";
	std::cout << " MGC dependence TWED vs baseline  = " << mgc_twed.value << "  vs  " << mgc_base.value << "\n";
	std::cout << "=====================================================================\n";

	return 0;
}
