
#include <iostream>
//#include <stack>

#include "../../../../modules/mapping/DSPCC.hpp"

#include "examples/mapping_examples/assets/helpers.cpp" // for .csv reader

#include "../../../../modules/utils/visualizer.hpp"




template <template <class, class> class Container, class ValueType, class A1, class A2>
Container<Container<ValueType, A1>, A2> transpose_timeseries(
        Container<Container<ValueType, A1>, A2> ts) // the workaround thing. TODO remove and update csv reader this way
{
    auto output = Container<Container<ValueType, A1>, A2>();
    size_t n_values = ts[0].size();
    for (size_t j=0; j<n_values; ++j) // loop of timeseries
    {
        output.push_back(Container<ValueType, A1>());
    }
    for (size_t i=0; i<ts.size(); ++i) // loop of values in original timeseries
    {
        for (size_t j=0; j<n_values; ++j) // loop of timeseries
            output[j].push_back(ts[i][j]);
    }
    return output;
}






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

    // small dataset
    /*

    using recType = std::vector<double>;

    recType d0 {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
    recType d1 {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 100};
    std::vector<recType> d = {d0, d1};

    float freq_time_balance = 1; // try values from 0 to 1 (e g 0, 0.5, 1) to get the following portions of freq-domain: 0, 4/9, 8/9
    auto bundle = metric::DSPCC1<recType, void>(d, 2, 4, freq_time_balance, 0.5);

    //auto pre_encoded = bundle.test_public_wrapper_encode(d);
    //auto pre_decoded = bundle.test_public_wrapper_decode(pre_encoded);

    auto encoded = bundle.encode(d);
    auto decoded = bundle.decode(encoded);

    std::cout << "\noriginal:\n";
    print_table(d);

    std::cout << "\ndecoded:\n";
    print_table(decoded); // some normalization issue when using DCT persists..

    std::cout << "\nmix_index: " << bundle.get_mix_idx() << "\n";

    std::cout << "\nsimple test done\n";

    //return 0;

    //*/


    // vibration example
    //*

    float magnitude = 80;

    auto raw_vdata = read_csv_num<double>("vibration_smaller_3.csv", ",");
    auto vdata =  transpose_timeseries(raw_vdata);

    mat2bmp::blaze2bmp_norm(vdata, "input.bmp", magnitude);

//    std::stack<size_t> length_stack;
//    auto decomposed = metric::sequential_DWT(vdata[0], length_stack, 5, 8);
//    auto restored = metric::sequential_iDWT(decomposed, length_stack, 5);

//    return 0;

//    auto vDSPCC = metric::DSPCC_single_PCFA<std::vector<double>, void>(vdata, 8, 8, 0.5, 0); // dataset, PCFA features, DWT subbands, freq share
    auto vDSPCC = metric::DSPCC<std::vector<double>, void>(vdata, 8, 10, 0.2, 50); // dataset, number of features of freq and time PCFAs, DWT subbands, share of freq features in the mixed code, top PCFA features

    auto v_encoded = vDSPCC.time_freq_PCFA_encode(vdata);
    auto v_decoded = vDSPCC.time_freq_PCFA_decode(v_encoded);

    mat2bmp::blaze2bmp_norm(v_decoded, "decoded.bmp", magnitude);
    write_csv(transpose_timeseries(v_decoded), "decoded.csv", ";");

    std::cout << "\nmix_index:     " << vDSPCC.get_crop_idx() << "\n";
    std::cout << "record length:  " << vdata[0].size() << "\n";


    std::cout << "\nmain vibration test done, decoded data saved\n";

    std::cout << "\ncomputing pre-encoded and pre_decoded vibration data...\n";

    auto v_pre_encoded = vDSPCC.test_public_wrapper_encode(vdata);
    auto v_pre_decoded = vDSPCC.test_public_wrapper_decode(v_pre_encoded);

    write_csv(transpose_timeseries(v_pre_decoded), "pre_decoded.csv", ";");
    mat2bmp::blaze2bmp_norm(v_pre_decoded, "pre_decoded.bmp", magnitude);

    std::cout << "\ndone, pre_decoded data saved\n";

    auto v_encoded2 = vDSPCC.encode(vdata);
    auto v_decoded2 = vDSPCC.decode(v_encoded2);

    mat2bmp::blaze2bmp_norm(v_encoded2, "encoded2.bmp", magnitude);
    write_csv(transpose_timeseries(v_encoded2), "encoded2.csv", ";");
    mat2bmp::blaze2bmp_norm(v_decoded2, "decoded2.bmp", magnitude);
    write_csv(transpose_timeseries(v_decoded2), "decoded2.csv", ";");

    std::cout << "\ncompletely encoded data saved\n";

    //*/


    return 0;

}
