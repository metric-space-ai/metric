

#include "../ups_solver/get_nabla.hpp"

#include "../../../../../3rdparty/blaze/Blaze.h"

#include <iostream>


int main()
{

//    blaze::DynamicMatrix<unsigned char, blaze::columnMajor> m (3, 3, 0);
//    m(0, 1) = 1;
//    m(1, 1) = 1;
//    m(2, 2) = 1;

//    blaze::DynamicMatrix<unsigned char, blaze::columnMajor> m {
//        {0, 1, 0},
//        {0, 1, 0},
//        {0, 0, 1},
//        {0, 1, 1}
//    };

    blaze::DynamicMatrix<unsigned char, blaze::columnMajor> m {
        {0, 0, 1},
        {0, 1, 1},
        {1, 1, 1},
        {0, 0, 1}
    };

    auto res = getNabla<double>(m);

    std::cout << (blaze::DynamicMatrix<unsigned short, blaze::columnMajor>)m << "\n\n";
    std::cout << std::get<0>(res) << "\n\n";
    std::cout << std::get<1>(res) << "\n";

    return 0;
}
