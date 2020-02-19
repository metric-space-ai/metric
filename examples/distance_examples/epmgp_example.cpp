
#include <vector>
#include <iostream>
#include <tuple>
#include "../../modules/distance/k-random/epmgp.cpp"



int main() {

    std::vector<double> lower = {-0.5, -0.5};
    std::vector<double> upper = {0.5, 0.5};
    std::vector<double> mu = {0, 0};
    std::vector<double> sigma = {1, 1};

    auto result = epmgp::truncNormMoments(lower, upper, mu, sigma);

    auto logZ = std::get<0>(result);
    auto muOUT = std::get<1>(result);
    auto sigmaOUT = std::get<2>(result);

    std::cout << "\n\nlogZ mu sigma\n";
    for (size_t i = 0; i<logZ.size(); ++i) {
        std::cout << logZ[i] << " " << muOUT[i] << " " << sigmaOUT[i] << "\n";
    }

    blaze::DynamicVector<double> m = {0, 0};
    blaze::DynamicMatrix<double> K = {{1, 0}, {0, 1}};
    blaze::DynamicVector<double> lowerB = {-0.5, -0.5};
    blaze::DynamicVector<double> upperB = {0.5, 0.5};

    auto result2 = epmgp::local_gaussian_axis_aligned_hyperrectangles(m, K, lowerB, upperB);

    auto logZ2 = std::get<0>(result2);
    auto muOUT2 = std::get<1>(result2);
    auto sigmaOUT2 = std::get<2>(result2);

    std::cout << "\n\logZ2 = " << logZ2 << "\n";
    std::cout << "muOUT2 = \n" << muOUT2 << "\n";
    std::cout << "sigmaOUT2 = \n" << sigmaOUT2 << "\n";

    return 0;
}
