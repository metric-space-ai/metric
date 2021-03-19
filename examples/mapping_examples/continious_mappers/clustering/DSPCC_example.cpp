

#include "../../../../modules/mapping/DSPCC.hpp"
#include "../../../../modules/utils/metric_err.hpp"
#include "../../../../modules/distance/k-related/Standards.hpp" // we use Euclidean metric for mean squared error evaluation

#include <iostream>




template <typename Container>
void print_table(Container table) {
    for (size_t rec_idx = 0; rec_idx<table.size(); ++rec_idx) {
        for (size_t el_idx = 0; el_idx<table[0].size(); ++el_idx)
            std::cout << table[rec_idx][el_idx] << " ";
        std::cout << "\n";
    }
}




template <
 template <typename, typename> class OuterContainer,
 typename OuterAllocator,
 template <typename, typename> class InnerContainer,
 typename InnerAllocator,
 typename ValueType >
double mean_square_error(
        const OuterContainer<InnerContainer<ValueType, InnerAllocator>, OuterAllocator> & M1,
        const OuterContainer<InnerContainer<ValueType, InnerAllocator>, OuterAllocator> & M2
        )
{
    double overall_sum = 0;
    double row_sum;
    size_t row, col;
    for (row = 0; row < M1.size(); row++)  // we assume all inner vectors both in M1 and M2 are of the same langth
    {
        row_sum = 0;
        for (col = 0; col < M1[0].size(); col++)  // we assume M1 and M2 are of the same length too
            row_sum += pow(M1[row][col] - M2[row][col], 2);
        overall_sum += sqrt(row_sum / M1[0].size());
    }
    return overall_sum / M1.size();
}



void print_stats(std::tuple<double, double, double, double, double, double> stats) {
    std::cout << " average norm of original waveforms : " << std::get<0>(stats) << "\n";
    std::cout << "      original waveform norm stddev : " << std::get<1>(stats) << "\n";
    std::cout << "             average absolute error : " << std::get<2>(stats) << "\n";
    std::cout << "           stddev of absolute error : " << std::get<3>(stats) << "\n";
    std::cout << "           average normalized error : " << std::get<4>(stats) << "\n";
    std::cout << "         stddev of normalized error : " << std::get<5>(stats) << "\n";
    std::cout << "\n";
}







int main()
{

    using RecType = std::deque<double>;

    RecType d0 {0, 1, 2, 3, 4, 5, 6, 100, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
    RecType d1 {0, 1, 2, 3, 4, 5, 6, 7,   8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 100};
    std::vector<RecType> d = {d0, d1};

    float freq_time_balance = 0.5; // try values from 0 to 1 (e g 0, 0.5, 1) to get the following portions of freq-domain: 0, 4/9, 8/9
    auto bundle = metric::DSPCC<RecType, void>(d, 4, 2, freq_time_balance, 4);

    auto pre_encoded = bundle.test_public_wrapper_encode(d);
    auto pre_decoded = bundle.test_public_wrapper_decode(pre_encoded);

    auto encoded = bundle.encode(d);
    auto decoded = bundle.decode(encoded);

    std::cout << "\noriginal:\n";
    print_table(d);

    std::cout << "\npre-decoded:\n";
    print_table(pre_decoded);

    std::cout << "\ndecoded:\n";
    print_table(decoded);

    std::cout << "\nsimple test done\n";
    auto err_full_1 = normalized_err_stats<metric::Euclidean<double>>(d, decoded);
    print_stats(err_full_1);
    std::cout << "average RMSE = " << mean_square_error(d, decoded) << "\n";



    // test Blaze vector input

    //using RecTypeBlaze = blaze::DynamicVector<double, blaze::columnVector>; // also works
    using RecTypeBlaze = blaze::DynamicVector<double, blaze::rowVector>;

    RecTypeBlaze dBlaze1 {0, 1, 2, 3, 4, 5, 6, 100, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
    RecTypeBlaze dBlaze2 {0, 1, 2, 3, 4, 5, 6, 7,   8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 100};
    std::vector<RecTypeBlaze> dBlaze {dBlaze1, dBlaze2};
    auto bundleBlaze = metric::DSPCC<RecTypeBlaze, void>(dBlaze, 3, 2, 0.5, 3);
    auto encodedBlaze = bundleBlaze.encode(dBlaze);
    auto decodedBlaze = bundleBlaze.decode(encodedBlaze);

    std::cout << "decoded Blaze vector:\n";

    for (size_t i=0; i<decodedBlaze.size(); ++i) {
        std::cout << decodedBlaze[i];
    }



    return 0;

}
