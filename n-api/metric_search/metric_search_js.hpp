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
#include "../../modules/distance.hpp"
enum {
    is_Euclidean,
    is_manhatten,
    is_Euclidean_thresholded,
    is_cosine,
    is_p_norm,
    is_emd,
    is_twed,
    //     is_edit,
    is_ssim
};

struct MetricWrapper {
    //namespace md = namespace metric::distance;
    using metric_input_type_t = std::variant<std::vector<double>, std::vector<char>, std::vector<std::vector<double>>>;
    napi_env env;
    int metric_type;
    std::variant<metric::Euclidean<double>, metric::Euclidean_thresholded<double>, metric::Manhatten<double>,
        metric::P_norm<double>, metric::Cosine<double>, metric::EMD<double>, metric::TWED<double>,
        //               metric::Edit<char>,
        metric::SSIM<std::vector<double>>>
        metric_variant;

    MetricWrapper() = default;
    MetricWrapper(napi_env env_, int metric_type_)
        : env(env_)
        , metric_type(metric_type_)
    {
        switch (metric_type) {
        case is_Euclidean:
            metric_variant = metric::Euclidean<double>();
            break;
        case is_manhatten:
            metric_variant = metric::Manhatten<double>();
            break;
        case is_Euclidean_thresholded:
            metric_variant = metric::Euclidean_thresholded<double>();
            break;
        case is_cosine:
            metric_variant = metric::Cosine<double>();
            break;
        case is_p_norm:
            metric_variant = metric::P_norm<double>();
            break;
        // case is_emd:
        //     metric_variant = metric::EMD<double>();
        //     break;
        case is_twed:
            metric_variant = metric::TWED<double>(0, 1);
            break;
        // case is_edit:
        //     metric_variant = metric::Edit<char>();
        //     break;
        case is_ssim:
            metric_variant = metric::SSIM<std::vector<double>>();
            break;
        }
    }
    metric_input_type_t convert(napi_ref p)
    {
        napi_value v;
        napi_get_reference_value(env, p, &v);

        napi_valuetype vt;
        napi_typeof(env, v, &vt);
        switch (vt) {
        case napi_number:
            return std::vector<double> { convert_to<double>(env, v) };
        case napi_string: {
            std::vector<char> res;
            auto s = convert_to<std::string>(env, v);
            for (auto c : s) {
                res.push_back(c);
            }
            return res;
        }
        case napi_object: {
            bool is_array = false;
            napi_is_array(env, v, &is_array);
            if (is_array) {
                uint32_t size;
                napi_get_array_length(env, v, &size);
                std::vector<double> res(size);
                for (uint32_t i = 0; i < size; i++) {
                    napi_value e;
                    napi_get_element(env, v, i, &e);
                    res[i] = convert_to<double>(env, e);
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
    template <typename T>
    double metric(T&& t1, T&& t2) const
    {
        switch (metric_type) {
        case is_Euclidean:
            return std::get<0>(metric_variant)(std::get<0>(t1), std::get<0>(t2));
        case is_manhatten:
            return std::get<1>(metric_variant)(std::get<0>(t1), std::get<0>(t2));
        case is_Euclidean_thresholded:
            return std::get<2>(metric_variant)(std::get<0>(t1), std::get<0>(t2));
        case is_cosine:
            return std::get<3>(metric_variant)(std::get<0>(t1), std::get<0>(t2));
        case is_p_norm:
            return std::get<4>(metric_variant)(std::get<0>(t1), std::get<0>(t2));
        case is_emd:
            return std::get<5>(metric_variant)(std::get<0>(t1), std::get<0>(t2));
        case is_twed:
            return std::get<6>(metric_variant)(std::get<0>(t1), std::get<0>(t2));
        // case is_edit:
        //     return std::get<7>(metric_variant)(t1,t2);
        case is_ssim:
            return std::get<7>(metric_variant)(std::get<2>(t1), std::get<2>(t2));
        }
        return 0;
    }
    double operator()(const metric_input_type_t& v1, const metric_input_type_t& v2) const
    {
        if (v1.index() != v2.index()) {
            throw_js_error(env, "can't calculate metric on diferrent types");
            return 0;
        }
        return metric(v1, v2);
        return 0;
    }
};

struct metric_search_js {
    static napi_ref constructor;
    napi_env env_;
    napi_value js_this;
    int metric_type;

    napi_ref wrapper_ = nullptr;
    metric::Tree<MetricWrapper::metric_input_type_t, MetricWrapper> tree;

    metric_search_js(napi_env e, napi_value this_, int metric_type_)
        : env_(e)
        , js_this(this_)
        , metric_type(metric_type_)
        , tree(-1, MetricWrapper(env_, metric_type_))
    {
    }

    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint)
    {
        auto obj = reinterpret_cast<metric_search_js*>(nativeObject);
        obj->~metric_search_js();
    }

    static napi_value insert(napi_env env, napi_callback_info info);
    static napi_value insert_if(napi_env env, napi_callback_info info);
    static napi_value erase(napi_env env, napi_callback_info info);
    static napi_value get(napi_env env, napi_callback_info info);  // operator[]
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
