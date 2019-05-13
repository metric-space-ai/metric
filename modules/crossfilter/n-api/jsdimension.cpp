#include "jsdimension.hpp"
#include "jsfeature.hpp"

napi_value  jsdimension::create_feature(napi_env env, jsfeature * feature) {
  napi_value f_this;
  NAPI_CALL(napi_create_object(env, &f_this));
  NAPI_CALL(napi_wrap(env, f_this, reinterpret_cast<void *>(feature),
                      jsfeature::Destructor,
                      nullptr, // finalize_hint
                      &feature->wrapper));
  add_function(env, f_this,jsfeature::all, "all");
  add_function(env, f_this,jsfeature::top, "top");
  add_function(env, f_this,jsfeature::value,"value");
  add_function(env, f_this,jsfeature::size,"size");
  add_function(env, f_this,jsfeature::order,"order");
  add_function(env, f_this,jsfeature::order_natural,"order_natural");
  feature->env_ = env;
  return f_this;
}


void jsdimension::Destructor(napi_env env, void* nativeObject, void* finalize_hint) {
    auto obj = reinterpret_cast<jsdimension*>(nativeObject);
    obj->~jsdimension();
  }

