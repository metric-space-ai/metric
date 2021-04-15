#ifndef _UPS_INDEXING_HPP
#define _UPS_INDEXING_HPP

#include "../../../../../3rdparty/blaze/Blaze.h"

//#include <iostream>  // TODO remove




// -----------------------------
// indices_cw

template <typename R, typename C>
R indicesCwImpl(const C & m) {
// returns vector of 1d indices of all nonzero elements in column-major order

    size_t s = blaze::sum(m);
    size_t el_n = m.columns()*m.rows();
    R indices (s);
    size_t idx = 0;
    for (size_t i = 0; i < el_n; ++i) {
        if (m(i%m.rows(), i/m.rows())) {
            indices[idx] = i;
            ++idx;
        }
    }

    return indices;
}


blaze::DynamicVector<size_t> indicesCw(const blaze::DynamicMatrix<unsigned char, blaze::columnMajor> & m) {
    return indicesCwImpl<blaze::DynamicVector<size_t>, blaze::DynamicMatrix<unsigned char, blaze::columnMajor>>(m);
}

template <typename T>
std::vector<size_t> indicesCwStd(const blaze::CompressedMatrix<T, blaze::columnMajor> & m) {
    return indicesCwImpl<std::vector<size_t>, blaze::CompressedMatrix<T, blaze::columnMajor>>(m);
}




// -----------------------------
// indices2d_cw

std::tuple<blaze::DynamicVector<size_t>, blaze::DynamicVector<size_t>>
indices2dCw(const blaze::DynamicMatrix<unsigned char, blaze::columnMajor> & m) {
// returns vectos of row and column indices of all nonzero elements in column-major order  // TODO debug

    size_t s = blaze::sum(m);
    //size_t el_n = m.columns()*m.rows();
    blaze::DynamicVector<size_t> xx (s);
    blaze::DynamicVector<size_t> yy (s);
    size_t idx = 0;
    for (size_t j = 0; j < m.columns(); ++j) {  // we use column-major order, as in Matlab
        for (size_t i = 0; i < m.rows(); ++i) {
            //std::cout << i << ", " << j << ": " << idx << "\n"; // TODO remove
            if (m(i, j)) {
                xx[idx] = i;
                yy[idx] = j;
                //std::cout << i << ", " << j << ": " << idx << "\n"; // TODO remove
                ++idx;
            }
        }
    }

    return std::make_tuple(yy, xx);  // rows, columns
}





// -----------------------------
// flatten_to_col


template <typename C, typename T>
blaze::DynamicVector<T> flattenToColImpl(const C & mat) {

    blaze::DynamicVector<T> out (mat.columns()*mat.rows(), 0);
    for (size_t i=0; i<mat.columns(); ++i) {
        blaze::subvector(out, i*mat.rows(), mat.rows()) = blaze::column(mat, i);
    }

    return out;
}


template <template<class> class C, typename T>
blaze::DynamicVector<T> flattenToCol(const C<T> & mat) {
    return flattenToColImpl<C<T>, T>(mat);
}

template <template<class, bool> class C, typename T, bool B>
blaze::DynamicVector<T> flattenToCol(const C<T, B> & mat) {
    return flattenToColImpl<C<T, B>, T>(mat);
}





#endif // _UPS_INDEXING_HPP
