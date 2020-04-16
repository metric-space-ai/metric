/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#include <node_api.h>
#include "metric_search_js.hpp"

napi_value New(napi_env env, napi_callback_info info)
{
    napi_status status;
    js_function jsf = extract_function(env, info, 1);
    metric_search_js* obj;
    if (jsf.args.empty()) {
        obj = new metric_search_js(env, jsf.jsthis, is_Euclidean);
    } else {
        napi_valuetype valuetype;
        napi_typeof(env, jsf.args[0], &valuetype);
        if (valuetype == napi_number) {
            int mt = convert_to<int32_t>(env, jsf.args[0]);
            obj = new metric_search_js(env, jsf.jsthis, mt);
        } else {
            throw_js_error(env, "Unknown metric type");
            return nullptr;
        }
    }
    status = napi_wrap(env, jsf.jsthis, reinterpret_cast<void*>(obj), metric_search_js::Destructor,
        nullptr,  // finalize_hint
        &obj->wrapper_);
    obj->env_ = env;
    assert(status == napi_ok);
    return jsf.jsthis;
}
napi_value Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor properties[] = { DECLARE_NAPI_METHOD("insert", metric_search_js::insert),
        DECLARE_NAPI_METHOD("insert_if", metric_search_js::insert_if),
        DECLARE_NAPI_METHOD("erase", metric_search_js::erase), DECLARE_NAPI_METHOD("nn", metric_search_js::nn),
        DECLARE_NAPI_METHOD("knn", metric_search_js::knn), DECLARE_NAPI_METHOD("rnn", metric_search_js::rnn),
        DECLARE_NAPI_METHOD("size", metric_search_js::size),
        DECLARE_NAPI_METHOD("traverse", metric_search_js::traverse),
        DECLARE_NAPI_METHOD("level_size", metric_search_js::level_size),
        DECLARE_NAPI_METHOD("print", metric_search_js::print),
        DECLARE_NAPI_METHOD("print_levels", metric_search_js::print_levels),
        DECLARE_NAPI_METHOD("empty", metric_search_js::empty),
        DECLARE_NAPI_METHOD("check_covering", metric_search_js::check_covering),
        DECLARE_NAPI_METHOD("to_vector", metric_search_js::to_vector),
        DECLARE_NAPI_METHOD("to_json", metric_search_js::to_json),
        DECLARE_NAPI_METHOD("clustering", metric_search_js::clustering),
        DECLARE_NAPI_METHOD("get", metric_search_js::get) };

    //InitDim(env, exports);

    napi_value cons;

    status = napi_define_class(env, "metric_search", NAPI_AUTO_LENGTH, New, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons);
    assert(status == napi_ok);

    status = napi_create_reference(env, cons, 1, &metric_search_js::constructor);
    assert(status == napi_ok);

    status = napi_set_named_property(env, exports, "metric_search", cons);
    assert(status == napi_ok);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
