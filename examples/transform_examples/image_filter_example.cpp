
//#include "../../modules/utils/image_processing/image_filter.hpp"
//#include "../../modules/utils/image_processing/convolution.hpp"
//#include "../../modules/transform/wavelet2d.hpp"

#include "../../modules/transform/distance_potential_minimization.hpp"


#include "assets/helpers.cpp"
#include "modules/utils/visualizer.hpp"

int main() {

    blaze::DynamicMatrix<double> cm = read_csv_blaze<double>("assets/cameraman.csv", ",");
    mat2bmp::blaze2bmp_norm(cm, "input_cameraman.bmp");

    double sigma = 1.45;

    /* // using image_filter.hpp
    size_t filtersize = round(sigma * 6); // 3 sigma
    //metric::imfilter<double, 1, metric::FilterType::GAUSSIAN, metric::PadDirection::BOTH, metric::PadType::SYMMETRIC> f(filtersize, filtersize, sigma);
    metric::imfilter<double, 1, metric::FilterType::GAUSSIAN, metric::PadDirection::BOTH, metric::PadType::CONST> f(filtersize, filtersize, sigma);
    auto cm_blurred = f(cm);
    mat2bmp::blaze2bmp_norm(cm_blurred, "blurred_cameraman.bmp");
    cm_blurred = blaze::submatrix(cm_blurred, (cm_blurred.rows() - cm.rows()) / 2, (cm_blurred.columns() - cm.columns()) / 2, cm.rows(), cm.columns());
    //cm_blurred = blaze::submatrix(cm_blurred, 0, 0, cm.rows(), cm.columns());
    mat2bmp::blaze2bmp_norm(cm_blurred, "cropped_blurred_cameraman.bmp");
    // */

    /* using convolution.hpp

    auto kernel = gaussianKernel(sigma);
    mat2bmp::blaze2bmp_norm(kernel, "gkernel.bmp");

    //auto conv = metric::Convolution2d<double, 1>(cm.columns(), cm.rows(), kernel.columns(), kernel.rows());
    auto conv = metric::Convolution2d<double, 1>(cm.rows(), cm.columns(), kernel.rows(), kernel.columns());
    auto cm_blurred = conv({cm}, kernel)[0];
    mat2bmp::blaze2bmp_norm(cm_blurred, "cameraman_blurred.bmp");

    // */

    //* using wavelet2d.hpp

//    auto kernel = gaussianKernel(sigma);
//    mat2bmp::blaze2bmp_norm(kernel, "gkernel.bmp");

//    //auto conv = wavelet::Convolution2dCustom<double, 1>(cm.rows(), cm.columns(), kernel.rows(), kernel.columns());
//    auto conv = Convolution2dCustomStride1<double, 1>(cm.columns(), cm.rows(), kernel.columns(), kernel.rows());
//    //auto conv = wavelet::Convolution2dCustom<double, 1>(cm.columns(), cm.rows(), kernel.columns(), kernel.rows());
//    auto cm_blurred = conv({cm}, kernel)[0];
//    mat2bmp::blaze2bmp_norm(cm_blurred, "cameraman_blurred.bmp");
//    blaze::DynamicMatrix<double> cm_blurred_padded (cm.rows(), cm.columns(), 0);
//    blaze::submatrix(
//                cm_blurred_padded,
//                (cm.rows() - cm_blurred.rows())/2, (cm.columns() - cm_blurred.columns())/2,
//                cm_blurred.rows(), cm_blurred.columns()
//                ) = cm_blurred;
//    mat2bmp::blaze2bmp_norm(cm_blurred_padded, "cameraman_blurred_padded.bmp");
    auto cm_blurred_padded = metric::DPM_detail::gaussianBlur(cm, sigma);
    mat2bmp::blaze2bmp_norm(cm_blurred_padded, "cameraman_blurred_padded.bmp");


    // */

    return 0;

}

