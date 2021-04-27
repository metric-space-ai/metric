

#include "../../../../modules/mapping/ESN.hpp"
//#include "../../../../modules/correlation/entropy.hpp"

#include "../../../../modules/utils/visualizer.hpp"

#include <iostream>
#include <fstream>

#ifndef M_PI
        // MSVC does not define M_PI
    #define M_PI 3.14159265358979323846
#endif


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
ValueType class_entropy(blaze::DynamicVector<ValueType> data, ValueType threshold) {
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


    std::cout << "started" << std::endl << std::endl;

    bool visualize = false;  // only for small datasets that can be represented in an image point to point

    using value_type = double;


    // dataset passed as Blaze matrix, data points in COLUMNS

    auto start_time = std::chrono::steady_clock::now();

//    blaze::DynamicMatrix<value_type> ds = blaze::trans(read_csv_blaze<value_type>("rgblog_fragm_labeled.csv"));
//    blaze::DynamicMatrix<value_type> ds = blaze::trans(read_csv_blaze<value_type>("rgblog_labeled_short.csv"));
    //blaze::DynamicMatrix<value_type> ds = blaze::trans(read_csv_blaze<value_type>("training_ds_1.csv"));
    //blaze::DynamicMatrix<value_type> ds = blaze::trans(read_csv_blaze<value_type>("training_ds_1_fragm.csv"));
    //blaze::DynamicMatrix<value_type> ds = blaze::trans(read_csv_blaze<value_type>("training_ds_1_fragm_shift.csv"));
//    blaze::DynamicMatrix<value_type> ds = blaze::trans(read_csv_blaze<value_type>("training_ds_1_fragm_shift_30.csv"));
    //blaze::DynamicMatrix<value_type> ds = blaze::trans(read_csv_blaze<value_type>("training_ds_1_fragm_binary_labels.csv"));
    //blaze::DynamicMatrix<value_type> ds = blaze::trans(read_csv_blaze<value_type>("training_ds_1_fragm_feature_diff.csv"));
//    blaze::DynamicMatrix<value_type> ds = blaze::trans(read_csv_blaze<value_type>("training_ds_1_fragm_feature_stddev.csv"));

    //std::cout << "training dataset read & transposed" << std::endl << std::endl;

    //*
    // preprocessing

    blaze::DynamicMatrix<value_type> ds_in = read_csv_blaze<value_type>("training_ds_1_fragm.csv"); //, ",", 10000);
//    blaze::DynamicMatrix<value_type> ds_in = read_csv_blaze<value_type>("training_ds_1_fragm.csv");

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

    blaze::DynamicMatrix<value_type, blaze::rowMajor>  test_data = data;

    blaze_dm_to_csv(ds_all, "data.csv");
    blaze_dm_to_csv<value_type>(blaze::trans(target), "target.csv");

    // */


    /*
    //blaze::DynamicMatrix<value_type, blaze::rowMajor> data = blaze::submatrix(ds, 0, 0, 3, ds.columns());
    //blaze::DynamicMatrix<value_type, blaze::rowMajor> data = blaze::submatrix(ds, 1, 0, 3, ds.columns()); // 3 features
    blaze::DynamicMatrix<value_type, blaze::rowMajor> data = blaze::submatrix(ds, 1, 0, 4, ds.columns()); // 4 features
    // first COLUMN represents zero time moment, second represents time = 1, etc

    std::cout << "data selected" << std::endl << std::endl;

    //blaze::DynamicMatrix<value_type, blaze::rowMajor> target = blaze::submatrix(ds, 3, 0, 1, ds.columns());
    //blaze::DynamicMatrix<value_type, blaze::rowMajor> target = blaze::submatrix(ds, 4, 0, 1, ds.columns()); // 3 features
    blaze::DynamicMatrix<value_type, blaze::rowMajor> target = blaze::submatrix(ds, 5, 0, 1, ds.columns()); // 4 features

    std::cout << "label selected" << std::endl << std::endl;

    //blaze::DynamicMatrix<value_type, blaze::rowMajor>  test_data =
    //        blaze::trans(read_csv_blaze<value_type>("rgblog_test.csv"));
    blaze::DynamicMatrix<value_type, blaze::rowMajor>  test_data = data;


    std::cout << "test dataset read & transposed" << std::endl << std::endl;

    // */

    auto end_time = std::chrono::steady_clock::now();
    std::cout << "data prepared in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;

    if (visualize)
    {
        mat2bmp::blaze2bmp_norm(data, "ESN_SlicesR.bmp");
        mat2bmp::blaze2bmp_norm(target, "ESN_TargetR.bmp");
        mat2bmp::blaze2bmp_norm(test_data, "ESN_SlicesTestR.bmp");
    }

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

    auto prediction = esn.predict(test_data);

    end_time = std::chrono::steady_clock::now();
    std::cout << "prediction completed in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;


    start_time = std::chrono::steady_clock::now();

    blaze::DynamicMatrix<value_type, blaze::rowMajor> out = blaze::trans(prediction);  // columnMajor does not comfort csv writer
    //blaze::DynamicMatrix<value_type, blaze::columnMajor> out = blaze::trans(prediction);
    blaze_dm_to_csv(out, "prediction.csv");

    end_time = std::chrono::steady_clock::now();
    std::cout << "prediction written in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;

    if (visualize)
        mat2bmp::blaze2bmp_norm(prediction, "ESN_prediction.bmp");

    //*
    //postprocessing
    //auto ekpn_eucl = metric::Entropy<void, metric::Euclidean<value_type>>(metric::Euclidean<value_type>(), 3, 10);
    blaze::DynamicMatrix<value_type> sl_entropy (out.rows(), 1, 0);
    for (size_t i = wnd_size; i < out.rows(); ++i) {
        blaze::DynamicMatrix<value_type> wnd_row = blaze::submatrix(out, i - wnd_size, 0, wnd_size, 1);
        blaze::DynamicVector<value_type> wnd = blaze::column(wnd_row, 0); //blaze::trans(blaze::column(wnd_row, 0));
        // TODO convert to std::vector
        //sl_entropy(i, 0) = ekpn_eucl(wnd);
        sl_entropy(i, 0) = class_entropy(wnd, 0.5);
    }

    blaze_dm_to_csv(sl_entropy, "entropy.csv");
    // */


    std::cout << "all done" << std::endl;

    return 0;

}
