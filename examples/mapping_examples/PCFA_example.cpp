
#include <iostream>
#include "modules/mapping.hpp"


int main()
{

    blaze::DynamicVector<float, blaze::rowVector> d0_blaze {0, 1, 2};
    blaze::DynamicVector<float, blaze::rowVector> d1_blaze {0, 1, 3};
    auto d_train = std::vector{d0_blaze, d1_blaze};

    auto pcfa = metric::PCFA<blaze::DynamicVector<float, blaze::rowVector>, void>(d_train, 2);

    blaze::DynamicVector<float, blaze::rowVector> d2_blaze {0, 1, 4};
    blaze::DynamicVector<float, blaze::rowVector> d3_blaze {0, 2, 2};
    auto d_test = std::vector{d0_blaze, d2_blaze, d3_blaze};

    auto d_compressed = pcfa.encode(d_test);

    //std::cout << "compressed:\n" << d_compressed << "\n";

    auto d_restored = pcfa.decode(d_compressed);

    //std::cout << "restored:\n" << d_restored << "\n";

    return 0;
}
