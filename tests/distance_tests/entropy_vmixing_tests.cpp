/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#include <catch2/catch.hpp>

#include "metric/correlation/entropy.hpp"
#include "metric/metric/catalog/vector/Standards.hpp"
#include "metric/metric/catalog/structured/Edit.hpp"
#include "metric/utils/type_traits.hpp"

#include <array>
#include <deque>
#include <stdexcept>
#include <vector>

using namespace Catch::literals;

TEMPLATE_TEST_CASE("entropy", "[distance]", float, double)
{
	std::vector<std::vector<TestType>> v1 = {{5, 5}, {2, 2}, {3, 3}, {5, 1}};
	std::vector<std::deque<TestType>> v2 = {{5, 5}, {2, 2}, {3, 3}, {5, 1}};
	std::vector<std::array<TestType, 2>> v3 = {{5, 5}, {2, 2}, {3, 3}, {5, 1}};
	std::deque<std::vector<TestType>> v4 = {{5, 5}, {2, 2}, {3, 3}, {5, 1}};
	std::deque<std::array<TestType, 2>> v7 = {{5, 5}, {2, 2}, {3, 3}, {5, 1}};

	using Metric = mtrc::Chebyshev<TestType>;

	REQUIRE(mtrc::Entropy<void, Metric>(Metric(), 3, 2)(v1) == -4.4489104772539489_a); //-4.4489104772539374));
	REQUIRE(mtrc::Entropy<void, Metric>(Metric(), 3, 2)(v2) == -4.4489104772539489_a);
	REQUIRE(mtrc::Entropy<void, Metric>(Metric(), 3, 2)(v3) == -4.4489104772539489_a);
	REQUIRE(mtrc::Entropy<void, Metric>(Metric(), 3, 2)(v4) == -4.4489104772539489_a);
	REQUIRE(mtrc::Entropy<void, Metric>(Metric(), 3, 2)(v7) == -4.4489104772539489_a);

	REQUIRE(mtrc::Entropy<void, Metric>()(v1) ==
			-5.3989104772539491_a); //-5.3989104772539376)); // -5.3989104772539225));
	REQUIRE(mtrc::Entropy<void, Metric>()(v2) == -5.3989104772539491_a);
	REQUIRE(mtrc::Entropy<void, Metric>()(v3) == -5.3989104772539491_a);
	REQUIRE(mtrc::Entropy<void, Metric>()(v4) == -5.3989104772539491_a);
	REQUIRE(mtrc::Entropy<void, Metric>()(v7) == -5.3989104772539491_a);

	std::vector<std::string> v8 = {
		"AAA", "HJGJHFG", "BBB", "AAAA", "long long long long long long string", "abcdefghjklmnopqrstuvwxyz"};

	REQUIRE(mtrc::Entropy<void, mtrc::Edit<int>>(mtrc::Edit<int>(), 3, 2.0)(v8) ==
			-9.3586210470159283_a); // 0.58333333333333337));
}

TEMPLATE_TEST_CASE("vmixing", "[distance]", float, double)
{
	std::vector<std::vector<TestType>> v11 = {{5, 5}, {2, 2}, {3, 3}, {5, 0}};
	std::vector<std::vector<TestType>> v12 = {{5, 5}, {2, 2}, {3, 3}, {1, 0}};

	auto vms = mtrc::VMixing_simple<void, mtrc::Euclidean<TestType>>();
	auto vm = mtrc::VMixing<void, mtrc::Euclidean<TestType>>();

	// REQUIRE(Approx(mtrc::VOI_simple(v11, v12), 11.813781191217037));
	REQUIRE(vms(v11, v12) == 0.0608977897890286_a); // 0.75859128220107586));
	// REQUIRE(Approx(mtrc::VOI(v11, v12), -34.913083805038639)); //-31.36271401282686));
	REQUIRE(vm(v11, v12) == 11.821247820859364_a); // 12.70867240400823)); //12.708672404008247));

	std::vector<std::deque<TestType>> v21 = {{5, 5}, {2, 2}, {3, 3}, {5, 0}};
	std::vector<std::deque<TestType>> v22 = {{5, 5}, {2, 2}, {3, 3}, {1, 0}};
	// REQUIRE(Approx(mtrc::VOI_simple(v21, v22), 11.813781191217037));
	REQUIRE(vms(v21, v22) == 0.0608977897890286_a);
	// REQUIRE(Approx(mtrc::VOI(v21, v22), -34.913083805038639));
	REQUIRE(vm(v21, v22) == 11.821247820859364_a);

	std::deque<std::vector<TestType>> v31 = {{5, 5}, {2, 2}, {3, 3}, {5, 0}};
	std::deque<std::vector<TestType>> v32 = {{5, 5}, {2, 2}, {3, 3}, {1, 0}};
	// REQUIRE(Approx(mtrc::VOI_simple(v31, v32), 11.813781191217037));
	REQUIRE(vms(v31, v32) == 0.0608977897890286_a);
	// REQUIRE(Approx(mtrc::VOI(v31, v32), -34.913083805038639));
	REQUIRE(vm(v31, v32) == 11.821247820859364_a);

	std::deque<std::deque<TestType>> v41 = {{5, 5}, {2, 2}, {3, 3}, {5, 0}};
	std::deque<std::deque<TestType>> v42 = {{5, 5}, {2, 2}, {3, 3}, {1, 0}};
	// REQUIRE(Approx(mtrc::VOI_simple(v41, v42), 11.813781191217037));
	REQUIRE(vms(v41, v42) == 0.0608977897890286_a);
	// REQUIRE(Approx(mtrc::VOI(v41, v42), -34.913083805038639));
	REQUIRE(vm(v41, v42) == 11.821247820859364_a);

	std::vector<std::vector<TestType>> ds1{{0, 1, 0}, {0, 0, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0},
										   {0, 1, 0}, {0, 0, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0},
										   {0, 1, 0}, {0, 0, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}};
	std::vector<std::vector<TestType>> ds2{{0, 0, 0}, {1, 1, 0}, {2, 2, 0}, {2, 1, 0}, {0, 0, 0},
										   {0, 0, 0}, {1, 1, 0}, {2, 2, 0}, {2, 1, 0}, {0, 0, 0},
										   {0, 0, 0}, {1, 1, 0}, {2, 2, 0}, {2, 1, 0}, {0, 0, 0}};

	REQUIRE(vms.estimate(ds1, ds2, 5) == 1.2722224834467826_a);
	REQUIRE(vm.estimate(ds1, ds2, 5) == 0.096469697241235622_a);
}
