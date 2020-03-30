#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <limits>

#include <iostream>
#include <modules/utils/datasets.hpp>
#include "modules/transform/hog.hpp"


using namespace metric;

using Matrix = blaze::DynamicMatrix<double>;
using Vector = blaze::DynamicVector<double>;


BOOST_AUTO_TEST_CASE(base)
{
	auto hog = HOG<double>(9, 512, 1);

	auto [shape, data] = Datasets::loadImages("images-list");

	blaze::DynamicMatrix<double> image(shape[1], shape[2]);
	size_t index = 0;
	for (size_t i = 0; i < shape[1]; ++i) {
		for (auto e = image.begin(i); e != image.end(i); ++e) {
			*e = data[index++];
		}
	}

	//blaze::DynamicMatrix<double> image = blaze::generate( 320, 320, []( size_t i, size_t j ){ return 2.1F + 1.1F*(i*3UL+j); } );
	//std::cout << image << std::endl;

	blaze::DynamicVector<double, blaze::rowVector> r = blaze::trans(hog.encode(image));
	std::cout << r.size() << std::endl;
	std::cout << r << std::endl;
}
