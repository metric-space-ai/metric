
// #include "metric/utils/image_processing/image_filter.hpp"
// #include "metric/utils/image_processing/convolution.hpp"
// #include "metric/transform/wavelet2d.hpp"

#include "metric/transform/distance_potential_minimization.hpp"

#include "donuts.hpp" // for only png reader

#include "assets/helpers.cpp"
#include "metric/utils/visualizer.hpp"

int main()
{

	mtrc::numeric::DynamicMatrix<double> cm = read_csv_matrix<double>("assets/cameraman.csv", ",");
	mat2bmp::matrix_to_bmp_norm(cm, "input_cameraman.bmp");

	double sigma = 5; // 1.45;

	/* // using image_filter.hpp
	size_t filtersize = round(sigma * 6); // 3 sigma
	//mtrc::imfilter<double, 1, mtrc::FilterType::GAUSSIAN, mtrc::PadDirection::BOTH, mtrc::PadType::SYMMETRIC>
	f(filtersize, filtersize, sigma); mtrc::imfilter<double, 1, mtrc::FilterType::GAUSSIAN,
	mtrc::PadDirection::BOTH, mtrc::PadType::CONST> f(filtersize, filtersize, sigma); auto cm_blurred = f(cm);
	mat2bmp::matrix_to_bmp_norm(cm_blurred, "blurred_cameraman.bmp");
	cm_blurred = mtrc::numeric::submatrix(cm_blurred, (cm_blurred.rows() - cm.rows()) / 2, (cm_blurred.columns() -
	cm.columns()) / 2, cm.rows(), cm.columns());
	//cm_blurred = mtrc::numeric::submatrix(cm_blurred, 0, 0, cm.rows(), cm.columns());
	mat2bmp::matrix_to_bmp_norm(cm_blurred, "cropped_blurred_cameraman.bmp");
	// */

	/* using convolution.hpp

	auto kernel = gaussianKernel(sigma);
	mat2bmp::matrix_to_bmp_norm(kernel, "gkernel.bmp");

	//auto conv = mtrc::Convolution2d<double, 1>(cm.columns(), cm.rows(), kernel.columns(), kernel.rows());
	auto conv = mtrc::Convolution2d<double, 1>(cm.rows(), cm.columns(), kernel.rows(), kernel.columns());
	auto cm_blurred = conv({cm}, kernel)[0];
	mat2bmp::matrix_to_bmp_norm(cm_blurred, "cameraman_blurred.bmp");

	// */

	//* using wavelet2d.hpp

	//    auto kernel = gaussianKernel(sigma);
	//    mat2bmp::matrix_to_bmp_norm(kernel, "gkernel.bmp");

	//    //auto conv = wavelet::Convolution2dCustom<double, 1>(cm.rows(), cm.columns(), kernel.rows(),
	//    kernel.columns()); auto conv = Convolution2dCustomStride1<double, 1>(cm.columns(), cm.rows(),
	//    kernel.columns(), kernel.rows());
	//    //auto conv = wavelet::Convolution2dCustom<double, 1>(cm.columns(), cm.rows(), kernel.columns(),
	//    kernel.rows()); auto cm_blurred = conv({cm}, kernel)[0]; mat2bmp::matrix_to_bmp_norm(cm_blurred,
	//    "cameraman_blurred.bmp"); mtrc::numeric::DynamicMatrix<double> cm_blurred_padded (cm.rows(), cm.columns(),
	//    0); mtrc::numeric::submatrix(
	//                cm_blurred_padded,
	//                (cm.rows() - cm_blurred.rows())/2, (cm.columns() - cm_blurred.columns())/2,
	//                cm_blurred.rows(), cm_blurred.columns()
	//                ) = cm_blurred;
	//    mat2bmp::matrix_to_bmp_norm(cm_blurred_padded, "cameraman_blurred_padded.bmp");

	// cm = read_png_donut<double>("assets/donuts/crop/donut_6_radial_outer_128.png");
	std::cout << "input: " << mtrc::numeric::max(cm) << ".." << mtrc::numeric::min(cm) << "\n";
	mat2bmp::matrix_to_bmp_norm(cm, "input_1.bmp");
	vector2bmp(matrix2vv(mtrc::numeric::DynamicMatrix<double>(cm / 256.0)), "input_2.bmp");
	vector2bmp(matrix2vv(cm), "input_3.bmp");

	// auto cm_blurred_padded = mtrc::DPM_detail::gaussianBlur(cm, sigma);
	auto kernel = mtrc::DPM_detail::gaussianKernel(sigma);
	auto cm_blurred_padded = mtrc::DPM_detail::blackPaddedConv(cm, kernel);

	mat2bmp::matrix_to_bmp_norm(cm_blurred_padded, "blurred_padded_1.bmp");
	vector2bmp(matrix2vv(mtrc::numeric::DynamicMatrix<double>(cm_blurred_padded / 256.0)), "blurred_padded_2.bmp");
	vector2bmp(matrix2vv(cm_blurred_padded), "blurred_padded_3.bmp");
	// std::cout << cm_blurred_padded << "\n";
	std::cout << "output: " << mtrc::numeric::max(cm_blurred_padded) << ".."
			  << mtrc::numeric::min(cm_blurred_padded) << "\n";

	// */

	return 0;
}
