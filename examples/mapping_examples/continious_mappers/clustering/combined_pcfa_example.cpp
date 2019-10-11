
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

    auto bundle = metric::PCFA_combined<recType, void>(d, 2, 0.8, 0.5);

    //auto pre_encoded = bundle.test_public_wrapper_encode(d);
    //auto pre_decoded = bundle.test_public_wrapper_decode(pre_encoded);

    auto encoded = bundle.encode(d);
    auto decoded = bundle.decode(encoded);

    std::cout << "\noriginal:\n";
    print_table(d);

    std::cout << "\ndecoded:\n";
    print_table(decoded);

    std::cout << "\n";

    return 0;
}
