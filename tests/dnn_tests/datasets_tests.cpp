#include <catch2/catch.hpp>

#include <iostream>
#include "metric/utils/datasets.hpp"

using namespace metric;


TEST_CASE("notexist", "[datasets]")
{
	auto [shape, data] = Datasets::loadImages("not-exist");
	REQUIRE(shape.empty());
}

TEST_CASE("empty", "[datasets]")
{
	auto [shape, data] = Datasets::loadImages("empty");
	std::vector<uint32_t> emptyShape = {0, 0, 0, 1};
	REQUIRE(shape == emptyShape);
}

TEST_CASE("wrong_shape", "[datasets]")
{
	auto [shape, data] = Datasets::loadImages("wrong-shape");
	REQUIRE(shape.empty());
}

TEST_CASE("images_list", "[datasets]")
{
	auto [shape, data] = Datasets::loadImages("images-list");
	std::vector<uint32_t> trueShape = {2, 3, 2, 1};
	std::vector<uint8_t> trueData = {10, 20, 30, 40, 50, 60,
								        70, 80, 90, 100, 110, 120};
	REQUIRE(trueShape == shape);
	REQUIRE(trueData == data);
}