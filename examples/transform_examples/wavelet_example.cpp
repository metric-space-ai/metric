
#include <vector>
#include <iostream>

//#include "../../modules/transform.hpp"
//#include "transform/wavelet.hpp"
#include "modules/transform/wavelet.hpp"

#include "assets/helpers.cpp"
#include "modules/utils/visualizer.hpp"

//#include "transform/helper_functions.cpp" // TODO remove

int main() {

    //std::vector<double> data = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0};
    std::vector<double> data = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    //std::vector<double> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    int wavelet = 4;

    auto encoded = wavelet::dwt(data, wavelet);
    auto decoded = wavelet::idwt(std::get<0>(encoded), std::get<1>(encoded), wavelet, data.size());

    std::cout << "\n1d:\n";
    std::cout << "splitted L:\n";
    auto encoded_l = std::get<0>(encoded);
    for (size_t i = 0; i<encoded_l.size(); ++i)
        std::cout << encoded_l[i] << "\n";
    std::cout << "restored:\n";
    for (size_t i = 0; i<decoded.size(); ++i)
        std::cout << decoded[i] << "\n";

    using Container = std::deque<double>;
    Container zeros = Container(10, 0);
    Container peak = zeros;
    peak[4] = 1;
    std::vector<Container> data2d = {zeros, zeros, zeros, zeros, peak, zeros, zeros, zeros, zeros, zeros, zeros, zeros, zeros, zeros};
    //std::vector<Container> data2d = {zeros, peak}; // TODO add data size control


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


    // cameraman image test

    auto cm = read_csv_num<double>("assets/cameraman.csv", ",");

    auto cm_splitted = wavelet::dwt2(cm, 4);
    auto cm_restored = wavelet::idwt2(cm_splitted, 4, cm.size(), cm[0].size());

    mat2bmp::blaze2bmp(cm_restored, "cm_restored.bmp", 1.0/256.0);
    mat2bmp::blaze2bmp(std::get<0>(cm_splitted), "cm_ll.bmp", 1.0/256.0);
    mat2bmp::blaze2bmp(std::get<1>(cm_splitted), "cm_lh.bmp", 1.0/256.0);
    mat2bmp::blaze2bmp(std::get<2>(cm_splitted), "cm_hl.bmp", 1.0/256.0);
    mat2bmp::blaze2bmp(std::get<3>(cm_splitted), "cm_hh.bmp", 1.0/256.0);

    // showing reference file

    //auto cm_splitted_ref = read_csv_num<double>("assets/cm_ref_8.csv", ",");
    //mat2bmp::blaze2bmp(cm_splitted_ref, "cm_splitted_ref.bmp", 1.0/256.0);


    return 0;
}
