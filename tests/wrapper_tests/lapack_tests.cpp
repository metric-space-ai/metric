#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <limits>

#include <iostream>
#include "modules/utils/wrappers/lapack.cpp"


using namespace metric::dnn;

using Matrix = blaze::DynamicMatrix<double>;
using Vector = blaze::DynamicVector<double>;


BOOST_AUTO_TEST_CASE(fullyconnected_json)
{

}