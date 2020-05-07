
#include "modules/transform/wavelet.hpp"

//#include "3rdparty/blaze/Blaze.h"

#include <vector>
#include <iostream>


template<typename T>
std::ostream& operator<<(std::ostream& out, std::deque<T> const& a) {
    //out << "[ ";
    for(auto x : a) {
        out << x << "\n";
    }
    //out << "]";
    return out;
}


int main() {

    {
        //using T = blaze::DynamicVector<double>;
        using T = std::deque<double>;
        T a {0, 0, 0, 0, 1, 0, 0, 0, 0};
        T b {0, 1, 1, 1, 0};
        auto result = wavelet::conv_valid(a, b);
        std::cout << result << "\n";
        std::cout << wavelet::conv(a, b) << "\n";
        auto coeffs = wavelet::dbwavf<T>(4, double(1));
        std::cout << "\n" << coeffs << "\n";
        auto filters = wavelet::orthfilt(coeffs);
        std::cout << "\nfilters:\n" << std::get<0>(filters) << "\n" << std::get<1>(filters) << "\n" << std::get<2>(filters) << "\n" << std::get<3>(filters) << "\n" ;
        auto filtered = wavelet::dwt(a, 4);
        std::cout << "\nfiltered:\n" << std::get<0>(filtered) << "\n" << std::get<1>(filtered) << "\n";
        auto restored = wavelet::idwt(std::get<0>(filtered), std::get<1>(filtered), 4, a.size());
        std::cout << "\nrestored:\n" << restored << "\n";

    }
    //return 0;

    // old test

    using Container = std::deque<double>;

    int wavelet = 4;

    Container zeros = Container(10, 0);
    Container peak = zeros;
    peak[4] = 1;
    std::vector<Container> data2d = {zeros, zeros, zeros, zeros, peak, zeros, zeros, zeros, zeros, zeros, zeros, zeros, zeros, zeros};


    std::cout << "\n2d:\n";
    std::cout << "input:\n";
    for (size_t i = 0; i<data2d.size(); ++i) {
        for (size_t j = 0; j<data2d[0].size(); ++j)
            std::cout << data2d[i][j] << " ";
        std::cout << "\n";
    }

    auto splitted = wavelet::dwt2(data2d, wavelet);

    std::cout << "slpitted LL:\n";
    for (size_t i = 0; i<std::get<0>(splitted).size(); ++i) {
        for (size_t j = 0; j<std::get<0>(splitted)[0].size(); ++j)
            std::cout << std::get<0>(splitted)[i][j] << " ";
        std::cout << "\n";
    }

    auto restored = wavelet::idwt2(std::get<0>(splitted), std::get<1>(splitted), std::get<2>(splitted), std::get<3>(splitted), wavelet, data2d.size(), data2d[0].size());

    std::cout << "restored:\n";
    for (size_t i = 0; i<restored.size(); ++i) {
        for (size_t j = 0; j<restored[0].size(); ++j)
            std::cout << restored[i][j] << " ";
        std::cout << "\n";
    }



    return 0;
}
