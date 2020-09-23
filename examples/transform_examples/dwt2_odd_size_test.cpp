
#include "modules/transform/wavelet.hpp"

#include "3rdparty/blaze/Blaze.h"

#include <iostream>
#include <cmath>



int main() {

    using T = blaze::CompressedMatrix<double>;
    //using T = std::vector<blaze::DynamicVector<double>>; // runs old overload



    T data2d (75, 101, 0);
    data2d(4, 4) = 1;


    auto db4_w = wavelet::DaubechiesMat<double>(data2d.columns(), 6); // transform matrix for ROWS of approptiate size (as width of the image)
    auto db4_h = wavelet::DaubechiesMat<double>(data2d.rows(), 6); // for COLUMNS (image height)
    // order (number of coefficients) should not be greater than length of input vector!

    blaze::DynamicMatrix<double> db4_w_t = blaze::trans(db4_w); // transposed matrices for inverse trancform
    blaze::DynamicMatrix<double> db4_h_t = blaze::trans(db4_h);

    //auto splitted = wavelet::dwt2s(data2d, db4_w, db4_h);
    auto splitted = wavelet::dwt2(data2d, db4_w, db4_h);
    // auto splitted = wavelet::dwt2(data2d, 4);
    std::cout <<
                 "\n\nblaze 2d test: splitted " <<
                 std::get<0>(splitted).rows() << "*" << std::get<0>(splitted).columns() <<
                 //splitted.rows() << "*" << splitted.columns() <<
                 "\n";
    //auto restored = wavelet::dwt2s(splitted, db4_w_t, db4_h_t); // for non-matrix type
    //auto restored = wavelet::idwt2(std::get<0>(splitted), std::get<1>(splitted), std::get<2>(splitted), std::get<3>(splitted), 4, data2d.size(), data2d[0].size());
    auto restored = wavelet::idwt2(std::get<0>(splitted), std::get<1>(splitted), std::get<2>(splitted), std::get<3>(splitted), db4_w_t, db4_h_t);
    auto maxerr = blaze::max(restored - data2d);
    auto minerr = blaze::min(restored - data2d);
    std::cout << "blaze 2d test: restored: " <<
                 restored.rows() << "*" << restored.columns() <<
                 ", max error: " << (maxerr > -minerr ? maxerr : -minerr) << "\n\n";


    return 0;
}
