#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <limits>

#include <iostream>
#include "modules/utils/dnn.hpp"
#include "3rdparty/blaze/math/blas/gemm.h"


using namespace metric::dnn;

using Matrix = blaze::DynamicMatrix<double>;
using Vector = blaze::DynamicVector<double>;


BOOST_AUTO_TEST_CASE(base)
{
	Network<double> net;

	VerboseCallback<double> verboseCallback;

	RMSProp<double> opt;
	//RegressionMSE<double> output;
}

BOOST_AUTO_TEST_CASE(identity)
{
	Matrix input{ {-2, -1, 0, 1, 2, 3, 4, 5}};
	Matrix output{ {-2, -1, 0, 1, 2, 3, 4, 5}};
	Matrix result;

	Identity<double>::activate(input, result);
	BOOST_CHECK_EQUAL(result, output);
}

BOOST_AUTO_TEST_CASE(relu)
{
	Matrix input{ {-2, -1, 0, 1, 2, 3, 4, 5}};
	Matrix output{ {0, 0, 0, 1, 2, 3, 4, 5}};
	Matrix result(input.rows(), input.columns());

	ReLU<double>::activate(input, result);
	BOOST_CHECK_EQUAL(result, output);
}

BOOST_AUTO_TEST_CASE(sigmoid)
{
	Matrix input{ {-10, -1, 0, 1, 10}};
	Matrix output{ {0.00004539786870,
					0.26894142137000,
					0.5,
					0.73105857863001,
					0.99995460213130}};
	Matrix result;

	Sigmoid<double>::activate(input, result);
	BOOST_CHECK_EQUAL(result.rows(), output.rows());
	for (auto i = 0; i < output.columns(); ++i) {
		BOOST_CHECK_SMALL(result(0, i) - output(0, i), 1e-13);
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

BOOST_AUTO_TEST_CASE(fullyconnected)
{
	FullyConnected<double, ReLU<double>> fc(4, 2);
	fc.initConstant(1, 0.5);

	Matrix input{{0, 1, 2, 3}};
	fc.forward(input);

	Matrix output{{6.5, 6.5}};

	BOOST_CHECK_EQUAL(fc.output(), output);
}

BOOST_AUTO_TEST_CASE(convolutional)
{
	Conv2d<double, Identity<double>> convLayer(3, 3, 1, 1, 2, 2);
	blaze::DynamicMatrix<double> X{{0, 1, 2, 3, 4, 5, 6, 7, 8}};

	std::vector<double> K = {0, 1, 2, 3};
	std::vector<double> bias = {0};

	convLayer.setParameters({K, bias});

	convLayer.forward(X);

	blaze::DynamicMatrix<double> Y {{19, 25, 37, 43}};

	BOOST_CHECK_EQUAL(convLayer.output(), Y);

	/*blaze::DynamicMatrix<double> A = {{0, 1, 2, 3},
	                                  {4, 5, 6, 7}};
	blaze::DynamicMatrix<double> B = {{8, 9, 10, 1.2},
	                                  {1.2, 1.3, 1.4, 1.5}};

	blaze::DynamicMatrix<double> C(2, 2);
*/
	//blaze::gemm(CblasRowMajor, CblasNoTrans, CblasTrans, 2, 2, 4,
	 //           1, A.data(), 4, B.data(), 2, 1, C.data(), 2);
	/*double A0[] = {0, 1, 2, 3, 4, 5, 6, 7};
	double B0[] = {8, 9, 10, 1.2, 1.3, 1.4, 1.5};
	double C0[4];
	blaze::gemm(CblasRowMajor, CblasNoTrans, CblasTrans, 2, 2, 4,
	           1, A.data(), 4, B.data(), 4, 0, C.data(), 2);
	//blaze::gemm(CblasRowMajor, CblasNoTrans, CblasTrans, 2, 2, 4,
	//		   1, A0, 4, B0, 4, 0, C0, 2);
	std::cout << C << std::endl;
	std::cout << A * blaze::trans(B) << std::endl;*/
}

/*BOOST_AUTO_TEST_CASE(deconvolutional)
{
	Conv2dTranspose<double, Identity<double>> convTransposeLayer(2, 2, 1, 1, 2, 2);
	blaze::DynamicMatrix<double, blaze::columnMajor> X{{1, 2, 3, 4}};
	std::vector<double> K = {1, 2, 3, 4};
	std::vector<double> bias = {0};

	convTransposeLayer.setParameters({K, bias});

	convTransposeLayer.forward(X);

	blaze::DynamicMatrix<double, blaze::columnMajor> Y {{1, 4, 4, 6, 20, 16, 9, 24, 16}};

	BOOST_CHECK_EQUAL(convTransposeLayer.output(), Y);
}

 */
