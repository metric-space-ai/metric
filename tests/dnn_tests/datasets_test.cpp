#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include <iostream>
#include "modules/utils/dnn/Utils/datasets.hpp"


BOOST_AUTO_TEST_CASE(notexist)
{
	auto [shape, data] = Datasets::loadImages("not-exist");
	BOOST_TEST_CHECK(shape.empty());
}

BOOST_AUTO_TEST_CASE(empty)
{
	auto [shape, data] = Datasets::loadImages("empty");
	std::vector<uint32_t> emptyShape = {0, 0, 0, 1};
	BOOST_CHECK_EQUAL_COLLECTIONS(shape.begin(), shape.end(), emptyShape.begin(), emptyShape.end());
}

BOOST_AUTO_TEST_CASE(wrong_shape)
{
	auto [shape, data] = Datasets::loadImages("wrong-shape");
	BOOST_TEST_CHECK(shape.empty());
}

BOOST_AUTO_TEST_CASE(images_list)
{
	auto [shape, data] = Datasets::loadImages("images-list");
	std::vector<uint32_t> trueShape = {2, 3, 2, 1};
	std::vector<uint32_t> trueData = {10, 20, 30, 40, 50, 60,
								        70, 80, 90, 100, 110, 120};
	BOOST_CHECK_EQUAL_COLLECTIONS(trueShape.begin(), trueShape.end(), shape.begin(), shape.end());
	BOOST_CHECK_EQUAL_COLLECTIONS(trueData.begin(), trueData.end(), data.begin(), data.end());
}

