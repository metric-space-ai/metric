

#include "../../../../modules/mapping/ESN.hpp"

#include "../../../../modules/utils/visualizer.hpp"

#include <iostream>
#include <fstream>

#ifndef M_PI
        // MSVC does not define M_PI
    #define M_PI 3.14159265358979323846
#endif


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



template <class ValueType>
blaze::DynamicMatrix<ValueType, blaze::rowMajor> read_csv_blaze(const std::string & filename, std::string sep = ",")
{
    auto array = read_csv<std::vector<std::vector<std::string>>>(filename, sep);
    auto m = blaze::DynamicMatrix<ValueType, blaze::rowMajor>(array.size(), array[0].size());
    for (size_t i=0; i<array.size(); ++i)
        for (size_t j=0; j<array[0].size(); ++j)
            m(i, j) = convert_to<ValueType>(array[i][j]);
    return m;
}









int main()
{


    std::cout << "started" << std::endl << std::endl;

    bool visualize = false;  // only for small datasets that can be represented in an image point to point

    using value_type = double;


    // dataset passed as Blaze matrix, data points in COLUMNS

    auto start_time = std::chrono::steady_clock::now();

//    blaze::DynamicMatrix<value_type> ds = blaze::trans(read_csv_blaze<value_type>("rgblog_fragm_labeled.csv"));
//    blaze::DynamicMatrix<value_type> ds = blaze::trans(read_csv_blaze<value_type>("rgblog_labeled_short.csv"));
    blaze::DynamicMatrix<value_type> ds = blaze::trans(read_csv_blaze<value_type>("training_ds_1.csv"));

    std::cout << "training dataset read & transposed" << std::endl << std::endl;

    //blaze::DynamicMatrix<value_type, blaze::rowMajor> data = blaze::submatrix(ds, 0, 0, 3, ds.columns());
    blaze::DynamicMatrix<value_type, blaze::rowMajor> data = blaze::submatrix(ds, 1, 0, 3, ds.columns());
    // first COLUMN represents zero time moment, second represents time = 1, etc

    std::cout << "data selected" << std::endl << std::endl;

    //blaze::DynamicMatrix<value_type, blaze::rowMajor> target = blaze::submatrix(ds, 3, 0, 1, ds.columns());
    blaze::DynamicMatrix<value_type, blaze::rowMajor> target = blaze::submatrix(ds, 4, 0, 1, ds.columns());

    std::cout << "label selected" << std::endl << std::endl;

    //blaze::DynamicMatrix<value_type, blaze::rowMajor>  test_data =
    //        blaze::trans(read_csv_blaze<value_type>("rgblog_test.csv"));
    blaze::DynamicMatrix<value_type, blaze::rowMajor>  test_data = data;


    std::cout << "test dataset read & transposed" << std::endl << std::endl;

    auto end_time = std::chrono::steady_clock::now();
    std::cout << "data parepared in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;

    if (visualize)
    {
        mat2bmp::blaze2bmp_norm(data, "ESN_SlicesR.bmp");
        mat2bmp::blaze2bmp_norm(target, "ESN_TargetR.bmp");
        mat2bmp::blaze2bmp_norm(test_data, "ESN_SlicesTestR.bmp");
    }

    start_time = std::chrono::steady_clock::now();

    auto esn = metric::ESN<std::vector<value_type>, void>(500, 10, 0.99, 0.1, 2500, 0.5); // echo
    //auto esn = metric::ESN<std::vector<value_type>, void>(500, 4, 0.99, 1, 0, 0.9); // no echo (alpha=1 and no washout)
    // w_size, w_connections, w_sr, alpha, washout, beta
    esn.train(data, target);

    end_time = std::chrono::steady_clock::now();
    std::cout << "training completed in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;


    start_time = std::chrono::steady_clock::now();

    esn.save("esn_image.blaze");

    end_time = std::chrono::steady_clock::now();
    std::cout << "network saved in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;


    start_time = std::chrono::steady_clock::now();

    auto prediction = esn.predict(test_data);

    end_time = std::chrono::steady_clock::now();
    std::cout << "prediction completed in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;


    start_time = std::chrono::steady_clock::now();

    blaze::DynamicMatrix<value_type, blaze::rowMajor> out = blaze::trans(prediction);
    blaze_dm_to_csv(out, "prediction.csv");

    end_time = std::chrono::steady_clock::now();
    std::cout << "prediction written in " <<
                 double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                 std::endl << std::endl;

    if (visualize)
        mat2bmp::blaze2bmp_norm(prediction, "ESN_prediction.bmp");


    std::cout << "all done" << std::endl;

    return 0;

}
