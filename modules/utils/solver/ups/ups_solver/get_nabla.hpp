#ifndef _UPS_GET_NABLA_HPP
#define _UPS_GET_NABLA_HPP

#include "../helpers/indexing.hpp"

#include "../../../../../3rdparty/blaze/Blaze.h"

#include <tuple>
//#include <iostream>  // TODO remove



enum nabla_approximation {Forward, Backward, Central};
enum nabla_boundary_condition {DirichletHomogeneous, NeumannHomogeneous}; //, NeumannConstant};



template <typename T>
std::tuple<blaze::CompressedMatrix<T>, blaze::CompressedMatrix<T>>
getNabla(
        const blaze::DynamicMatrix<unsigned char, blaze::columnMajor> & mask,
        nabla_approximation approx = Central,
        nabla_boundary_condition boundary = NeumannHomogeneous
        )
{

    //size_t out_size = mask.columns()*mask.rows();

    blaze::CompressedMatrix<T> Dx;
    blaze::CompressedMatrix<T> Dy;
    switch (approx) {
    case Forward:
        Dx = blaze::IdentityMatrix<T>(mask.columns()) * -1;
        Dy = blaze::IdentityMatrix<T>(mask.rows()) * -1;
        for (size_t i = 0; i < mask.columns() - 1; ++i) {
            Dx(i, i+1) = 1;
        }
        for (size_t i = 0; i < mask.rows() - 1; ++i) {
            Dy(i, i+1) = 1;
        }
        break;
    case Backward:
        Dx = blaze::IdentityMatrix<T>(mask.columns());
        Dy = blaze::IdentityMatrix<T>(mask.rows());
        for (size_t i = 0; i < mask.columns() - 1; ++i) {
            Dx(i+1, i) = -1;
        }
        for (size_t i = 0; i < mask.rows() - 1; ++i) {
            Dy(i+1, i) = -1;
        }
        break;
    default:  // Central
        Dx = blaze::CompressedMatrix<T>(mask.columns(), mask.columns(), 0);
        Dy = blaze::CompressedMatrix<T>(mask.rows(), mask.rows(), 0);
        for (size_t i = 0; i < mask.columns() - 1; ++i) {
            Dx(i, i+1) = 0.5;
            Dx(i+1, i) = -0.5;
        }
        for (size_t i = 0; i < mask.rows() - 1; ++i) {
            Dy(i, i+1) = 0.5;
            Dy(i+1, i) = -0.5;
        }
        break;
    }

    blaze::CompressedMatrix<T> nx = blaze::kron(Dx, blaze::IdentityMatrix<T>(mask.rows()));
    blaze::CompressedMatrix<T> ny = blaze::kron(blaze::IdentityMatrix<T>(mask.columns()), Dy);

//    std::cout << "Dx:\n" << Dx << "\n"; // TODO remove
//    std::cout << "Dy:\n" << Dy << "\n";
//    std::cout << "nx:\n" << nx << "\n";
//    std::cout << "ny:\n" << ny << "\n";

    // apply NeumannHomogeneous

    auto idx = indicesCw(mask);

    size_t s = blaze::sum(mask);  // should be equal to length of idx and height and width of nabla_x, nabla_y

    blaze::CompressedMatrix<T> nabla_x (s, s);
    blaze::CompressedMatrix<T> nabla_y (s, s);
    size_t r = 0;
    size_t c;
    for (blaze::DynamicVector<size_t>::Iterator it=idx.begin(); it!=idx.end(); ++it) {
        c = 0;
        for (blaze::DynamicVector<size_t>::Iterator it2=idx.begin(); it2!=idx.end(); ++it2) {
            nabla_x(r, c) = nx(*it, *it2);
            nabla_y(r, c) = ny(*it, *it2);
            ++c;
        }
        ++r;
    }

    //std::cout << "nabla_x:\n" << nabla_x << "\n"; // TODO remove
    //std::cout << "nabla_y:\n" << nabla_y << "\n";
    //std::cout << "idx:\n" << idx << "\n";

    if (boundary == NeumannHomogeneous) {  // here nabla matches the DirichletHomogeneous option
        for (size_t i = 0; i < s; ++i) {  // set rows where at least one neighbour does not exist to 0.
            if (sum(blaze::row(nabla_x, i)) != 0)
                blaze::row(nabla_x, i) *= 0;
            if (sum(blaze::row(nabla_y, i)) != 0)
                blaze::row(nabla_y, i) *= 0;
        }
    }


    return std::make_tuple(nabla_x, nabla_y);
}





#endif // _UPS_GET_NABLA_HPP
