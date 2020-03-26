
#include <vector>
#include <iostream>
#include <chrono>
#include <random>

//#include "transform/wavelet.hpp"
#include "modules/transform/energy_encoder.cpp"


template <typename T>
std::vector<T>
extract_energies_static(std::vector<T> x)
{
    auto [x0, tmp1] = wavelet::dwt(x, 5);
    auto [x1, tmp2] = wavelet::dwt(x0, 5);
    auto [x11, tmp3] = wavelet::dwt(x1, 5);
    auto [x111, x112] = wavelet::dwt(x11, 5);
    auto [x1111, x1112] = wavelet::dwt(x111, 5);
    auto [x1121, x1122] = wavelet::dwt(x112, 5);
    auto [x11111, x11112] = wavelet::dwt(x1111, 5);
    auto [x11121, x11122] = wavelet::dwt(x1112, 5);
    auto [x11211, x11212] = wavelet::dwt(x1121, 5);
    auto [x11221, x11222] = wavelet::dwt(x1122, 5);
    std::vector<std::vector<T>> subbands = {x11111, x11112, x11121, x11122, x11211, x11212, x11221, x11222};

    std::vector<T> energies(subbands.size());
    for (int i = 0; i < subbands.size(); ++i)
    {
        T sum = 0;
        for (int j = 0; j < subbands[i].size(); ++j)
        {
            sum += subbands[i][j] * subbands[i][j];
        }
        energies[i] = sum / T(subbands[0].size());
    }
    //std::cout << subbands[0].size() << std::endl;
    return energies;
}



template <typename T>
std::vector<T>
extract_energies_static_3(std::vector<T> x)
{
    auto [x111, x112] = wavelet::dwt(x, 5);
    auto [x1111, x1112] = wavelet::dwt(x111, 5);
    auto [x1121, x1122] = wavelet::dwt(x112, 5);
    auto [x11111, x11112] = wavelet::dwt(x1111, 5);
    auto [x11121, x11122] = wavelet::dwt(x1112, 5);
    auto [x11211, x11212] = wavelet::dwt(x1121, 5);
    auto [x11221, x11222] = wavelet::dwt(x1122, 5);
    std::vector<std::vector<T>> subbands = {x11111, x11112, x11121, x11122, x11211, x11212, x11221, x11222};

    std::vector<T> energies(subbands.size());
    for (int i = 0; i < subbands.size(); ++i)
    {
        T sum = 0;
        for (int j = 0; j < subbands[i].size(); ++j)
        {
            sum += subbands[i][j] * subbands[i][j];
        }
        energies[i] = sum / T(subbands[0].size());
    }
    //std::cout << subbands[0].size() << std::endl;
    return energies;
}





int main() {

    size_t runs = 1000; // set number of runs
    size_t len  = 48000; // set length of test random vector

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<> dist(-1, 1);

    std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> t1, t2;

    double sum;

    std::vector<double> x, result, result2;

    for (size_t i = 0; i<len; ++i)
        x.push_back(dist(rng));


    std::cout << "running energy extractors " << runs << " times on vector of length " << len << "\n";

    auto e = metric::energy_encoder(5, 3);

    sum = 0;
    for (size_t i = 0; i<runs; ++i) {
        t1 = std::chrono::steady_clock::now();
        result = e(x);
        t2 = std::chrono::steady_clock::now();
        sum += double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
    }


    std::cout << "\n\n\ntesting functor:\nenergies per subband:\n";
    for (size_t i = 0; i<result.size(); ++i) {
        std::cout << result[i] << " ";
    }

    std::cout << "\n" << result.size() << " energy values\n\n";

    std::cout //<< "\n\nresult1: " //<< result1
              << "average time: " << sum/runs
              << " s" << std::endl;
    std::cout << "" << std::endl;



    sum = 0;
    for (size_t i = 0; i<runs; ++i) {
        t1 = std::chrono::steady_clock::now();
        result2 = extract_energies_static_3(x);
        t2 = std::chrono::steady_clock::now();
        sum += double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
    }

    std::cout << "\n\n\ntesting static ref function:\n\nenergies per subband:\n";
    for (size_t i = 0; i<result2.size(); ++i) {
        std::cout << result2[i] << " ";
    }

    std::cout << "\n" << result2.size() << " energy values\n\n";

    std::cout //<< "\n\nresult1: " //<< result1
              << "average time: " << sum/runs
              << " s" << std::endl;
    std::cout << "" << std::endl;




    return 0;
}
