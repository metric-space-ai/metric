#define BOOST_TEST_MODULE dwt_tests
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>
#include <vector>
//#include <deque>
//#include <array>
//#include <numeric>
//#include <stdexcept>
//#include "3rdparty/blaze/Blaze.h"

#include "modules/transform/wavelet.hpp"


using DM = blaze::DynamicMatrix<double>;
using SM = blaze::CompressedMatrix<double>;
using VBM = std::vector<blaze::DynamicVector<double>>;
using VVM = std::vector<std::vector<double>>;
//using MatrixTypes = boost::mpl::list<DM, SM, VBM, VVM>; //TODO fix old container support
using MatrixTypes = boost::mpl::list<DM, SM>;

using DV = blaze::DynamicVector<double>;
using SV = blaze::CompressedVector<double>;
using VectorTypes = boost::mpl::list<DV, SV>;

BOOST_AUTO_TEST_CASE_TEMPLATE(dwt2, T, MatrixTypes) {

    T data2d = {
        { 0, 0, 0, 0, 0, 0, 1, 2, 3, 4 },
        { 0, 0, 0, 0, 0, 0, 8, 7, 6, 5 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 5, 1, 5, 0, 0, 0, 0, 0 },
        { 0, 0, 1, 5, 1, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, -2, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
        { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
        { 1, 1, -1, 0, 0, 0, 0, 0, 0, 0 },
    };
    std::tuple<T, T, T, T> splitted;
    T restored;
    BOOST_CHECK_NO_THROW(splitted = wavelet::dwt2(data2d, 4););
    BOOST_CHECK_NO_THROW(restored = wavelet::idwt2(std::get<0>(splitted), std::get<1>(splitted), std::get<2>(splitted), std::get<3>(splitted), 4, data2d.rows(), data2d.columns()););
    auto maxdiff = double(blaze::max(blaze::abs(restored - data2d)));
    BOOST_TEST( maxdiff <= std::numeric_limits<double>::epsilon()*1e6 );
}


BOOST_AUTO_TEST_CASE(dwt) {
    using T = blaze::DynamicVector<double>;
    T data = {1, 2, 3, 4, 1, 0, 20, 0, 0, 5, -10, 0, -4, -2, 0, 0, 0, 0, 0, 1, 2, 3, 4};
    std::tuple<T, T> splitted;
    T restored;
    int wavelet = 4;
    BOOST_CHECK_NO_THROW(splitted = wavelet::dwt(data, wavelet););
    BOOST_CHECK_NO_THROW(restored = wavelet::idwt(std::get<0>(splitted), std::get<1>(splitted), wavelet, data.size()););
    auto maxdiff = double(blaze::max(blaze::abs(restored - data)));
    BOOST_TEST( maxdiff <= std::numeric_limits<double>::epsilon()*1e6 );
}


