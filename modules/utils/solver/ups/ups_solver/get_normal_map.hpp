#ifndef _UPS_GET_NORMAL_MAP_HPP
#define _UPS_GET_NORMAL_MAP_HPP

#include "modules/utils/solver/ups/helpers/indexing.hpp"

#include "3rdparty/blaze/Blaze.h"

#include <tuple>
//#include <iostream>  // TODO remove


/*
template <typename T>
std::tuple<blaze::DynamicMatrix<T>, blaze::DynamicVector<T>, blaze::DynamicMatrix<T>>
getNormalMap(
        const blaze::DynamicVector<T> & z,  // we assume that blaze::defaultTransposeFlag is ColumnVector by default
        const blaze::DynamicVector<T> & zx,
        const blaze::DynamicVector<T> & zy,
        const blaze::DynamicMatrix<T> & K,
        const blaze::DynamicVector<T> & xx,
        const blaze::DynamicVector<T> & yy
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

    blaze::DynamicVector<T> dz = blaze::sqrt(blaze::sum<blaze::rowwise>(N_unnormalized % N_unnormalized)); // TODO compare to Eps if needed

    //blaze::DynamicVector<T, blaze::columnMajor> den = 1.0/dz;
    //blaze::DynamicVector<T> n_norm_0 = N_unnorma_view0 * den;  // should be elementwize, TODO check
    //blaze::DynamicVector<T> n_norm_1 = N_unnorma_view1 * den;
    //blaze::DynamicVector<T> n_norm_2 = N_unnorma_view2 * den;

    blaze::DynamicMatrix<T, blaze::columnMajor> N_normalized (pixels, 3, 0);
    auto N_norm_view0 = blaze::column(N_normalized, 0);
    N_norm_view0 = N_unnorm_view0 / dz; // * den;  // should be elementwize, TODO check
    auto N_norm_view1 = blaze::column(N_normalized, 1);
    N_norm_view1 = N_unnorm_view1 / dz; // * den;  // should be elementwize, TODO check
    auto N_norm_view2 = blaze::column(N_normalized, 2);
    N_norm_view2 = N_unnorm_view2 / dz; // * den;  // should be elementwize, TODO check

    return std::make_tuple(N_normalized, dz, N_unnormalized);

}
// */


//*

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

// */


#endif // _UPS_GET_NORMAL_MAP_HPP
