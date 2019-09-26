
#include <iostream>
#include "modules/mapping.hpp"


int main()
{

//    using recType = blaze::DynamicVector<float, blaze::rowVector>;
    using recType = std::vector<float>;


    recType d0_blaze {0, 1, 2};
    recType d1_blaze {0, 1, 3};
    auto d_train = std::vector{d0_blaze, d1_blaze};

    auto pcfa = metric::PCFA<recType, void>(d_train, 2);

    recType d2_blaze {0, 1, 4};
    recType d3_blaze {0, 2, 2};
    auto d_test = std::vector{d0_blaze, d2_blaze, d3_blaze};

    auto d_compressed = pcfa.encode(d_test);

    std::cout << "compressed:\n"; // << d_compressed << "\n";
    for (size_t i = 0; i<d_compressed.size(); i++) {
        for (size_t j = 0; j<d_compressed[i].size(); j++) {
            std::cout << d_compressed[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    auto d_restored = pcfa.decode(d_compressed);

    std::cout << "restored:\n";
    for (size_t i = 0; i<d_restored.size(); i++) {
        for (size_t j = 0; j<d_restored[i].size(); j++) {
            std::cout << d_restored[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    auto d_eigenmodes = pcfa.eigenmodes();

    std::cout << "eigenmodes:\n";
    for (size_t i = 0; i<d_eigenmodes.size(); i++) {
        for (size_t j = 0; j<d_eigenmodes[i].size(); j++) {
            std::cout << d_eigenmodes[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    return 0;
}
