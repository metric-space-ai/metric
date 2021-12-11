
#include "blaze/Blaze.h"

#include <iostream>
#include <fstream>




template <class ValueType>
void blaze_dm_to_csv(const blaze::DynamicMatrix<ValueType> & data, const std::string filename, const std::string sep=",")
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
ContainerType read_csv(const std::string filename, const std::string sep=",", const size_t lines = 0)
{  // works with string, does not convert to numbers
    typedef typename ContainerType::value_type LINE;
    std::string line;
    int pos;
    ContainerType array = {};
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cout << "Failed to open file: " << filename << std::endl;
        return array;
    }
    size_t cnt = 0;
    while (getline(in, line)) {
        LINE ln;
        while( (pos = line.find(sep)) >= 0)	{
            std::string field = line.substr(0, pos);
            line = line.substr(pos+1);
            ln.push_back(field);
        }
        ln.push_back(line);
        array.push_back(ln);
        if (lines > 0 && cnt >= lines-1)
            break;
        ++cnt;
    }
    return array;
}



template <class ValueType>
blaze::DynamicMatrix<ValueType, blaze::rowMajor> read_csv_blaze(
        const std::string & filename,
        const std::string sep = ",",
        const size_t lines = 0
        )
{
    auto array = read_csv<std::vector<std::vector<std::string>>>(filename, sep, lines);
    auto m = blaze::DynamicMatrix<ValueType, blaze::rowMajor>(array.size(), array[0].size());
    for (size_t i=0; i<array.size(); ++i)
        for (size_t j=0; j<array[0].size(); ++j)
            m(i, j) = convert_to<ValueType>(array[i][j]);
    return m;
}




template <class ValueType>
ValueType class_entropy(const blaze::DynamicVector<ValueType> & data, const ValueType threshold) {
    int sum = 0;
    ValueType sz = data.size();
    for (size_t i = 0; i<sz; ++i) {
        if (data[i] > threshold)
            ++sum;
    }
    ValueType p1 = sum/sz;
    if (sum == 0 || sum == sz)
        return 0;
    else
        return -p1*log2(p1) - (1 - p1)*log2(1 - p1);
}






int main()  // DetSwitchDetector internals reproduced with intermediate output added
{

    using value_type = float; //double;


    size_t wnd_size = 15;

    value_type update_rate = 0.0025;

    value_type w_r = 2;
    value_type w_g = 2;
    value_type w_b = 2;
    value_type w_s = 15;
    value_type magn = 0.002;
    value_type rgb_offs = -5;
    value_type s_offs = 0;

    blaze::DynamicMatrix<value_type> W1 = {
            {magn*w_r, 0, 0, 0, magn*rgb_offs},
            {0, magn*w_g, 0, 0, magn*rgb_offs},
            {0, 0, magn*w_b, 0, magn*rgb_offs},
            {0, 0, 0, magn*w_s, magn*  s_offs},
        };

    blaze::DynamicMatrix<value_type> Wo = {
            {1, 1, 1, 2}
        };



    std::cout << "started" << std::endl << std::endl;

    // dataset passed as Blaze matrix, data points in COLUMNS


    auto start_time = std::chrono::steady_clock::now();

    blaze::DynamicMatrix<value_type> ds_in = read_csv_blaze<value_type>("training_ds_2_fragm.csv"); //, ",", 10000);
//    blaze::DynamicMatrix<value_type> ds_in = read_csv_blaze<value_type>("slice.csv"); //, ",", 10000);
//    blaze::DynamicMatrix<value_type> ds_in = read_csv_blaze<value_type>("slice_small.csv"); //, ",", 10000);
    blaze::DynamicMatrix<value_type> raw_labels (ds_in.rows(), 1);
    blaze::column(raw_labels, 0) = blaze::column(ds_in, 4);
    blaze_dm_to_csv(raw_labels, "raw_labels_2.csv");

    blaze::DynamicVector<value_type> feature_stddev (ds_in.rows(), 0);
    value_type new_label = 0;
    for (size_t i = wnd_size; i < feature_stddev.size(); ++i) {
        auto wnd1 = blaze::submatrix(ds_in, i - wnd_size, 1, wnd_size, 1);
        auto wnd2 = blaze::submatrix(ds_in, i - wnd_size, 2, wnd_size, 1);
        auto wnd3 = blaze::submatrix(ds_in, i - wnd_size, 3, wnd_size, 1);
        feature_stddev[i] = stddev(wnd1) + stddev(wnd2) + stddev(wnd3);
        if (ds_in(i, 4) >= 1)
            new_label = 1;
        if (ds_in(i, 4) <= -1)
            new_label = 0;
        ds_in(i, 4) = new_label;
    }

    blaze::DynamicMatrix<value_type> ds_all (ds_in.rows(), 4, 0);
    //blaze::submatrix(ds_all, 0, 0, ds_in.rows(), 3) = blaze::submatrix(ds_in, 0, 1, ds_in.rows(), 3);
    blaze::column(ds_all, 0) = blaze::column(ds_in, 1);
    blaze::column(ds_all, 1) = blaze::column(ds_in, 2);
    blaze::column(ds_all, 2) = blaze::column(ds_in, 3);
    blaze::column(ds_all, 3) = feature_stddev;
    blaze_dm_to_csv(ds_all, "data_2.csv");


    auto end_time = std::chrono::steady_clock::now();
    std::cout << "data prepared in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;



    // filtering

    std::cout << std::endl << "filtering started" << std::endl;

    blaze::DynamicMatrix<value_type> layer1 (ds_in.rows(), W1.rows(), 0);
    blaze::DynamicMatrix<value_type> mixed_out (ds_in.rows(), 1, 0);

    blaze::DynamicMatrix<value_type> latency_out (ds_in.rows(), 1, 0);
    value_type sliding_prev = mixed_out(wnd_size, 0);

    blaze::DynamicMatrix<int> bin_out (ds_in.rows(), 1, 0);

    for (size_t i = wnd_size; i < ds_in.rows(); ++i) {

        blaze::DynamicVector<value_type> in1 (ds_all.columns() + 1);
        blaze::subvector(in1, 0, ds_all.columns()) = blaze::trans(blaze::row(ds_all, i));
        in1[ds_all.columns()] = 1;  // offset element
        blaze::row(layer1, i) = blaze::tanh(blaze::trans(W1 * in1));
        //blaze::row(layer1, i) = blaze::trans(W1 * in1);  // TODO remove

        //mixed_out(i, 0) = blaze::sum(blaze::row(layer1, i)) / ((value_type)layer1.columns());
        mixed_out(i, 0) = blaze::sum(Wo*blaze::trans(blaze::row(layer1, i))) / blaze::sum(Wo);

        if (mixed_out(i, 0) > 0.5) {
            latency_out(i, 0) = 1;
        } else { // off latency
            value_type upd = ( mixed_out(i, 0)*update_rate + sliding_prev*(1 - update_rate) );
            latency_out(i, 0) = mixed_out(i, 0) > upd ? mixed_out(i, 0) : upd;
        }
        sliding_prev = latency_out(i, 0);

        if (latency_out(i, 0) > 0.5) { // binarize
            bin_out(i, 0) = 1;
        }
    }

    blaze_dm_to_csv(layer1, "layer1.csv");
    blaze_dm_to_csv(mixed_out, "out1.csv");
    blaze_dm_to_csv(latency_out, "out_smoothed.csv");
    blaze_dm_to_csv(bin_out, "out_bin.csv");



    std::cout << "all done" << std::endl;

    return 0;

}
