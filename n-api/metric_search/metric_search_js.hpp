/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#ifndef METRIC_SEARCH_JS_H_GUARD
#define METRIC_SEARCH_JS_H_GUARD
#include "utils.hpp"
#include <variant>
#include <vector>
#include <string>
#include "distance.hpp"
enum {
    is_euclidian,
    is_manhatten,
     is_euclidian_thresholded,
     is_cosine,
     is_p_norm,
     is_emd,
     is_twed,
//     is_edit,
     is_ssim
};

struct MetricWrapper {
    //namespace md = namespace metric::distance;
    using metric_input_type_t = std::variant<std::vector<double>, std::vector<char>, std::vector<std::vector<double>> >;
    napi_env env;
    int metric_type;
    std::variant<metric::distance::Euclidian<double>,
                 metric::distance::Euclidian_thresholded<double>,
                 metric::distance::Manhatten<double>,
                 metric::distance::P_norm<double>,
                 metric::distance::Cosine<double>,
                 metric::distance::EMD<double>,
                 metric::distance::TWED<double>,
                 //               metric::distance::Edit<char>,
                 metric::distance::SSIM<std::vector<double>>> metric_variant;

    MetricWrapper() = default;
    MetricWrapper(napi_env env_, int metric_type_):env(env_), metric_type(metric_type_) {
        switch(metric_type) {
        case is_euclidian:
            metric_variant = metric::distance::Euclidian<double>();
            break;
        case is_manhatten:
            metric_variant = metric::distance::Manhatten<double>();
            break;
        case is_euclidian_thresholded:
            metric_variant = metric::distance::Euclidian_thresholded<double>();
            break;
        case is_cosine:
            metric_variant = metric::distance::Cosine<double>();
            break;
        case is_p_norm:
            metric_variant = metric::distance::P_norm<double>();
            break;
        // case is_emd:
        //     metric_variant = metric::distance::EMD<double>();
        //     break;
        case is_twed:
            metric_variant = metric::distance::TWED<double>();
            break;
        // case is_edit:
        //     metric_variant = metric::distance::Edit<char>();
        //     break;
        case is_ssim:
            metric_variant = metric::distance::SSIM<std::vector<double>>();
            break;
        }
    }
    metric_input_type_t convert(napi_ref p) {
        napi_value v;
        napi_get_reference_value(env, p, &v);

        napi_valuetype vt;
        napi_typeof(env, v, &vt);
        switch(vt) {
        case napi_number:
            return std::vector<double>{convert_to<double>(env,v)};
        case napi_string: {
            std::vector<char> res;
            auto s = convert_to<std::string>(env,v);
            for(auto c : s) {
                res.push_back(c);
            }
            return res;
        }
        case napi_object: {
            bool is_array = false;
            napi_is_array(env, v, &is_array);
            if(is_array) {
                uint32_t size;
                napi_get_array_length(env, v, &size);
                std::vector<double> res(size);
                for(uint32_t i = 0 ; i < size; i++) {
                    napi_value e;
                    napi_get_element(env, v, i, &e);
                    res[i] = convert_to<double>(env,e);
                }
                return res;
            }
            throw_js_error(env, "cannot calculate metric on objects");
            break;
        }
        default:
            throw_js_error(env, "unknown type for metric");
        }
        return std::vector<double>();
    }
    template<typename T>
    double metric(T && t1, T && t2) const {
        // auto v = [&t1, &t2 ](auto && d) -> double {
        //              if constexpr (std::is_same<decltype(d), metric::distance::Edit<char>>::value && std::is_same<typename std::decay<T>::type, std::vector<char>>::value) {
        //                               return static_cast<double>(d(t1,t2));
        //                  }  else {
        //                  return static_cast<double>(d(t1,t2));
        //              }
        //          };
        // return std::visit(v, metric_variant);
        switch(metric_type) {
        case is_euclidian:
            return std::get<0>(metric_variant)(std::get<0>(t1),std::get<0>(t2));
        case is_manhatten:
            return std::get<1>(metric_variant)(std::get<0>(t1),std::get<0>(t2));
        case is_euclidian_thresholded:
            return std::get<2>(metric_variant)(std::get<0>(t1),std::get<0>(t2));
        case is_cosine:
            return std::get<3>(metric_variant)(std::get<0>(t1),std::get<0>(t2));
        case is_p_norm:
            return std::get<4>(metric_variant)(std::get<0>(t1),std::get<0>(t2));
        case is_emd:
            return std::get<5>(metric_variant)(std::get<0>(t1),std::get<0>(t2));
        case is_twed:
            return std::get<6>(metric_variant)(std::get<0>(t1),std::get<0>(t2));
        // case is_edit:
        //     return std::get<7>(metric_variant)(t1,t2);
        case is_ssim:
            return std::get<7>(metric_variant)(std::get<2>(t1),std::get<2>(t2));
        }
        return 0;
    }
    double operator()(const metric_input_type_t & v1, const metric_input_type_t & v2) const {
        if(v1.index() != v2.index()) {
            throw_js_error(env,"can't calculate metric on diferrent types");
            return 0;
        }
        return metric(v1,v2);
        // switch(metric_type) {
        // case is_euclidian:
        // case is_manhatten:
        // case is_euclidian_thresholded:
        // case is_cosine:
        // case is_p_norm:
        // case is_emd:
        // case is_twed: {
        //     return metric(v1, v2);
        //     // if (v1.index() == 0) {
        //     //     return metric(std::get<0>(v1), std::get<0>(v2));
        //     // } else {
        //     //     throw_js_error(env, "this type of metric accept only number array");
        //     //     return 0;
        //     // }
        // }
        // // case is_edit:
        // //     if (v1.index() == 1) {
        // //         return metric(std::get<1>(v1), std::get<1>(v2));
        // //     } else {
        // //         throw_js_error(env, "this type of metric accept only strings");
        // //         return 0;
        // //     }
        // case is_ssim:
        //     return metric(v1,v2);
        //     // if (v1.index() == 2) {
        //     //     return metric(std::get<2>(v1), std::get<2>(v2));
        //     // } else {
        //     //     throw_js_error(env, "this type of metric accept only strings");
        //     //     return 0;
        //     // }
        // }
        return 0;
    }
};

