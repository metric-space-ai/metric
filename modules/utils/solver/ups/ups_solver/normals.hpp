#ifndef _UPS_NORMALS_HPP
#define _UPS_NORMALS_HPP

#include "modules/utils/solver/ups/helpers/indexing.hpp"

#include "3rdparty/blaze/Blaze.h"

#include <tuple>
//#include <iostream>  // TODO remove




template <typename T>
blaze::DynamicMatrix<T>
pixNormals(
        const blaze::DynamicVector<T> & z,  // we assume that blaze::defaultTransposeFlag is ColumnVector by default
        const blaze::DynamicVector<T> & zx,
        const blaze::DynamicVector<T> & zy,
        const blaze::DynamicVector<T> & xx,
        const blaze::DynamicVector<T> & yy,
        const blaze::DynamicMatrix<T> & K
        )
{

    size_t pixels = zx.size();

    blaze::DynamicMatrix<T, blaze::columnMajor> N_unnormalized (pixels, 3, 0);
    auto N_unnorm_view0 = blaze::column(N_unnormalized, 0);
    N_unnorm_view0 = K(0, 0) * zx;
    auto N_unnorm_view1 = blaze::column(N_unnormalized, 1);
    N_unnorm_view1 = K(1, 1) * zy;
    auto N_unnorm_view2 = blaze::column(N_unnormalized, 2);
    N_unnorm_view2 = -z - (xx * zx) - (yy * zy);  // elementwize product

    //blaze::DynamicVector<T> dz = blaze::sqrt(blaze::sum<blaze::rowwise>(N_unnormalized % N_unnormalized)); // TODO compare to Eps if needed

    return N_unnormalized;  // TODO avoid implicit conversion to row-major!!
}


template <typename T>
blaze::DynamicMatrix<T>
normalizePixNormals(
        const blaze::DynamicMatrix<T> & N_unnormalized,
        const blaze::DynamicVector<T> & dz
        )
{
    size_t pixels = dz.size();

    blaze::DynamicMatrix<T, blaze::columnMajor> N_normalized (pixels, 3, 0);
    auto N_norm_view0 = blaze::column(N_normalized, 0);
    N_norm_view0 = blaze::column(N_unnormalized, 0) / dz;
    auto N_norm_view1 = blaze::column(N_normalized, 1);
    N_norm_view1 = blaze::column(N_unnormalized, 1) / dz;
    auto N_norm_view2 = blaze::column(N_normalized, 2);
    N_norm_view2 = blaze::column(N_unnormalized, 2) / dz;

    return N_normalized;

}





// ------------------
// augmented normals


#ifndef M_PI  // TODO replace
#define M_PI (3.14159265358979323846)
//#define M_PIl  3.141592653589793238462643383279502884L
#endif


enum harmo_order {ho_low, ho_high};


template <typename T>
blaze::DynamicMatrix<T>
normalsToSh(const blaze::DynamicMatrix<T> & normals, const int harmo_order = ho_low) {  //, int harmo_order = 1) {  // TODO generalize!!!!

    //int nb_harmo = (harmo_order+1)*(harmo_order+1);
    int nb_harmo = 9;
    if (harmo_order == ho_low)
        nb_harmo = 4;

    blaze::DynamicMatrix<T> spherical_harmonics (normals.rows(), nb_harmo, 0);

    T w0 = sqrt(3/(4*M_PI));
    T w3 = sqrt(1/(4*M_PI));
    // normals * w1  // write to view, then write repeated w4 to the last col
    auto c0 = blaze::column<0>(spherical_harmonics);
    auto c1 = blaze::column<1>(spherical_harmonics);
    auto c2 = blaze::column<2>(spherical_harmonics);
    auto c3 = blaze::column<3>(spherical_harmonics);
    c0 = w0 * blaze::column<0>(normals);
    c1 = w0 * blaze::column<1>(normals);
    c2 = w0 * blaze::column<2>(normals);
    c3 = w3;

    if (harmo_order != ho_low) {
        auto c4 = blaze::column<4>(spherical_harmonics);
        auto c5 = blaze::column<5>(spherical_harmonics);
        auto c6 = blaze::column<6>(spherical_harmonics);
        auto c7 = blaze::column<7>(spherical_harmonics);
        auto c8 = blaze::column<8>(spherical_harmonics);
        T w4 = 3*sqrt(5/(12*M_PI));
        T w5 = 3*sqrt(5/(12*M_PI));
        T w7 = 0.5*w5; // 3/2.0*sqrt(5/(12*M_PI));
        T w8 = 0.5*sqrt(5/(4*M_PI));
        c4 = w4 * blaze::column<0>(normals) * blaze::column<1>(normals); // elementwise multiplication
        c5 = w5 * blaze::column<0>(normals) * blaze::column<2>(normals);
        c6 = w5 * blaze::column<1>(normals) * blaze::column<2>(normals);
        c7 = w7 * (blaze::pow(blaze::column<0>(normals), 2) - blaze::pow(blaze::column<1>(normals), 2));
        c8 = w8 * (3 * blaze::pow(blaze::column<2>(normals), 2) - 1);
    }

    //std::cout << "sph harm:\n" << spherical_harmonics << "\n\n";
    return spherical_harmonics;
}



#endif // _UPS_NORMALS_HPP
