
#include <iostream>

//#include "../../../../modules/mapping.hpp"
#include "../../../../modules/mapping/combined_dwt_dct_pcfa.hpp"

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
    using recType = std::vector<double>;

    recType d0 {0, 1, 2, 3, 4, 5, 6, 7, 8};
    recType d1 {0, 1, 2, 3, 4, 5, 6, 7, 100};
    std::vector<recType> d = {d0, d1};

    float freq_time_balance = 0.5; // try values from 0 to 1 (e g 0, 0.5, 1) to get the following portions of freq-domain: 0, 4/9, 8/9
    auto bundle = metric::PCFA_combined<recType, void>(d, 2, freq_time_balance, 0.5);

    //auto pre_encoded = bundle.test_public_wrapper_encode(d);
    //auto pre_decoded = bundle.test_public_wrapper_decode(pre_encoded);

    auto encoded = bundle.encode(d);
    auto decoded = bundle.decode(encoded);

    std::cout << "\noriginal:\n";
    print_table(d);

    std::cout << "\ndecoded:\n";
    print_table(decoded); // some normalization issue when using DCT persists..

    std::cout << "\n";

    return 0;
}
