#ifndef _UPS_DEPTH_TO_NORMALS_HPP
#define _UPS_DEPTH_TO_NORMALS_HPP

#include "modules/utils/solver/ups/helpers/indexing.hpp"
#include "modules/utils/solver/ups/ups_solver/get_normal_map.hpp"
#include "modules/utils/solver/ups/ups_solver/get_nabla.hpp"

#include "3rdparty/blaze/Blaze.h"

//#include <iostream>  // TODO remove



/*

template <typename T, bool B>
std::tuple<
           blaze::DynamicVector<T>,  // z_vector_masked
           blaze::DynamicVector<T>,  // zx
           blaze::DynamicVector<T>,  // zy
           blaze::DynamicVector<T>,  // xx
           blaze::DynamicVector<T>,  // yy
           blaze::CompressedMatrix<T>,  // Dx
           blaze::CompressedMatrix<T>   // Dy
           //blaze::DynamicMatrix<T>,  // Dx
           //blaze::DynamicMatrix<T>   // Dy
          >
getMaskedGradients(
                 const blaze::DynamicMatrix<T, B> & Z,
                 //const blaze::CompressedMatrix<bool, blaze::columnMajor> & M,
                 const blaze::DynamicMatrix<bool, blaze::columnMajor> & M,
                 const blaze::DynamicMatrix<T> & K
                )
{

    auto nM = getNabla<T>(M, Forward, DirichletHomogeneous);
    blaze::CompressedMatrix<T> Dx = std::get<0>(nM);
    blaze::CompressedMatrix<T> Dy = std::get<1>(nM);
    //blaze::DynamicMatrix<T> Dx = std::get<0>(nM);
    //blaze::DynamicMatrix<T> Dy = std::get<1>(nM);

    std::vector<size_t> z_idc = indicesCwStd(M);
    blaze::DynamicVector<T> z_vector = flattenToCol(Z);
    blaze::DynamicVector<T> z_vector_masked = blaze::elements(z_vector, z_idc);  // Matlab z = z(mask);

    blaze::DynamicVector<T> zx = Dx * z_vector_masked;  // matrix multiplication
    blaze::DynamicVector<T> zy = Dy * z_vector_masked;


    auto xxyy = indices2dCw(M);
    blaze::DynamicVector<T> xx = std::get<0>(xxyy) - K(0, 2);
    blaze::DynamicVector<T> yy = std::get<1>(xxyy) - K(1, 2);

    return std::make_tuple(
                z_vector_masked,
                zx, zy,
                xx, yy,
                Dx, Dy);
}

// */


/*
template <typename T, bool B>
std::tuple<
           blaze::DynamicMatrix<T>,  // N_normalized
           blaze::DynamicVector<T>,  // dz
           blaze::DynamicMatrix<T>,  // N_unnormalized
           blaze::DynamicVector<T>,  // zx
           blaze::DynamicVector<T>,  // zy
           blaze::DynamicVector<T>,  // xx
           blaze::DynamicVector<T>,  // yy
           blaze::CompressedMatrix<T>,  // Dx
           blaze::CompressedMatrix<T>   // Dy
          >
depthToNormals(
                 blaze::DynamicMatrix<T, B> Z,
                 blaze::CompressedMatrix<bool, blaze::columnMajor> M,
                 blaze::DynamicMatrix<T> K
                )
{

    auto nM = getNabla<T>(M, Forward, DirichletHomogeneous);
    blaze::CompressedMatrix<T> Dx = std::get<0>(nM);
    blaze::CompressedMatrix<T> Dy = std::get<1>(nM);


    std::vector<size_t> z_idc = indices_cw_std(M);
    blaze::DynamicVector<T> z_vector = flatten_to_col(Z);
    blaze::DynamicVector<T> z_vector_masked = blaze::elements(z_vector, z_idc);  // Matlab z = z(mask);

    blaze::DynamicVector<T> zx = Dx * z_vector_masked;  // matrix multiplication
    blaze::DynamicVector<T> zy = Dy * z_vector_masked;


    auto xxyy = indices2d_cw(M);
    blaze::DynamicVector<T> xx = std::get<0>(xxyy) - K(0, 2);
    blaze::DynamicVector<T> yy = std::get<1>(xxyy) - K(1, 2);


    auto res = getNormalMap(z_vector_masked, zx, zy, K, xx, yy);

    return std::make_tuple(
                std::get<0>(res), std::get<1>(res), std::get<2>(res),
                zx, zy,
                xx, yy,
                Dx, Dy);
}
// */


