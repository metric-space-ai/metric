

#include "modules/mapping/esn_switch_detector.hpp"


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

    std::vector<std::vector<value_type>> ds = read_csv_num<value_type>("training_ds_1_fragm.csv"); //, ",", 10000);
    std::vector<std::vector<value_type>> labels = {};
    for (size_t i = 0; i < ds.size(); ++i) {
        std::vector<value_type> el = ds[i];
        std::vector<value_type> l = {el[4]};
        labels.push_back(l);
        ds[i] = {el[1], el[2], el[3]}; // remove 1st and last columns
    }

    auto model = SwitchPredictor<value_type>(ds, labels);

    model.save("model.blaze");

    auto end_time = std::chrono::steady_clock::now();
    std::cout << "training completed in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;



    return 0;
}
