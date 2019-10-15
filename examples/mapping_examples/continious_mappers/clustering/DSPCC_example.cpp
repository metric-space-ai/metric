
#include <iostream>

//#include "../../../../modules/mapping.hpp"
#include "../../../../modules/mapping/DSPCC.hpp"

template <typename Container>
void print_table(Container table) {
    for (size_t rec_idx = 0; rec_idx<table.size(); ++rec_idx) {
        for (size_t el_idx = 0; el_idx<table[0].size(); ++el_idx)
            std::cout << table[rec_idx][el_idx] << " ";
        std::cout << "\n";
    }
}


int main()
{
    // temporary

    //double arr[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

//    double * arr = new double[16];
//    for (size_t v = 0; v < 16; v++)
//        arr[v] = (double)v;

//    FastDctLee_inverseTransform(arr, 16);

//    free(arr);

    //return 0;


    // trying to reproduce mem corruption

//    std::vector<double> dt0 {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
//    std::deque<std::vector<double>> subbands = wavelet::wavedec<double>(dt0, 8, 5);
//    //std::deque<std::vector<double>> subbands2(subbands);

    //metric::apply_DCT_STL(subbands, false, 16);

//    double * sample = new double[16];

//    size_t idx = 0;
//    for (auto it = dt0.begin(); it != dt0.begin()+16; ++it) {
//        sample[idx++] = *it;
//    }

//    FastDctLee_inverseTransform(sample, 16);

//    idx = 0;
//    for (auto it = dt0.begin(); it != dt0.begin()+16; ++it) {
//        *it = sample[idx++];
//    }

//    delete[] sample;



//    return 0;


    // main

    using recType = std::vector<double>;

    recType d0 {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
    recType d1 {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 100};
    std::vector<recType> d = {d0, d1};

    float freq_time_balance = 1; // try values from 0 to 1 (e g 0, 0.5, 1) to get the following portions of freq-domain: 0, 4/9, 8/9
    auto bundle = metric::DSPCC<recType, void>(d, 2, freq_time_balance, 0.5);

    //auto pre_encoded = bundle.test_public_wrapper_encode(d);
    //auto pre_decoded = bundle.test_public_wrapper_decode(pre_encoded);

    auto encoded = bundle.encode(d);
    auto decoded = bundle.decode(encoded);

    std::cout << "\noriginal:\n";
    print_table(d);

    std::cout << "\ndecoded:\n";
    print_table(decoded); // some normalization issue when using DCT persists..

    std::cout << "\nmix_index: " << bundle.get_mix_idx() << "\n";

    std::cout << "\ndone\n";

    return 0;
}
