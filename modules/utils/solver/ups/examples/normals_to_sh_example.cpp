
#include "../ups_solver/depth_to_normals.hpp"
#include "../ups_solver/normals_to_sh.hpp"

#include "../../../../../3rdparty/blaze/Blaze.h"

#include <tuple>
#include <iostream>


int main()
{

    blaze::CompressedMatrix<unsigned char, blaze::columnMajor> M {
        {0, 0, 1, 1, 0},
        {0, 1, 1, 1, 1},
        {0, 1, 1, 0, 0},
        {0, 0, 1, 0, 0}
    };

    blaze::DynamicMatrix<float, blaze::columnMajor> Z {
        {0.8, 0.8, 0.8, 0.8, 0.8},
        {0.8, 0.8, 0.8, 0.9, 0.8},
        {0.8, 1.0, 0.8, 0.8, 0.8},
        {0.8, 0.8, 0.8, 0.8, 0.8}};

    blaze::DynamicMatrix<float> K {
        {1, 0, 1},
        {0, 1, 1},
        {0, 0, 1}};


    auto res = depthToNormals(Z, M, K);

    blaze::DynamicMatrix<float> normals (std::get<0>(res));  // unnormalized
    std::cout << "\nnormals:\n" << normals << "\n\n";
    for (size_t i = 0; i<3; ++i) {
        auto c = blaze::column(normals, i);
        c = c * std::get<1>(res);  // elementwize multiply each column by dz column vector
    }  // this is not needed if we already have unnormalized output
    std::cout << "normals multiplied:\n" << normals << "\n\n";
    std::cout << "unnormalized:\n" << std::get<2>(res) << "\n\n";

    blaze::DynamicMatrix<float> sph_harm = normalsToSh(normals);

    std::cout << "sph harmonics:\n" << sph_harm << "\n\n";





    return 0;
}

