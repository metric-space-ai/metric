#include <catch2/catch.hpp>

#include <iostream>
#include <limits>

#include "metric/transform/hog.hpp"
#include "metric/utils/datasets.hpp"

using namespace mtrc;

using Matrix = mtrc::numeric::DynamicMatrix<double>;
using Vector = mtrc::numeric::DynamicVector<double>;

TEST_CASE("hog", "[transform][hog]")
{
	auto hog = HOG<double>(9, 128, 2);

	auto [shape, data] = Datasets::loadImages("images-list");

	mtrc::numeric::DynamicMatrix<double> image(shape[1], shape[2]);
	size_t index = 0;
	for (size_t i = 0; i < shape[1]; ++i) {
		for (auto e = image.begin(i); e != image.end(i); ++e) {
			*e = data[index++];
		}
	}

	// mtrc::numeric::DynamicMatrix<double> image = mtrc::numeric::generate( 320, 320, []( size_t i, size_t j ){
	// return 2.1F + 1.1F*(i*3UL+j); } ); std::cout << image << std::endl;

	mtrc::numeric::DynamicVector<double, mtrc::numeric::rowVector> r = mtrc::numeric::trans(hog.encode(image));
	// std::cout << r.size() << std::endl;
	// std::cout << r << std::endl;

	auto distance = hog.getGroundDistance(image, 1, 1);
	// std::cout << distance. << std::endl;
	// std::cout << distance << std::endl;
}
