
//#include <vector>
#include <iostream>

//#include "../../modules/transform.hpp"
#include "modules/transform/wavelet.hpp"


int main() {

    auto db4 = wavelet::DaubechiesMat<double>(14);

    std::cout << db4 << "\n";

    return 0;
}
