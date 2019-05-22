/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Dmitry Vinokurov (c) 2018
*/
#include <node_api.h>
#include <iomanip>
#include "utils.hpp"
#include "metric_search_js.hpp"


napi_ref metric_search_js::constructor;
static bool check_is_array(napi_env env, napi_value value) {
    bool is_array = false;
    NAPI_CALL(napi_is_array(env, value, &is_array));
    return is_array;
}

static MetricWrapper::metric_input_type_t extract_value(napi_env env, napi_value value) {
    napi_valuetype vt;
    napi_typeof(env, value, &vt);
    if(vt == napi_object) {
        if(check_is_array(env, value)) {
            uint32_t size;
            napi_get_array_length(env, value, &size);
            napi_value e1;
            napi_get_element(env, value, 0, &e1);
            if(check_is_array(env, e1)) {
                std::vector<std::vector<double>> res(size);
                for(uint32_t i = 0 ; i < size; i++) {
                    napi_value s;
                    napi_get_element(env, value, i, &s);
                    uint32_t size1;
                    napi_get_array_length(env, s, &size1);
                    res[i].resize(size1);
                    for(uint32_t j = 0; j< size1; ++j) {
                        napi_value e;
                        napi_get_element(env, s, j, &e);
                        res[i][j] = convert_to<double>(env, e);
                    }
                }
                return res;
            } else {
                std::vector<double> res(size);
                for(uint32_t i = 0 ; i < size; i++) {
                    napi_value e;
                    napi_get_element(env, value, i, &e);
                    res[i] = convert_to<double>(env, e);
                }
                return res;
            }
        } else {
            throw_js_error(env, "can insert only scalar, string, vector, or matrix");
            return MetricWrapper::metric_input_type_t{};;
        }
    } else if(vt == napi_number) {
        return std::vector<double>{convert_to<double>(env, value)};
    } else if (vt == napi_string) {
        auto s = convert_to<std::string>(env, value);
        return std::vector<char>(s.begin(), s.end());
    } else {
        throw_js_error(env, "can insert only scalar, string, vector or matrix");
        return MetricWrapper::metric_input_type_t{};;
    }
    return MetricWrapper::metric_input_type_t{};;
}

template<>
MetricWrapper::metric_input_type_t convert_to(napi_env env, napi_value value) {
    return extract_value(env, value);
}
template<typename T>
static std::vector<T> extract_vector(napi_env env, napi_value value) {
    uint32_t size;
    napi_get_array_length(env, value, &size);
    std::vector<T> res(size);
    for(uint32_t i = 0 ; i < size; i++) {
        napi_value e;
        napi_get_element(env, value, i, &e);
        res[i] = convert_to<T>(env, e);
    }
    return res;
}

template<typename It>
static napi_value make_array(napi_env env, It begin, It end) {
    napi_value array;
    NAPI_CALL(napi_create_array_with_length(env, std::distance(end, begin), &array));
    for(std::size_t i = 0; begin != end; ++begin, ++i) {
        NAPI_CALL(napi_set_element(env, array, i, convert_from(env, *begin)));
    }
    return array;
}
napi_value metric_search_js::insert(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env, info, 1);
    if(jsf.args.size() != 1) {
        throw_js_error(env, "insert should accept 1 argument");
        return nullptr;
    }
    metric_search_js * obj = get_object<metric_search_js>(env, jsf.jsthis);
    auto v  = extract_value(env, jsf.args[0]);
    auto rc = obj->tree.insert(v);
    return convert_from<int32_t>(env, (int32_t)rc);
}

napi_value metric_search_js::insert_if(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env, info, 2);
    if(jsf.args.size() != 2) {
        throw_js_error(env, "insert_if should accept 2 arguments");
        return nullptr;
    }
    metric_search_js * obj = get_object<metric_search_js>(env, jsf.jsthis);
    double threshold = convert_to<double>(env, jsf.args[1]);
    // napi_valuetype vt;
    // napi_typeof(env, jsf.args[0], &vt);
    // if(vt == is_object) {
    //     bool is_array = false;
    //     napi_is_array(env, jsf.args[0], &is_array);
    //     if(is_array) {
    //         uint32_t size;
    //         napi_get_array_length(env, jsf.args[0], &size);
    //         std::vector<double> res(size);
    //         for(uint32_t i = 0 ; i < size; i++) {
    //             napi_value e;
    //             napi_get_element(env, jsf.args[0], i, &e);
    //             res.push_back(convert_to<double>(env, e));
    //         }
    //         auto s = obj->tree.insert_if(res, threshold);
    //         return convert_from<std::size_t>(env, s);
    //     } else {
    //         throw_js_error(env, "can insert only scalar or string or vector");
    //         return nullptr;
    //     }
    // } else if(vt == is_number) {
    //     auto s = obj->tree.insert_if(convert_to<double>(env, jsf.args[0]), threshold);
    //     return convert_from<bool>(env, s);
    // } else if(vt == is_string) {
    //     auto s = convert_do<std::string>(env, jsf.args[0]);
    //     std::vector<char> r(s.begin(), s.end());
    //     obj->tree.insert_if(r, threshold);
    // }
    // return jsf.jsthis;
    auto v  = extract_value(env, jsf.args[0]);
    auto rc = obj->tree.insert_if(v, threshold);
    return convert_from<int32_t>(env, (int32_t)rc);
}

