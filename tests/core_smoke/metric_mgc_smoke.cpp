#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <deque>
#include <iomanip>
#include <iostream>
#include <vector>

#include "metric/correlation.hpp"
#include "metric/distance.hpp"

int main()
{
    std::vector<std::vector<double>> first = {
        {-1.08661677587398}, {-1.00699896410939}, {-0.814135753976830}, {-0.875364720432552},
        {-0.659607023272462}, {-0.798949992922930}, {-0.431585448024267}, {-0.619123703544758},
        {-0.351492263653510}, {-0.394814371972061}, {-0.309693618374598}, {-0.352009525808777},
        {-0.0803413535982411}, {0.0103940699342647}, {-0.130735385695596}, {-0.138214899507693},
        {0.0279270082022143}, {0.141670765995995}, {0.112221224566625}, {0.376767573021755},
        {0.186729429735154}, {0.597349318463320}, {0.451380104139401}, {0.639237742050564},
        {0.797420868050314}, {0.690091614630087}, {0.921722674141222}, {0.852593762434809},
        {0.954771723842945}, {1.03297970279357}};

    std::deque<std::array<float, 1>> second = {
        {2.70625143351230F}, {1.41259513494005F}, {0.666086793692617F}, {0.647856446084279F},
        {0.887764969338737F}, {0.286220905202707F}, {0.543682026943014F}, {0.0402339224257120F},
        {0.105812168910424F}, {0.0230915137205610F}, {0.00298976085950325F}, {0.00366997150982423F},
        {0.000384825484363474F}, {7.27293780465119e-05F}, {2.50809340229209e-07F},
        {0.00306636655437742F}, {0.000456283181338950F}, {0.00801756105329616F},
        {1.17238339150888e-09F}, {0.0803830108071682F}, {0.0774478107095828F},
        {0.0474847202878941F}, {0.0818772460512609F}, {0.486406609209630F},
        {0.197547677770060F}, {0.628321368933714F}, {1.02400551043736F}, {0.552591658802459F},
        {1.52144482984914F}, {3.43908991254968F}};

    using FirstRecord = std::vector<double>;
    using SecondRecord = std::array<float, 1>;

    metric::MGC<FirstRecord, metric::Euclidean<double>, SecondRecord, metric::Manhattan<float>> mgc;

    const double result = mgc(first, second);
    const double expected = 0.28845660296530595;

    std::cout << "MGC = " << result << "\n";
    assert(std::abs(result - expected) < 1e-12);

    metric::MGC<SecondRecord, metric::Manhattan<float>, FirstRecord, metric::Euclidean<double>> reversed_mgc;
    const double reversed_result = reversed_mgc(second, first);
    assert(std::abs(reversed_result - expected) < 1e-12);

    const std::vector<double> expected_xcorr = {
        0.3406052387919164,
        0.36670317239506234,
        0.35527060120466675,
        0.3899053662191226,
        0.2884565911772877,
        0.37837904236096026,
        0.4589100366738734,
        0.3811881240213428,
        0.44657481646290537};
    const auto xcorr = mgc.xcorr(first, second, 4);

    constexpr double xcorr_tolerance = 1e-7;

    assert(xcorr.size() == expected_xcorr.size());
    for (std::size_t index = 0; index < xcorr.size(); ++index) {
        if (std::abs(xcorr[index] - expected_xcorr[index]) >= xcorr_tolerance) {
            std::cerr << std::setprecision(17)
                      << "xcorr[" << index << "] = " << xcorr[index]
                      << ", expected " << expected_xcorr[index] << "\n";
        }
        assert(std::abs(xcorr[index] - expected_xcorr[index]) < xcorr_tolerance);
    }

    return 0;
}
