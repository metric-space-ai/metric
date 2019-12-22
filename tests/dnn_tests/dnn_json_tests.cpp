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
						"type": "FullyConnected",
						"inputSize": 100,
						"outputSize": 10,
						"activation": "Identity"
					}
				)"_json;

	FullyConnected<double, Identity<double>> fc(json);

	BOOST_CHECK_EQUAL(fc.getInputSize(), 100);
	BOOST_CHECK_EQUAL(fc.getOutputSize(), 10);

	BOOST_CHECK_EQUAL(json, fc.toJson());
}

//BOOST_AUTO_TEST_CASE(maxpolling_json)
//{
//	auto json = R"(
//					{
//						"inputSize": 100,
//						"outputSize": 10
//					}
//				)"_json;
//
//	MaxPooling<double, Identity<double>> mp(json);
//
//	BOOST_CHECK_EQUAL(mp.getInputSize(), 100);
//	BOOST_CHECK_EQUAL(mp.getOutputSize(), 10);
//
//	BOOST_CHECK_EQUAL(json, mp.toJson());
//}

BOOST_AUTO_TEST_CASE(network_json)
{
	auto json = R"({
					"0":
						{
							"type": "FullyConnected",
							"inputSize": 100,
							"outputSize": 10,
							"activation": "ReLU"
						},
					"train":
						{
							"loss": "RegressionMSE",
							"optimizer": {"type": "RMSProp",
											"learningRate": 0.01,
											"eps": 1e-6,
											"decay": 0.9}
						}
					}
				)"_json;

	Network<double> nt(json.dump());
	BOOST_CHECK_EQUAL(json, nt.toJson());

	nt.save("/tmp/net.cereal");
	nt.load("/tmp/net.cereal");

	BOOST_CHECK_EQUAL(json, nt.toJson());
}
