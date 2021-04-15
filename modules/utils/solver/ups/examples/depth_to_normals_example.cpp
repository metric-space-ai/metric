
#include "../ups_solver/depth_to_normals.hpp"

#include "../../../../../3rdparty/blaze/Blaze.h"

#include <iostream>


int main()
{

    blaze::CompressedMatrix<unsigned char, blaze::columnMajor> M {
        {0, 0, 1, 1, 0},
        {0, 1, 1, 1, 1},
        {0, 1, 1, 0, 0},
        {0, 0, 1, 0, 0}
    };

    //blaze::DynamicMatrix <float> Z {
    blaze::DynamicMatrix <float, blaze::columnMajor> Z {
        {0.8, 0.8, 0.8, 0.8, 0.8},
        {0.8, 0.8, 0.8, 0.9, 0.8},
        {0.8, 1.0, 0.8, 0.8, 0.8},
        {0.8, 0.8, 0.8, 0.8, 0.8}};

    blaze::DynamicMatrix <float> K {
        {1, 0, 1},
        {0, 1, 1},
        {0, 0, 1}};

    auto res = depthToNormals(Z, M, K);


    std::cout << "\n\nM:\n" << (blaze::DynamicMatrix<unsigned short, blaze::columnMajor>)M << "\n\n";
    std::cout << "N_normalized:\n" << std::get<0>(res) << "\n\n";
    std::cout << "dz:\n" << std::get<1>(res) << "\n\n";
    std::cout << "N_unnormalized:\n" << std::get<2>(res) << "\n\n";
    std::cout << "zx:\n" << std::get<3>(res) << "\n\n";
    std::cout << "zy:\n" << std::get<4>(res) << "\n\n";
    std::cout << "xx:\n" << std::get<5>(res) << "\n\n";
    std::cout << "yy:\n" << std::get<6>(res) << "\n\n";
    std::cout << "Dx:\n" << std::get<7>(res) << "\n\n";
    std::cout << "Dy:\n" << std::get<8>(res) << "\n\n";

    return 0;
}
