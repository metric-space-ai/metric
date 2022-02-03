#include <catch2/catch.hpp>

#include <limits>

#include <iostream>
#include "metric/utils/dnn.hpp"


using namespace metric::dnn;

using Matrix = blaze::DynamicMatrix<double>;
using Vector = blaze::DynamicVector<double>;


TEST_CASE("fullyconnected_json", "[dnn]")
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

	REQUIRE(fc.getInputSize() == 100);
	REQUIRE(fc.getOutputSize() == 10);

	REQUIRE(json == fc.toJson());
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

TEST_CASE("network_json", "[dnn]")
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
	REQUIRE(json == nt.toJson());

	nt.save("net.cereal");
	nt.load("net.cereal");

	REQUIRE(json == nt.toJson());

	std::stringstream ss;
	nt.save(ss);
	nt.load(ss);

	REQUIRE(json == nt.toJson());
}
