#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>


#include <iostream>
#include "modules/utils/image_processing/convolution.hpp"


using namespace metric;


TEMPLATE_TEST_CASE("base", "[convolution]", float, double)
{
	using Conv = Convolution2d<TestType, 1>;

	typename Conv::Image image{
								typename Conv::Channel{{0,1,2,2},
													   {3,4,5,5},
													   {6,7,8,1}}
								};

	typename Conv::FilterKernel kernel{{5,7,2},
									  {1,9,9},
									  {4,3,2}};

	auto conv = Conv(image[0].columns(), image[0].rows(),
				                kernel.columns(), kernel.rows());

	typename Conv::Image result{
								typename Conv::Channel{{26, 61, 78, 55,},
														{97, 156, 171, 109},
													   {146, 194, 153, 77}}
								};

	conv.setKernel(kernel);

	REQUIRE(conv(image) == result);
}
