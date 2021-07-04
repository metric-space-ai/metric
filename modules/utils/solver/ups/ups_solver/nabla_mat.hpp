#ifndef _UPS_NABLA_HPP
#define _UPS_NABLA_HPP

#include "modules/utils/solver/ups/helpers/indexing.hpp"

#include "3rdparty/blaze/Blaze.h"

#include <tuple>



enum nabla_approximation {Forward, Backward, Central};
enum nabla_boundary_condition {DirichletHomogeneous, NeumannHomogeneous}; //, NeumannConstant};


/**
 * @brief computes nabla operator matrix for given pixel mask
 * @param mask - bool pixel mask of same size as image
 * @param approx - approximation mode
 * @param boundary - the way boundaries are processed
 * @return nabla operators by x and by y (as matrices the vectorized image should be multiplied by in order to obtain gradients)
 */
template <typename T>
std::tuple<blaze::CompressedMatrix<T>, blaze::CompressedMatrix<T>>
//std::tuple<blaze::DynamicMatrix<T>, blaze::DynamicMatrix<T>>
nablaMat(
        const blaze::DynamicMatrix<bool, blaze::columnMajor> & mask,
        const nabla_approximation approx = Central,
        const nabla_boundary_condition boundary = NeumannHomogeneous
        )
{

    //size_t out_size = mask.columns()*mask.rows();

    //blaze::CompressedMatrix<T> Dx;
    //blaze::CompressedMatrix<T> Dy;
    blaze::DynamicMatrix<T> Dx;
    blaze::DynamicMatrix<T> Dy;

    switch (approx) {
    case Forward:
        Dx = blaze::IdentityMatrix<T>(mask.columns()) * -1;
        Dy = blaze::IdentityMatrix<T>(mask.rows()) * -1;
        for (size_t i = 0; i < mask.columns() - 1; ++i) {
            Dx(i, i+1) = 1;
        }
        for (size_t i = 0; i < mask.rows() - 1; ++i) {
            Dy(i, i+1) = 1;
        }
        break;
    case Backward:
        Dx = blaze::IdentityMatrix<T>(mask.columns());
        Dy = blaze::IdentityMatrix<T>(mask.rows());
        for (size_t i = 0; i < mask.columns() - 1; ++i) {
            Dx(i+1, i) = -1;
        }
        for (size_t i = 0; i < mask.rows() - 1; ++i) {
            Dy(i+1, i) = -1;
        }
        break;
    default:  // Central
        //Dx = blaze::CompressedMatrix<T>(mask.columns(), mask.columns(), 0);
        //Dy = blaze::CompressedMatrix<T>(mask.rows(), mask.rows(), 0);
        Dx = blaze::DynamicMatrix<T>(mask.columns(), mask.columns(), 0);
        Dy = blaze::DynamicMatrix<T>(mask.rows(), mask.rows(), 0);
        for (size_t i = 0; i < mask.columns() - 1; ++i) {
            Dx(i, i+1) = 0.5;
            Dx(i+1, i) = -0.5;
        }
        for (size_t i = 0; i < mask.rows() - 1; ++i) {
            Dy(i, i+1) = 0.5;
            Dy(i+1, i) = -0.5;
        }
        break;
    }

    //blaze::CompressedMatrix<T> nx = blaze::kron(Dx, blaze::IdentityMatrix<T>(mask.rows()));
    //blaze::CompressedMatrix<T> ny = blaze::kron(blaze::IdentityMatrix<T>(mask.columns()), Dy);
    //blaze::DynamicMatrix<T> nx = blaze::kron(Dx, blaze::IdentityMatrix<T>(mask.rows()));
    //blaze::DynamicMatrix<T> ny = blaze::kron(blaze::IdentityMatrix<T>(mask.columns()), Dy);
    blaze::CompressedMatrix<T, blaze::columnMajor> Ir = blaze::IdentityMatrix<T>(mask.rows());
    blaze::CompressedMatrix<T, blaze::columnMajor> Ic = blaze::IdentityMatrix<T>(mask.columns());
    blaze::CompressedMatrix<T, blaze::columnMajor> nx = blaze::kron(Dx, Ir);
    blaze::CompressedMatrix<T, blaze::columnMajor> ny = blaze::kron(Ic, Dy); // TODO optimize
    //blaze::DynamicMatrix<T> nx = nx_sp;
    //blaze::DynamicMatrix<T> ny = ny_sp;


    /*  // old code  // apply NeumannHomogeneous

    auto idx = indicesCw(mask);

    size_t s = blaze::nonZeros(mask);  // should be equal to length of idx and height and width of nabla_x, nabla_y

    //blaze::CompressedMatrix<T> nabla_x (s, s);
    //blaze::CompressedMatrix<T> nabla_y (s, s);
    blaze::DynamicMatrix<T> nabla_x (s, s, 0);
    blaze::DynamicMatrix<T> nabla_y (s, s, 0);

    size_t r = 0;
    size_t c;
    for (blaze::DynamicVector<size_t>::Iterator it=idx.begin(); it!=idx.end(); ++it) {
        c = 0;
        for (blaze::DynamicVector<size_t>::Iterator it2=idx.begin(); it2!=idx.end(); ++it2) {
            nabla_x(r, c) = nx(*it, *it2);
            nabla_y(r, c) = ny(*it, *it2);
            ++c;
        }
        ++r;
    }
    // */

    auto idx = indicesCwStd(mask);

    size_t s = blaze::nonZeros(mask);  // should be equal to length of idx and height and width of nabla_x, nabla_y

    blaze::CompressedMatrix<T> nx_filtering (s, mask.columns());
    blaze::CompressedMatrix<T> ny_filtering (s, mask.columns());
    nx_filtering = blaze::columns(nx, idx);
    ny_filtering = blaze::columns(ny, idx);

    blaze::CompressedMatrix<T> nabla_x (s, s);
    blaze::CompressedMatrix<T> nabla_y (s, s);
    nabla_x = blaze::rows(nx_filtering, idx);
    nabla_y = blaze::rows(ny_filtering, idx);


    if (boundary == NeumannHomogeneous) {  // here nabla matches the DirichletHomogeneous option
        for (size_t i = 0; i < s; ++i) {  // set rows where at least one neighbour does not exist to 0.
            if (sum(blaze::row(nabla_x, i)) != 0)
                blaze::row(nabla_x, i) *= 0;
            if (sum(blaze::row(nabla_y, i)) != 0)
                blaze::row(nabla_y, i) *= 0;
        }
    }

    return std::make_tuple(nabla_x, nabla_y);
}





#endif // _UPS_NABLA_HPP
