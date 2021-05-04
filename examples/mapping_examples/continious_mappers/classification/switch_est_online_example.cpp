

#include "../../../../modules/mapping/esn_switch_detector.hpp"


#include <iostream>
#include <fstream>




template <class ValueType>
void blaze_dm_to_csv(blaze::DynamicMatrix<ValueType> data, std::string filename, std::string sep=",")
{
    std::ofstream outputFile;
    outputFile.open(filename);
    for (auto i = 0; i < data.rows(); ++i) {
        for (auto j = 0; j < data.columns(); j++) {
            outputFile << std::to_string(data(i, j));
            if (j < data.columns() - 1)
                outputFile << sep;
        }
        outputFile << std::endl;
    }
    outputFile.close();
}



template <typename T> T convert_to(const std::string & str)
{
    std::istringstream s(str);
    T num;
    s >> num;
    return num;
} // templated version of stof, stod, etc., thanks to https://gist.github.com/mark-d-holmberg/862733




template <class ContainerType>
void v_to_csv(ContainerType data, std::string filename)  // single column
{
    std::ofstream outputFile;
    outputFile.open(filename);
        for (auto i = 0; i < data.size(); ++i) {
            outputFile << std::to_string(data[i]);
            outputFile << std::endl;
        }
        outputFile.close();
}



template <class ValueType>
std::vector<std::vector<ValueType>> read_csv_num(std::string filename, std::string sep=",")
{ // code dubbing with read_csv, TODO unify and remove one of these functions
    typedef typename std::vector<ValueType> LINE;
    std::string line;
    int pos;
    std::vector<std::vector<ValueType>> array = {};
    std::ifstream in(filename);
    if(!in.is_open()) {
        std::cout << "Failed to open file" << std::endl;
        return array;
    }
    while( getline(in,line) ) {
        LINE ln;
        while( (pos = line.find(sep)) >= 0) {
            std::string field = line.substr(0, pos);
            line = line.substr(pos+1);
            ln.push_back(convert_to<ValueType>(field));
        }
        ln.push_back(convert_to<ValueType>(line));
        array.push_back(ln);
    }
    return array;
}






