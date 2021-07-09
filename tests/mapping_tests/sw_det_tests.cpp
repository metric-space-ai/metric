
/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

//#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

//#include "modules/mapping.hpp"
#include "modules/mapping/deterministic_switch_detector.hpp"

//#include <iostream>




TEMPLATE_TEST_CASE("switch_detector", "[mapping]", float, double) {

    blaze::DynamicMatrix<TestType> dataset = {
        {0, 10, 10, 10},
        {0, 10, 10, 10},
        {0, 10, 10, 10},
        {0, 10, 10, 10},
        {0, 10, 10, 10},
        {0, 10, 10, 10},
        {0, 10, 10, 10},
        {0, 10, 10, 10},
        {0, 10, 10, 10},
        {0, 10, 10, 10},
        {0, 10, 10, 10},
        {0, 10, 10, 15},
        {0, 13, 17, 20},
        {0, 25, 22, 25},
        {0, 10, 15, 10},
        {0, 10, 10, 10},
        {0,  5,  5, 10},
        {0,  5,  2,  5},
        {0, 10, 10, 10},
        {0, 10, 10, 10},
        {0, 20, 20, 20},
        {0, 30, 30, 30},
        {0, 100, 100, 100},
        {0, 100, 100, 100},
        {0, 100, 100, 100},
        {0, 100, 100, 100},
        {0, 100, 100, 100},
        {0, 100, 100, 100},
        {0, 100, 100, 100},
        {0, 100, 100, 100},
        {0, 100, 100, 100},
        {0, 100, 100, 100},
        {0, 10, 10, 10},
        {0, 10, 10, 10},
        {0, 10, 10, 10},
        {0, 10, 10, 10},
        {0, 10, 10, 10},
        {0, 10, 10, 10}
    };

    auto d = DetSwitchDetector<TestType>(8, 0.5);
    auto switches = d.encode(dataset);
    //std::cout << std::endl << "switches:" << std::endl << switches << std::endl;
    REQUIRE( ((blaze::sum(blaze::abs(switches)) == 2) && (switches(23, 0) == 1) && (switches(31, 0) == -1)) );

    d = DetSwitchDetector<TestType>(8);
    switches = d.encode(dataset);

    //std::cout << std::endl << "switches:" << std::endl << switches << std::endl;
    REQUIRE( ((blaze::sum(blaze::abs(switches)) == 1) && (switches(23, 0) == 1)) );

}
