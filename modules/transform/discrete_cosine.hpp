/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_TRANSFORMATION_DISCRETE_COSINE_DISCRETE_COSINE_HPP
#define _METRIC_TRANSFORMATION_DISCRETE_COSINE_DISCRETE_COSINE_HPP

#include "../../3rdparty/DCT/fast-dct-lee.c"
#include "../../3rdparty/blaze/Blaze.h"



namespace dct
{

// apply forward or invese DCT depending on bool flag

template <class BlazeMatrix>
bool apply_DCT(BlazeMatrix & Slices, bool inverse = false)
{
    bool return_value = true;
    size_t n = 0;
    double * sample = new double[Slices.rows()];
    double maxval = 0;
    for  (n=0; n<Slices.columns(); n++)
    {
        //std::cout << "slice " << n << ":\n";
        auto current_slice = column(Slices, n);
        size_t idx = 0;
        for ( auto it=current_slice.begin(); it!=current_slice.end(); ++it )
        {
            sample[idx++] = *it;
        }
        //std::cout << "read:   \n" << current_slice << "\n";
        bool success = false;
        if (inverse)
            success = FastDctLee_inverseTransform(sample, idx);
        else
            success = FastDctLee_transform(sample, idx);
        if (success)
        {
            idx = 0;
            for ( auto it=current_slice.begin(); it!=current_slice.end(); ++it )
            {
                *it = sample[idx++];
                if (abs(*it) > maxval)
                    maxval = *it;
            }
            //std::cout << "written:\n" << current_slice << "\n";
        }
        else
            return_value = false; // flag is dropped in case of any failure
    }
    //std::cout << "updated matrix:\n" << Slices << "\n";
    delete [] sample;
    if (maxval > 1)
        Slices = evaluate(Slices / maxval);
    //std::cout << "maxval: " << maxval << "\n";
    return return_value;
}



//// looks like totally incorrect attempt: we can not hope to restore well due to cumulative error // TODO remove
//// precompressor based on multiple sequential application of DCT

//template <template <class, bool> class BlazeContainerType, bool SO, class ValueType>
//BlazeContainerType<ValueType, blaze::columnMajor> compress_mDCT(
//        BlazeContainerType<ValueType, SO> & input,
//        size_t blocksize_uncompressed,
//        size_t blocksize_compressed,
//        size_t steps
//        )
//{
//    BlazeContainerType<ValueType, blaze::columnMajor> output();
//    // TODO assert blocksizes and blocksize_uncompressed/blocksize_compressed are degrees of 2

//    for (size_t step=0; step<steps; ++steps) {
//        for (size_t block=0; block<blocksize_compressed; ++block) {
              // TODO continue
//        }
//
//    }

//    return output;
//}



} // namespace dct

#endif //  _METRIC_TRANSFORMATION_DISCRETE_COSINE_DISCRETE_COSINE_HPP
