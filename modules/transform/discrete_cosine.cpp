/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_TRANSFORM_DISCRETE_COSINE_CPP
#define _METRIC_TRANSFORM_DISCRETE_COSINE_CPP

#include "../../3rdparty/DCT/fast-dct-lee.c"

namespace metric {

// apply forward or invese DCT depending on bool flag

template <class BlazeMatrix>
bool apply_DCT(BlazeMatrix& Slices, bool inverse)
{
    bool return_value = true;
    size_t n = 0;
    double* sample = new double[Slices.rows()];
    double maxval = 0;
    for (n = 0; n < Slices.columns(); n++) {
        auto current_slice = column(Slices, n);
        size_t idx = 0;
        for (auto it = current_slice.begin(); it != current_slice.end(); ++it) {
            sample[idx++] = *it;
        }
        bool success = false;
        if (inverse)
            success = FastDctLee_inverseTransform(sample, idx);
        else
            success = FastDctLee_transform(sample, idx);
        if (success) {
            idx = 0;
            for (auto it = current_slice.begin(); it != current_slice.end(); ++it) {
                *it = sample[idx++];
                if (abs(*it) > maxval)
                    maxval = *it;
            }
        } else
            return_value = false;  // flag is dropped in case of any failure
    }

    delete[] sample;
    if (maxval > 1)
        Slices = evaluate(Slices / maxval);
    return return_value;
}

}

#endif
