

#include "metric/mapping/esn_switch_detector.hpp"


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



template <class ContainerType>
void v_to_csv(const ContainerType data, const std::string filename)  // single column
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
std::vector<std::vector<ValueType>> read_csv_num(const std::string filename, const std::string sep=",")
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

    using value_type = float; //double;


    // In all examples the input files must be comma-separated .scv of the following format:
    // timestamp,r,g,b,label
    // e.g.:
    // 1618830448306,1089,1396,1400,0
    // no header is expected in the file
    // When files are used for prediction, labels are ignored.



    // Blaze example: dataset passed as Blaze matrix, data points in COLUMNS

    // read dataset and save model
    {
        auto start_time = std::chrono::steady_clock::now();

        blaze::DynamicMatrix<value_type> ds_in = read_csv_blaze<value_type>("training_ds_1_fragm.csv"); //, ",", 10000);
        blaze::DynamicMatrix<value_type> training_ds (ds_in.rows(), 3, 0);
        blaze::submatrix(training_ds, 0, 0, ds_in.rows(), 3) = blaze::submatrix(ds_in, 0, 1, ds_in.rows(), 3);

        blaze::DynamicMatrix<value_type> labels (ds_in.rows(), 1);
        blaze::column(labels, 0) = blaze::column(ds_in, 4);

        //auto model = SwitchPredictor<value_type>(training_ds, labels);
        //auto model = SwitchPredictor<value_type>(training_ds, labels, 15, 150, 100, 0.2, 0.4, 0.5);
        auto model = SwitchPredictor<value_type>(training_ds, labels, 15, 80, 100, 0.2, 0.4);
        // training_data, labels, wnd_size, cmp_wnd_sz, washout, contrast_threshold, alpha, beta

        auto end_time = std::chrono::steady_clock::now();
        std::cout << "training completed in " <<
                     double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                     std::endl << std::endl;

        model.save("tmp.blaze");
    }

    // load and apply the model
    {
        auto model = SwitchPredictor<value_type>("tmp.blaze");

        auto start_time = std::chrono::steady_clock::now();

        blaze::DynamicMatrix<value_type> ds_pred = read_csv_blaze<value_type>("training_ds_2_fragm.csv"); //, ",", 10000);
        blaze::DynamicMatrix<value_type> ds (ds_pred.rows(), 3, 0);
        blaze::submatrix(ds, 0, 0, ds_pred.rows(), 3) = blaze::submatrix(ds_pred, 0, 1, ds_pred.rows(), 3);

        auto est = model.encode(ds);

        blaze_dm_to_csv(est, "estimation1.csv");

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




    // STL example, same with data sored in STL container
    std::cout << std::endl << std::endl << std::endl << " ---- the same with STL vectors: " << std::endl << std::endl;

    // read dataset and save model
    {
        auto start_time = std::chrono::steady_clock::now();

        std::vector<std::vector<value_type>> ds = read_csv_num<value_type>("training_ds_1_fragm.csv"); //, ",", 10000);
        std::vector<std::vector<value_type>> labels = {};
        for (size_t i = 0; i < ds.size(); ++i) {
            std::vector<value_type> el = ds[i];
            std::vector<value_type> l = {el[4]};
            labels.push_back(l);
            ds[i] = {el[1], el[2], el[3]}; // remove 1st and last columns
        }

        auto model = SwitchPredictor<value_type>(ds, labels);

        model.save("tmp2.blaze");

        auto end_time = std::chrono::steady_clock::now();
        std::cout << "training completed in " <<
                     double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                     std::endl << std::endl;


    }

    // load and apply the model
    {
        auto start_time = std::chrono::steady_clock::now();

        auto model = SwitchPredictor<value_type>("tmp2.blaze");

        auto end_time = std::chrono::steady_clock::now();
        std::cout << "model loaded in " <<
                     double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                     std::endl << std::endl;



        start_time = std::chrono::steady_clock::now();

        std::vector<std::vector<value_type>> ds = read_csv_num<value_type>("training_ds_2_fragm.csv"); //, ",", 10000);
        for (size_t i = 0; i < ds.size(); ++i) {
            std::vector<value_type> el = ds[i];
            ds[i] = {el[1], el[2], el[3]}; // remove 1st and last columns
        }

        auto est = model.encode(ds);

        v_to_csv(est, "estimation2.csv");

        end_time = std::chrono::steady_clock::now();
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
