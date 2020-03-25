
#include "modules/transform/energy_encoder.cpp"

#include <iostream>




int main() {

    //std::vector<double> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    std::vector<double> data = {0, 1, 2, 3, 4, 5, 6, 7};
    //std::vector<double> data = {0};
//    std::vector<double> data;
//    for (size_t i = 0; i<50000; ++i) {
//        data.push_back((double)i);
//    }

    auto e = metric::energy_encoder(5, 1);
    //auto e = metric::energy_encoder();
    auto result = e(data);

    std::cout << "\n\nenergies per subband:\n";
    for (size_t i = 0; i<result.size(); ++i) {
        std::cout << result[i] << " ";
    }
    std::cout << "\n\n";



    std::cout << "testing subband size computation:\n";
    std::cout << "\nl_split_estim l_split_actual | input_extim_estim input_estim_by_actual | l_split_estim_by_input_estim\n";
    size_t lx = 50000, lf = 10;
    size_t subbands_num = 2;
    std::vector<double> x(lx, 0.0);
    for (size_t i = 1; i < 18; ++i) {
        //std::cout << metric::subband_size(lx, i, lf) << "\n";
        std::stack<size_t> subband_length;
        auto split = metric::sequential_DWT<std::vector, std::vector<double>, std::allocator<std::vector<double>>>(x, subband_length, 5, subbands_num);
        auto lr = split[0].size();

        for (size_t j = 1; j<split.size(); ++j) { // check of size equality of subbands
            if (lr != split[j].size())
                std::cout << "\ninequal split size: " << j << " " << split[j].size()  << "\n";
        }
        auto lr_estim = metric::subband_size(lx, i, lf);
        auto lx_estim = metric::original_size(lr, i, lf);
        std::cout << lr_estim << " " << lr << " | "
                  << metric::original_size(lr_estim, i, lf) << " " << lx_estim << " | "
                  << metric::subband_size(lx_estim, i, lf)
                  << "\n";

        subbands_num *= 2;
    }


    std::cout << "\n\ntesting computation of frequency bounds for 4 splits, 40000 points:\n";

    auto e2 = metric::energy_encoder(5, 4);
    auto bounds = e2.freq_bounds(40000);
    for (size_t i=0; i< bounds.size(); ++i)
        std::cout << bounds[i] << "\n";

    std::cout << "\n\ntesting computation of frequency bounds for 4 splits, 39999 points:\n";

    bounds = e2.freq_bounds(39999);
    for (size_t i=0; i< bounds.size(); ++i)
        std::cout << bounds[i] << "\n";

    std::cout << "\n\n";


    return 0;
}
