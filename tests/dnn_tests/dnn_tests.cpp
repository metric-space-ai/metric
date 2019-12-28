#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <limits>

#include <iostream>
#include "modules/utils/dnn.hpp"
#include "modules/utils/dnn/Layer.h"
#include "modules/utils/dnn/Utils/Serializer.h"
#include "modules/utils/dnn/Layer/Conv2d.h"
#include "modules/utils/dnn/Layer/Conv2d-transpose.h"
#include "modules/utils/dnn/Layer/FullyConnected.h"
#include "modules/utils/dnn/Layer/MaxPooling.h"
#include "modules/utils/dnn/Activation/Identity.h"


using namespace MiniDNN;

using Matrix = blaze::DynamicMatrix<double, blaze::columnMajor>;
using Vector = blaze::DynamicVector<double>;

BOOST_AUTO_TEST_CASE(base)
{
	Network<double> net;

	RNG rng(1);
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

// these are prerequisites for any serialization to work:
// - CEREAL_SIMPLIFY_FOR_BASE_CLASS() for each necessary Layer+Scalar
//   combination
// - register types with names
// - register polymorphic relations
typedef Layer<float>::LayerSerialProxy LayerParent;
CEREAL_SIMPLIFY_FOR_BASE_CLASS(LayerParent);
CEREAL_REGISTER_TYPE_WITH_NAME(LayerParent, "Layer_float");

typedef typename Conv2d<float, typename MiniDNN::Identity<float>>::LayerSerialProxy Conv2dProxy;
CEREAL_REGISTER_TYPE_WITH_NAME(Conv2dProxy, "Conv2d_float_Identity");
CEREAL_REGISTER_POLYMORPHIC_RELATION(LayerParent, Conv2dProxy);

typedef typename Conv2dTranspose<float, typename MiniDNN::Identity<float>>::LayerSerialProxy Conv2dTransposeProxy;
CEREAL_REGISTER_TYPE_WITH_NAME(Conv2dTransposeProxy, "Conv2dTranspose_float_Identity");
CEREAL_REGISTER_POLYMORPHIC_RELATION(LayerParent, Conv2dTransposeProxy);

typedef typename FullyConnected<float, typename MiniDNN::Identity<float>>::LayerSerialProxy FullyConnectedProxy;
CEREAL_REGISTER_TYPE_WITH_NAME(FullyConnectedProxy, "FullyConnected_float_Identity");
CEREAL_REGISTER_POLYMORPHIC_RELATION(LayerParent, FullyConnectedProxy);

typedef typename MaxPooling<float, typename MiniDNN::Identity<float>>::LayerSerialProxy MaxPoolingProxy;
CEREAL_REGISTER_TYPE_WITH_NAME(MaxPoolingProxy, "MaxPooling_float_Identity");
CEREAL_REGISTER_POLYMORPHIC_RELATION(LayerParent, MaxPoolingProxy);


BOOST_AUTO_TEST_CASE(serialization_load_non_existing_file)
{
    try {
        std::vector<std::shared_ptr<Layer<float>>> layers = loadFrom<float>("");
        throw std::runtime_error("exception expected");
    } catch (std::runtime_error& e) {
        // nothing here
    }
}

BOOST_AUTO_TEST_CASE(serialization_load_empty)
{
    try {
        std::vector<std::shared_ptr<Layer<float>>> layers = loadFrom<float>("../../../tests/dnn_tests/serialization_load_empty.json");
        throw std::runtime_error("exception expected");
    } catch (cereal::RapidJSONException& e) {
        // nothing here
    }
}

BOOST_AUTO_TEST_CASE(serialization_load_empty_object)
{
    std::vector<std::shared_ptr<Layer<float>>> layers = loadFrom<float>("../../../tests/dnn_tests/serialization_load_empty_object.json");
    BOOST_TEST(layers.size() == 0);    
}

BOOST_AUTO_TEST_CASE(serialization_load_one)
{
    std::string f1 = "../../../tests/dnn_tests/serialization_load_one_layer.json";
    std::string f2 = "/tmp/test_serialization.json";
    std::vector<std::shared_ptr<Layer<float>>> layers = loadFrom<float>(f1);
    BOOST_TEST(layers.size() == 1);
    saveTo<float>(f2, layers);

    {
        std::ifstream ifs1(f1);
        std::ifstream ifs2(f2);

        std::istream_iterator<char> b1(ifs1), e1;
        std::istream_iterator<char> b2(ifs2), e2;

        BOOST_CHECK_EQUAL_COLLECTIONS(b1, e1, b2, e2);
    }
}

BOOST_AUTO_TEST_CASE(serialization_load_all)
{
    std::string f1 = "../../../tests/dnn_tests/serialization_load_all_layers.json";
    std::string f2 = "/tmp/test_serialization.json";
    std::vector<std::shared_ptr<Layer<float>>> layers = loadFrom<float>(f1);
    BOOST_TEST(layers.size() == 4);
    saveTo<float>(f2, layers);

    {
        std::ifstream ifs1(f1);
        std::ifstream ifs2(f2);

        std::istream_iterator<char> b1(ifs1), e1;
        std::istream_iterator<char> b2(ifs2), e2;

        BOOST_CHECK_EQUAL_COLLECTIONS(b1, e1, b2, e2);
    }
}
