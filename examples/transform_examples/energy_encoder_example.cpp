
#include "transform/energy_encoder.cpp"

#include <iostream>




int main() {

    std::vector<double> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    //auto e = metric::energy_encoder(4, 2);
    auto e = metric::energy_encoder();
    auto result = e(data);

    std::cout << "\n\nenergies per subband:\n";
    for (size_t i = 0; i<result.size(); ++i) {
        std::cout << result[i] << " ";
    }
    std::cout << "\n\n";

    return 0;
}
