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



template <typename>
struct determine_container_type  // checks whether container is STL container (1) or Blaze vector (2)
{
    constexpr static int code = 0;
};

template <template <typename, typename> class Container, typename ValueType, typename Allocator>
struct determine_container_type<Container<ValueType, Allocator>>
{
    constexpr static int code = 1; // STL
};

template <template <typename, bool> class Container, typename ValueType, bool F>
struct determine_container_type<Container<ValueType, F>>
{
    constexpr static int code = 2; // blaze::DynamicVector
};

template <template <typename, int, int, int, int, int> class Container, typename ValueType, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
struct determine_container_type<Container<ValueType, _Rows, _Cols, _Options, _MaxRows, _MaxCols>>
{
    constexpr static int code = 3; // Eigen::Array
};



template<typename C, int = determine_container_type<C>::code>
struct determine_ValueType  // determines type of element both for STL containers and Blaze vectors
{
    using type = void;
};

template<typename C>
struct determine_ValueType<C, 1>
{
    using type = typename C::value_type;
};

template<typename C>
struct determine_ValueType<C, 2>
{
    using type = typename C::ElementType;
};

template<typename C>
struct determine_ValueType<C, 3>
{
    using type = typename C::Scalar;
};




}  // namespace metric


#endif // _METRIC_UTILS_HPP
