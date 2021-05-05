
#include "modules/mapping/ESN.hpp"
#include "modules/utils/visualizer.hpp"

#include <iostream>
#include <fstream>




template <class ValueType>
void blaze_dm_to_csv(const blaze::DynamicMatrix<ValueType> & data, std::string filename, std::string sep=",")
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
ContainerType read_csv(std::string filename, std::string sep=",", size_t lines = 0)
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
blaze::DynamicMatrix<ValueType, blaze::rowMajor> read_csv_blaze(const std::string & filename, std::string sep = ",", size_t lines = 0)
{
    auto array = read_csv<std::vector<std::vector<std::string>>>(filename, sep, lines);
    auto m = blaze::DynamicMatrix<ValueType, blaze::rowMajor>(array.size(), array[0].size());
    for (size_t i=0; i<array.size(); ++i)
        for (size_t j=0; j<array[0].size(); ++j)
            m(i, j) = convert_to<ValueType>(array[i][j]);
    return m;
}




template <class ValueType>
ValueType class_entropy(const blaze::DynamicVector<ValueType> & data, ValueType threshold) {
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






int main()
{


    size_t wnd_size = 15;

    using value_type = double;



    std::cout << "started" << std::endl << std::endl;

    bool visualize = false;  // only for small datasets that can be represented in an image point to point


    // dataset passed as Blaze matrix, data points in COLUMNS

    auto start_time = std::chrono::steady_clock::now();


    // preprocessing

    blaze::DynamicMatrix<value_type> ds_in = read_csv_blaze<value_type>("training_ds_1_fragm.csv"); //, ",", 10000);

    blaze::DynamicMatrix<value_type> raw_labels (ds_in.rows(), 1);
    blaze::column(raw_labels, 0) = blaze::column(ds_in, 4);
    blaze_dm_to_csv(raw_labels, "raw_labels.csv");

    blaze::DynamicVector<value_type> feature_stddev (ds_in.rows(), 0);
    int new_label = 0;
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
    blaze::submatrix(ds_all, 0, 0, ds_in.rows(), 3) = blaze::submatrix(ds_in, 0, 1, ds_in.rows(), 3);
    blaze::column(ds_all, 3) = feature_stddev;
    blaze::DynamicMatrix<value_type, blaze::rowMajor> data = blaze::trans(ds_all);

    blaze::DynamicMatrix<value_type, blaze::rowMajor> target = blaze::trans(blaze::submatrix(ds_in, 0, 4, ds_in.rows(), 1));

    blaze_dm_to_csv(ds_all, "data.csv");
    blaze_dm_to_csv<value_type>(blaze::trans(target), "target.csv");


    // prediction dataset
    blaze::DynamicMatrix<value_type> ds_pred = read_csv_blaze<value_type>("training_ds_2_fragm.csv"); //, ",", 10000);
    blaze::DynamicMatrix<value_type> raw_labels_pred (ds_pred.rows(), 1);
    blaze::column(raw_labels_pred, 0) = blaze::column(ds_pred, 4);
    blaze_dm_to_csv(raw_labels_pred, "raw_labels_pred.csv");

    blaze::DynamicVector<value_type> feature_stddev_pred (ds_pred.rows(), 0);
    new_label = 0;
    for (size_t i = wnd_size; i < feature_stddev_pred.size(); ++i) {
        auto wnd1 = blaze::submatrix(ds_pred, i - wnd_size, 1, wnd_size, 1);
        auto wnd2 = blaze::submatrix(ds_pred, i - wnd_size, 2, wnd_size, 1);
        auto wnd3 = blaze::submatrix(ds_pred, i - wnd_size, 3, wnd_size, 1);
        feature_stddev_pred[i] = stddev(wnd1) + stddev(wnd2) + stddev(wnd3);
        if (ds_pred(i, 4) >= 1)
            new_label = 1;
        if (ds_pred(i, 4) <= -1)
            new_label = 0;
        ds_pred(i, 4) = new_label;
    }

    blaze::DynamicMatrix<value_type> ds_all_pred (ds_pred.rows(), 4, 0);
    blaze::submatrix(ds_all_pred, 0, 0, ds_pred.rows(), 3) = blaze::submatrix(ds_pred, 0, 1, ds_pred.rows(), 3);
    blaze::column(ds_all_pred, 3) = feature_stddev_pred;
    blaze::DynamicMatrix<value_type, blaze::rowMajor> data_pred = blaze::trans(ds_all_pred);
    blaze_dm_to_csv(ds_all_pred, "data_pred.csv");


    auto end_time = std::chrono::steady_clock::now();
    std::cout << "data prepared in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;



    start_time = std::chrono::steady_clock::now();

    auto esn = metric::ESN<std::vector<value_type>, void>(500, 5, 0.99, 0.1, 2500, 0.5); // echo
    // w_size, w_connections, w_sr, alpha, washout, beta
    // currently best for old labels: (500, 10, 0.99, 0.9, 2500, 0.5)
    // and (500, 5, 0.99, 0.1, 2500, 0.5) for binary state labels
    esn.train(data, target);

    end_time = std::chrono::steady_clock::now();
    std::cout << "training completed in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;


    start_time = std::chrono::steady_clock::now();

    esn.save("esn_image.blaze");

    end_time = std::chrono::steady_clock::now();
    std::cout << "network saved in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;


    start_time = std::chrono::steady_clock::now();

    //auto prediction = esn.predict(test_data);
    auto prediction = esn.predict(data_pred);

    end_time = std::chrono::steady_clock::now();
    std::cout << "prediction completed in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;


    start_time = std::chrono::steady_clock::now();

    blaze::DynamicMatrix<value_type, blaze::rowMajor> out = blaze::trans(prediction);  // columnMajor does not comfort csv writer
    //blaze::DynamicMatrix<value_type, blaze::columnMajor> out = blaze::trans(prediction);
    blaze_dm_to_csv(out, "prediction_pred.csv");

    end_time = std::chrono::steady_clock::now();
    std::cout << "prediction written in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;

    if (visualize)
        mat2bmp::blaze2bmp_norm(prediction, "ESN_prediction.bmp");



    blaze::DynamicMatrix<value_type> sl_entropy (out.rows(), 1, 0);
    for (size_t i = wnd_size; i < out.rows(); ++i) {
        blaze::DynamicMatrix<value_type> wnd_row = blaze::submatrix(out, i - wnd_size, 0, wnd_size, 1);
        blaze::DynamicVector<value_type> wnd = blaze::column(wnd_row, 0); //blaze::trans(blaze::column(wnd_row, 0));
        sl_entropy(i, 0) = class_entropy(wnd, 0.5);
    }

    blaze_dm_to_csv(sl_entropy, "entropy_pred.csv");



    std::cout << std::endl << "postprocessing started" << std::endl;

    size_t cmp_wnd_sz = 150;

    value_type contrast_threshold = 0.3;
    blaze::DynamicMatrix<value_type> postproc_pred (out.rows(), 1, 0);
    bool prev_l_flag = false;
    for (size_t i = cmp_wnd_sz; i < out.rows() - cmp_wnd_sz; ++i) {
        bool l_flag = false;
        if (sl_entropy(i, 0) > 0.4) {
            blaze::DynamicMatrix<value_type> wnd_past = blaze::submatrix(out, i - cmp_wnd_sz, 0, cmp_wnd_sz, 1);
            blaze::DynamicMatrix<value_type> wnd_fut  = blaze::submatrix(out, i, 0, cmp_wnd_sz, 1);
            int label = 0;
            if (blaze::mean(wnd_past) - blaze::mean(wnd_fut) < -contrast_threshold) {  // TODO determine!!
                label = 1;
                l_flag = true;
            }
            if (blaze::mean(wnd_past) - blaze::mean(wnd_fut) > contrast_threshold) {  // TODO determine!!
                label = -1;
                l_flag = true;
            }
            if (!prev_l_flag)
                postproc_pred(i, 0) = label;
        }
        prev_l_flag = l_flag;
    }

    blaze_dm_to_csv(postproc_pred, "postproc_pred.csv");



    std::cout << "all done" << std::endl;

    return 0;

}
