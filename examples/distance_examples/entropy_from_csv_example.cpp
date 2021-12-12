#include "modules/correlation/entropy.hpp"

//#include <iostream>
//#include <fstream>
//#include <vector>
//#include <string>
//#include <sstream>


template <typename T> T convert_to(const std::string & str)
{ // copied from helpers.cpp
    std::istringstream s(str);
    T num;
    s >> num;
    return num;
} // templated version of stof, stod, etc., thanks to https://gist.github.com/mark-d-holmberg/862733


template <class ValueType>
std::vector<std::vector<ValueType>> read_csv_num(std::string filename, std::string sep=";")
{ // copied from helpers.cpp
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
        while( (pos = line.find(sep)) >= 0)	{
            std::string field = line.substr(0, pos);
            line = line.substr(pos+1);
            ln.push_back(convert_to<ValueType>(field));
        }
        ln.push_back(convert_to<ValueType>(line));
        array.push_back(ln);
    }
    return array;
}


int main() {

    //auto x_cyr = read_csv_num<double>("assets/x_cyr.csv", ",");
    //auto x_sin = read_csv_num<double>("assets/x_sin.csv", ",");
    auto x_cyr = read_csv_num<double>("assets/x_uni_3d.csv", ",");
    auto x_sin = read_csv_num<double>("assets/x_saw_3d.csv", ",");

    //std::cout << "\nx_cyr of size " << x_cyr.size() << ":\n";
    std::cout << "\nx_uni of size " << x_cyr.size() << ":\n";
    for (size_t i = 0; i<10; ++i) {
        for (size_t j = 0; j<x_cyr[0].size(); ++j) {
            std::cout << x_cyr[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "...\n";
    //std::cout << "\nx_sin of size " << x_sin.size() << ":\n";
    std::cout << "\nx_saw of size " << x_sin.size() << ":\n";
    for (size_t i = 0; i<10; ++i) {
        for (size_t j = 0; j<x_sin[0].size(); ++j) {
            std::cout << x_sin[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "...\n\n";

    //auto estimator = metric::Entropy<std::vector<double>>();
    auto estimator = metric::Entropy<void, metric::Euclidean<double>>();

//    std::cout << "entropy(x_cyr): " << estimator(x_cyr) << std::endl;
//    std::cout << "entropy(x_sin): " << estimator(x_sin) << std::endl;
    std::cout << "entropy(x_uni): " << estimator(x_cyr) << std::endl;
    std::cout << "entropy(x_saw): " << estimator(x_sin) << std::endl;

    return 0;
}
