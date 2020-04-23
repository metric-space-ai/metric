/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/

#ifndef _METRIC_UTILS_HPP
#define _METRIC_UTILS_HPP

namespace metric {



//-------- global settings



using DistanceType = double;



//-------- common type trait metafunctions
enum class Container {OTHER = 0, STL = 1, BLAZE = 2, EIGEN = 4};


template <typename>
struct container_type  // checks whether container is STL container (1) or Blaze vector (2)
{
    constexpr static int code = 0;
};

template <template <typename, typename> class Container, typename ValueType, typename Allocator>
struct container_type<Container<ValueType, Allocator>>
{
    constexpr static int code = 1; // STL
};

template <template <typename, bool> class Container, typename ValueType, bool F>
struct container_type<Container<ValueType, F>>
{
    constexpr static int code = 2; // blaze::DynamicVector
};

template <template <typename, int, int, int, int, int> class Container, typename ValueType, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
struct container_type<Container<ValueType, _Rows, _Cols, _Options, _MaxRows, _MaxCols>>
{
    constexpr static int code = 3; // Eigen::Array
};

template<typename C>
using container_type_c = typename container_type<C>::code;

template<typename C, int = container_type<C>::code>
struct contained_value  // determines type of element both for STL containers and Blaze vectors
{
    using type = void;
};

template<typename C>
struct contained_value<C, 1>
{
    using type = typename C::value_type;
};

template<typename C>
struct contained_value<C, 2>
{
    using type = typename C::ElementType;
};

template<typename C>
struct contained_value<C, 3>
{
    using type = typename C::Scalar;
};

template<typename C>
using contained_value_t = typename contained_value<C>::type;


template <typename>
struct isBlazeDynamicVector{
    constexpr static bool value = false;
};

template <typename ElementType, bool F>
struct isBlazeDynamicVector<blaze::DynamicVector<ElementType, F>> {
    constexpr static bool value = true;
};


// the same but universal, TODO test
template <typename>
struct isContainerOfType{
    constexpr static bool value = false;
};

template <template<typename...> class T, typename ElementType>
struct isContainerOfType<T<ElementType>> {
    constexpr static bool value = true;
};




template<typename C, int = container_type<C>::code>
struct isIterCompatibleStruct // specialized below all but 2 and 3
{
    constexpr static bool value = false;
};

template<typename C>
struct isIterCompatibleStruct<C, 1>
{
    constexpr static bool value = true;
};

template<typename C>
struct isIterCompatibleStruct<C, 4>
{
    constexpr static bool value = true;
};

template <typename T>
using isIterCompatible = std::enable_if_t<isIterCompatibleStruct<T>::value, DistanceType>;



template<typename C, int = container_type<C>::code>
struct isBlazeCompatibleStruct
{
    constexpr static bool value = false;
};

template<typename C>
struct isBlazeCompatibleStruct<C, 2>
{
    constexpr static bool value = true;
};

template <typename T>
using isBlazeCompatible = std::enable_if_t<isBlazeCompatibleStruct<T>::value, DistanceType>;



template<typename C, int = container_type<C>::code>
struct isEigenCompatibleStruct
{
    constexpr static bool value = false;
};

template<typename C>
struct isEigenCompatibleStruct<C, 3>
{
    constexpr static bool value = true;
};

template <typename T>
using isEigenCompatible = std::enable_if_t<isEigenCompatibleStruct<T>::value, DistanceType>;




}  // namespace metric


#endif // _METRIC_UTILS_HPP
