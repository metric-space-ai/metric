#ifndef CONVERT_H_GUARD
#define CONVERT_H_GUARD
/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/

template <typename T>
T convert_to(napi_env env, napi_value v);

template <typename T>
napi_value convert_from(napi_env env, const T& v);

#endif
