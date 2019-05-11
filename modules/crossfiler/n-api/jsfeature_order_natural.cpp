//#include <functional>
#include "jsfeature.hpp"
//#include "crossfilter.hpp"
#include "utils.hpp"
#include "ext/jsfeature_order_natural_bool_ext.hpp"
#include "ext/jsfeature_order_natural_double_ext.hpp"
#include "ext/jsfeature_order_natural_string_ext.hpp"
#include "ext/jsfeature_order_natural_i64_ext.hpp"
#include "ext/jsfeature_order_natural_i32_ext.hpp"
#include "ext/jsfeature_order_natural_ui64_ext.hpp"
//#include "feature_cast_extern.hpp"

napi_value  jsfeature::order_natural(napi_env env, napi_callback_info info) {
  js_function jsf = extract_function(env, info, 0);
  jsfeature* obj = get_object<jsfeature>(env, jsf.jsthis);
  CALL_DISPATCH_F(obj->key_type, obj->value_type, obj->dim_type,obj->is_group_all, obj->is_iterable,  order_natural_, env, jsf, obj);
  return nullptr;
}
