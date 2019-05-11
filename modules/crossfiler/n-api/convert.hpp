#ifndef CONVERT_H_GUARD
#define CONVERT_H_GUARD
template <typename T>
T convert_to(napi_env env, napi_value v);

template<typename T>
napi_value convert_from(napi_env env, const T & v);

#endif
