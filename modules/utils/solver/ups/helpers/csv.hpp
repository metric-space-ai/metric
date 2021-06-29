#ifndef _UPS_CSV_HELPERS_HPP
#define _UPS_CSV_HELPERS_HPP

#include "../3rdparty/metric/3rdparty/blaze/Blaze.h"

#include <string>
#include <fstream>
//#include <iostream>




template <typename T> T convertTo(const std::string & str)
{
    std::istringstream s(str);
    T num;
    s >> num;
    return num;
} // templated version of stof, stod, etc., thanks to https://gist.github.com/mark-d-holmberg/862733




template <template <class, bool> class BlazeContainerType, class ValueType>
bool readCsvBlaze(const std::string & filename, const BlazeContainerType<ValueType, blaze::rowMajor> & matrix, std::string sep = ",") {
    std::string line;
    int pos;
    std::ifstream in(filename);
    if(!in.is_open()) {
        //std::cout << "Failed to open file" << std::endl;
        return false;
    }
    size_t row_idx = 0;
    while( getline(in, line) ) {
        size_t col_idx = 0;
        while( (pos = line.find(sep) ) >= 0) {
            std::string field = line.substr(0, pos);
            //std::cout << field << "\n";
            //std::cout << line << "\n";
            line = line.substr(pos+1);
            matrix(row_idx, col_idx) = convertTo<ValueType>(field);
            ++col_idx;
        }
        matrix(row_idx, col_idx) = convertTo<ValueType>(line);
        ++row_idx;
    }
    return true;
}




template <class ValueType>
void writeCsvBlaze(std::string filename, const blaze::DynamicMatrix<ValueType> & data, std::string sep = ",")
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



#endif  // _UPS_CSV_HELPERS_HPP
