#include <catch2/catch.hpp>

#include <limits>

#include <iostream>
#include "metric/utils/dnn.hpp"


using namespace metric::dnn;

using Matrix = blaze::DynamicMatrix<double>;
using Vector = blaze::DynamicVector<double>;
using RowVector = blaze::DynamicVector<double, blaze::rowVector>;


TEST_CASE("base", "[dnn]")
{
	Network<double> net;

	VerboseCallback<double> verboseCallback;

	RMSProp<double> opt;
	//RegressionMSE<double> output;
}

TEST_CASE("identity", "[dnn]")
{
	Matrix input{{-2, -1, 0, 1, 2, 3, 4, 5}};
	Matrix output{{-2, -1, 0, 1, 2, 3, 4, 5}};
	Matrix result;

	Identity<double>::activate(input, result);
	REQUIRE(result == output);
}

TEST_CASE("relu")
{
	Matrix input{ {-2, -1, 0, 1, 2, 3, 4, 5}};
	Matrix output{ {0, 0, 0, 1, 2, 3, 4, 5}};
	Matrix result(input.rows(), input.columns());

	ReLU<double>::activate(input, result);
	REQUIRE(result == output);
}

TEST_CASE("sigmoid")
{
	Matrix input{ {-10, -1, 0, 1, 10}};
	Matrix output{ {0.00004539786870,
					0.26894142137000,
					0.5,
					0.73105857863001,
					0.99995460213130}};
	Matrix result;

	Sigmoid<double>::activate(input, result);
	REQUIRE(result.rows() == output.rows());
	for (auto i = 0; i < output.columns(); ++i) {
		REQUIRE(result(0, i)  == Approx(output(0, i)));
	}
}

//BOOST_AUTO_TEST_CASE(maxpolling)
//{
//	MaxPooling<double, Identity<double>> maxPolling(4, 4, 1, 2, 2);
//
//	Matrix input{ {0}, {1}, {2}, {3}, {8}, {7}, {5}, {6}, {4}, {3}, {1}, {2}, {0}, {-1}, {-2}, {-3}};
//	Matrix output = { {8}, {6}, {4}, {2} };
//
//	maxPolling.forward(input);
//
//	BOOST_CHECK_EQUAL(maxPolling.output(), output);
//}

TEST_CASE("fullyconnected")
{
	FullyConnected<double, ReLU<double>> fc(4, 2);
	fc.initConstant(1, 0.5);

	Matrix input{{0, 1, 2, 3}};
	fc.forward(input);

	Matrix output{{6.5, 6.5}};

	REQUIRE(fc.output() == output);
}

TEST_CASE("convolutional")
{
	Conv2d<double, Identity<double>> convLayer(3, 3, 1, 2, 2, 2, 1);
	blaze::DynamicMatrix<double> X{{0, 1, 2, 3, 4, 5, 6, 7, 8}};

	std::vector<double> K = {0, 1, 2, 3, 4, 5, 6, 7};
	std::vector<double> bias = {1, 2};

	convLayer.setParameters({K, bias});

	convLayer.forward(X);

	blaze::DynamicMatrix<double> Y{{20, 26, 38, 44, 53, 75, 119, 141}};

	REQUIRE(convLayer.output() == Y);
}

/*blaze::DynamicMatrix<double> A = {{0, 1, 2, 3},
	                                  {4, 5, 6, 7}};
	blaze::DynamicMatrix<double> B = {{8, 9, 10, 1.2},
	                                  {1.2, 1.3, 1.4, 1.5}};

	blaze::DynamicMatrix<double> C(2, 2);

	//blaze::gemm(CblasRowMajor, CblasNoTrans, CblasTrans, 2, 2, 4,
	 //           1, A.data(), 4, B.data(), 2, 1, C.data(), 2);
	*/
/*double A0[] = {0, 1, 2, 3, 4, 5, 6, 7};
	double B0[] = {8, 9, 10, 1.2, 1.3, 1.4, 1.5};
	double C0[4];
	blaze::gemm(CblasRowMajor, CblasNoTrans, CblasTrans, 2, 2, 4,
	           1, A.data(), 4, B.data(), 4, 0, C.data(), 2);
	//blaze::gemm(CblasRowMajor, CblasNoTrans, CblasTrans, 2, 2, 4,
	//		   1, A0, 4, B0, 4, 0, C0, 2);
	std::cout << C << std::endl;
	std::cout << A * blaze::trans(B) << std::endl;*//*

}
*/
TEST_CASE("convolution_same")
{
	Conv2d<double, Identity<double>> convLayer(3, 3, 1, 1, 3, 3, 1, true);
	blaze::DynamicMatrix<double> X{{0, 1, 2, 3, 4, 5, 6, 7, 8}};

	std::vector<double> K = {0, 0, 0, 0, 1, 0, 0, 0, 0};
	std::vector<double> bias = {0};

	convLayer.setParameters({K, bias});

	convLayer.forward(X);

	blaze::DynamicMatrix<double> Y{{0, 1, 2, 3, 4, 5, 6, 7, 8}};

	REQUIRE(convLayer.output() == Y);
}

TEST_CASE("deconvolutional")
{
	Conv2dTranspose<double, Identity<double>> convTransposeLayer(2, 2, 1, 1, 2, 2);
	blaze::DynamicMatrix<double> X{{0, 1, 2, 3}};
	std::vector<double> K = {0, 1, 2, 3};
	std::vector<double> bias = {1};

	convTransposeLayer.setParameters({K, bias});

	convTransposeLayer.forward(X);

	blaze::DynamicMatrix<double> Y {{1, 1, 2, 1, 5, 7, 5, 13, 10}};

	REQUIRE(convTransposeLayer.output() == Y);
}


/*BOOST_AUTO_TEST_CASE(network_json)
{
	auto json = R"({
					"0":
						{
							"type": "FullyConnected",
							"inputSize": 3,
							"outputSize": 2,
							"activation": "ReLU"
						},
					"1":
						{
							"type": "FullyConnected",
							"inputSize": 2,
							"outputSize": 1,
							"activation": "Sigmoid"
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

	using Scalar = double;
	Network<Scalar> nt(json.dump());

	blaze::DynamicMatrix<Scalar> data = {{0.1, 0.2, 0.3}};
	blaze::DynamicMatrix<Scalar> labels = {{0.4}};

	nt.fit(data, labels, 1, 1);

	std::cout << nt.predict(data) << std::endl;
}*//*


BOOST_AUTO_TEST_CASE(rmsprop)
{
	RowVector weights   = {-0.021, 1.03, -0.05, -.749, 0.009};
	RowVector gradients = {1.000, -3.24, -0.60, 2.79, 1.820};

	// Defining the expected updates
	RowVector first_update  = {-0.0220, 1.0310, -0.0490, -0.7500, 0.0080};
	RowVector second_update = {-0.0227, 1.0317, -0.0482, -0.7507, 0.0072};

	// Testing
	auto optimizer = RMSProp<double>(0.0001, 1e-8, 0.99);
	optimizer.update(gradients, weights);

	for (size_t i = 0; i < weights.size(); i++) {
		BOOST_TEST(std::abs(first_update[i] - weights[i]) < 1e-3);
	}

	optimizer.update(gradients, weights);

	for (size_t i = 0; i < weights.size(); i++) {
		BOOST_TEST(std::abs(second_update[i] - weights[i]) < 1e-3);
	}

}*/
