#ifndef _UPS_VEC_TO_IMAGE_HPP
#define _UPS_VEC_TO_IMAGE_HPP


#include "3rdparty/blaze/Blaze.h"


template <typename T>
blaze::DynamicMatrix<T> vecToImage(
        blaze::DynamicVector<T> vec,
        blaze::CompressedMatrix<bool, blaze::columnMajor> M,
        size_t h,
        size_t w
        )
{
    blaze::DynamicMatrix<T> image (h, w, 0);
    size_t mask_idx = 0;
    for (size_t c = 0; c < w; ++c) {
        for (size_t r = 0; r < h; ++r) {
            if (M(r, c) != 0) {
                image(r, c) = vec[mask_idx];
                ++mask_idx;
            }
        }
    }
    return image;
}


#endif // _UPS_VEC_TO_IMAGE_HPP
