/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#include <vector>
#include "modules/distance.hpp"

#define BOOST_TEST_MODULE Main

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(basic_use) 
{
	/*** here are some data records ***/
	std::vector<double> samples_1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	std::vector<double> samples_2 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
	   
    metric::CramervonNises<std::vector<double>, double> distance;

    auto result = distance(samples_1, samples_2);
	
    std::cout << "basic use result: " << result << std::endl;

    double t = 5.0; // 5.0%
    BOOST_CHECK_CLOSE(result, 0.311, t);
}

BOOST_AUTO_TEST_CASE(different_dimensions) 
{
	/*** here are some data records ***/
	std::vector<double> samples_1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	std::vector<double> samples_2 = { 1, 2, 3, 4, 5 };
	   
    metric::CramervonNises<std::vector<double>, double> distance;

    auto result = distance(samples_1, samples_2);
	
    std::cout << "different dimensions result: " << result << std::endl;
	
    double t = 5.0; // 5.0%
    BOOST_CHECK_CLOSE(result, 0.81, t);
}

BOOST_AUTO_TEST_CASE(equal_samples) 
{
	/*** here are some data records ***/
	std::vector<double> samples_1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	std::vector<double> samples_2 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	   
    metric::CramervonNises<std::vector<double>, double> distance;

    auto result = distance(samples_1, samples_2);
	
    std::cout << "equal samples result: " << result << std::endl;
	
    double t = 0.01; // 0.01%
    BOOST_CHECK_CLOSE(result, 0.0, t);
}

BOOST_AUTO_TEST_CASE(single_value_distribution) 
{
	/*** here are some data records ***/
	std::vector<double> samples_1 = { 0, 0, 0, 0, 0, 0, 0 };
	std::vector<double> samples_2 = { 1, 1, 1, 1, 1, 1, 1 };
	   
    metric::CramervonNises<std::vector<double>, double> distance;

    auto result = distance(samples_1, samples_2);
	
    std::cout << "single value distribution result: " << result << std::endl;

    BOOST_CHECK( std::isnan( result ) );
}

BOOST_AUTO_TEST_CASE(non_intersect_distribution) 
{
	/*** here are some data records ***/
	std::vector<double> samples_1 = { 0, 1, 1, 2, 2, 2, 2, 1, 1, 0 };
	std::vector<double> samples_2 = { 10, 11, 11, 12, 12, 12, 12, 11, 11, 10 };
	   
    metric::CramervonNises<std::vector<double>, double> distance;

    auto result = distance(samples_1, samples_2);
	
    std::cout << "non intersect distributionn result: " << result << std::endl;

    BOOST_CHECK( std::isnan( result ) );
}
