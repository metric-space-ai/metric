
#include <iostream>
//#include <stack>

#include "modules/mapping/DSPCC.hpp"

#include "examples/mapping_examples/assets/helpers.cpp" // for .csv reader

#include "modules/utils/visualizer.hpp"
#include "modules/utils/metric_err.hpp"

#include "modules/distance/k-related/Standards.hpp" // we use Euclidean metric for mean squared error evaluation





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

    // RMSE and metric error check
    //*

    {
        using R = std::vector<double>;

        R d0 {0, 1, 2, 3};
        R d1 {0, 1, 2, 3};
        std::vector<R> d = {d0, d1};

        auto d_upd = d;
        d_upd[0][3] = 5;

        std::cout << std::endl <<"d:" << std::endl;
        print_table(d); // some normalization issue when using DCT persists..

        std::cout << std::endl << "d_upd:" << std::endl;
        print_table(d_upd); // some normalization issue when using DCT persists..

        print_stats(normalized_err_stats<metric::Euclidean<double>>(d, d_upd));

        std::cout << "average RMSE = " << mean_square_error(d, d_upd) << std::endl;
    }


    // */





    // vibration example
    //*

    float magnitude = 15;

    auto raw_vdata = read_csv_num<double>("assets/vibration_smaller_3.csv", ",");
    //auto raw_vdata = read_csv_num<double>("assets/vibration.csv", ",");
//    auto raw_vdata = read_csv_num<double>("assets/vibration_smaller_3_no_peaks.csv", ",");
//    auto raw_vdata = read_csv_num<double>("assets/vibration_smaller_3_added_peaks.csv", ",");
    auto vdata =  transpose_timeseries(raw_vdata);

    mat2bmp::blaze2bmp_norm(vdata, "input.bmp", magnitude);

//    std::stack<size_t> length_stack;
//    auto decomposed = metric::sequential_DWT(vdata[0], length_stack, 5, 8);
//    auto restored = metric::sequential_iDWT(decomposed, length_stack, 5);

//    return 0;


    std::vector<double> errs_pre, errs_tf, errs_full;

    bool visualize = false;

    for (float mix = 0; mix<=1; mix+=0.25) {
    //float mix  = 0.5; {



        if (mix == 0.5)
            visualize = true;
        else
            visualize = false;


        auto vDSPCC = metric::DSPCC<std::vector<double>, void>(vdata, 10, 16, mix, 10);
        // dataset,
        // number of features of freq and time PCFAs,
        // DWT subbands, share of freq features in the mixed code,
        // top PCFA features



        auto v_encoded = vDSPCC.time_freq_PCFA_encode(vdata);
        auto v_decoded = vDSPCC.time_freq_PCFA_decode(v_encoded);

        if (visualize) {
            mat2bmp::blaze2bmp_norm(v_decoded, "decoded.bmp", magnitude);
            write_csv(transpose_timeseries(v_decoded), "decoded.csv", ";");
        }

        std::cout << std::endl << "        subband_length:  " << vDSPCC.get_subband_size() << std::endl;
        std::cout << "original record length:  " << vdata[0].size() << std::endl;
        std::cout << " decoded record length:  " << v_decoded[0].size() << std::endl;

        std::cout << std::endl << "decompression with only time-freq PSFAs done, decoded data saved" << std::endl;
        auto err_tf = normalized_err_stats<metric::Euclidean<double>>(vdata, v_decoded);
        print_stats(err_tf);
        errs_tf.push_back(std::get<4>(err_tf));



        //std::cout << std::endl << "computing pre-encoded and pre_decoded vibration data..." << std::endl;

        auto v_pre_encoded = vDSPCC.test_public_wrapper_encode(vdata);
        auto v_pre_decoded = vDSPCC.test_public_wrapper_decode(v_pre_encoded);

        if (visualize) {
            write_csv(transpose_timeseries(v_pre_decoded), "pre_decoded.csv", ";");
            mat2bmp::blaze2bmp_norm(v_pre_decoded, "pre_decoded.bmp", magnitude);
        }

        std::cout << std::endl << "test of pre-compression done, pre-decoded data saved" << std::endl;
        auto err_pre = normalized_err_stats<metric::Euclidean<double>>(vdata, v_pre_decoded);
        print_stats(err_pre);
        errs_pre.push_back(std::get<4>(err_pre));




        auto v_encoded2 = vDSPCC.encode(vdata);
        auto v_decoded2 = vDSPCC.decode(v_encoded2);

        if (visualize) {
            mat2bmp::blaze2bmp_norm(v_encoded2, "encoded2.bmp", magnitude);
            write_csv(transpose_timeseries(v_encoded2), "encoded2.csv", ";");
            mat2bmp::blaze2bmp_norm(v_decoded2, "decoded2.bmp", magnitude);
            write_csv(transpose_timeseries(v_decoded2), "decoded2.csv", ";");
        }


        std::cout << std::endl << "completely encoded data saved" << std::endl;
        auto err_full = normalized_err_stats<metric::Euclidean<double>>(vdata, v_decoded2);
        print_stats(err_full);
        errs_full.push_back(std::get<4>(err_full));

        std::cout << "average RMSE = " << mean_square_error(v_decoded2, vdata) << std::endl;

        std::cout << std::endl;

    }

    std::cout << std::endl << "Overall results:" << std::endl << "      pre\t      tf\t      full" << std::endl;
    for (size_t i = 0; i<errs_full.size(); ++i) {
        std::cout << errs_pre[i] << "\t" << errs_tf[i] << "\t" << errs_full[i] << std::endl;
    }




    //*/


    return 0;

}
