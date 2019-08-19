/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#ifndef JSFEATURE_ORDER_NATURAL_H_GUARD
#define JSFEATURE_ORDER_NATURAL_H_GUARD
#include "jsfeature.hpp"
#include "utils.hpp"
#include "feature_cast_extern.hpp"

template <typename Key, typename Value, typename DimType, bool is_group_all, typename I>
napi_value order_natural_(napi_env env, js_function& jsf, jsfeature* obj)
{
    auto& feature = cast_feature<Key, Value, DimType, is_group_all, I>(obj->ptr);
    feature.order_natural();
    return nullptr;
}

#endif
