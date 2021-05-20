

#include "modules/mapping/esn_switch_detector.hpp"


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
blaze::DynamicMatrix<ValueType, blaze::rowMajor> read_csv_blaze(const std::string & filename, const std::string sep = ",", const size_t lines = 0)
{
    auto array = read_csv<std::vector<std::vector<std::string>>>(filename, sep, lines);
    auto m = blaze::DynamicMatrix<ValueType, blaze::rowMajor>(array.size(), array[0].size());
    for (size_t i=0; i<array.size(); ++i)
        for (size_t j=0; j<array[0].size(); ++j)
            m(i, j) = convert_to<ValueType>(array[i][j]);
    return m;
}







int main()
{

    using value_type = double;


    std::cout << "started" << std::endl << std::endl;



    // dataset passed as Blaze matrix, data points in COLUMNS

    {
        auto start_time = std::chrono::steady_clock::now();

        blaze::DynamicMatrix<value_type> ds_in = read_csv_blaze<value_type>("training_ds_1_fragm.csv"); //, ",", 10000);
        blaze::DynamicMatrix<value_type> training_ds (ds_in.rows(), 3, 0);
        blaze::submatrix(training_ds, 0, 0, ds_in.rows(), 3) = blaze::submatrix(ds_in, 0, 1, ds_in.rows(), 3);

        blaze::DynamicMatrix<value_type> labels (ds_in.rows(), 1);
        blaze::column(labels, 0) = blaze::column(ds_in, 4);

        //auto model = SwitchPredictor<value_type>(training_ds, labels);
        auto model = SwitchPredictor<value_type>(training_ds, labels, 15, 150, 100, 0.2, 0.4, 0.5);
        // training_data, labels, wnd_size, cmp_wnd_sz, washout, contrast_threshold, alpha, beta

        auto end_time = std::chrono::steady_clock::now();
        std::cout << "training completed in " <<
                     double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                     std::endl << std::endl;

        model.save("tmp.blaze");
    }


    {
        auto model = SwitchPredictor<value_type>("tmp.blaze");

        auto start_time = std::chrono::steady_clock::now();

        blaze::DynamicMatrix<value_type> ds_pred = read_csv_blaze<value_type>("training_ds_2_fragm.csv"); //, ",", 10000);
        blaze::DynamicMatrix<value_type> ds (ds_pred.rows(), 3, 0);
        blaze::submatrix(ds, 0, 0, ds_pred.rows(), 3) = blaze::submatrix(ds_pred, 0, 1, ds_pred.rows(), 3);

        auto est = model.encode(ds);

        blaze_dm_to_csv(est, "estimation.csv");

        auto end_time = std::chrono::steady_clock::now();
        std::cout << "estimation completed in " <<
                     double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                     std::endl << std::endl;

        auto params = model.get_parameters();
        std::cout << std::endl << "used parameters: " << std::endl <<
                     "wnd_size: " << std::get<0>(params) << std::endl <<
                     "cmp_wnd_sz: " << std::get<1>(params) << std::endl <<
                     "washout: " << std::get<2>(params) << std::endl <<
                     "contrast_threshold: " << std::get<3>(params) << std::endl <<
                     "alpha: " << std::get<4>(params) << std::endl <<
                     "beta: " << std::get<5>(params) << std::endl;
    }



    return 0;
}
