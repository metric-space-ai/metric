
#include "../ups_solver/normals_to_sh.hpp"

#include "../../../../../3rdparty/blaze/Blaze.h"

#include <tuple>
#include <iostream>


int main() {

    blaze::DynamicMatrix<float> normals {
        {            0,     0.242536,      -1.8854 },
        {    -0.196116,    -0.980581,     0.288608 },
        {            0,            0,     -5.74038 },
        {     0.110431,            0,     -3.97278 },
        {           -1,            0,       1.5798 },
        {    -0.408248,    -0.408248,     0.710648 },
        {    -0.662085,    0.0827606,      1.06377 },
        {   -0.0873704,    -0.786334,    -0.847277 },
        {    -0.408248,    -0.408248,     0.540049 }
    };

    blaze::DynamicMatrix<float> sph_harm = normalsToSh(normals);

    std::cout << "sph harmonics:\n" << sph_harm << "\n\n";

    return 0;
}
