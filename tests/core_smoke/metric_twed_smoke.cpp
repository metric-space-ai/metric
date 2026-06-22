// TWED admission hardening smoke test (Track C1).
//
// Covers, for the dense (positionally indexed) sequence domain:
//   * trait promotion to metric_law::metric / record_kind::sequence,
//   * hand-checkable distance fixtures,
//   * metric contracts (non-negativity, symmetry, identity of indiscernibles,
//     triangle inequality) over a small finite domain and a deterministic
//     randomized search,
//   * parameter gates (penalty >= 0, elastic > 0, finite) on construction and
//     on every evaluation,
//   * invalid-input rejection (empty and non-finite sequences),
//   * cache-key behavior.
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "metric/metric/catalog.hpp"

using Seq = std::vector<double>;

namespace {

constexpr double kEps = 1e-9;

bool close(double a, double b) { return std::abs(a - b) <= kEps; }

template <typename Metric> void assert_twed_metric_contracts(const std::vector<Seq> &records, const Metric &metric)
{
	for (const auto &a : records) {
		assert(std::abs(metric(a, a)) <= kEps);

		for (const auto &b : records) {
			const double dab = metric(a, b);
			const double dba = metric(b, a);

			assert(dab >= -kEps);				// non-negativity
			assert(std::abs(dab - dba) <= kEps); // symmetry

			if (dab <= kEps) {					// identity of indiscernibles (needs elastic > 0)
				assert(a == b);
			}

			for (const auto &c : records) {
				assert(metric(a, c) <= dab + metric(b, c) + kEps); // triangle inequality
			}
		}
	}
}

// Tiny deterministic xorshift so the randomized property search is reproducible.
struct Rng {
	uint64_t s = 0x9E3779B97F4A7C15ull;
	uint64_t next()
	{
		s ^= s << 13;
		s ^= s >> 7;
		s ^= s << 17;
		return s;
	}
	int below(int n) { return static_cast<int>(next() % static_cast<uint64_t>(n)); }
};

Seq random_sequence(Rng &rng, int max_len, int alphabet)
{
	const int len = 1 + rng.below(max_len);
	Seq v(static_cast<std::size_t>(len));
	for (auto &x : v) {
		x = static_cast<double>(rng.below(alphabet));
	}
	return v;
}

template <typename Metric> void assert_random_metric_contracts(const Metric &metric, int trials)
{
	Rng rng;
	for (int t = 0; t < trials; ++t) {
		const Seq a = random_sequence(rng, 5, 4);
		const Seq b = random_sequence(rng, 5, 4);
		const Seq c = random_sequence(rng, 5, 4);

		const double dab = metric(a, b);
		const double dbc = metric(b, c);
		const double dac = metric(a, c);
		const double dba = metric(b, a);

		const double scale = std::max({1.0, std::abs(dab), std::abs(dbc), std::abs(dac)});
		assert(std::abs(dab - dba) <= kEps * scale);  // symmetry
		assert(dac <= dab + dbc + kEps * scale);	  // triangle inequality
	}
}

} // namespace

