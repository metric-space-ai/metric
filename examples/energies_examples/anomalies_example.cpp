
#include "modules/mapping/clustering_som_anomaly_detector.hpp"
#include "modules/mapping/PCFA.hpp"


#include <iostream>




// ----- csv reader


template <typename T> T convert_to(const std::string & str)
{
    std::istringstream s(str);
    T num;
    s >> num;
    return num;
} // templated version of stof, stod, etc., thanks to https://gist.github.com/mark-d-holmberg/862733


template <class ValueType>
std::vector<std::vector<ValueType>> read_csv_num(std::string filename, std::string sep=",")
{
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


template <class ContainerType>
void v_to_csv(ContainerType data, std::string filename, std::string sep=",")  // container
{
    std::ofstream outputFile;
    outputFile.open(filename);
        for (auto i = 0; i < data.size(); ++i) {
            outputFile << std::to_string(data[i]) << std::endl;
        }
        outputFile.close();
}


template <class ContainerType>
void vv_to_csv(ContainerType data, std::string filename, std::string sep=",")  // container
{
    std::ofstream outputFile;
    outputFile.open(filename);
        for (auto i = 0; i < data.size(); ++i) {
            for (auto j = 0; j < data[i].size(); j++) {
                outputFile << std::to_string(data[i][j]);
                if (j < data[i].size() - 1)
                    outputFile << sep;
            }
            outputFile << std::endl;
        }
        outputFile.close();
}


// ------ helper functions (copied from energies_example.cpp)


// ------ vector to console


template<typename T>
std::ostream & operator<<(std::ostream & out, std::vector<T> const & a) {
    out << "[ " << std::endl;
    for(auto x : a) {
        out << x << std::endl;
    }
    out << "]" << std::endl;
    return out;
}


template<typename T>
std::ostream & operator<<(std::ostream & out, std::vector<std::vector<T>> const & a) {
    out << "[ " << std::endl;
    for(auto x : a) {
        out << x << std::endl;
    }
    out << "]" << std::endl;
    return out;
}




// ------

//*
int main() {

    using T = double;

    //auto ds = read_csv_num<T>("anomaly_detector_data_1/script/energies01_short.csv");
    auto ds_raw = read_csv_num<T>("anomaly_detector_data_1/script/cat_energies_100ms_sp8.csv");

    // PCA
    auto pcfa = metric::PCFA<std::vector<T>, void>(ds_raw, 30);
    auto ds = pcfa.encode(ds_raw);

    auto csad = metric::ClusteringSomAnomalyDetector(ds);
    csad.save("class_model.json");

//    auto csad2 = metric::ClusteringSomAnomalyDetector<T>("class_model.json");

    auto out1 = csad.encode(ds);
//    auto out2 = csad2.encode(ds);

//    std::cout << "out1:" << std::endl;
//    std::cout << out1 << std::endl;
//    std::cout << "out2:" << std::endl;
//    std::cout << out2 << std::endl;

    //std::vector<std::vector<T>> out = {out1, out2};
    v_to_csv(out1, "anomaly_detector_data_1/script/class_out_score.csv");


    std::cout << "done" << std::endl;
}
// */


/*
int main() {

    using T = double;

    std::vector<T> sample = {0, 1, 2, 3, 3, 4, 4.5, 5, 8, 9};  // {7,1,2,2,7,4,4,4,5.5,7}; //
    std::sort(sample.begin(), sample.end());

    auto dcdf = metric::clustering_som_anomaly_detector_details::discrete_cdf(sample, 6);

    std::cout << sample << std::endl;
    std::cout << std::get<0>(dcdf) << std::endl;
    std::cout << std::get<1>(dcdf) << std::endl;


}
// */


/*
int main() {

    using T = double;

    std::vector<T> sample1 = {1, 2, 2, 5};
    std::vector<T> sample2 = {3, 4, 4, 7};

    std::sort(sample1.begin(), sample1.end());
    std::sort(sample2.begin(), sample2.end());

    auto dcdf1 = metric::clustering_som_anomaly_detector_details::discrete_cdf(sample1);
    auto dcdf2 = metric::clustering_som_anomaly_detector_details::discrete_cdf(sample2);

    std::cout << std::endl;
    std::cout << sample1 << std::endl;
    std::cout << std::get<0>(dcdf1) << std::endl;
    std::cout << std::get<1>(dcdf1) << std::endl;

    std::cout << std::endl;
    std::cout << sample2 << std::endl;
    std::cout << std::get<0>(dcdf2) << std::endl;
    std::cout << std::get<1>(dcdf2) << std::endl;

    auto d = metric::clustering_som_anomaly_detector_details::discrete_randomEMD(dcdf1, dcdf2);

    std::cout << std::endl;
    std::cout << d << std::endl;

}
// */



