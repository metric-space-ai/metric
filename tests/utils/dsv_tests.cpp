#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <limits>

#include <iostream>
#include "modules/utils/dsv.hpp"

using namespace std;
using namespace metric;


using Matrix = blaze::DynamicMatrix<double, blaze::columnMajor>;
using Vector = blaze::DynamicVector<double>;


BOOST_AUTO_TEST_CASE(base)
{
	string dsvLine = "1 2 3 4";

	get

}