napi_value metric_search_js::erase(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env, info, 1);
    if(jsf.args.size() != 1) {
        throw_js_error(env, "erase should accept 1 argument");
        return nullptr;
    }
    metric_search_js * obj = get_object<metric_search_js>(env, jsf.jsthis);
    auto v = extract_value(env, jsf.args[0]);
    auto res = obj->tree.erase(v);
    return convert_from<bool>(env, res);
}

napi_value convert_variant_to_napi(napi_env env, const std::variant<std::vector<double>,std::vector<char>, std::vector<std::vector<double>>> & p) {
    switch(p.index()) {
    case 0: {
        auto v = std::get<0>(p);
        if(v.size() == 1) {
            return convert_from<double>(env, v[0]);
        }
        return make_array(env, v.begin(), v.end());
    }
    case 1: {
        auto v = std::get<1>(p);
        napi_value result;
        NAPI_CALL(napi_create_string_utf8(env, v.data(), v.size(), &result));
        return result;
    }
    case 2:
        std::vector<std::vector<double>> v = std::get<2>(p);
        napi_value result;
        napi_create_array_with_length(env, v.size(), &result);
        for(std::size_t i = 0; i < v.size(); i++) {
            napi_value vv = make_array(env, v[i].begin(), v[i].end());
            napi_set_element(env, result, i, vv);
        }
        return result;
    }
    return nullptr;
}
napi_value metric_search_js::get(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env, info, 1);
    if(jsf.args.size() != 1) {
        throw_js_error(env, "erase should accept 1 argument");
        return nullptr;
    }
    metric_search_js * obj = get_object<metric_search_js>(env, jsf.jsthis);
    std::size_t index = convert_to<int32_t>(env, jsf.args[0]);
    auto p = obj->tree[index];
    return convert_variant_to_napi(env,p);
}

template<typename Node>
static napi_value create_node_object(napi_env env, Node n) {
    napi_value result;
    NAPI_CALL(napi_create_object(env,&result));
    NAPI_CALL(napi_set_named_property(env, result, "ID", convert_from<uint64_t>(env, n->ID)));
    NAPI_CALL(napi_set_named_property(env, result, "level", convert_from<int32_t>(env, n->level)));
    NAPI_CALL(napi_set_named_property(env, result, "data", convert_variant_to_napi(env, n->data)));
    NAPI_CALL(napi_set_named_property(env, result, "parent_dist", convert_from<double>(env, n->parent_dist)));

    return result;
}
napi_value metric_search_js::nn(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env, info, 1);
    if(jsf.args.size() != 1) {
        throw_js_error(env, "nn should accept 1 argument");
        return nullptr;
    }
    metric_search_js * obj = get_object<metric_search_js>(env, jsf.jsthis);
    auto v = extract_value(env, jsf.args[0]);
    auto res = obj->tree.nn(v);
    return create_node_object(env, res);
}

napi_value metric_search_js::knn(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env, info, 2);
    if(jsf.args.size() != 2) {
        throw_js_error(env, "knn should accept 2 argument");
        return nullptr;
    }
    metric_search_js * obj = get_object<metric_search_js>(env, jsf.jsthis);
    auto v = extract_value(env, jsf.args[0]);
    auto k = convert_to<int32_t>(env, jsf.args[1]);
    auto res = obj->tree.knn(v,k);

    napi_value result;
    napi_create_array_with_length(env, res.size(), &result);
    for(std::size_t i = 0; i < res.size(); i++) {
        napi_value v = create_node_object(env, res[i].first);
        NAPI_CALL(napi_set_named_property(env, v, "distance", convert_from<double>(env, res[i].second)));
        NAPI_CALL(napi_set_element(env, result, i, v));
    }
    return result;
}

napi_value metric_search_js::rnn(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env, info, 2);
    double distance = 1.0;
    if(jsf.args.size() == 2) {
        distance = convert_to<double>(env, jsf.args[1]);
    } else if(jsf.args.size() != 1) {
        throw_js_error(env, "rnn should accept one or two arguments");
        return nullptr;
    }
    metric_search_js * obj = get_object<metric_search_js>(env, jsf.jsthis);
    auto v = extract_value(env, jsf.args[0]);
    auto res = obj->tree.rnn(v,distance);

    napi_value result;
    napi_create_array_with_length(env, res.size(), &result);
    for(std::size_t i = 0; i < res.size(); i++) {
        napi_value v = create_node_object(env, res[i].first);
        NAPI_CALL(napi_set_named_property(env, v, "distance", convert_from<double>(env, res[i].second)));
        NAPI_CALL(napi_set_element(env, result, i, v));
    }
    return result;
}

napi_value metric_search_js::size(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env, info, 0);
    metric_search_js * obj = get_object<metric_search_js>(env, jsf.jsthis);
    auto sz = obj->tree.size();
    return convert_from<uint64_t>(env, sz);
}

