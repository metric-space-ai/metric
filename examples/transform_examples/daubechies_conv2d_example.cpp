
#include <iostream>
#include <chrono>

#include "modules/transform/wavelet.hpp"
#include "modules/transform/wavelet2d.hpp"

#include "assets/helpers.cpp"
#include "modules/utils/visualizer.hpp"


int main() {



    // Cameraman

    int DaubechiesOrder = 4; //10; // SETUP HERE wavelet type (2 till 20 even only)
    int l_scale = 300; //3000; // SETUP HERE lum scaling in visualizer

    auto cm_b = read_csv_blaze<double>("assets/cameraman.csv", ",");
    //auto cm_b = read_csv_blaze<double>("assets/test.csv", ",");


//    std::vector<double> c (DaubechiesOrder);
//    double coeff = 2/sqrt(2);
//    c = dbwavf<std::vector<double>>(DaubechiesOrder/2, coeff);
//    for (size_t i = 0; i < c.size(); ++i) {
//        c[i] = c[i]*coeff;
//    }


    auto c2d = wavelet::Convolution2dCustom<double, 1>(cm_b.rows(), cm_b.columns(), DaubechiesOrder, DaubechiesOrder);

    auto kernels = wavelet::create2dKernels<blaze::DynamicMatrix<double>>(DaubechiesOrder);
    std::cout << std::get<0>(kernels) << "\n";
    std::cout << std::get<1>(kernels) << "\n";
    std::cout << std::get<2>(kernels) << "\n";
    std::cout << std::get<3>(kernels) << "\n";


    /*  // old example
    std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> t1, t2;

    {
        t1 = std::chrono::steady_clock::now();
        auto db4_w = wavelet::DaubechiesMat<double>(cm_b.columns(), DaubechiesOrder); // transform matrix for ROWS of approptiate size (as width of the image)
        auto db4_h = wavelet::DaubechiesMat<double>(cm_b.rows(), DaubechiesOrder); // for COLUMNS (image height)
        blaze::CompressedMatrix<double> db4_w_t = blaze::trans(db4_w); // transposed matrices for inverse trancform
        blaze::CompressedMatrix<double> db4_h_t = blaze::trans(db4_h);
        t2 = std::chrono::steady_clock::now();
        std::cout << "Daubechies transform matrices creation time: " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "\n";

        t1 = std::chrono::steady_clock::now();
        auto cm_splitted_periodized = wavelet::dwt2s(cm_b, db4_w, db4_h);
        auto cm_splitted_periodized_2 = wavelet::dwt2s(cm_splitted_periodized, db4_w, db4_h);
        auto cm_splitted_periodized_3 = wavelet::dwt2s(cm_splitted_periodized_2, db4_w, db4_h);
        auto cm_splitted_periodized_4 = wavelet::dwt2s(cm_splitted_periodized_3, db4_w, db4_h);
        t2 = std::chrono::steady_clock::now();
        std::cout << "4x split time: " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "\n";

        auto cm_decoded_periodized = wavelet::dwt2s(cm_splitted_periodized, db4_w_t, db4_h_t);

        mat2bmp::blaze2bmp(cm_splitted_periodized/l_scale, "cm_splited_per.bmp");
        mat2bmp::blaze2bmp(cm_decoded_periodized/l_scale, "cm_restored_per.bmp");

        t1 = std::chrono::steady_clock::now();
        auto cm_decoded_periodized_cascade = wavelet::dwt2s(cm_splitted_periodized_4, db4_w_t, db4_h_t);
        cm_decoded_periodized_cascade = wavelet::dwt2s(cm_decoded_periodized_cascade, db4_w_t, db4_h_t);
        cm_decoded_periodized_cascade = wavelet::dwt2s(cm_decoded_periodized_cascade, db4_w_t, db4_h_t);
        cm_decoded_periodized_cascade = wavelet::dwt2s(cm_decoded_periodized_cascade, db4_w_t, db4_h_t);
        t2 = std::chrono::steady_clock::now();
        std::cout << "4x compose time: " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "\n";

        mat2bmp::blaze2bmp(cm_splitted_periodized_2/l_scale, "cm_splited_per_2.bmp");
        mat2bmp::blaze2bmp(cm_splitted_periodized_3/l_scale, "cm_splited_per_3.bmp");
        mat2bmp::blaze2bmp(cm_splitted_periodized_4/l_scale, "cm_splited_per_4.bmp");
        mat2bmp::blaze2bmp(cm_decoded_periodized_cascade/l_scale, "cm_restored_per_cascade.bmp");

    }
    // */




    return 0;
}
