
//#include "../../modules/utils/solver/solver.hpp"
#include "../../helper/lapwrappers.hpp"
#include "../../pcg.hpp"

#include <iostream>



int main() {

    std::cout << "started" << std::endl;

    blaze::DynamicMatrix<float> A = {{1, 0, 0.5}, {0, 1, 0}, {0.5, 0, 1}};
    blaze::DynamicVector<float> b = {1, 1, 1};
    blaze::DynamicMatrix<float> Pre = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    std::vector<size_t> pcgIts = {0};
    //std::vector<size_t> pcgIts = {};

    //auto x = metric::pcg<float>(A, b, Pre, pcgIts, 1e-6, HUGE_VAL, 100000);
    //auto x = metric::pcg<float>(A, b, Pre, pcgIts, 0.001);
    auto x = metric::pcg<float>(A, b, Pre, pcgIts);

    std::cout << x << std::endl;
    for (size_t i = 0; i<pcgIts.size(); ++i) {
        std::cout << pcgIts[i] << " ";
    }

    std::cout << std::endl <<"done" << std::endl;



    return 0;
}