int main()
{
	// 1. Trait promotion.
	static_assert(mtrc::metric_traits<mtrc::TWED<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<mtrc::TWED<double>>::records == mtrc::record_kind::sequence);
	static_assert(mtrc::metric_thread_safe_v<mtrc::TWED<double>>);
	static_assert(mtrc::metric_traits<mtrc::TWED<float>>::law == mtrc::metric_law::metric);

	// 2. Hand-checkable fixtures (dense path, timestamps = positions).
	const mtrc::TWED<double> d{0.0, 1.0}; // penalty = 0, elastic = 1
	assert(close(d(Seq{1, 2, 3}, Seq{1, 2, 3}), 0.0)); // identical sequences
	assert(close(d(Seq{2}, Seq{5}), 3.0));			   // single samples: |2 - 5|
	assert(close(d(Seq{0}, Seq{0, 1}), 2.0));		   // append one sample
	assert(close(d(Seq{0, 1}, Seq{0}), 2.0));		   // symmetric counterpart
	assert(close(d(Seq{1, 3}, Seq{1, 2, 3}), 3.0));	   // insert the middle sample
	assert(close(d(Seq{0}, Seq{0, 0}), 1.0));		   // elastic-only temporal cost

	const mtrc::TWED<double> d_pen{1.0, 1.0}; // penalty = 1
	assert(close(d_pen(Seq{0}, Seq{0, 0}), 2.0)); // deletion now also pays penalty

	const mtrc::TWED<double> d_stiff{1.0, 5.0};	  // parameters irrelevant for single samples
	assert(close(d_stiff(Seq{2}, Seq{5}), 3.0));

	// 3. Metric contracts over a small finite domain (varied lengths/values).
	const std::vector<Seq> domain = {
		{0}, {1}, {2}, {0, 0}, {0, 1}, {1, 0}, {1, 1}, {0, 2}, {1, 2, 3}, {3, 2, 1}, {0, 1, 2, 1},
	};
	assert_twed_metric_contracts(domain, mtrc::TWED<double>{0.0, 1.0});
	assert_twed_metric_contracts(domain, mtrc::TWED<double>{1.0, 1.0});
	assert_twed_metric_contracts(domain, mtrc::TWED<double>{0.5, 2.0});
	assert_twed_metric_contracts(domain, mtrc::TWED<double>{2.0, 0.5});
	assert_twed_metric_contracts(domain, mtrc::TWED<double>{0.0, 0.25});

	// 4. Deterministic randomized property search.
	assert_random_metric_contracts(mtrc::TWED<double>{0.0, 1.0}, 40000);
	assert_random_metric_contracts(mtrc::TWED<double>{1.0, 1.0}, 40000);
	assert_random_metric_contracts(mtrc::TWED<double>{0.5, 3.0}, 40000);

	// 5. Parameter gates on construction.
	auto rejects_construction = [](double penalty, double elastic, const char *needle) {
		bool rejected = false;
		try {
			(void)mtrc::TWED<double>{penalty, elastic};
		} catch (const std::invalid_argument &error) {
			rejected = std::string(error.what()).find(needle) != std::string::npos;
		}
		assert(rejected);
	};
	rejects_construction(-1.0, 1.0, "penalty");
	rejects_construction(-1e-12, 1.0, "penalty");
	rejects_construction(0.0, 0.0, "elastic");
	rejects_construction(0.0, -1.0, "elastic");
	rejects_construction(std::numeric_limits<double>::quiet_NaN(), 1.0, "finite");
	rejects_construction(0.0, std::numeric_limits<double>::infinity(), "finite");

	// Admitted defaults and boundaries construct successfully.
	(void)mtrc::TWED<double>{};			 // penalty = 0, elastic = 1
	(void)mtrc::TWED<double>{0.0, 1e-6}; // penalty = 0 and a small positive elastic are admitted

	// 6. Invalid-input rejection on evaluation.
	auto rejects_call = [](auto call, const char *needle) {
		bool rejected = false;
		try {
			call();
		} catch (const std::invalid_argument &error) {
			rejected = std::string(error.what()).find(needle) != std::string::npos;
		}
		assert(rejected);
	};
	const mtrc::TWED<double> probe{0.0, 1.0};
	rejects_call([&] { (void)probe(Seq{}, Seq{1, 2}); }, "non-empty");
	rejects_call([&] { (void)probe(Seq{1, 2}, Seq{}); }, "non-empty");
	rejects_call([&] { (void)probe(Seq{1, std::numeric_limits<double>::quiet_NaN()}, Seq{1, 2}); }, "finite");
	rejects_call([&] { (void)probe(Seq{1, 2}, Seq{std::numeric_limits<double>::infinity(), 2}); }, "finite");

	// A parameter mutated into a non-metric configuration is rejected on evaluation.
	mtrc::TWED<double> mutated{0.0, 1.0};
	mutated.elastic = 0.0;
	rejects_call([&] { (void)mutated(Seq{1, 2}, Seq{3, 4}); }, "elastic");

	// 7. Cache key reflects parameters and separates configurations.
	const std::string key = mtrc::metric_cache_key(mtrc::TWED<double>{0.5, 2.0});
	assert(key.find("penalty=0.500000") != std::string::npos);
	assert(key.find("elastic=2.000000") != std::string::npos);
	assert(mtrc::metric_cache_key(mtrc::TWED<double>{0.0, 1.0}) !=
		   mtrc::metric_cache_key(mtrc::TWED<double>{1.0, 1.0}));

	return 0;
}
