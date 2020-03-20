/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#ifndef METRIC_UTILS_TYPE_TRAITS_HPP
#define METRIC_UTILS_TYPE_TRAITS_HPP
#include <utility>
#include <type_traits>

namespace metric::type_traits {
/**
 * @brief if T is a container and implemented operator[] value is true, otherwise value is false
 *
 * @tparam T checking type
 *
 */
template <typename T>
class is_has_index_operator {
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
 * @brief if T is a class that has resize() method value is true, otherwise value is false
 *
 * @tparam T checking type
 */
template <typename T>
class is_has_resize_method {
    struct nil_t {
    };
    template <typename U>
    static constexpr auto test(U*) -> decltype(std::declval<U&>().resize(0));
    template <typename>
    static constexpr auto test(...) -> nil_t;

public:
    using type = typename std::decay<decltype(test<T>(nullptr))>::type;
    static const bool value = !std::is_same<type, nil_t>::value;
};

/**
 * @brief extract returning type of operator[] in container, in case of STL containers is equivalent ot value_type
 * for example: underlaying_type<std::vector<int>> == int,
 *              underlaying_type<std::vector<std::vector<int>>> == std::vector<int>,
 
 * @tparam T Container type
 */
template <typename T>
using index_value_type_t = typename is_has_index_operator<T>::type;

/**
 * @brief helper for is_has_index_operator metafunction
 */
template <typename T>
inline constexpr bool is_has_index_operator_v = is_has_index_operator<T>::value;

/**
 * @brief helper for is_has_resize_method metafunction
 */
template <typename T>
inline constexpr bool is_has_resize_method_v = is_has_resize_method<T>::value;

/**
 * @brief if T is a container supported [] operator and underlaying type of container is integral type
 * value is true, otherwise value is false
 */
template <typename T>
struct is_container_of_integrals {
    static constexpr auto test()
    {
        if constexpr (is_has_index_operator_v<T>) {
            using T1 = index_value_type_t<T>;
            if constexpr (is_has_index_operator_v<T1>) {
                using T2 = index_value_type_t<T1>;
                return std::is_integral_v<T2>;
            } else {
                return std::is_integral_v<T1>;
            }
        } else {
            return false;
        }
    }
    static const bool value = test();
};

/**
 * @brief helper for is_container_of_integrals metafunction
 */
template <typename T>
inline constexpr bool is_container_of_integrals_v = is_container_of_integrals<T>::value;

template <typename T>
struct underlaying_type_impl0 {
    using type = typename std::decay<T>::type;
};
template <typename T>
using underlaying0_t = typename underlaying_type_impl0<T>::type;

template <typename T>
struct underlaying_type_impl1 {
    using type = typename std::decay<typename is_has_index_operator<T>::type>::type;
};
template <typename T>
using underlaying1_t = typename underlaying_type_impl1<T>::type;

template <typename T>
struct underlaying_type_impl2 {
    using type =
        typename std::decay<typename is_has_index_operator<typename is_has_index_operator<T>::type>::type>::type;
};
template <typename T>
using underlaying2_t = typename underlaying_type_impl2<T>::type;

/**
 * @brief extract underlaying type of container,
 * for example: underlaying_type<std::vector<int>> == int,
 *              underlaying_type<std::vector<std::vector<int>>> == int,
 *
 */
template <typename T>
struct underlaying_type {
    static constexpr auto level() -> int
    {
        if constexpr (is_has_index_operator_v<T>) {
            using T1 = index_value_type_t<T>;
            if constexpr (is_has_index_operator_v<T1>) {
                return 2;
            }
            return 1;
        }
        return 0;
    }
    using type = std::conditional_t<level() == 2, underlaying2_t<T>,
        std::conditional_t<level() == 1, underlaying1_t<T>, underlaying0_t<T>>>;
};

/**
 * @brief helper for underlaying_type metafunction
 */
template <typename T>
using underlaying_type_t = typename underlaying_type<T>::type;

};
#endif