/*  // last ver, but never called
template <typename T, bool B>
std::tuple<
           blaze::DynamicMatrix<T>,  // N_normalized
           blaze::DynamicVector<T>,  // dz
           blaze::DynamicMatrix<T>,  // N_unnormalized
           blaze::DynamicVector<T>,  // zx
           blaze::DynamicVector<T>,  // zy
           blaze::DynamicVector<T>,  // xx
           blaze::DynamicVector<T>,  // yy
           //blaze::CompressedMatrix<T>,  // Dx
           //blaze::CompressedMatrix<T>   // Dy
           blaze::DynamicMatrix<T>,  // Dx
           blaze::DynamicMatrix<T>   // Dy
          >
depthToNormals(
                 const blaze::DynamicMatrix<T, B> & Z,
                 //const blaze::CompressedMatrix<bool, blaze::columnMajor> & M,
                 const blaze::DynamicMatrix<bool, blaze::columnMajor> & M,
                 const blaze::DynamicMatrix<T> & K
                )
{

    auto gradients = getMaskedGradients(Z, M, K);
    blaze::DynamicVector<T> z_vector_masked = std::get<0>(gradients);  // Matlab z = z(mask);
    blaze::DynamicVector<T> zx = std::get<1>(gradients);
    blaze::DynamicVector<T> zy = std::get<2>(gradients);
    blaze::DynamicVector<T> xx = std::get<3>(gradients);
    blaze::DynamicVector<T> yy = std::get<4>(gradients);
    //blaze::CompressedMatrix<T> Dx = std::get<5>(gradients);
    //blaze::CompressedMatrix<T> Dy = std::get<6>(gradients);
    blaze::DynamicMatrix<T> Dx = std::get<5>(gradients);
    blaze::DynamicMatrix<T> Dy = std::get<6>(gradients);


    auto res = getNormalMap(z_vector_masked, zx, zy, K, xx, yy);

    return std::make_tuple(
                std::get<0>(res), std::get<1>(res), std::get<2>(res),
                zx, zy,
                xx, yy,
                Dx, Dy);
}
// */


template <typename T>
std::tuple<std::vector<blaze::CompressedMatrix<T>>, blaze::CompressedMatrix<T>>
//std::tuple<std::vector<blaze::DynamicMatrix<T>>, blaze::DynamicMatrix<T>>
calcJacobian(
        const blaze::DynamicMatrix<T> & normals,
        const blaze::DynamicVector<T> & z_vector_masked,
        const blaze::DynamicMatrix<T> & K,
        const blaze::DynamicVector<T> & xx,
        const blaze::DynamicVector<T> & yy,
        const blaze::CompressedMatrix<T> & Dx,
        const blaze::CompressedMatrix<T> & Dy
        //const blaze::DynamicMatrix<T> & Dx,
        //const blaze::DynamicMatrix<T> & Dy
        )
{
    size_t npix = z_vector_masked.size();
    auto J_n_un_1 = K(0, 0) * Dx;
    auto J_n_un_2 = K(1, 1) * Dy;
    auto eye = blaze::IdentityMatrix<T, blaze::columnMajor>(npix);

    blaze::CompressedMatrix<T> xx_diag (xx.size(), xx.size());
    blaze::CompressedMatrix<T> yy_diag (xx.size(), xx.size());
    //blaze::DynamicMatrix<T> xx_diag (xx.size(), xx.size(), 0);
    //blaze::DynamicMatrix<T> yy_diag (xx.size(), xx.size(), 0);

    {
        auto diag = blaze::diagonal(xx_diag);
        diag = xx;
    }
    {
        auto diag = blaze::diagonal(yy_diag);
        diag = yy;
    }
    auto J_n_un_3 = -eye - xx_diag*Dx - yy_diag*Dy;

    //std::cout << std::endl << "xx:" << std::endl << xx << std::endl;
    //std::cout << std::endl << "yy:" << std::endl << yy << std::endl;
    //std::cout << std::endl << "eye:" << std::endl << eye << std::endl;
    //std::cout << std::endl << "term1:" << std::endl << term1 << std::endl;
    //std::cout << std::endl << "term2:" << std::endl << term2 << std::endl;
    //std::cout << std::endl << "J_n_un_3:" << std::endl << J_n_un_3 << std::endl;

    blaze::CompressedMatrix<T> x_scale (npix, npix);
    blaze::CompressedMatrix<T> y_scale (npix, npix);
    blaze::CompressedMatrix<T> z_scale (npix, npix);
    //blaze::DynamicMatrix<T> x_scale (npix, npix, 0);
    //blaze::DynamicMatrix<T> y_scale (npix, npix, 0);
    //blaze::DynamicMatrix<T> z_scale (npix, npix, 0);

    {
        auto diag = blaze::diagonal(x_scale);
        diag = blaze::column(normals, 0);
    }
    {
        auto diag = blaze::diagonal(y_scale);
        diag = blaze::column(normals, 1);
    }
    {
        auto diag = blaze::diagonal(z_scale);
        diag = blaze::column(normals, 2);
    }

    blaze::CompressedMatrix<T> J_dz =
    //blaze::DynamicMatrix<T> J_dz =
            x_scale * J_n_un_1 +
            y_scale * J_n_un_2 +
            z_scale * J_n_un_3;

    std::vector<blaze::CompressedMatrix<T>> J_n_un (3);
    //std::vector<blaze::DynamicMatrix<T>> J_n_un (3);
    J_n_un[0] = J_n_un_1;
    J_n_un[1] = J_n_un_2;
    J_n_un[2] = J_n_un_3;

    return std::make_tuple(J_n_un, J_dz);
}



#endif // _UPS_DEPTH_TO_NORMALS_HPP
