#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <limits>

#include <iostream>
#include "modules/utils/dnn.hpp"


using namespace metric::dnn;

using Matrix = blaze::DynamicMatrix<double>;
using Vector = blaze::DynamicVector<double>;


BOOST_AUTO_TEST_CASE(fullyconnected_json)
{
	auto json = R"(
					{
						"inputSize": 100,
						"outputSize": 10
					}
				)"_json;

	FullyConnected<double, Identity<double>> fc(json);

	BOOST_CHECK_EQUAL(fc.getInputSize(), 100);
	BOOST_CHECK_EQUAL(fc.getOutputSize(), 10);
}
