
#include "modules/utils/solver/ups/ups_solver/ups_solver.hpp"

#include "3rdparty/blaze/Blaze.h"

#include <tuple>
#include <vector>
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



//template <template <class, bool> class BlazeContainerType, class ValueType, bool SO>
//bool read_csv_blaze(const std::string & filename, BlazeContainerType<ValueType, SO> & matrix, std::string sep = ";") {
template <template <class, bool> class BlazeContainerType, class ValueType>
bool read_csv_blaze(const std::string & filename, BlazeContainerType<ValueType, blaze::columnMajor> & matrix, std::string sep = ",") {
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
            //std::cout << field << "\n";
            //std::cout << line << "\n";
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






int main()
{

    //blaze::CompressedMatrix<float, blaze::columnMajor> Mf;
    //blaze::CompressedMatrix<bool, blaze::columnMajor> M (165, 172);// = Mf;
    blaze::DynamicMatrix<bool, blaze::columnMajor> M (165, 172, 0);
    read_csv_blaze("mask.csv", M);
    std::cout << std::endl << M(0, 0) << " " << M(30, 30) << " " << blaze::nonZeros(M) << " " << blaze::sum(M) << std::endl;

//    {
//        blaze::CompressedMatrix<char, blaze::columnMajor> M2 (165, 172);
//        read_csv_blaze("mask.csv", M2);
//        std::cout << std::endl << M2(0, 0) << " " << M2(30, 30) << " " << blaze::nonZeros(M2) << " " << blaze::sum(M2) << std::endl;
//    }
//    {
//        blaze::CompressedMatrix<unsigned int, blaze::columnMajor> M2 (165, 172);
//        read_csv_blaze("mask.csv", M2);
//        std::cout << std::endl << M2(0, 0) << " " << M2(30, 30) << " " << blaze::nonZeros(M2) << " " << blaze::sum(M2) << std::endl;
//    }
//    {
//        blaze::CompressedMatrix<unsigned short, blaze::columnMajor> M2 (165, 172);
//        read_csv_blaze("mask.csv", M2);
//        std::cout << std::endl << M2(0, 0) << " " << M2(30, 30) << " " << blaze::nonZeros(M2) << " " << blaze::sum(M2) << std::endl;
//    }
//    {
//        blaze::CompressedMatrix<int, blaze::columnMajor> M2 (165, 172);
//        read_csv_blaze("mask.csv", M2);
//        std::cout << std::endl << M2(0, 0) << " " << M2(30, 30) << " " << blaze::nonZeros(M2) << " " << blaze::sum(M2) << std::endl;
//    }
//    {
//        blaze::CompressedMatrix<bool, blaze::columnMajor> M2 (165, 172);
//        read_csv_blaze("mask.csv", M2);
//        std::cout << std::endl << M2(0, 0) << " " << M2(30, 30) << " " << blaze::nonZeros(M2) << " " << blaze::sum(M2) << std::endl;
//    }

    blaze::DynamicMatrix<float, blaze::columnMajor> Z (165, 172, 0);
    read_csv_blaze("z_init.csv", Z);

    blaze::DynamicMatrix<float> K {
        {400,    0,  82.5},
        {  0,  400,    86},
        {  0,    0,     1}};


    // images

    blaze::DynamicMatrix<float, blaze::columnMajor> I1R (165, 172);
    blaze::DynamicMatrix<float, blaze::columnMajor> I2R (165, 172);
    blaze::DynamicMatrix<float, blaze::columnMajor> I3R (165, 172);
    blaze::DynamicMatrix<float, blaze::columnMajor> I4R (165, 172);
    read_csv_blaze("input_corrected1.csv", I1R);
    read_csv_blaze("input_corrected2.csv", I2R);
    read_csv_blaze("input_corrected3.csv", I3R);
    read_csv_blaze("input_corrected4.csv", I4R);


    std::vector<blaze::DynamicMatrix<float, blaze::columnMajor>> I1 {I1R, I1R, I1R};
    std::vector<blaze::DynamicMatrix<float, blaze::columnMajor>> I2 {I2R, I2R, I2R};
    std::vector<blaze::DynamicMatrix<float, blaze::columnMajor>> I3 {I3R, I3R, I3R};
    std::vector<blaze::DynamicMatrix<float, blaze::columnMajor>> I4 {I4R, I4R, I4R};

    std::vector<std::vector<blaze::DynamicMatrix<float, blaze::columnMajor>>> I {I1, I2, I3, I4};


    auto r = ups_solver(Z, M, K, I);

//    std::cout << std::endl << "final result:" << std::endl
//              << "z: " << std::endl << std::get<0>(r) << std::endl
//              << "rho1: " << std::endl << std::get<1>(r)[0] << std::endl
//              << "rho2: " << std::endl << std::get<1>(r)[1] << std::endl
//              << "rho3: " << std::endl << std::get<1>(r)[2] << std::endl;



    return 0;

}
