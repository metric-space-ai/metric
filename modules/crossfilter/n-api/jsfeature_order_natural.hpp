#ifndef JSFEATURE_ORDER_NATURAL_H_GUARD
#define JSFEATURE_ORDER_NATURAL_H_GUARD
#include "jsfeature.hpp"
#include "utils.hpp"
#include "feature_cast_extern.hpp"

template<typename Key, typename Value, typename DimType, bool is_group_all, typename I>
napi_value order_natural_(napi_env env, js_function & jsf, jsfeature * obj) {
  auto & feature = cast_feature<Key,Value, DimType, is_group_all, I>(obj->ptr);
  feature.order_natural();
  return nullptr;
}

#endif
