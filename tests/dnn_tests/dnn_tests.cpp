#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <limits>

#include <iostream>
#include "modules/utils/dnn.hpp"


using namespace MiniDNN;

using Matrix = blaze::DynamicMatrix<double, blaze::columnMajor>;
using Vector = blaze::DynamicVector<double>;


BOOST_AUTO_TEST_CASE(base)
{
	Network<double> net;

	std::mt19937 rng;
	Network<double> netRng(rng);

	VerboseCallback<double> verboseCallback;

	RMSProp<double> opt;
	//RegressionMSE<double> output;
}

BOOST_AUTO_TEST_CASE(identity)
{
	Identity<double> identity;
	Matrix input{ {-2}, {-1}, {0}, {1}, {2}, {3}, {4}, {5}};
	Matrix output{ {-2}, {-1}, {0}, {1}, {2}, {3}, {4}, {5}};
	Matrix result;

	identity.activate(input, result);
	BOOST_CHECK_EQUAL(result, output);
}

BOOST_AUTO_TEST_CASE(relu)
{
	ReLU<double> relu;
	Matrix input{ {-2}, {-1}, {0}, {1}, {2}, {3}, {4}, {5}};
	Matrix output{ {0}, {0}, {0}, {1}, {2}, {3}, {4}, {5}};
	Matrix result(input.rows(), input.columns());

	relu.activate(input, result);
	BOOST_CHECK_EQUAL(result, output);
}

BOOST_AUTO_TEST_CASE(sigmoid)
{
	Sigmoid<double> sigmoid;
	Matrix input{ {-10}, {-1}, {0}, {1}, {10}};
	Matrix output{ {0.00004539786870},
					{0.26894142137000},
					{0.5},
					{0.73105857863001},
					{0.99995460213130}};
	Matrix result;

	sigmoid.activate(input, result);
	BOOST_CHECK_EQUAL(result.rows(), output.rows());
	for (auto i = 0; i < output.rows(); ++i) {
		BOOST_CHECK_SMALL(result(i, 0) - output(i, 0), 1e-13);
	}
}

BOOST_AUTO_TEST_CASE(maxpolling)
{
	MaxPooling<double, Identity<double>> maxPolling(4, 4, 1, 2, 2);

	Matrix input{ {0}, {1}, {2}, {3}, {8}, {7}, {5}, {6}, {4}, {3}, {1}, {2}, {0}, {-1}, {-2}, {-3}};
	Matrix output = { {8}, {6}, {4}, {2} };

	maxPolling.forward(input);

	BOOST_CHECK_EQUAL(maxPolling.output(), output);
}

BOOST_AUTO_TEST_CASE(fullyconnected)
{
	using FC = FullyConnected<double, Identity<double>>;
	FullyConnected<double, Identity<double>> fc(4, 2);
	fc.initConstant(1, 0.5);

	Matrix input{ {0}, {1}, {2}, {3}};
	fc.forward(input);

	Matrix output{{6.5}, {6.5}};

	BOOST_CHECK_EQUAL(fc.output(), output);
}

BOOST_AUTO_TEST_CASE(convolutional)
{
	Conv2d<double, Identity<double>> convLayer(3, 3, 1, 1, 2, 2);
	blaze::DynamicMatrix<double, blaze::columnMajor> X{{0, 1, 2, 3, 4, 5, 6, 7, 8}};
	blaze::transpose(X);

	Vector K = {0, 1, 2, 3};
	Vector bias = {0};

	convLayer.setParameters(K, bias);

	convLayer.forward(X);

	blaze::DynamicMatrix<double, blaze::columnMajor> Y {{19, 25, 37, 43}};
	blaze::transpose(Y);

	BOOST_CHECK_EQUAL(convLayer.output(), Y);
}

BOOST_AUTO_TEST_CASE(deconvolutional)
{
	Conv2dTranspose<double, Identity<double>> convTransposeLayer(2, 2, 1, 1, 2, 2);
	blaze::DynamicMatrix<double, blaze::columnMajor> X{{1, 2, 3, 4}};
	blaze::transpose(X);
	Vector K = {1, 2, 3, 4};
	Vector bias = {0};

	convTransposeLayer.setParameters(K, bias);

	convTransposeLayer.forward(X);

	blaze::DynamicMatrix<double, blaze::columnMajor> Y {{1, 4, 4, 6, 20, 16, 9, 24, 16}};
	blaze::transpose(Y);

	BOOST_CHECK_EQUAL(convTransposeLayer.output(), Y);
}

