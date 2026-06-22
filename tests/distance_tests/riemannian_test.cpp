/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#include <catch2/catch.hpp>

#include "metric/metric/catalog/space/Riemannian.hpp"

#include <numeric>
#include <stdexcept>

using namespace Catch::literals;

TEMPLATE_TEST_CASE("riemannian_distance", "[distance]", double)
{

	mtrc::numeric::DynamicMatrix<TestType> A{{1, 2, -3}, {0, -2, 4}, {0, 0, 1}};
	mtrc::numeric::DynamicMatrix<TestType> B{{2, 0, -2}, {0, 1, 1}, {0, 0, 1}};

	auto rd = mtrc::RiemannianDistance<void, mtrc::Euclidean<TestType>>();
	REQUIRE(rd.matDistance(A, B) == 0.6931471805599455_a);

	std::vector<std::vector<TestType>> ds1{{0, 1}, {0, 0}, {1, 1}, {1, 0}};
	std::vector<std::vector<TestType>> ds2{{0, 0}, {1, 1}, {2, 2}, {2, 1}};
	std::vector<std::vector<TestType>> ds3{{0, 2}, {1, 0}, {2, 2}, {0, 1}};

	// The affine-invariant Riemannian distance (AIRM) between two equal-size finite
	// metric spaces. The value changed when the operand-encoding defect was fixed
	// (both operands are now built as the same regularized-Laplacian SPD matrix).
	REQUIRE(rd(ds1, ds2) == 0.2038157977919085_a);

	// Metric laws on a small finite domain (previously broken by the asymmetric
	// operand encoding): non-negativity, identity of indiscernibles, symmetry, and
	// the triangle inequality.
	REQUIRE(rd(ds1, ds2) >= 0.0);
	REQUIRE(rd(ds1, ds1) == Approx(0.0).margin(1e-9));
	REQUIRE(rd(ds2, ds2) == Approx(0.0).margin(1e-9));
	REQUIRE(rd(ds2, ds1) == Approx(rd(ds1, ds2)));					 // symmetry
	REQUIRE(rd(ds1, ds2) <= rd(ds1, ds3) + rd(ds3, ds2) + 1e-9);	 // triangle
	REQUIRE(rd(ds2, ds3) <= rd(ds2, ds1) + rd(ds1, ds3) + 1e-9);	 // triangle

	// Equal-size, non-empty finite spaces are required.
	REQUIRE_THROWS_AS(rd(ds1, std::vector<std::vector<TestType>>{{0, 0}}), std::invalid_argument);

	REQUIRE(rd.estimate(ds1, ds2, 2) == 0.0_a); // TODO run with larger dataset
}