struct metric_search_js {
    static napi_ref constructor;
    napi_env env_;
    napi_value js_this;
    int metric_type;

    napi_ref wrapper_ = nullptr;
    metric::space::Tree<MetricWrapper::metric_input_type_t,MetricWrapper> tree;

    metric_search_js(napi_env e, napi_value this_, int metric_type_):env_(e),
                                                                     js_this(this_),
                                                                     metric_type(metric_type_),
                                                                     tree(-1, MetricWrapper(env_, metric_type_)) { }

    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint) {
        auto obj = reinterpret_cast<metric_search_js*>(nativeObject);
        obj->~metric_search_js();
    }

    static napi_value insert(napi_env env, napi_callback_info info);
    static napi_value insert_if(napi_env env, napi_callback_info info);
    static napi_value erase(napi_env env, napi_callback_info info);
    static napi_value get(napi_env env, napi_callback_info info); // operator[]
    static napi_value nn(napi_env env, napi_callback_info info);
    static napi_value knn(napi_env env, napi_callback_info info);
    static napi_value rnn(napi_env env, napi_callback_info info);

    static napi_value size(napi_env env, napi_callback_info info);
    static napi_value traverse(napi_env env, napi_callback_info info);
    static napi_value level_size(napi_env env, napi_callback_info info);
    static napi_value print(napi_env env, napi_callback_info info);
    static napi_value print_levels(napi_env env, napi_callback_info info);
    static napi_value empty(napi_env env, napi_callback_info info);
    static napi_value check_covering(napi_env env, napi_callback_info info);
    static napi_value to_vector(napi_env env, napi_callback_info info);
    static napi_value to_json(napi_env env, napi_callback_info info);
    static napi_value clustering(napi_env env, napi_callback_info info);
};
#endif
