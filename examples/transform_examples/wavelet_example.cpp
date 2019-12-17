
#include <vector>
#include <iostream>

//#include "../../modules/transform.hpp"
//#include "transform/wavelet.hpp"
#include "transform/wavelet_new.hpp"

int main() {

    std::vector<double> data = {0, 0, 0, 1, 0, 0, 0, 0};

    auto encoded = wavelet::dwt(data, 5);
    auto decoded = wavelet::idwt(std::get<0>(encoded), std::get<1>(encoded), 5, 8);

    for (size_t i = 0; i<decoded.size(); ++i)
        std::cout << decoded[i] << "\n";

    return 0;
}
