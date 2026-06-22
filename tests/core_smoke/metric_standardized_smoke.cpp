// Standardized vector metric admission smoke test.
//
// Covers, for the aligned finite-real-vector domain:
//   * trait promotion to metric_law::metric / record_kind::aligned_vector for
//     mtrc::Euclidean_standardized<V> and mtrc::Manhattan_standardized<V>,
//   * hand-checkable distance fixtures (explicit scale and fitted scale),
//   * metric contracts (non-negativity, symmetry, identity of indiscernibles,
//     triangle inequality) over a small finite domain and a deterministic
//     randomized search,
//   * the positive-scale gate (every sigma finite and > 0) on the fitting
//     constructor, the explicit (mean, sigma) constructor, and every evaluation,
//   * invalid-input rejection (unaligned, dimension mismatch, non-finite),
//   * cache-key behavior (keyed on sigma, independent of the cancelling mean).
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "metric/metric/catalog.hpp"

using Vec = std::vector<double>;

namespace {

constexpr double kEps = 1e-9;

bool close(double a, double b) { return std::abs(a - b) <= kEps; }

// Metric contract check including identity of indiscernibles (needs sigma > 0).
template <typename Metric> void assert_metric_contracts(const std::vector<Vec> &records, const Metric &metric)
{
	for (const auto &a : records) {
		assert(std::abs(metric(a, a)) <= kEps);

		for (const auto &b : records) {
			const double dab = metric(a, b);
			const double dba = metric(b, a);

			assert(dab >= -kEps);				 // non-negativity
			assert(std::abs(dab - dba) <= kEps); // symmetry

			if (dab <= kEps) { // identity of indiscernibles
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
	double real(double lo, double hi)
	{
		const double unit = static_cast<double>(next() % 100000u) / 100000.0;
		return lo + unit * (hi - lo);
	}
};

Vec random_vector(Rng &rng, std::size_t dim, int alphabet)
{
	Vec v(dim);
	for (auto &x : v) {
		x = static_cast<double>(rng.below(alphabet));
	}
	return v;
}

Vec random_positive_sigma(Rng &rng, std::size_t dim)
{
	Vec s(dim);
	for (auto &x : s) {
		x = rng.real(0.1, 5.0);
	}
	return s;
}

template <typename Metric> void assert_random_metric_contracts(Rng &rng, const Metric &metric, std::size_t dim, int trials)
{
	for (int t = 0; t < trials; ++t) {
		const Vec a = random_vector(rng, dim, 6);
		const Vec b = random_vector(rng, dim, 6);
		const Vec c = random_vector(rng, dim, 6);

		const double dab = metric(a, b);
		const double dbc = metric(b, c);
		const double dac = metric(a, c);
		const double dba = metric(b, a);

		const double scale = std::max({1.0, std::abs(dab), std::abs(dbc), std::abs(dac)});
		assert(dab >= -kEps * scale);				   // non-negativity
		assert(std::abs(dab - dba) <= kEps * scale);   // symmetry
		assert(dac <= dab + dbc + kEps * scale);	   // triangle inequality
		if (dab <= kEps * scale) {					   // identity of indiscernibles
			assert(a == b);
		}
	}
}

template <typename Construct> void rejects_invalid(Construct &&construct, const char *needle)
{
	bool rejected = false;
	try {
		construct();
	} catch (const std::invalid_argument &error) {
		rejected = std::string(error.what()).find(needle) != std::string::npos;
	}
	assert(rejected);
}

const double kInf = std::numeric_limits<double>::infinity();
const double kNan = std::numeric_limits<double>::quiet_NaN();

} // namespace

int main()
{
	using mtrc::Euclidean_standardized;
	using mtrc::Manhattan_standardized;

	// 1. Trait promotion.
	static_assert(mtrc::metric_traits<Euclidean_standardized<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<Euclidean_standardized<double>>::records == mtrc::record_kind::aligned_vector);
	static_assert(mtrc::metric_thread_safe_v<Euclidean_standardized<double>>);
	static_assert(mtrc::metric_traits<Euclidean_standardized<float>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<Manhattan_standardized<double>>::law == mtrc::metric_law::metric);
	static_assert(mtrc::metric_traits<Manhattan_standardized<double>>::records == mtrc::record_kind::aligned_vector);
	static_assert(mtrc::metric_thread_safe_v<Manhattan_standardized<double>>);
	static_assert(mtrc::metric_traits<Manhattan_standardized<float>>::law == mtrc::metric_law::metric);

	// 2a. Hand-checkable fixtures with explicit (mean, sigma).
	{
		const Euclidean_standardized<double> unit(Vec{0.0, 0.0}, Vec{1.0, 1.0});
		assert(close(unit(Vec{0.0, 0.0}, Vec{3.0, 4.0}), 5.0)); // reduces to Euclidean
		assert(close(unit(Vec{1.0, 1.0}, Vec{1.0, 1.0}), 0.0));

		const Euclidean_standardized<double> scaled(Vec{0.0, 0.0}, Vec{2.0, 2.0});
		assert(close(scaled(Vec{0.0, 0.0}, Vec{4.0, 0.0}), 2.0)); // |4/2|

		// The centering mean cancels out of the distance entirely.
		const Euclidean_standardized<double> shifted(Vec{5.0, -7.0}, Vec{1.0, 1.0});
		assert(close(shifted(Vec{0.0, 0.0}, Vec{3.0, 4.0}), 5.0));

		const Manhattan_standardized<double> m_unit(Vec{0.0, 0.0}, Vec{1.0, 1.0});
		assert(close(m_unit(Vec{0.0, 0.0}, Vec{3.0, 4.0}), 7.0));
		const Manhattan_standardized<double> m_scaled(Vec{0.0, 0.0}, Vec{2.0, 1.0});
		assert(close(m_scaled(Vec{0.0, 0.0}, Vec{2.0, 3.0}), 4.0)); // |2/2| + |3/1|
	}

	// 2b. Hand-checkable fixtures from a fitted scale.
	{
		// A = {{0,0},{2,4}} -> mean = {1,2}, sigma = {1,2}.
		const std::vector<Vec> training{{0.0, 0.0}, {2.0, 4.0}};
		const Euclidean_standardized<double> fitted(training);
		assert(close(fitted.mean[0], 1.0) && close(fitted.mean[1], 2.0));
		assert(close(fitted.sigma[0], 1.0) && close(fitted.sigma[1], 2.0));
		assert(close(fitted(Vec{0.0, 0.0}, Vec{2.0, 4.0}), std::sqrt(8.0))); // sqrt(2^2 + 2^2)

		const Manhattan_standardized<double> m_fitted(training);
		assert(close(m_fitted(Vec{0.0, 0.0}, Vec{2.0, 4.0}), 4.0)); // |2/1| + |4/2|
	}

	// 3. Metric contracts over a small finite domain for several scale configs.
	const std::vector<Vec> domain = {
		{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 2.0}, {3.0, 1.0}, {2.0, 3.0}, {3.0, 3.0}, {1.0, 4.0},
	};
	assert_metric_contracts(domain, Euclidean_standardized<double>(Vec{0.0, 0.0}, Vec{1.0, 1.0}));
	assert_metric_contracts(domain, Euclidean_standardized<double>(Vec{0.0, 0.0}, Vec{2.5, 0.5}));
	assert_metric_contracts(domain, Euclidean_standardized<double>(Vec{-1.0, 9.0}, Vec{0.3, 4.0}));
	assert_metric_contracts(domain, Manhattan_standardized<double>(Vec{0.0, 0.0}, Vec{1.0, 1.0}));
	assert_metric_contracts(domain, Manhattan_standardized<double>(Vec{0.0, 0.0}, Vec{2.5, 0.5}));
	assert_metric_contracts(domain, Manhattan_standardized<double>(Vec{-1.0, 9.0}, Vec{0.3, 4.0}));

	// 4. Deterministic randomized property search across dimensions and scales.
	{
		Rng rng;
		for (std::size_t dim = 1; dim <= 4; ++dim) {
			const Vec mean = random_vector(rng, dim, 6);
			const Vec sigma = random_positive_sigma(rng, dim);
			assert_random_metric_contracts(rng, Euclidean_standardized<double>(mean, sigma), dim, 8000);
			assert_random_metric_contracts(rng, Manhattan_standardized<double>(mean, sigma), dim, 8000);
		}
	}

	// 5. Positive-scale gate on the explicit (mean, sigma) constructor.
	rejects_invalid([] { Euclidean_standardized<double>(Vec{0.0, 0.0}, Vec{1.0, 0.0}); }, "positive");
	rejects_invalid([] { Euclidean_standardized<double>(Vec{0.0, 0.0}, Vec{1.0, -2.0}); }, "positive");
	rejects_invalid([] { Euclidean_standardized<double>(Vec{0.0, 0.0}, Vec{1.0, kInf}); }, "positive");
	rejects_invalid([] { Euclidean_standardized<double>(Vec{0.0, 0.0}, Vec{1.0, kNan}); }, "positive");
	rejects_invalid([] { Euclidean_standardized<double>(Vec{0.0}, Vec{1.0, 2.0}); }, "equal dimension");
	rejects_invalid([] { Euclidean_standardized<double>(Vec{}, Vec{}); }, "fitted positive scale");
	rejects_invalid([] { Manhattan_standardized<double>(Vec{0.0, 0.0}, Vec{0.0, 1.0}); }, "positive");
	rejects_invalid([] { Manhattan_standardized<double>(Vec{kInf}, Vec{1.0}); }, "finite fitted mean");

	// 6. Positive-scale gate and aligned-data checks on the fitting constructor.
	rejects_invalid([] { Euclidean_standardized<double>(std::vector<Vec>{}); }, "non-empty training data");
	rejects_invalid([] { Euclidean_standardized<double>(std::vector<Vec>{{0.0, 1.0}, {1.0}}); }, "aligned");
	// A single record and a constant feature both produce a zero scale -> rejected.
	rejects_invalid([] { Euclidean_standardized<double>(std::vector<Vec>{{1.0, 2.0}}); }, "positive");
	rejects_invalid([] { Euclidean_standardized<double>(std::vector<Vec>{{1.0, 5.0}, {3.0, 5.0}}); }, "positive");
	rejects_invalid([] { Manhattan_standardized<double>(std::vector<Vec>{{1.0, 2.0}}); }, "positive");

	// 7. Evaluation-time gates: mutated scale, dimension mismatch, non-finite, unaligned.
	{
		Euclidean_standardized<double> mutated(Vec{0.0, 0.0}, Vec{1.0, 1.0});
		mutated.sigma[1] = 0.0;
		rejects_invalid([&] { (void)mutated(Vec{0.0, 0.0}, Vec{1.0, 1.0}); }, "positive");

		const Euclidean_standardized<double> probe(Vec{0.0, 0.0}, Vec{1.0, 1.0});
		rejects_invalid([&] { (void)probe(Vec{0.0}, Vec{0.0}); }, "does not match fitted dimension");
		rejects_invalid([&] { (void)probe(Vec{0.0, 1.0}, Vec{0.0}); }, "equal size");
		rejects_invalid([&] { (void)probe(Vec{kNan, 1.0}, Vec{0.0, 1.0}); }, "finite vector entries");
		rejects_invalid([&] { (void)probe(Vec{0.0, 1.0}, Vec{kInf, 1.0}); }, "finite vector entries");

		const Manhattan_standardized<double> m_probe(Vec{0.0, 0.0}, Vec{1.0, 1.0});
		rejects_invalid([&] { (void)m_probe(Vec{kNan, 1.0}, Vec{0.0, 1.0}); }, "finite vector entries");
	}

	// 8. Cache key: keyed on sigma, separates configs, independent of mean.
	{
		const auto key = mtrc::metric_cache_key(Euclidean_standardized<double>(Vec{0.0, 0.0}, Vec{2.0, 3.0}));
		// Keyed on dimension and the exact sigma serialization (sigma = {2, 3}).
		assert(key.find("Euclidean_standardized:n=2") != std::string::npos);
		assert(key.find("n=2:2:3") != std::string::npos);

		// Same sigma, different mean -> identical distances -> identical key.
		assert(mtrc::metric_cache_key(Euclidean_standardized<double>(Vec{0.0, 0.0}, Vec{2.0, 3.0})) ==
			   mtrc::metric_cache_key(Euclidean_standardized<double>(Vec{9.0, -4.0}, Vec{2.0, 3.0})));
		// Different sigma -> different key.
		assert(mtrc::metric_cache_key(Euclidean_standardized<double>(Vec{0.0, 0.0}, Vec{2.0, 3.0})) !=
			   mtrc::metric_cache_key(Euclidean_standardized<double>(Vec{0.0, 0.0}, Vec{2.0, 4.0})));
		// Euclidean and Manhattan standardized keys are distinct families.
		assert(mtrc::metric_cache_key(Euclidean_standardized<double>(Vec{0.0}, Vec{1.0})) !=
			   mtrc::metric_cache_key(Manhattan_standardized<double>(Vec{0.0}, Vec{1.0})));
	}

	// 9. A float instantiation computes and gates as expected.
	{
		const Euclidean_standardized<float> f(std::vector<float>{0.0f, 0.0f}, std::vector<float>{1.0f, 1.0f});
		assert(std::abs(f(std::vector<float>{0.0f, 0.0f}, std::vector<float>{3.0f, 4.0f}) - 5.0f) < 1e-5f);
		rejects_invalid([] { Euclidean_standardized<float>(std::vector<float>{0.0f}, std::vector<float>{0.0f}); },
						"positive");
	}

	return 0;
}