int main()
{

    using value_type = double;


    std::cout << "started" << std::endl << std::endl;


    auto start_time = std::chrono::steady_clock::now();

    auto model = SwitchPredictor<value_type>("model2.blaze");
    //auto model = SwitchPredictor<value_type>("esn_image.blaze");

    auto end_time = std::chrono::steady_clock::now();
    std::cout << "model loaded in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;

    std::vector<std::vector<value_type>> ds = read_csv_num<value_type>("training_ds_2_fragm.csv"); //, ",", 10000);


    start_time = std::chrono::steady_clock::now();

    /*
    // old 1-sample version
    std::vector<std::tuple<size_t, value_type>> pairs = {};
    std::vector<size_t> response_pos = {};
    std::vector<size_t> n_pairs = {};
    for (size_t i = 0; i < ds.size(); ++i) {
        std::vector<value_type> sample = {ds[i][1], ds[i][2], ds[i][2]};
        std::vector<std::tuple<size_t, value_type>> pair_result = model.encode(sample);
        if (pair_result.size() > 0) {
            pairs.insert(pairs.end(), pair_result.begin(), pair_result.end());
            response_pos.push_back(i);
            n_pairs.push_back(pair_result.size());
        }
    }

    std::cout << std::endl << "all pairs:" << std::endl;
    size_t pair_cnt = 0;
    for (size_t i = 0; i < response_pos.size(); ++i) {
        std::cout << "response " << i << " at " << response_pos[i] << ", number of pairs: " << n_pairs[i] << ": " << std::endl;
        for (size_t j = 0; j < n_pairs[i]; ++j) {
            std::cout << "    pair: " << std::get<0>(pairs[pair_cnt]) << ", " << std::get<1>(pairs[pair_cnt]) << std::endl;
            ++pair_cnt;
        }
    }
    // */


    std::vector<size_t> sizes = {12, 350, 145};

    std::vector<std::tuple<unsigned long long int, value_type>> all_pairs;

    {
        size_t passed = 0;
        size_t s_idx = 0;
        all_pairs = {};
        while (passed + sizes[s_idx] < ds.size()) {  // we still can fetch the slice of given size
            std::vector<std::vector<value_type>> slice = {};
            std::vector<unsigned long long int> slice_indices = {};
            for (size_t i = passed; i < passed + sizes[s_idx]; ++i) {
                std::vector<value_type> sample = {ds[i][1], ds[i][2], ds[i][2]};
                slice.push_back(sample);
                slice_indices.push_back(ds[i][0]);
            }
            auto pairs = model.encode(slice_indices, slice);
            all_pairs.insert(all_pairs.end(), pairs.begin(), pairs.end());

            passed += sizes[s_idx];
            s_idx++;
            if (s_idx >= sizes.size())
                s_idx = 0;
        }
    }


    end_time = std::chrono::steady_clock::now();
    std::cout << "online estimation with pair output completed in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;

    std::cout << std::endl << "all pairs:" << std::endl;

    for (size_t j = 0; j < all_pairs.size(); ++j) {
        std::cout << "pair: " << std::get<0>(all_pairs[j]) << ", " << std::get<1>(all_pairs[j]) << std::endl;
    }



    // the same but with raw output

    start_time = std::chrono::steady_clock::now();

    {
        model = SwitchPredictor<value_type>("model2.blaze");  // reset
        size_t passed = 0;
        size_t s_idx = 0;
        std::vector<value_type> predictions = {};
        all_pairs = {};
        while (passed + sizes[s_idx] < ds.size()) {  // we still can fetch the slice of given size
            std::vector<std::vector<value_type>> slice = {};
            std::vector<unsigned long long int> slice_indices = {};
            for (size_t i = passed; i < passed + sizes[s_idx]; ++i) {
                std::vector<value_type> sample = {ds[i][1], ds[i][2], ds[i][2]};
                //unsigned long long int index = ds[i][0];
                slice.push_back(sample);
                slice_indices.push_back(ds[i][0]);
            }
            auto raw_res = model.encode_raw(slice_indices, slice);
            std::vector<unsigned long long int> switch_indices = std::get<0>(raw_res);
            std::vector<value_type> switches = std::get<1>(raw_res);
            predictions.insert(predictions.end(), switches.begin(), switches.end());
            auto pairs = model.make_pairs(switch_indices, switches);
            all_pairs.insert(all_pairs.end(), pairs.begin(), pairs.end());

            passed += sizes[s_idx];
            s_idx++;
            if (s_idx >= sizes.size())
                s_idx = 0;
        }
        v_to_csv(predictions, "online_estimation.csv");
    }

    end_time = std::chrono::steady_clock::now();
    std::cout << std::endl << std::endl << "online estimation with pair and raw output completed in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;

    std::cout << std::endl << "all pairs:" << std::endl;

    for (size_t j = 0; j < all_pairs.size(); ++j) {
        std::cout << "pair: " << std::get<0>(all_pairs[j]) << ", " << std::get<1>(all_pairs[j]) << std::endl;
    }

    /*
    // raw prediction output, uses same model with filled buffer, thus no warmup
    start_time = std::chrono::steady_clock::now();

    //std::vector<std::vector<value_type>> ds = read_csv_num<value_type>("training_ds_2_fragm.csv"); //, ",", 10000);
    std::vector<value_type> predictions = {};
    for (size_t i = 0; i < ds.size(); ++i) {
        std::vector<value_type> sample = {ds[i][1], ds[i][2], ds[i][2]};
        std::vector<value_type> result = model.encode_raw(sample);
        if (result.size() > 0) {
            predictions.insert(predictions.end(), result.begin(), result.end());
        }
    }

    v_to_csv(predictions, "online_estimation.csv");

    end_time = std::chrono::steady_clock::now();
    std::cout << "raw online estimation completed in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;
    // */

    return 0;
}
