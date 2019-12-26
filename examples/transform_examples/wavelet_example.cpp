
#include <vector>
#include <iostream>

//#include "../../modules/transform.hpp"
//#include "transform/wavelet.hpp"
#include "transform/wavelet_new.hpp"

int main() {

    std::vector<double> data = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0};
    //std::vector<double> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    auto encoded = wavelet::dwt(data, 4);
    auto decoded = wavelet::idwt(std::get<0>(encoded), std::get<1>(encoded), 4, 8);

    for (size_t i = 0; i<decoded.size(); ++i)
        std::cout << decoded[i] << "\n";

    using Container = std::deque<double>;
    Container zeros = Container(10, 0);
    Container peak = zeros;
    peak[4] = 1;
    std::vector<Container> data2d = {zeros, zeros, zeros, zeros, peak, zeros, zeros}; //, zeros, zeros, zeros};

    std::cout << "\n";
    for (size_t i = 0; i<data2d.size(); ++i) {
        for (size_t j = 0; j<data2d[0].size(); ++j)
            std::cout << data2d[i][j] << " ";
        std::cout << "\n";
    }

    auto dec = wavelet::dwt2(data2d, 2);

    std::cout << "\n";
    for (size_t i = 0; i<std::get<0>(dec).size(); ++i) {
        for (size_t j = 0; j<std::get<0>(dec)[0].size(); ++j)
            std::cout << std::get<0>(dec)[i][j] << " ";
        std::cout << "\n";
    }

    auto rec = wavelet::idwt2(std::get<0>(dec), std::get<1>(dec), std::get<2>(dec), std::get<3>(dec), 2, data2d.size(), data2d[0].size());

    std::cout << "\n";
    for (size_t i = 0; i<rec.size(); ++i) {
        for (size_t j = 0; j<rec[0].size(); ++j)
            std::cout << rec[i][j] << " ";
        std::cout << "\n";
    }

    return 0;
}
