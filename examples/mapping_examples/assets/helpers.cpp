/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Panda Team
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#include "3rdparty/blaze/Blaze.h"


template <typename T> T convert_to(const std::string & str)
{
    std::istringstream s(str);
    T num;
    s >> num;
    return num;
} // templated version of stof, stod, etc., thanks to https://gist.github.com/mark-d-holmberg/862733



template <class ContainerType>
ContainerType read_csv(std::string filename, std::string sep=",")
{  // works with string, does not convert to numbers
    typedef typename ContainerType::value_type LINE;
    std::string line;
    int pos;
    ContainerType array = {};
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cout << "Failed to open file" << std::endl;
        return array;
    }
    while (getline(in, line)) {
        LINE ln;
        while( (pos = line.find(sep)) >= 0)	{
            std::string field = line.substr(0, pos);
            line = line.substr(pos+1);
            ln.push_back(field);
        }
        ln.push_back(line);
        array.push_back(ln);
    }
    return array;
}








template <class ContainerType>
void write_csv(ContainerType data, std::string filename, std::string sep=",")  // container of containers expected, TODO add check
{
    std::ofstream outputFile;
    outputFile.open(filename);
        for (auto i=0; i<data.size(); ++i) {
            for (auto j = 0; j < data[i].size(); j++) {
                outputFile << std::to_string(data[i][j]) << sep;
            }
            outputFile << std::endl;
        }
        outputFile.close();
} // TODO add return flag





template <class ValueType>
std::vector<std::vector<ValueType>> read_csv_num(std::string filename, std::string sep=";")
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



template <class ValueType>
blaze::DynamicMatrix<ValueType, blaze::rowMajor> read_csv_blaze(const std::string & filename)
{
    auto array = read_csv<std::vector<std::vector<std::string>>>(filename, ";");
    auto m = blaze::DynamicMatrix<ValueType, blaze::rowMajor>(array.size(), array[0].size());
    for (size_t i=0; i<array.size(); ++i)
        for (size_t j=0; j<array[0].size(); ++j)
            m(i, j) = convert_to<ValueType>(array[i][j]);
    return m;
}



//template <template <class, bool> class BlazeContainerType, class ValueType, bool SO>
//bool read_csv_blaze(const std::string & filename, BlazeContainerType<ValueType, SO> & matrix, std::string sep = ";") {
template <template <class, bool> class BlazeContainerType, class ValueType>
bool read_csv_blaze(const std::string & filename, BlazeContainerType<ValueType, blaze::rowMajor> & matrix, std::string sep = ";") {
    //typedef typename std::vector<std::string> LINE;
    std::string line;
    int pos;
    //std::vector<LINE> array = {};
    std::ifstream in(filename);
    if(!in.is_open()) {
        std::cout << "Failed to open file" << std::endl;
        return false;
    }
    size_t row_idx = 0;
    while( getline(in, line) ) {
        //LINE ln;
        size_t col_idx = 0;
        while( (pos = line.find(sep) ) >= 0) {
            std::string field = line.substr(0, pos);
            std::cout << field << "\n";
            std::cout << line << "\n";
            line = line.substr(pos+1);
            //ln.push_back(field);
            matrix(row_idx, col_idx) = convert_to<ValueType>(field);
            ++col_idx;
        }
        matrix(row_idx, col_idx) = convert_to<ValueType>(line);
        ++row_idx;
        //ln.push_back(line);
        //array.push_back(ln);
    }
    return true;
}


