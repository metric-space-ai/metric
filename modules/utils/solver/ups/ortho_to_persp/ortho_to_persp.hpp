#ifndef _UPS_ORTHO_TO_PERSP_HPP
#define _UPS_ORTHO_TO_PERSP_HPP

#include "../../../../../3rdparty/blaze/Blaze.h"

#include <tuple>



template <typename T>
std::tuple<blaze::DynamicMatrix<T>, blaze::DynamicMatrix<T>>
orthoToPersp(const blaze::DynamicMatrix<T> & Z, const blaze::DynamicMatrix<T> & K)
{

    size_t h = Z.rows();
    size_t w = Z.columns();

    T f_u = K(0, 0);
    T f_v = K(1, 1);
    T u_0 = K(0, 2);
    T v_0 = K(1, 2);

    //blaze::DynamicMatrix<T> U = generate(h, w, []( size_t i, size_t j ){ return i + 1; } );  // not supported bu used version of Blaze
    //blaze::DynamicMatrix<T> V = generate(h, w, []( size_t i, size_t j ){ return j + 1; } );
    blaze::DynamicMatrix<T> U (h, w);
    blaze::DynamicMatrix<T> V (h, w);
    for (size_t i = 0; i<h; ++i) {
        for (size_t j = 0; j<w; ++j) {
            U(i, j) = j + 1;
            V(i, j) = i + 1;
        }
    }

    blaze::DynamicMatrix<T> X = Z % (U - u_0)/f_u;
    blaze::DynamicMatrix<T> Y = Z % (V - v_0)/f_v;

    return std::make_tuple(X, Y);
}



#endif // _UPS_ORTHO_TO_PERSP_HPP
