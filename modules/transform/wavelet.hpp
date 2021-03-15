/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Michael Welsch
*/

#ifndef _METRIC_TRANSFORM_WAVELET_HPP
#define _METRIC_TRANSFORM_WAVELET_HPP

#include <iostream>
#include <tuple>
#include <cmath>
#include <vector>
#include <algorithm>
#include <functional>
#include <deque>
#include "../../3rdparty/blaze/Blaze.h"

namespace wavelet {


namespace types {

// type traits // TODO move to some commonly included file


/**
 * @brief if T is a container and implemented operator[] value is true, otherwise value is false
 *
 * @tparam T checking type
 *
 */
template <typename T>
class has_index_operator {
    struct nil_t {
    };
    template <typename U>
    static constexpr auto test(U*) -> decltype(std::declval<U&>()[0]);
    template <typename>
    static constexpr auto test(...) -> nil_t;

public:
    using type = typename std::decay<decltype(test<T>(nullptr))>::type;
    static const bool value = !std::is_same<type, nil_t>::value;
};

/**
 * @brief extract returning type of operator[] in container, in case of STL containers is equivalent ot value_type
 * for example: underlying_type<std::vector<int>> == int,
 *              underlying_type<std::vector<std::vector<int>>> == std::vector<int>,

 * @tparam T Container type
 */
template <typename T>
using index_value_type_t = typename has_index_operator<T>::type;


}




template <typename Container2d, bool SO, bool F = blaze::IsDenseMatrix<Container2d>::value>
struct InternalBlazeType {}; // internal matrix and vector types of sparsity as Container2d, TODO use on dwt2 or remove

template <typename Container2d, bool SO>
struct InternalBlazeType <Container2d, SO, true> {
    using El = typename Container2d::ElementType;
    using vector_type = blaze::DynamicVector<El, SO>;
    using matrix_type = blaze::DynamicMatrix<El, SO>;
};

template <typename Container2d, bool SO>
struct InternalBlazeType <Container2d, SO, false> {
    using El = typename Container2d::ElementType;
    using vector_type = blaze::CompressedVector<El, SO>;
    using matrix_type = blaze::CompressedMatrix<El, SO>;
};

template <typename Container2d, bool SO = blaze::rowMajor>
using InternalBlazeVecT = typename InternalBlazeType<Container2d, SO>::vector_type;

template <typename Container2d, bool SO = blaze::rowMajor>
using InternalBlazeMatT = typename InternalBlazeType<Container2d, SO>::matrix_type;




// wavelet functions


///**
// * @brief valid convolution
// *
// * @param f
// * @param g
// * @return
// */
//template <typename T>
//std::vector<T> conv_valid(std::vector<T> const& f, std::vector<T> const& g);

/**
 * @brief valid convolution
 *
 * @param f
 * @param g
 * @return
 */
template <typename Container>
Container conv_valid(Container const& f, Container const& g); // overload added by Max F


///**
// * @brief full convolution
// *
// * @param f
// * @param g
// * @return
// */
//template <typename T>
//std::vector<T> conv(std::vector<T> const& f, std::vector<T> const& g);

/**
 * @brief full convolution
 *
 * @param f
 * @param g
 * @return
 */
template <typename Container>
Container conv(Container const& f, Container const& g); // overload added by Max F


/**
 * @brief linspace (erzeugt einen linearen Datenvektor)
 *
 * @param a
 * @param b
 * @param n
 * @return
 */
template <typename Container>
Container linspace(typename Container::value_type a, typename Container::value_type b, int n);


///**
// * @brief upsconv
// *
// * @param x
// * @param f
// * @param len
// * @return
// */
//template <typename T>
//std::vector<T> upsconv(std::vector<T> const& x, std::vector<T> const& f, int len);

/**
 * @brief upsconv
 *
 * @param x
 * @param f
 * @param len
 * @return
 */
template <typename Container>
Container upsconv(Container const& x, Container const& f, int len); // overload added by Max F


///**
// * @brief
// *
// * @param wnum
// * @param dings
// * @return
// */
//template <typename T>
//std::vector<T> dbwavf(int const wnum, T dings);

/**
 * @brief
 *
 * @param wnum
 * @param dings
 * @return
 */
template <typename Container>
//Container dbwavf(int const wnum, typename Container::value_type dings); // overload added by Max F
Container dbwavf(int const wnum, types::index_value_type_t<Container> dings); // overload added by Max F

///**
// * @brief
// *
// * @param W_in
// * @return
// */
//template <typename T>
//std::tuple<std::vector<T>, std::vector<T>, std::vector<T>, std::vector<T>> orthfilt(std::vector<T> const& W_in);

/**
 * @brief
 *
 * @param W_in
 * @return
 */
template <typename Container>
std::tuple<Container, Container, Container, Container> orthfilt(Container const& W_in); // added by Max F


///**
// * @brief
// *
// * @param x
// * @param waveletType
// * @return
// */
//template <typename T>
//std::tuple<std::vector<T>, std::vector<T>> dwt(std::vector<T> const& x, int waveletType);

/**
 * @brief
 *
 * @param x
 * @param waveletType
 * @return
 */
template <typename Container>
std::tuple<Container, Container> dwt(Container const& x, int waveletType); // overload added by Max F


///**
// * @brief
// *
// * @param a
// * @param d
// * @param waveletType
// * @param lx
// * @return
// */
//template <typename T>
//std::vector<T> idwt(std::vector<T> a, std::vector<T> d, int waveletType, int lx);

/**
 * @brief
 *
 * @param a
 * @param d
 * @param waveletType
 * @param lx
 * @return
 */
template <typename Container>
Container idwt(Container a, Container d, int waveletType, int lx); // overload added by Max F, called in DSPCC


/**
 * @brief 
 * 
 * @param sizeX 
 * @param waveletType 
 * @return
 */
static int wmaxlev(int sizeX, int waveletType);

///**
// * @brief
// *
// * @param x
// * @param order
// * @param waveletType
// * @return
// */
//template <typename T>
//std::deque<std::vector<T>> wavedec(std::vector<T> const& x, int order, int waveletType);

/**
 * @brief
 *
 * @param x
 * @param order
 * @param waveletType
 * @return
 */
template <typename Container>
std::deque<Container> wavedec(Container const& x, int order, int waveletType);




///**
// * @brief
// *
// * @param subBands
// * @param waveletType
// * @return
// */
//template <typename T>
//std::vector<T> waverec(std::deque<std::vector<T>> const& subBands, int waveletType);

/**
 * @brief
 *
 * @param subBands
 * @param waveletType
 * @return
 */
template <typename Container>
Container waverec(std::deque<Container> const& subBands, int waveletType);





///**
// * @brief
// *
// * @param data
// * @param tresh
// * @return
// */
//template <typename T>
//std::deque<std::vector<T>> denoise(std::deque<std::vector<T>> const& data, T const& tresh); // not implemented


/**
 * @brief
 *
 * @param data
 * @param tresh
 * @return
 */
template <typename Container>
std::deque<Container> denoise(std::deque<Container> const& data, typename Container::value_type const& tresh); // not implemented




///**
// * @brief
// *
// * @param data
// * @return
// */
//template <typename T>
//std::tuple<std::deque<std::vector<T>>, std::deque<std::vector<T>>> sparse(std::deque<std::vector<T>> const& data);  // not implemented


/**
* @brief
*
* @param data
* @return
*/
template <typename Container>
std::tuple<std::deque<Container>, std::deque<Container>> sparse(std::deque<Container> const& data);  // not implemented



/**
 * @brief distance measure by time elastic cost matrix.
 * 
 * @param As 
 * @param Bs 
 * @param penalty 
 * @param elastic 
 * @return 
 */
template <typename T>
//T TWED(blaze::CompressedVector<T> const& As, blaze::CompressedVector<T> const& Bs, T const& penalty = 0,
//    T const& elastic = 1); // original code
T TWED(blaze::CompressedVector<T> const& As, blaze::CompressedVector<T> const& Bs, T const& penalty,
    T const& elastic);  // edited by Max F because of "redefinition of default argument" compile-time error. I hope this does not break anything..


// 2d functions


///**
// * @brief non-blaze dwt2
// *
// * @param x
// * @param waveletType
// * @return
// */
//template <typename Container>
//dwt2(std::vector<Container> const & x, int waveletType);

/**
 * @brief blaze matrix overload of dwt2
 *
 * @param x
 * @param waveletType
 * @return
 */
template <typename Container>
typename std::enable_if<
 !blaze::IsMatrix<
  Container>::value,
  std::tuple<std::vector<Container>, std::vector<Container>, std::vector<Container>, std::vector<Container>>
 >::type
dwt2(std::vector<Container> const & x, int waveletType);


/**
 * @brief
 *
 * @param x
 * @param waveletType
 * @return
 */
template <typename Container2d>
//std::tuple<Container2d, Container2d, Container2d, Container2d>
typename std::enable_if<
 blaze::IsMatrix<Container2d>::value,
 std::tuple<Container2d, Container2d, Container2d, Container2d>
>::type
dwt2(Container2d const & x, int waveletType);



/**
 * @brief
 *
 * @param ll
 * @param lh
 * @param hl
 * @param hh
 * @param waveletType
 * @param hx
 * @param wx
 * @return
 */
template <typename Container>
std::vector<Container> idwt2(
            std::vector<Container> const & ll,
            std::vector<Container> const & lh,
            std::vector<Container> const & hl,
            std::vector<Container> const & hh,
            int waveletType,
            int hx,
            int wx);

/**
 * @brief
 *
 * @param in
 * @param waveletType
 * @param hx
 * @param wx
 * @return
 */
template <typename Container>
std::vector<Container> idwt2(
            std::tuple<std::vector<Container>, std::vector<Container>, std::vector<Container>, std::vector<Container>> in,
            int waveletType,
            int hx,
            int wx);


/**
 * @brief
 *
 * @param ll
 * @param lh
 * @param hl
 * @param hh
 * @param waveletType
 * @param hx
 * @param wx
 * @return
 */
template <typename Container2d>
//Container2d idwt2(
typename std::enable_if<blaze::IsMatrix<Container2d>::value, Container2d>::type idwt2(
            Container2d const & ll,
            Container2d const & lh,
            Container2d const & hl,
            Container2d const & hh,
            int waveletType,
            int hx,
            int wx);




}  // namespace


#include "wavelet.cpp"

#endif
