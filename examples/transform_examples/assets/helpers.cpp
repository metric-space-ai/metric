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
#include <cstring>
//#include <boost/gil/extension/io/png.hpp>  // for read_png_donut, may be enabled  // needs -lpng

#include "../../../3rdparty/blaze/Blaze.h"


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
        std::cout << "Failed to open file: " << filename << std::endl;
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
        for (auto i = 0; i < data.size(); ++i) {
            for (auto j = 0; j < data[i].size(); j++) {
                outputFile << std::to_string(data[i][j]) << sep;
            }
            outputFile << std::endl;
        }
        outputFile.close();
} // TODO add return flag



template <class ValueType>
void blaze_dm_to_csv(blaze::DynamicMatrix<ValueType> data, std::string filename, std::string sep=",")
{
    std::ofstream outputFile;
    outputFile.open(filename);
        for (auto i = 0; i < data.rows(); ++i) {
            for (auto j = 0; j < data.columns(); j++) {
                //outputFile << std::to_string(data(i, j)) << sep;
                outputFile << std::to_string(data(i, j));
                if (j < data.columns() - 1)
                    outputFile << sep;
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
blaze::DynamicMatrix<ValueType, blaze::rowMajor> read_csv_blaze(const std::string & filename, std::string sep = ";")
{
    auto array = read_csv<std::vector<std::vector<std::string>>>(filename, sep);
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






// ------ added for donut example


/*

blaze::DynamicMatrix<double> cv2blaze(cv::Mat in) {
    blaze::DynamicMatrix<double> out (in.rows, in.cols, 0);
    for (int y = 0; y<in.rows; y++) {
        for (int x = 0; x<in.cols; x++) {
            out(y, x) = in.at<double>(y, x);
        }
    }
    return out;
}

cv::Mat blaze2cv(blaze::DynamicMatrix<double> in) {
    cv::Mat out (in.rows(), in.columns(), cv::DataType<double>::type);
    for (std::size_t y = 0; y<in.rows(); y++) {
        for (std::size_t x = 0; x<in.columns(); x++) {
            out.at<double>(y, x) = in(y, x);
        }
    }
    return out;
}


// */


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
} // TODO add return flag



template <typename T>
void vector2bmp(std::vector<std::vector<T>> m, std::string filename, double amplify = 1)
{ // TODO combine with blaze2bmp

    int h = m.size();
    assert(h>0);
    int w = m[0].size();

    int x, y, r, g, b;

    FILE *f;
    unsigned char *img = NULL;
    int filesize = 54 + 3*w*h;

    img = (unsigned char *)malloc(3*w*h);
    std::memset(img,0,3*w*h);

    for(int i=0; i<w; i++)
    {
        for(int j=0; j<h; j++)
        {
            x=i; y=j;
            r = 0;
            g = 0;
            b = 0;
            int p = m[j][i]*255*amplify;
            if (p > 0) // green for positive, red for negative
            {
                g = p;
                b = p;
            }
            else
                r = -p;
            if (r > 255) r=255;
            if (g > 255) g=255;
            if (b > 255) b=255;
            img[(x+y*w)*3+2] = (unsigned char)(r);
            img[(x+y*w)*3+1] = (unsigned char)(g);
            img[(x+y*w)*3+0] = (unsigned char)(b);
        }
    }

    unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
    unsigned char bmppad[3] = {0,0,0};

    bmpfileheader[ 2] = (unsigned char)(filesize    );
    bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
    bmpfileheader[ 4] = (unsigned char)(filesize>>16);
    bmpfileheader[ 5] = (unsigned char)(filesize>>24);

    bmpinfoheader[ 4] = (unsigned char)(       w    );
    bmpinfoheader[ 5] = (unsigned char)(       w>> 8);
    bmpinfoheader[ 6] = (unsigned char)(       w>>16);
    bmpinfoheader[ 7] = (unsigned char)(       w>>24);
    bmpinfoheader[ 8] = (unsigned char)(       h    );
    bmpinfoheader[ 9] = (unsigned char)(       h>> 8);
    bmpinfoheader[10] = (unsigned char)(       h>>16);
    bmpinfoheader[11] = (unsigned char)(       h>>24);

    f = fopen(filename.c_str(),"wb");
    fwrite(bmpfileheader,1,14,f);
    fwrite(bmpinfoheader,1,40,f);
    for(int i=0; i<h; i++)
    {
        fwrite(img+(w*(h-i-1)*3),3,w,f);
        fwrite(bmppad,1,(4-(w*3)%4)%4,f);
    }

    free(img);
    fclose(f);
}



/* // disabled since requires boost png, may be enabled if dependency is satisfied

template <typename T>
blaze::DynamicMatrix<T> read_png_donut(std::string filename) {
    boost::gil::rgb8_image_t img;
    boost::gil::read_image(filename, img, boost::gil::png_tag());
    auto gray = boost::gil::color_converted_view<boost::gil::gray8_pixel_t>(const_view(img));

    blaze::DynamicMatrix<T> p (gray.height(), gray.width());
    for (int y=0; y<gray.height(); ++y) {
        for (int x=0; x<gray.width(); ++x) {
            p(y, x) = gray(x, y)/255.0;
        }
    }
    return p;
}

// */


template <typename T>
std::vector<std::vector<T>> matrix2vv(const blaze::DynamicMatrix<T> & mat) {
    std::vector<std::vector<double>> v;
    for (size_t i = 0; i < mat.rows(); ++i) {
        std::vector<double> line;
        for (size_t j = 0; j < mat.columns(); ++j) {
            line.push_back(mat(i, j));
        }
        v.push_back(line);
    }
    return v;
}






