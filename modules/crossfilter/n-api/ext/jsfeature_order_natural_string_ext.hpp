#include <node_api.h>
#include "jsfeature_order_natural.hpp"
#include "utils.hpp"
extern template napi_value order_natural_<std::string,int64_t,int64_t,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,js_array<int64_t>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,int64_t,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,js_array<int64_t>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,int32_t,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,js_array<int32_t>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,int32_t,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,js_array<int32_t>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,bool,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,js_array<bool>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,bool,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,js_array<bool>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,double,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,js_array<double>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,double,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,js_array<double>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,std::string,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,js_array<std::string>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,std::string,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,js_array<std::string>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,uint64_t,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,js_array<uint64_t>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,uint64_t,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int64_t,js_array<uint64_t>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,int64_t,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,js_array<int64_t>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,int64_t,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,js_array<int64_t>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,int32_t,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,js_array<int32_t>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,int32_t,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,js_array<int32_t>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,bool,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,js_array<bool>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,bool,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,js_array<bool>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,double,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,js_array<double>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,double,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,js_array<double>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,std::string,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,js_array<std::string>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,std::string,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,js_array<std::string>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,uint64_t,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,js_array<uint64_t>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,uint64_t,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,int32_t,js_array<uint64_t>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,int64_t,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,js_array<int64_t>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,int64_t,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,js_array<int64_t>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,int32_t,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,js_array<int32_t>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,int32_t,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,js_array<int32_t>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,bool,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,js_array<bool>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,bool,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,js_array<bool>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,double,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,js_array<double>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,double,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,js_array<double>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,std::string,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,js_array<std::string>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,std::string,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,js_array<std::string>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,uint64_t,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,js_array<uint64_t>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,uint64_t,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,bool,js_array<uint64_t>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,int64_t,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,js_array<int64_t>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,int64_t,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,js_array<int64_t>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,int32_t,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,js_array<int32_t>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,int32_t,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,js_array<int32_t>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,bool,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,js_array<bool>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,bool,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,js_array<bool>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,double,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,js_array<double>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,double,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,js_array<double>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,std::string,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,js_array<std::string>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,std::string,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,js_array<std::string>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,uint64_t,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,js_array<uint64_t>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,uint64_t,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,double,js_array<uint64_t>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,int64_t,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,js_array<int64_t>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,int64_t,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,js_array<int64_t>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,int32_t,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,js_array<int32_t>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,int32_t,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,js_array<int32_t>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,bool,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,js_array<bool>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,bool,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,js_array<bool>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,double,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,js_array<double>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,double,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,js_array<double>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,std::string,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,js_array<std::string>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,std::string,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,js_array<std::string>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,uint64_t,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,js_array<uint64_t>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,uint64_t,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,std::string,js_array<uint64_t>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,int64_t,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,js_array<int64_t>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,int64_t,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,js_array<int64_t>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,int32_t,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,js_array<int32_t>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,int32_t,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,js_array<int32_t>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,bool,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,js_array<bool>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,bool,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,js_array<bool>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,double,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,js_array<double>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,double,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,js_array<double>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,std::string,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,js_array<std::string>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,std::string,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,js_array<std::string>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,uint64_t,true,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,js_array<uint64_t>,true,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,uint64_t,false,cross::non_iterable>(napi_env env, js_function & jsf, jsfeature * obj);
extern template napi_value order_natural_<std::string,uint64_t,js_array<uint64_t>,false,cross::iterable>(napi_env env, js_function & jsf, jsfeature * obj);