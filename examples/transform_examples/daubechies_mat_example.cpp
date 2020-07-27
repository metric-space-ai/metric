
//#include <vector>
#include <iostream>
#include <chrono>

//#include "../../modules/transform.hpp"
#include "modules/transform/wavelet.hpp"

#include "assets/helpers.cpp"
#include "modules/utils/visualizer.hpp"


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



    // DWT periodizided examples


    { // serialized matrix example
        auto db4_w = wavelet::DaubechiesMat_e<double>(img.columns(), img.columns()*img.rows(), 6); // transform matrix for ROWS of approptiate size (as width of the image)
        auto db4_h = wavelet::DaubechiesMat_e<double>(img.rows(), img.columns()*img.rows(), 6); // for COLUMNS (image height)
        // order (number of coefficients) should not be greater than length of input vector!

        blaze::DynamicMatrix<double> db4_w_t = blaze::trans(db4_w); // transposed matrices for inverse trancform
        blaze::DynamicMatrix<double> db4_h_t = blaze::trans(db4_h);


        auto encoded_img = wavelet::dwt2s_e(img, db4_w, db4_h); // whole image transform, results in single image of all subbands concatenated
        auto decoded_img = wavelet::dwt2s_e(encoded_img, db4_w_t, db4_h_t); // same function, transposed transform matrices for inverse transform

        std::cout << "decomposed image (sm ver) :\n" << encoded_img << "\n";
        std::cout << "restored image (sm ver) :\n" << decoded_img << "\n";


        auto encoded_img_tuple = wavelet::dwt2_e(img, db4_w, db4_h); // transform with outputting subbands apart in a tuple
        auto decoded_img_2 = wavelet::idwt2_e(encoded_img_tuple, db4_w_t, db4_h_t); // here we also need transposed matrices for inverse transform

        std::cout << "low-low subband of decomposed image (sm ver) : \n" << std::get<0>(encoded_img_tuple) << "\n";
        std::cout << "restored image (sm ver) : \n" << decoded_img_2 << "\n";

    }


    // Cameraman

    int DaubechiesOrder = 6; //10; // SETUP HERE wavelet type (2 till 20 even only)
    int l_scale = 300; //3000; // SETUP HERE lum scaling in visualizer

    auto cm_b = read_csv_blaze<double>("assets/cameraman.csv", ",");
    //auto cm_b = read_csv_blaze<double>("assets/test.csv", ",");

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

    {  // serialized image version

        //auto dmat_n = wavelet::DaubechiesMat_e<double>(6, 24, 4);
        //std::cout << "dmat 10*6, d4: " << dmat_n << "\n";

        t1 = std::chrono::steady_clock::now();
        auto db4_w = wavelet::DaubechiesMat_e<double>(cm_b.columns(), cm_b.columns()*cm_b.rows(), DaubechiesOrder); // transform matrix for ROWS of approptiate size (as width of the image)
        auto db4_h = wavelet::DaubechiesMat_e<double>(cm_b.rows(), cm_b.columns()*cm_b.rows(), DaubechiesOrder); // for COLUMNS (image height)
        blaze::CompressedMatrix<double> db4_w_t = blaze::trans(db4_w); // transposed matrices for inverse trancform
        blaze::CompressedMatrix<double> db4_h_t = blaze::trans(db4_h);
        t2 = std::chrono::steady_clock::now();
        std::cout << "Daubechies transform matrices creation time (serialized ver): " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "\n";


        t1 = std::chrono::steady_clock::now();
        auto cm_splitted_periodized = wavelet::dwt2s_e(cm_b, db4_w, db4_h);
        auto cm_splitted_periodized_2 = wavelet::dwt2s_e(cm_splitted_periodized, db4_w, db4_h);
        auto cm_splitted_periodized_3 = wavelet::dwt2s_e(cm_splitted_periodized_2, db4_w, db4_h);
        auto cm_splitted_periodized_4 = wavelet::dwt2s_e(cm_splitted_periodized_3, db4_w, db4_h);
        t2 = std::chrono::steady_clock::now();
        std::cout << "4x split time (serialized ver): " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "\n";

        auto cm_decoded_periodized = wavelet::dwt2s_e(cm_splitted_periodized, db4_w_t, db4_h_t);

        mat2bmp::blaze2bmp(cm_splitted_periodized/l_scale, "cm_splited_per_e.bmp");
        mat2bmp::blaze2bmp(cm_decoded_periodized/l_scale, "cm_restored_per_e.bmp");

        t1 = std::chrono::steady_clock::now();
        auto cm_decoded_periodized_cascade = wavelet::dwt2s_e(cm_splitted_periodized_4, db4_w_t, db4_h_t);
        cm_decoded_periodized_cascade = wavelet::dwt2s_e(cm_decoded_periodized_cascade, db4_w_t, db4_h_t);
        cm_decoded_periodized_cascade = wavelet::dwt2s_e(cm_decoded_periodized_cascade, db4_w_t, db4_h_t);
        cm_decoded_periodized_cascade = wavelet::dwt2s_e(cm_decoded_periodized_cascade, db4_w_t, db4_h_t);
        t2 = std::chrono::steady_clock::now();
        std::cout << "4x decompose time (serialized ver): " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "\n";

        mat2bmp::blaze2bmp(cm_splitted_periodized_2/l_scale, "cm_splited_per_2_e.bmp");
        mat2bmp::blaze2bmp(cm_splitted_periodized_3/l_scale, "cm_splited_per_3_e.bmp");
        mat2bmp::blaze2bmp(cm_splitted_periodized_4/l_scale, "cm_splited_per_4_e.bmp");
        mat2bmp::blaze2bmp(cm_decoded_periodized_cascade/l_scale, "cm_restored_per_cascade_e.bmp");

    }

    {  // loop-based periodized convolution version

        t1 = std::chrono::steady_clock::now();
        auto cm_splitted_periodized = wavelet::dwt2_l(cm_b, DaubechiesOrder);
        auto cm_splitted_periodized_2 = wavelet::dwt2_l(cm_splitted_periodized, DaubechiesOrder);
        auto cm_splitted_periodized_3 = wavelet::dwt2_l(cm_splitted_periodized_2, DaubechiesOrder);
        auto cm_splitted_periodized_4 = wavelet::dwt2_l(cm_splitted_periodized_3, DaubechiesOrder);
        t2 = std::chrono::steady_clock::now();
        std::cout << "4x split time (loop ver): " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "\n";

        //auto cm_decoded_periodized = wavelet::dwt2s_e(cm_splitted_periodized, db4_w_t, db4_h_t);

        mat2bmp::blaze2bmp(cm_splitted_periodized/l_scale, "cm_splited_loop_e.bmp");
        //mat2bmp::blaze2bmp(cm_decoded_periodized/l_scale, "cm_restored_loop_per_e.bmp");

        // TODO add composition

        mat2bmp::blaze2bmp(cm_splitted_periodized_2/l_scale, "cm_splited_loop_2_e.bmp");
        mat2bmp::blaze2bmp(cm_splitted_periodized_3/l_scale, "cm_splited_loop_3_e.bmp");
        mat2bmp::blaze2bmp(cm_splitted_periodized_4/l_scale, "cm_splited_loop_4_e.bmp");

    }


//    {
//        // test reshape trick
//        blaze::DynamicVector<double> v {1, 2, 3, 4, 5, 6};
//        blaze::DynamicVector<double> u (6); // {0, 0, 0, 0, 0, 0};

//        blaze::DynamicMatrix<double> space_matrix{{1, 0}, {0, -1}};

//        blaze::CustomMatrix<double, blaze::unaligned, blaze::unpadded, blaze::rowMajor> vit(&v[0], 2, 3);
//        blaze::CustomMatrix<double, blaze::unaligned, blaze::unpadded, blaze::rowMajor> ujs(&u[0], 2, 3);

//        ujs = space_matrix * vit;

//        std::cout << ujs << '\n';
//        std::cout << u << std::endl;

//        std::cout << vit << '\n';
//        std::cout << v << std::endl;

//        std::cout << space_matrix << std::endl;
//        std::cout << "\ndone\n" << std::endl;


//    }


    return 0;
}
