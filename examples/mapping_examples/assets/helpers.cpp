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
ContainerType read_csv(std::string filename) {
    typedef typename ContainerType::value_type LINE;
    std::string line;
    int pos;
    ContainerType array = {};
    std::ifstream in(filename);
    if(!in.is_open()) {
        std::cout << "Failed to open file" << std::endl;
        return array;
    }
    while( getline(in,line) ) {
        LINE ln;
        while( (pos = line.find(',')) >= 0)	{
            std::string field = line.substr(0, pos);
            line = line.substr(pos+1);
            ln.push_back(field);
        }
        ln.push_back(line);
        array.push_back(ln);
    }
    return array;
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


