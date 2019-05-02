#include <vector>
#include <deque>
#include <array>
#include <string>

#include <iostream>
#include <fstream>

#include <chrono>

#include "../metric_correlation.hpp"
#include "../libs/blaze/Math.h"

template <typename T>
void matrix_print(const std::vector<std::vector<T>> &mat)
{

    std::cout << "[";
    for (int i = 0; i < mat.size(); i++)
    {
        for (int j = 0; j < mat[i].size() - 1; j++)
        {
            std::cout << mat[i][j] << ", ";
        }
        std::cout << mat[i][mat[i].size() - 1] << " ]" << std::endl;
        ;
    }
}

template <typename T>
void vector_print(const std::vector<T> &vec)
{

    std::cout << "[";
    for (int i = 0; i < vec.size(); i++)
    {
        std::cout << vec[i] << ", ";
    }
    std::cout << vec[vec.size() - 1] << " ]" << std::endl;
}

struct simple_user_euclidian
{
    double operator()(const std::vector<double> &a, const std::vector<double> &b) const
    {
        double sum = 0;
        for (size_t i = 0; i < a.size(); ++i)
        {
            sum += (a[i] - b[i]) * (a[i] - b[i]);
        }
        return std::sqrt(sum);
    }
};

struct blaze_euclidean
{
    double operator()(const blaze::DynamicVector<double> &p, const blaze::DynamicVector<double> &q) const
    {
        return blaze::l2Norm(p - q);
    }
};

std::vector<std::vector<double>> read_csv(const std::string filename, const long length = -1)
{
    std::vector<std::vector<double>> v;

    std::ifstream in(filename);
    if (!in.is_open())
    {
        std::cout << "Failed to open " << filename << std::endl;
        return v;
    }

    size_t count = 0;
    std::string record;
    while (std::getline(in, record))
    {
        if ((length > 0) and (count++ >= length))
        {
            break;
        }

        std::istringstream is(record);
        std::vector<double> row((std::istream_iterator<double>(is)),
                                std::istream_iterator<double>());
        v.push_back(row);
    }

    // for ( const auto &row : v )
    // {
    //     for ( double x : row ) std::cout << x << ' ';
    //     std::cout << std::endl;
    // }

    return v;
}

int main(int argc, char *argv[])
{
    using RecType = std::vector<double>;
    using RecType2 = blaze::DynamicVector<double>;

    if (argc < 4)
    {
        std::cout << "usage: " << argv[0] << " data-1.csv data-2.csv limit" << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<RecType> data1 = read_csv(argv[1], atoi(argv[3]));
    std::vector<RecType> data2 = read_csv(argv[2], atoi(argv[3]));
    if (data1.empty() or data2.empty())
    {
        return EXIT_FAILURE;
    }

    std::cout << "rows: " << data1.size() << ", cols: " << data1[0].size() << std::endl;
    std::cout << "rows: " << data2.size() << ", cols: " << data2[0].size() << std::endl;

    std::vector<RecType2> d1(data1.size());
    std::vector<RecType2> d2(data2.size());

    for (size_t i = 0; i < data1.size(); ++i)
    {
        blaze::DynamicVector<double> tmp(1UL, data1[i].size());
        for (size_t j = 0; j < data1[i].size(); ++j)
        {
            tmp[j] = data1[i][j];
        }
        d1[i] = tmp;
    }

    for (size_t i = 0; i < data2.size(); ++i)
    {
        blaze::DynamicVector<double> tmp(1UL, data2[i].size());
        for (size_t j = 0; j < data2[i].size(); ++j)
        {
            tmp[j] = data2[i][j];
        }
        d2[i] = tmp;
    }

    /* Build functors (function objects) with user types and metrics */
    typedef simple_user_euclidian Met;
    typedef blaze_euclidean Met2;

    /* Set up the correlation function */
    auto mgc_corr = metric::correlation::MGC<RecType, Met, RecType, Met>();

    /* Compute distance matrix 1 */
    // auto dist1= metric::correlation::blaze_distance_matrix<Met2>()(d1);
    // auto dist2= metric::correlation::blaze_distance_matrix<Met2>()(d2);

    // double result2= metric::correlation::MGC_direct()(dist1,dist2);

    /* Compute and benchmark */
    auto t1 = std::chrono::steady_clock::now();
    //auto result = mgc_corr(data1, data2); // A1 = std::vector<...>, A2 = std::deque<...>
    auto result = mgc_corr.estimate(data1, data2, 0, 0, 0); // A1 = std::vector<...>, A2 = std::deque<...>
    //auto result = metric::correlation::MGC()(data1, data2);
    //auto result = metric::correlation::MGC<RecType2, Met2, RecType2, Met2>()(d1, d2);
    auto t2 = std::chrono::steady_clock::now();
    std::cout << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;

    return 0;
}
