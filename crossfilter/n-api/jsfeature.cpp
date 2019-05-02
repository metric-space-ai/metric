#include "jsfeature.hpp"
//#include "feature_export.hpp"





void jsfeature::Destructor(napi_env env, void* nativeObject, void* finalize_hint) {
    auto obj = reinterpret_cast<jsfeature*>(nativeObject);
    obj->~jsfeature();
  }