napi_value metric_search_js::traverse(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env, info, 1);
    if(jsf.args.size() != 1) {
        throw_js_error(env, "traverse should accept 1 argument");
        return nullptr;
    }
    metric_search_js * obj = get_object<metric_search_js>(env, jsf.jsthis);
    napi_ref this_ref;
    NAPI_CALL(napi_create_reference(env, jsf.jsthis, 1, &this_ref));
    napi_ref lambda_ref;
    NAPI_CALL(napi_create_reference(env, jsf.args[0], 1, &lambda_ref));
    obj->tree.traverse([&obj, &this_ref, &lambda_ref](auto p) {
                           napi_value v = create_node_object(obj->env_, p);
                           napi_value this_value;
                           NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
                           napi_value lambda_value;
                           napi_value res;
                           NAPI_CALL(napi_get_reference_value(obj->env_, lambda_ref, &lambda_value));
                           NAPI_CALL(napi_call_function(obj->env_, this_value, lambda_value, 1,
                                                        &v, &res)); 
                       });
    return nullptr;
}

napi_value metric_search_js::level_size(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env, info, 0);
    metric_search_js * obj = get_object<metric_search_js>(env, jsf.jsthis);
    auto sz = obj->tree.levelSize();
    return convert_from<int32_t>(env, sz);
}


napi_value metric_search_js::print(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env, info, 0);
    metric_search_js * obj = get_object<metric_search_js>(env, jsf.jsthis);
    std::ostringstream ostr;
    obj->tree.print(ostr);
    std::string r = ostr.str();
    return convert_from<std::string>(env, r);
}
napi_value metric_search_js::print_levels(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env, info, 0);
    metric_search_js * obj = get_object<metric_search_js>(env, jsf.jsthis);
    auto res = obj->tree.print_levels();
    napi_value result;
    napi_create_object(env, &result);
    for(auto & e : res) {
        napi_set_property(env, result, convert_from<int32_t>(env, e.first), convert_from<int32_t>(env, e.second));
    }
    return result;
}

napi_value metric_search_js::empty(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env, info, 0);
    metric_search_js * obj = get_object<metric_search_js>(env, jsf.jsthis);
    auto b = obj->tree.empty();
    return convert_from<bool>(env, b);
}
napi_value metric_search_js::check_covering(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env, info, 0);
    metric_search_js * obj = get_object<metric_search_js>(env, jsf.jsthis);
    auto b = obj->tree.check_covering();
    return convert_from<bool>(env, b);
}


napi_value metric_search_js::to_vector(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env, info, 0);
    metric_search_js * obj = get_object<metric_search_js>(env, jsf.jsthis);
    auto v = obj->tree.toVector();
    napi_value result;
    napi_create_array_with_length(env, v.size(), &result);
    for(std::size_t i = 0; i < v.size(); i++) {
        napi_set_element(env, result, i, convert_variant_to_napi(env, v[i]));
    }
    return result;
}

template<typename V>
static void print_vector(std::ostream & ostr, const V & vec) {
    ostr << "[ ";
    for(std::size_t i =0 ; i < vec.size(); i++) {
        ostr << std::fixed << vec[i];
        if(i != vec.size() - 1)
            ostr << ", ";
    }
    ostr << " ]";
}
napi_value metric_search_js::to_json(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env, info, 0);
    metric_search_js * obj = get_object<metric_search_js>(env, jsf.jsthis);
    auto json = obj->tree.to_json([](const auto & v ){
                                      std::ostringstream os;
                                      if(v.index() == 0) {
                                          print_vector(os, std::get<0>(v));
                                      } else if(v.index() == 1) {
                                          auto s = std::get<1>(v);
                                          os << "\"" << std::string(s.begin(), s.end()) << "\"";
                                      } else if(v.index() == 2) {
                                          auto mat = std::get<2>(v);
                                          os << " [";
                                          for(std::size_t i = 0; i < mat.size(); i++) {
                                              print_vector(os, mat[i]);
                                              if(i != mat.size() - 1) {
                                                  os << ", ";
                                              }
                                          }
                                          os << " ]";
                                      }
                                      return os.str();
                                  });
    return convert_from<std::string>(env, json);
}

napi_value metric_search_js::clustering(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env, info, 3);
    metric_search_js * obj = get_object<metric_search_js>(env, jsf.jsthis);
    auto distribution = extract_vector<double>(env, jsf.args[0]);
    std::vector<std::size_t> IDS = extract_vector<std::size_t>(env, jsf.args[1]);
    auto points = extract_vector<MetricWrapper::metric_input_type_t>(env, jsf.args[2]);
    auto clusters = obj->tree.clustering(distribution, IDS, points);
    napi_value result;
    napi_create_array_with_length(env, clusters.size(), &result);
    for(std::size_t i = 0; i < clusters.size(); i++ ) {
        napi_value v;
        napi_create_array_with_length(env, clusters[i].size(), &v);
        for(std::size_t j = 0; j < clusters[i].size(); j++) {
            napi_set_element(env, v, j, convert_from<uint64_t>(env, clusters[i][j]));
        }
        napi_set_element(env, result, i, v);
    }
    return result;
}
