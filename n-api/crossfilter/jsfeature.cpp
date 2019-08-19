/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#include "jsfeature.hpp"
//#include "feature_export.hpp"

void jsfeature::Destructor(napi_env env, void* nativeObject, void* finalize_hint)
{
    auto obj = reinterpret_cast<jsfeature*>(nativeObject);
    obj->~jsfeature();
}
