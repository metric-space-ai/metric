
//#include <vector>
#include <iostream>

//#include "../../modules/transform.hpp"
#include "modules/transform/wavelet.hpp"


int main() {

    blaze::DynamicVector<double> v {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    //blaze::DynamicVector<double> v {0, 0.2, 0.4, 0.6, 0.8, 1};

    auto db4 = wavelet::DaubechiesMat<double>(14);
    std::cout << db4 << "\n";

    blaze::DynamicVector<double> encoded = db4*v; // dont use auto here, it will lead to matrix, whereas we need column vector
    std::cout << encoded << "\n";
    auto decoded = db4.transpose()*encoded;
    std::cout << decoded << "\n";

    return 0;
}
