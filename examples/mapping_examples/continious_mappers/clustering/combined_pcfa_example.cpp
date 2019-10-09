
#include <iostream>

//#include "../../../../modules/mapping.hpp"
#include "../../../../modules/mapping/combined_dwt_dct_pcfa.hpp"


int main()
{
    using recType = std::vector<double>;

    recType d0 {0, 1, 2};
    recType d1 {0, 1, 3};
    std::vector<recType> d = {d0, d1};

    auto bundle = metric::PCFA_combined<recType, void>(d, 2, 0.5, 0.5);

    auto pre_encoded = bundle.test_public_wrapper_encode(d);
    auto decoded = bundle.test_public_wrapper_decode(pre_encoded);

    return 0;
}
