#include <node_api.h>
#include <assert.h>
#include <string>
#include <cstdlib>
#include "utils.hpp"

#define NAPI_CALL(x)  assert((x) == napi_ok)

//#include "convert.hpp"

template<>
napi_value convert_to(napi_env, napi_value v) {
  return v;
}

template<>
int64_t convert_to(napi_env env, napi_value v) {
  int64_t i = 0;
  NAPI_CALL(napi_get_value_int64(env, v, &i));
  return i;
}

template<>
uint64_t convert_to(napi_env env, napi_value v) {
  int64_t i = 0;
  NAPI_CALL(napi_get_value_int64(env, v, &i));
  return (uint64_t)i;
}

template<>
bool convert_to(napi_env env, napi_value v) {
  bool b = false;
  if(napi_get_value_bool(env, v, &b) == napi_ok)
    return b;
  napi_value cv;
  if(napi_coerce_to_bool(env,v,&cv) == napi_ok) {
    return convert_to<bool>(env,cv);
  }
  napi_throw_error(env, nullptr, "cannot convert JS value to bool");
  return b;
}
template<>
int32_t convert_to(napi_env env, napi_value v) {
  int32_t i = 0;
  NAPI_CALL(napi_get_value_int32(env, v, &i));
  return i;
}
template<>
double convert_to(napi_env env, napi_value v) {
  double d = 0;
  if(napi_get_value_double(env, v, &d) == napi_ok)
    return d;

  napi_value cv;
  if(napi_coerce_to_number(env,v,&cv) == napi_ok) {
    return convert_to<double>(env,cv);
  }
  napi_throw_error(env, nullptr, "cannot convert JS value to double");
  return d;
}
template<>
std::string convert_to(napi_env env, napi_value v) {
 
  std::size_t bufsize;
  if(napi_get_value_string_utf8(env, v, nullptr,0,&bufsize) == napi_ok) {
    char * buf = new char[bufsize+1];
    std::size_t ns;
    NAPI_CALL(napi_get_value_string_utf8(env,v,buf,bufsize+1, &ns));
    std::string res(buf,ns);
    delete []buf;
    return res;
  }
  napi_value cv;
  if(napi_coerce_to_string(env, v, &cv) == napi_ok) {
    return convert_to<std::string>(env, cv);
  }
  napi_throw_error(env, nullptr, "cannot convert JS value to string");
  return std::string();
}

template<>
napi_value convert_from(napi_env env, const int64_t & v) {
  napi_value result;
  NAPI_CALL(napi_create_int64(env, v, &result));
  return result;
}
template<>
napi_value convert_from(napi_env env, const uint64_t & v) {
  napi_value result;
  int64_t i = (int64_t)v;
  NAPI_CALL(napi_create_int64(env, i, &result));
  return result;
}

template<>
napi_value convert_from(napi_env env, const int32_t & v) {
  napi_value result;
  NAPI_CALL(napi_create_int32(env, v, &result));
  return result;
}
template<>
napi_value convert_from(napi_env env, const bool & v) {
  napi_value result;
  NAPI_CALL(napi_get_boolean(env, v, &result));
  return result;
}
template<>
napi_value convert_from(napi_env env, const double & v) {
  napi_value result;
  NAPI_CALL(napi_create_double(env, v, &result));
  return result;
}
template<>
napi_value convert_from(napi_env env, const std::string & v) {
  napi_value result;
  NAPI_CALL(napi_create_string_utf8(env, v.c_str(), v.size(), &result));
  return result;
}

napi_value extract_value(napi_env env, const void * v, int object_type) {

  switch(object_type) {
    case is_object: {
      napi_value value;
      NAPI_CALL(napi_get_reference_value(env, (napi_ref)v, &value));
      return value;
    }
    case is_bool:
      return convert_from<bool>(env, *(bool*)v);
    case is_double:
      return convert_from<double>(env, *(double*)v);
    case is_string:
      return convert_from<std::string>(env, *(std::string*)v);
    default:
      break;
  }
  return nullptr;
}

