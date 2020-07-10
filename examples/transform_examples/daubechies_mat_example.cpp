
//#include <vector>
#include <iostream>

//#include "../../modules/transform.hpp"
#include "modules/transform/wavelet.hpp"


int main() {


    // ----- 1d DWT transform example

    blaze::DynamicVector<double> v {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    //blaze::DynamicVector<double> v {0, 0.2, 0.4, 0.6, 0.8, 1};

    auto db4 = wavelet::DaubechiesMat<double>(v.size());
    std::cout << "Daubechies D4 transform matrix, upper half for low-pass, lower hals for high-pass:\n" << db4 << "\n";

    blaze::DynamicVector<double> encoded = db4*v; // dont use 'auto' here!!, it will result in matrix type, whereas we need column vector
    std::cout << "decomposed vector:\n" << encoded << "\n";
    auto decoded = db4.transpose()*encoded;
    std::cout  << "restored vector:\n" << decoded << "\n";


    // ----- 2d DWT examples

    blaze::DynamicMatrix<double> img { // 8*10 image, even size required
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0.5},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    auto db4_w = wavelet::DaubechiesMat<double>(img.columns(), 6); // transform matrix for ROWS of approptiate size (as width of the image)
    auto db4_h = wavelet::DaubechiesMat<double>(img.rows(), 6); // for COLUMNS (image height)
    // order (number of coefficients) should not be greater than length of input vector!

    blaze::DynamicMatrix<double> db4_w_t = blaze::trans(db4_w); // transposed matrices for inverse trancform
    blaze::DynamicMatrix<double> db4_h_t = blaze::trans(db4_h);


    auto encoded_img = wavelet::dwt2s(img, db4_w, db4_h); // whole image transform, results in single image of all subbands concatenated
    auto decoded_img = wavelet::dwt2s(encoded_img, db4_w_t, db4_h_t); // same function, transposed transform matrices for inverse transform

    std::cout << "decomposed image:\n" << encoded_img << "\n";
    std::cout << "restored image:\n" << decoded_img << "\n";


    auto encoded_img_tuple = wavelet::dwt2(img, db4_w, db4_h); // transform with outputting subbands apart in a tuple
    auto decoded_img_2 = wavelet::idwt2(encoded_img_tuple, db4_w_t, db4_h_t); // here we also need transposed matrices for inverse transform

    std::cout << "low-low subband of decomposed image: \n" << std::get<0>(encoded_img_tuple) << "\n";
    std::cout << "restored image: \n" << decoded_img_2 << "\n";


    return 0;
}
