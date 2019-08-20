
#include "modules/transform/discrete_cosine.hpp"

#include "modules/utils/visualizer.hpp"
#include "examples/mapping_examples/assets/helpers.cpp"


#include "modules/transform/wavelet.hpp"
#include <vector>
#include <iostream>
#include <chrono>


template <typename T>
std::vector<T>
extract_energies(std::vector<T> x)
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
    std::cout << subbands[0].size() << std::endl;
    return energies;
}



template <typename T>
std::vector<std::vector<T>>
sequential_DWT(std::vector<T> x)
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
    return subbands;
}


template <typename T>
std::vector<T>
get_energies(std::vector<std::vector<T>> subbands)
{
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
    std::cout << subbands[0].size() << std::endl;
    return energies;
}


template <template <class, class> class Container, class ValueType, class A1, class A2>
Container<Container<Container<ValueType, A1>, A2>, std::allocator<Container<Container<ValueType, A1>, A2>>> divide_timeseries(
        Container<Container<ValueType, A1>, A2> ts,
        size_t block_length)
{
    auto output = Container<Container<Container<ValueType, A1>, A2>, std::allocator<Container<Container<ValueType, A1>, A2>>>();
    size_t n_values = ts[0].size();
    for (size_t j=0; j<n_values; ++j) // loop of timeseries
    {
        auto vector_of_blocks = Container<Container<ValueType, A1>, A2>();
        vector_of_blocks.push_back(Container<ValueType, A1>()); // create first block of current timeseries
        output.push_back(vector_of_blocks);
    }
    size_t block = 0;
    size_t pos_in_block = 0;
    for (size_t i=0; i<ts.size(); ++i) // loop of values in original timeseries
    {
        if (pos_in_block<block_length) // TODO ad check of last incomplete block
        {
            // push values
            for (size_t j=0; j<n_values; ++j) // loop of timeseries
            {
                output[j][block].push_back(ts[i][j]);
            }
            ++pos_in_block;
        }
        else
        {
            for (size_t j=0; j<n_values; ++j) // loop of timeseries
            {
                output[j].push_back(Container<ValueType, A1>()); // create next block of current timeseries
            }
            pos_in_block = 0;
            ++block;
        }
    }

    return output;
}





template <template <class, class> class Container, class ValueType, class A1, class A2>
Container<Container<ValueType, A1>, A2> transpose_timeseries(
        Container<Container<ValueType, A1>, A2> ts) // TODO remove and update csv reader this way
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



template <template <class, class> class Container, class ValueType, class A1, class A2>
Container<Container<Container<ValueType, A1>, A2>, std::allocator<Container<Container<ValueType, A1>, A2>>> apply_seq_DWT(
        Container<Container<ValueType, A1>, A2> ts)
{
    auto output = Container<Container<Container<ValueType, A1>, A2>, std::allocator<Container<Container<ValueType, A1>, A2>>>();

    for (size_t j=0; j<ts.size(); ++j) // loop of timeseries
    {
        output.push_back(sequential_DWT(ts[j]));
    }
    return output;
}







template <template <class, class> class Container, class ValueType, class A1, class A2, class A3>
Container<Container<ValueType, A1>, A2> apply_energy_extractor(
        Container<Container<Container<ValueType, A1>, A2>, A3> ts_blocks)
{
    auto output = Container<Container<ValueType, A1>, A2>();

    for (size_t j=0; j<ts_blocks.size(); ++j) // loop of timeseries
    {
        output.push_back(Container<ValueType, A1>());
        for (size_t i=0; i<ts_blocks[j].size(); ++i) // loop of blocks
        {
            auto energies = extract_energies(ts_blocks[j][i]);
            ValueType energy = 0;
            for (size_t k=0; k<energies.size(); ++k)
                energy += energies[k];
            output[j].push_back(energy);
        }
    }

    return output;
}




template <template <class, class> class Container, class ValueType, class A1, class A2, class A3>
Container<Container<Container<ValueType, A1>, A2>, A3> apply_energy_extractor_separate_subbands(
        Container<Container<Container<ValueType, A1>, A2>, A3> ts_blocks)
{
    auto output = Container<Container<Container<ValueType, A1>, A2>, A3>();

    for (size_t j=0; j<ts_blocks.size(); ++j) // loop of timeseries
    {
        output.push_back(Container<Container<ValueType, A1>, A2>());
        for (size_t i=0; i<ts_blocks[j].size(); ++i) // loop of blocks
        {
            auto energies = extract_energies(ts_blocks[j][i]);
            output[j].push_back(energies);
        }
    }

    return output;
}



int main() {

    /*
    auto x = wavelet::linspace(0.0, 1.0, 3200);

    std::cout << wavelet::wmaxlev(x.size(), 5) << std::endl;

    auto start = std::chrono::steady_clock::now();
    auto energies = extract_energies(x);
    auto end = std::chrono::steady_clock::now();
    std::cout << "elapsed time : "
        << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
        << " µs" << std::endl;
    std::cout << "result:        [" ;

    for (int i = 0; i < energies.size()-1; ++i)
    {
        std::cout << energies[i]  << ", " ;
    }
    std::cout<< energies[energies.size()-1] <<  "]"<< std::endl;

    return 0;
    //*/





    // single
//    auto training_dataset = read_csv_blaze<float>("Pt01_Ch01_Grooves_1_to_7.csv");
//    // std::cout << training_dataset << "\n";

//    auto test_data = read_csv_blaze<float>("Pt01_Ch01_Groove09.csv");

//    mat2bmp::blaze2bmp(training_dataset, "training_dataset.bmp");
//    mat2bmp::blaze2bmp(test_data, "test_data.bmp");



    // splt first

//    auto all_vibration_ts = read_csv_num<float>("Pt01_Ch01_Grooves_1_to_7.csv");
//    auto blocks = divide_timeseries(all_vibration_ts, 60);
//    auto energies = apply_energy_extractor(blocks);
//    write_csv(energies, "energy_per_groove_overall.csv");

//    auto energies_by_subbands = apply_energy_extractor_separate_subbands(blocks);
//    for (size_t i=0; i<energies_by_subbands.size(); ++i) {
//        write_csv(energies_by_subbands[i], "energy_per_groove_" + std::to_string(i) + ".csv");
//    }

    // DWT first

    auto all_vibration_ts = read_csv_num<float>("Pt01_Ch01_Grooves_1_to_7.csv");
    auto transposed = transpose_timeseries(all_vibration_ts);
    auto subband_waveforms = apply_seq_DWT(transposed);
    for (size_t i=0; i<subband_waveforms.size(); ++i) {
        write_csv(subband_waveforms[i], "filtered_waveform_groove_" + std::to_string(i) + ".csv");
    }


    return 0;
}
