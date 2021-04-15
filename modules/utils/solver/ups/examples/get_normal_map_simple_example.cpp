
#include "../ups_solver/get_normal_map.hpp"
//#include "../ups_solver/get_nabla.hpp"
//#include "../helpers/indexing.hpp"

#include "../../../../../3rdparty/blaze/Blaze.h"

#include <iostream>
#include <vector>


int main()
{

    blaze::DynamicMatrix <float> K {
        {1, 0, 1},
        {0, 1, 1},
        {0, 0, 1}};


    blaze::DynamicVector<float> z = {
             1.55474,
            0.705676,
             4.59231,
              3.4975,
           -0.463838,
             1.00742,
            0.414638,
             1.16975,
             1.34172
    };

    blaze::DynamicVector<float> zx = {
                   0,
                -0.2,
                   0,
                 0.1,
                -0.8,
                -0.8,
                -0.8,
                -0.1,
                -0.8
    };

    blaze::DynamicVector<float> zy = {
                 0.2,
                  -1,
                   0,
                   0,
                   0,
                -0.8,
                 0.1,
                -0.9,
                -0.8
    };

    blaze::DynamicVector<float> xx = {
                   0,
                   0,
                   1,
                   1,
                   1,
                   1,
                   2,
                   2,
                   3
    };

    blaze::DynamicVector<float> yy = {
                   0,
                   1,
                  -1,
                   0,
                   1,
                   2,
                  -1,
                   0,
                   0
    };




    auto res = getNormalMap(z, zx, zy, K, xx, yy);


    std::cout << "N_normalized:\n" << std::get<0>(res) << "\n\n";
    std::cout << "dz:\n" << std::get<1>(res) << "\n\n";
    std::cout << "N_unnormalized:\n" << std::get<2>(res) << "\n\n";
    //std::cout << "zx:\n" << zx << "\n\n";
    //std::cout << "zy:\n" << zy << "\n\n";
    //std::cout << "xx:\n" << xx << "\n\n";
    //std::cout << "yy:\n" << yy << "\n\n";


    return 0;
}
