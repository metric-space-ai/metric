
#include "metric/mapping/deterministic_switch_detector.hpp"

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







int main()
{

    using value_type = float; //double;

    blaze::DynamicMatrix<value_type> ds = read_csv_blaze<value_type>("training_ds_2_fragm.csv"); //, ",", 10000);
    //blaze::DynamicMatrix<value_type> ds = read_csv_blaze<value_type>("slice.csv"); //, ",", 10000);
    //blaze::DynamicMatrix<value_type> ds = read_csv_blaze<value_type>("slice_small.csv"); //, ",", 10000);

    // expected format of (comma separated) csv: unused_field, r, g, b, optional_other_fields
    // e.g.:
    // 1618831933393,53,49,28
    // 1618831933395,53,50,28
    // 1618831933397,53,50,27
    // ...
    // or
    // 1618831933393,53,49,28,0.0
    // 1618831933395,53,50,28,0.0
    // 1618831933397,53,50,27,0.0
    // ...


    //blaze_dm_to_csv(ds, "input.csv"); // resave with fixed name for ploting script

    auto d = DetSwitchDetector<value_type>();

    auto switches = d.encode(ds);

    blaze_dm_to_csv(switches, "switches.csv");
    // single column of switch flags: 1 - on, -1 - off, 0 - no switch


    return 0;
}

