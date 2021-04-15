
#include "../ups_solver/get_normal_map.hpp"
#include "../ups_solver/get_nabla.hpp"
#include "../helpers/indexing.hpp"

#include "../../../../../3rdparty/blaze/Blaze.h"

#include <iostream>
#include <vector>
#include <iostream>


int main()
{

    blaze::CompressedMatrix<unsigned char, blaze::columnMajor> M {
        {0, 0, 1, 1, 0},
        {0, 1, 1, 1, 1},
        {0, 1, 1, 0, 0},
        {0, 0, 1, 0, 0}
    };

    //blaze::DynamicMatrix <float, blaze::columnMajor> Z {
    blaze::DynamicMatrix <float> Z {
        {0.8, 0.8, 0.8, 0.8, 0.8},
        {0.8, 0.8, 0.8, 0.9, 0.8},
        {0.8, 1.0, 0.8, 0.8, 0.8},
        {0.8, 0.8, 0.8, 0.8, 0.8}};

    blaze::DynamicMatrix <float> K {
        {1, 0, 1},
        {0, 1, 1},
        {0, 0, 1}};



    auto nM = getNabla<float>(M, Forward, DirichletHomogeneous);
    blaze::CompressedMatrix<float> Dx = std::get<0>(nM);
    blaze::CompressedMatrix<float> Dy = std::get<1>(nM);


    std::vector<size_t> z_idc = indicesCwStd(M);
    blaze::DynamicVector<float> z_vector = flattenToCol(Z);
    blaze::DynamicVector<float> z_vector_masked = blaze::elements(z_vector, z_idc);  // Matlab z = z(mask);

    blaze::DynamicVector<float> zx = Dx * z_vector_masked;  // matrix multiplication
    blaze::DynamicVector<float> zy = Dy * z_vector_masked;


    auto xxyy = indices2dCw(M);
    blaze::DynamicVector<float> xx = std::get<0>(xxyy) - K(0, 2);
    blaze::DynamicVector<float> yy = std::get<1>(xxyy) - K(1, 2);


    auto res = getNormalMap(z_vector_masked, zx, zy, K, xx, yy);

    std::cout << "\n\nM:\n" << (blaze::DynamicMatrix<unsigned short, blaze::columnMajor>)M << "\n\n";
    std::cout << "N_normalized:\n" << std::get<0>(res) << "\n\n";
    std::cout << "dz:\n" << std::get<1>(res) << "\n\n";
    std::cout << "N_unnormalized:\n" << std::get<2>(res) << "\n\n";
    std::cout << "zx:\n" << zx << "\n\n";
    std::cout << "zy:\n" << zy << "\n\n";
    std::cout << "xx:\n" << xx << "\n\n";
    std::cout << "yy:\n" << yy << "\n\n";
    std::cout << "Dx:\n" << Dx << "\n\n";
    std::cout << "Dy:\n" << Dy << "\n\n";


    return 0;
}
