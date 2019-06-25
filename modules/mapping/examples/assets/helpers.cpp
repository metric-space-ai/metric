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
            std::string field = line.substr(0,pos);
            line = line.substr(pos+1);
            ln.push_back(field);
        }
        ln.push_back(line);
        array.push_back(ln);
    }
    return array;
}


