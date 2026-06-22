// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Smoke coverage for the strict EMD/Wasserstein metric split.
//
// `mtrc::Wasserstein<V>` is the admitted strict 1-Wasserstein true metric
// (equal nonnegative mass over an admitted metric ground cost). `mtrc::EMD<V>`
// stays the permissive compatibility route (arbitrary cost, extra-mass penalty)
// and is declared `metric_law::distance`. This file checks the metric axioms
// and every guard for the strict route, and demonstrates with a concrete
// counterexample why the permissive route is not admitted.

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "metric/metric/admission.hpp"
#include "metric/metric/catalog.hpp"

namespace {

int g_failures = 0;

#define CHECK(cond)                                                                                                    \
	do {                                                                                                               \
		if (!(cond)) {                                                                                                 \
			std::fprintf(stderr, "CHECK failed: %s (%s:%d)\n", #cond, __FILE__, __LINE__);                             \
			++g_failures;                                                                                              \
		}                                                                                                              \
	} while (0)

template <typename T> bool approx(T a, T b, T tol)
{
	return std::abs(a - b) <= tol;
}

template <typename F> bool throws_invalid_argument(F &&fn, const std::string &needle)
{
	try {
		fn();
	} catch (const std::invalid_argument &error) {
		return needle.empty() || std::string(error.what()).find(needle) != std::string::npos;
	} catch (...) {
		return false;
	}
	return false;
}

using Hist = std::vector<double>;

// 1-Wasserstein on a line equals the L1 distance of the cumulative masses.
// Independent oracle for the line-ground-cost fixtures.
double line_emd_oracle(const Hist &p, const Hist &q)
{
	double cum = 0.0;
	double total = 0.0;
	for (std::size_t i = 0; i < p.size(); ++i) {
		cum += p[i] - q[i];
		total += std::abs(cum);
	}
	return total;
}

void check_known_values()
{
	const double tol = 1e-6;

	// Identical distributions -> 0.
	{
		auto w = mtrc::Wasserstein<double>::on_line(5);
		Hist p = {0.1, 0.2, 0.3, 0.0, 0.4};
		CHECK(approx(w(p, p), 0.0, tol));
	}

	// Shifted unit mass on a line: e0 -> e2 costs the ground distance 2.
	{
		auto w = mtrc::Wasserstein<double>::on_line(3);
		CHECK(approx(w(Hist{1, 0, 0}, Hist{0, 0, 1}), 2.0, tol));
		CHECK(approx(w(Hist{0, 0, 1}, Hist{1, 0, 0}), 2.0, tol)); // symmetry
	}

	// Partially overlapping mass on a line.
	{
		auto w = mtrc::Wasserstein<double>::on_line(3);
		Hist p = {0.5, 0.5, 0.0};
		Hist q = {0.0, 0.5, 0.5};
		CHECK(approx(w(p, q), 1.0, tol));
		CHECK(approx(w(p, q), line_emd_oracle(p, q), tol));
	}

	// Uniform vs Dirac on a line of 4 bins (oracle = 1.5).
	{
		auto w = mtrc::Wasserstein<double>::on_line(4);
		Hist p = {0.25, 0.25, 0.25, 0.25};
		Hist q = {1.0, 0.0, 0.0, 0.0};
		CHECK(approx(w(p, q), 1.5, tol));
		CHECK(approx(w(p, q), line_emd_oracle(p, q), tol));
	}

	// Shifted mass with multi-unit total mass (oracle handles it).
	{
		auto w = mtrc::Wasserstein<double>::on_line(5);
		Hist p = {0, 0, 3, 0, 0};
		Hist q = {0, 0, 0, 0, 3};
		CHECK(approx(w(p, q), 6.0, tol)); // 3 units of mass over distance 2
		CHECK(approx(w(p, q), line_emd_oracle(p, q), tol));
	}

	// Explicit admitted (non-line) metric ground cost: the 1-D |i-j| matrix.
	{
		std::vector<std::vector<double>> ground = {
			{0.0, 1.0, 2.0, 3.0},
			{1.0, 0.0, 1.0, 2.0},
			{2.0, 1.0, 0.0, 1.0},
			{3.0, 2.0, 1.0, 0.0},
		};
		mtrc::Wasserstein<double> w(ground);
		CHECK(w.size() == 4);
		CHECK(approx(w(Hist{1, 0, 0, 0}, Hist{0, 0, 0, 1}), 3.0, tol));
		CHECK(approx(w(Hist{1, 0, 0, 0}, Hist{1, 0, 0, 0}), 0.0, tol));
	}

	// Degenerate equal zero-mass measures -> 0 (and no division by zero).
	{
		auto w = mtrc::Wasserstein<double>::on_line(3);
		CHECK(approx(w(Hist{0, 0, 0}, Hist{0, 0, 0}), 0.0, tol));
	}

	// Single-bin space.
	{
		auto w = mtrc::Wasserstein<double>::on_line(1);
		CHECK(approx(w(Hist{2.5}, Hist{2.5}), 0.0, tol));
	}
}

// Non-negativity, identity, symmetry, and triangle inequality over a set of
// equal-mass distributions, on both a line cost and an explicit metric cost.
void check_metric_axioms()
{
	const double eps = 1e-5;

	const std::vector<Hist> records = {
		{1.0, 0.0, 0.0, 0.0}, {0.0, 1.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 1.0},
		{0.25, 0.25, 0.25, 0.25}, {0.5, 0.0, 0.5, 0.0}, {0.1, 0.2, 0.3, 0.4},
	};

	std::vector<std::vector<double>> ground = {
		{0.0, 1.0, 2.0, 3.0},
		{1.0, 0.0, 1.0, 2.0},
		{2.0, 1.0, 0.0, 1.0},
		{3.0, 2.0, 1.0, 0.0},
	};

	const std::vector<mtrc::Wasserstein<double>> metrics = {
		mtrc::Wasserstein<double>::on_line(4),
		mtrc::Wasserstein<double>(ground),
	};

	for (const auto &w : metrics) {
		for (const auto &a : records) {
			CHECK(approx(w(a, a), 0.0, eps)); // identity
			for (const auto &b : records) {
				const double ab = w(a, b);
				const double ba = w(b, a);
				CHECK(ab >= -eps);				 // non-negativity
				CHECK(std::isfinite(ab));		 // finite output
				CHECK(approx(ab, ba, eps));		 // symmetry
				if (ab <= eps) {
					CHECK(&a == &b); // identity of indiscernibles on this fixture set
				}
				for (const auto &c : records) {
					CHECK(w(a, c) <= ab + w(b, c) + eps); // triangle
				}
			}
		}
	}
}

void check_call_guards()
{
	auto w = mtrc::Wasserstein<double>::on_line(3);

	// Unequal total mass.
	CHECK(throws_invalid_argument([&] { (void)w(Hist{1, 0, 0}, Hist{0, 0, 2}); }, "equal total mass"));

	// Negative mass entry.
	CHECK(throws_invalid_argument([&] { (void)w(Hist{1, -0.5, 0.5}, Hist{0, 0.5, 0.5}); }, "nonnegative"));

	// Non-finite mass entries.
	const double inf = std::numeric_limits<double>::infinity();
	const double nan = std::numeric_limits<double>::quiet_NaN();
	CHECK(throws_invalid_argument([&] { (void)w(Hist{inf, 0, 0}, Hist{1, 0, 0}); }, "finite"));
	CHECK(throws_invalid_argument([&] { (void)w(Hist{nan, 0, 0}, Hist{1, 0, 0}); }, "finite"));

	// Dimension mismatch with the ground cost.
	CHECK(throws_invalid_argument([&] { (void)w(Hist{1, 0}, Hist{1, 0}); }, "aligned"));
	CHECK(throws_invalid_argument([&] { (void)w(Hist{1, 0, 0}, Hist{0, 1}); }, "aligned"));
}

void check_construction_guards()
{
	// Non-square ground cost.
	CHECK(throws_invalid_argument(
		[] { (void)mtrc::Wasserstein<double>(std::vector<std::vector<double>>{{0.0, 1.0}, {1.0}}); }, "square"));

	// Non-zero diagonal (identity broken).
	CHECK(throws_invalid_argument(
		[] { (void)mtrc::Wasserstein<double>(std::vector<std::vector<double>>{{1.0, 1.0}, {1.0, 0.0}}); },
		"zero diagonal"));

	// Negative entry.
	CHECK(throws_invalid_argument(
		[] { (void)mtrc::Wasserstein<double>(std::vector<std::vector<double>>{{0.0, -1.0}, {-1.0, 0.0}}); },
		"nonnegative"));

	// Asymmetric ground cost.
	CHECK(throws_invalid_argument(
		[] { (void)mtrc::Wasserstein<double>(std::vector<std::vector<double>>{{0.0, 1.0}, {2.0, 0.0}}); },
		"symmetric"));

	// Pseudometric: zero distance between distinct bins.
	CHECK(throws_invalid_argument(
		[] { (void)mtrc::Wasserstein<double>(std::vector<std::vector<double>>{{0.0, 0.0}, {0.0, 0.0}}); },
		"positive distance between distinct bins"));

	// Triangle-violating ground cost: C[0][2] = 10 > C[0][1] + C[1][2] = 2.
	CHECK(throws_invalid_argument(
		[] {
			(void)mtrc::Wasserstein<double>(std::vector<std::vector<double>>{
				{0.0, 1.0, 10.0},
				{1.0, 0.0, 1.0},
				{10.0, 1.0, 0.0},
			});
		},
		"triangle inequality"));

	// Empty ground cost.
	CHECK(throws_invalid_argument([] { (void)mtrc::Wasserstein<double>(std::vector<std::vector<double>>{}); },
								  "non-empty"));

	// Invalid mass tolerance.
	CHECK(throws_invalid_argument(
		[] { (void)mtrc::Wasserstein<double>::on_line(3, -1.0); }, "mass tolerance"));
}

// The heart of the split: a non-metric ground cost makes the permissive EMD
// route violate the triangle inequality, while the strict Wasserstein route
// refuses to be constructed with that same cost. Equal-mass Dirac histograms
// turn EMD(e_i, e_j) into exactly the ground cost C[i][j].
void check_permissive_route_is_not_a_metric()
{
	const std::vector<std::vector<double>> non_metric_ground = {
		{0.0, 1.0, 10.0},
		{1.0, 0.0, 1.0},
		{10.0, 1.0, 0.0},
	};

	mtrc::EMD<double> emd(non_metric_ground);
	const Hist e0 = {1, 0, 0};
	const Hist e1 = {0, 1, 0};
	const Hist e2 = {0, 0, 1};

	const double d02 = emd(e0, e2);
	const double d01 = emd(e0, e1);
	const double d12 = emd(e1, e2);

	CHECK(approx(d01, 1.0, 1e-4));
	CHECK(approx(d12, 1.0, 1e-4));
	CHECK(approx(d02, 10.0, 1e-3));
	// Triangle inequality is violated by the permissive route: 10 > 1 + 1.
	CHECK(d02 > d01 + d12 + 1.0);

	// The strict route rejects this ground cost outright.
	CHECK(throws_invalid_argument([&] { (void)mtrc::Wasserstein<double>(non_metric_ground); }, "triangle inequality"));

	// The permissive route also tolerates unbalanced mass (returning a finite
	// extra-mass-penalised number) where the strict route must refuse.
	auto strict = mtrc::Wasserstein<double>::on_line(3);
	const double unbalanced = emd(Hist{1, 0, 0}, Hist{0, 0, 2});
	CHECK(std::isfinite(unbalanced));
	CHECK(throws_invalid_argument([&] { (void)strict(Hist{1, 0, 0}, Hist{0, 0, 2}); }, "equal total mass"));
}

// The float specialisation must compile and stay numerically sane.
void check_float_specialisation()
{
	auto w = mtrc::Wasserstein<float>::on_line(4);
	std::vector<float> p = {1.0f, 0.0f, 0.0f, 0.0f};
	std::vector<float> q = {0.0f, 0.0f, 0.0f, 1.0f};
	CHECK(approx(w(p, q), 3.0f, 1e-2f));
	CHECK(approx(w(p, p), 0.0f, 1e-2f));
	CHECK(throws_invalid_argument([&] { (void)w(p, std::vector<float>{0.0f, 0.0f, 0.0f, 2.0f}); }, "equal total mass"));
}

void check_cache_key()
{
	auto a = mtrc::Wasserstein<double>::on_line(3);
	auto b = mtrc::Wasserstein<double>::on_line(3);
	auto c = mtrc::Wasserstein<double>::on_line(4);
	CHECK(mtrc::metric_cache_key(a) == mtrc::metric_cache_key(b));
	CHECK(mtrc::metric_cache_key(a) != mtrc::metric_cache_key(c));

	// Ground costs differing only in low-order digits must not alias to one key.
	mtrc::Wasserstein<double> g1(std::vector<std::vector<double>>{{0.0, 1.0}, {1.0, 0.0}});
	mtrc::Wasserstein<double> g2(std::vector<std::vector<double>>{{0.0, 1.000000001}, {1.000000001, 0.0}});
	CHECK(mtrc::metric_cache_key(g1) != mtrc::metric_cache_key(g2));
}

// Coverage added after adversarial review: non-finite ground-cost entries, a
// non-vector container, and the within-tolerance acceptance path.
void check_additional_coverage()
{
	const double inf = std::numeric_limits<double>::infinity();
	const double nan = std::numeric_limits<double>::quiet_NaN();

	// Non-finite ground-cost entries must be rejected by the finiteness guard
	// (not silently passed through to the triangle/symmetry arithmetic).
	CHECK(throws_invalid_argument(
		[&] { (void)mtrc::Wasserstein<double>(std::vector<std::vector<double>>{{0.0, inf}, {inf, 0.0}}); }, "finite"));
	CHECK(throws_invalid_argument(
		[&] { (void)mtrc::Wasserstein<double>(std::vector<std::vector<double>>{{0.0, nan}, {nan, 0.0}}); }, "finite"));

	// Generic (non-vector) container path: std::array of fixed size.
	{
		auto w = mtrc::Wasserstein<double>::on_line(3);
		std::array<double, 3> p = {1.0, 0.0, 0.0};
		std::array<double, 3> q = {0.0, 0.0, 1.0};
		CHECK(approx(w(p, q), 2.0, 1e-6));
		CHECK(approx(w(p, p), 0.0, 1e-6));
		CHECK(throws_invalid_argument([&] { (void)w(p, std::array<double, 3>{0.0, 0.0, 2.0}); }, "equal total mass"));
	}

	// Within-tolerance acceptance: an explicit ground tolerance admits a cost
	// that is asymmetric by less than that tolerance, and the tolerance is honored.
	{
		mtrc::Wasserstein<double> w(std::vector<std::vector<double>>{{0.0, 1.0}, {1.005, 0.0}}, 1e-6, 0.01);
		CHECK(approx(w.ground_tolerance(), 0.01, 1e-12));
		CHECK(std::isfinite(w(std::vector<double>{1.0, 0.0}, std::vector<double>{0.0, 1.0})));
	}

	// Single-bin nonzero equal mass returns exactly zero (n==1 short-circuit).
	{
		auto w = mtrc::Wasserstein<double>::on_line(1);
		CHECK(approx(w(std::vector<double>{7.0}, std::vector<double>{7.0}), 0.0, 1e-12));
		CHECK(throws_invalid_argument([&] { (void)w(std::vector<double>{7.0}, std::vector<double>{8.0}); },
									  "equal total mass"));
	}
}

} // namespace

// Trait split: Wasserstein is an admitted true metric; EMD is not.
static_assert(mtrc::metric_traits<mtrc::Wasserstein<double>>::law == mtrc::metric_law::metric);
static_assert(mtrc::metric_traits<mtrc::Wasserstein<double>>::records == mtrc::record_kind::structured);
static_assert(mtrc::metric_thread_safe_v<mtrc::Wasserstein<double>>);
static_assert(mtrc::metric_traits<mtrc::Wasserstein<float>>::law == mtrc::metric_law::metric);
static_assert(mtrc::metric_traits<mtrc::EMD<double>>::law == mtrc::metric_law::distance);
static_assert(!mtrc::metric_thread_safe_v<mtrc::EMD<double>>);
static_assert(mtrc::metric::is_admitted_metric_v<mtrc::Wasserstein<double>>);
static_assert(!mtrc::metric::is_admitted_metric_v<mtrc::EMD<double>>);
static_assert(mtrc::metric::is_quarantined_metric_v<mtrc::EMD<double>>);

int main()
{
	check_known_values();
	check_metric_axioms();
	check_call_guards();
	check_construction_guards();
	check_permissive_route_is_not_a_metric();
	check_float_specialisation();
	check_cache_key();
	check_additional_coverage();

	if (g_failures != 0) {
		std::fprintf(stderr, "metric_wasserstein_smoke: %d check(s) failed\n", g_failures);
		return 1;
	}
	return 0;
}
