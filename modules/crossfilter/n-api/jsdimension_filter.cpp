#include "jsdimension.hpp"

template<typename T, typename I >
  static void filter_function_(js_function & jsf, jsdimension * dim) {
    auto & d = cast_dimension<T, I>(dim->dim);
    napi_ref this_ref;
    NAPI_CALL(napi_create_reference(dim->env_,jsf.jsthis,1, &this_ref));
    napi_ref lambda_ref;
    NAPI_CALL(napi_create_reference(dim->env_,jsf.args[0],1, &lambda_ref));

    napi_value this_value;
    NAPI_CALL(napi_get_reference_value(dim->env_, this_ref, &this_value));
    napi_value lambda_value;
    NAPI_CALL(napi_get_reference_value(dim->env_, lambda_ref, &lambda_value));

    d.filter_function([dim,this_value,lambda_value](auto v) -> bool {
                         napi_value value = convert_from(dim->env_, v);
                         napi_value result;
                         NAPI_CALL(napi_call_function(dim->env_,this_value,lambda_value,1,&value,&result));
                         return convert_to<bool>(dim->env_, result);
                       });
  }

template<typename T, typename I >
  static void filter_all_(js_function & jsf, jsdimension * dim) {
    auto & d = cast_dimension<T, I>(dim->dim);
    d.filter_all();
  }
template<typename T, typename I >
  static void filter_range_(js_function & jsf, jsdimension * dim) {
    auto & d = cast_dimension<T, I>(dim->dim);
    using value_type = typename std::remove_reference<decltype(d)>::type::value_type_t;
    auto lhs = convert_to<value_type>(dim->env_,jsf.args[0]);
    auto rhs = convert_to<value_type>(dim->env_,jsf.args[1]);
    d.filter_range(lhs,rhs);
  }

// template<typename T, typename I >
// static std::enable_if<std::is_same<I,cross::iterable>::value,void> filter_exact_(js_function & jsf, jsdimension * dim) {
//     auto & d = cast_dimension<T, I>(dim->dim);
//     auto v = convert_to<typename T::value_type>(dim->env_,jsf.args[0]);
//     d.filter_exact(v);
// }
template<typename T, typename I>
static void filter_exact_(js_function & jsf, jsdimension * dim) {
  auto & d = cast_dimension<T, I>(dim->dim);
  auto v = convert_to<typename std::remove_reference<decltype(d)>::type::value_type_t>(dim->env_,jsf.args[0]);
  d.filter_exact(v);
}

// template<typename T, typename I, typename = typename std::enable_if<std::is_same<I, cross::iterable>::value>::type>
// static void filter_exact_(js_function & jsf, jsdimension * dim) {
//   auto & d = cast_dimension<T, I>(dim->dim);
//   auto v = convert_to<typename T::value_type>(dim->env_,jsf.args[0]);
//   d.filter_exact(v);
// }

void  jsdimension::filter_function(js_function & jsf, jsdimension * dim) {
  CALL_FOR_TYPE(filter_function_, dim->dim_type, dim->is_iterable, jsf, dim);
  }
 void  jsdimension::filter_exact(js_function & jsf, jsdimension * dim) {
   CALL_FOR_TYPE(filter_exact_, dim->dim_type, dim->is_iterable, jsf, dim);
  }
 void  jsdimension::filter_range(js_function & jsf, jsdimension * dim) {
   CALL_FOR_TYPE(filter_range_, dim->dim_type, dim->is_iterable, jsf, dim);
  }
 void  jsdimension::filter_all(js_function & jsf, jsdimension * dim) {
   CALL_FOR_TYPE(filter_all_, dim->dim_type, dim->is_iterable, jsf, dim);
  }

 napi_value  jsdimension::filter(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env, info, 2);
    auto dim = get_object<jsdimension>(env, jsf.jsthis);
    switch(jsf.args.size()) {
      case 0:
        filter_all(jsf,dim);
        break;
      case 1: {
        napi_valuetype vt;
        NAPI_CALL(napi_typeof(dim->env_,jsf.args[0],&vt));
        if(vt == napi_function) {
          filter_function(jsf,dim);
        } else {
          filter_exact(jsf, dim);
        }
        break;
      }
      case 2:
        filter_range(jsf,dim);
        break;
    }
    return jsf.jsthis;
  }
