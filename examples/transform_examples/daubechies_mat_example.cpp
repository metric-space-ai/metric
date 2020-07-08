
//#include <vector>
#include <iostream>

//#include "../../modules/transform.hpp"
#include "modules/transform/wavelet.hpp"


int main() {

    blaze::DynamicVector<double> v {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    //blaze::DynamicVector<double> v {0, 0.2, 0.4, 0.6, 0.8, 1};

    auto db4 = wavelet::DaubechiesMat<double>(v.size());
    std::cout << db4 << "\n";

    blaze::DynamicVector<double> encoded = db4*v; // dont use auto here, it will lead to matrix, whereas we need column vector
    std::cout << encoded << "\n";
    auto decoded = db4.transpose()*encoded;
    std::cout << decoded << "\n";

    blaze::DynamicMatrix<double> img {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    auto db4_w = wavelet::DaubechiesMat<double>(img.columns());
    auto db4_h = wavelet::DaubechiesMat<double>(img.rows());
    auto encoded_img = wavelet::dwt2(img, db4_w, db4_h);
    std::cout << std::get<0>(encoded_img) << "\n";
    blaze::DynamicMatrix<double> db4_w_t = blaze::trans(db4_w);
    blaze::DynamicMatrix<double> db4_h_t = blaze::trans(db4_h);
    blaze::DynamicMatrix<double> encoded_img_all (img.rows(), img.columns());
    blaze::submatrix(encoded_img_all, 0, 0, img.rows()/2, img.columns()/2) = std::get<0>(encoded_img);
    blaze::submatrix(encoded_img_all, img.rows()/2, 0, img.rows()/2, img.columns()/2) = std::get<1>(encoded_img);
    blaze::submatrix(encoded_img_all, 0, img.columns()/2, img.rows()/2, img.columns()/2) = std::get<2>(encoded_img);
    blaze::submatrix(encoded_img_all, img.rows()/2, img.columns()/2, img.rows()/2, img.columns()/2) = std::get<3>(encoded_img);
    auto decoded_img = wavelet::dwt2(encoded_img_all, db4_w_t, db4_h_t);
    std::cout << std::get<0>(decoded_img) << "\n";

    return 0;
}
