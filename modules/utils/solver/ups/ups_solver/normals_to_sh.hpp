#ifndef _UPS_NORMALS_TO_SH_HPP
#define _UPS_NORMALS_TO_SH_HPP

#include "3rdparty/blaze/Blaze.h"

//#include <iostream>  // TODO remove


#ifndef M_PI
#define M_PI (3.14159265358979323846)
//#define M_PIl  3.141592653589793238462643383279502884L
#endif


/* // old version
template <typename T>
blaze::DynamicMatrix<T>
normalsToSh(blaze::DynamicMatrix<T> normals) {  //, int harmo_order = 1) {  // TODO generalize!!!!

    //int nb_harmo = (harmo_order+1)*(harmo_order+1);
    int nb_harmo = 4;

    blaze::DynamicMatrix<T> spherical_harmonics (normals.rows(), 4, 0);

    T w1 = sqrt(3/(4*M_PI));
    T w4 = sqrt(1/(4*M_PI));
    // normals * w1  // write to view, then write repeater w4 to the last col
    auto c0 = blaze::column<0>(spherical_harmonics);
    auto c1 = blaze::column<1>(spherical_harmonics);
    auto c2 = blaze::column<2>(spherical_harmonics);
    auto c3 = blaze::column<3>(spherical_harmonics);
    c0 = w1 * blaze::column<0>(normals);
    c1 = w1 * blaze::column<1>(normals);
    c2 = w1 * blaze::column<2>(normals);
    c3 = w4;

    //std::cout << "sph harm:\n" << spherical_harmonics << "\n\n";
    return spherical_harmonics;
}

// */


//*  // new version


enum harmo_order {ho_low, ho_high};



template <typename T>
blaze::DynamicMatrix<T>
normalsToSh(const blaze::DynamicMatrix<T> & normals, const int harmo_order = ho_low) {  //, int harmo_order = 1) {  // TODO generalize!!!!

    //int nb_harmo = (harmo_order+1)*(harmo_order+1);
    int nb_harmo = 9;
    if (harmo_order == ho_low)
        nb_harmo = 4;

    blaze::DynamicMatrix<T> spherical_harmonics (normals.rows(), nb_harmo, 0);

    T w0 = sqrt(3/(4*M_PI));
    T w3 = sqrt(1/(4*M_PI));
    // normals * w1  // write to view, then write repeated w4 to the last col
    auto c0 = blaze::column<0>(spherical_harmonics);
    auto c1 = blaze::column<1>(spherical_harmonics);
    auto c2 = blaze::column<2>(spherical_harmonics);
    auto c3 = blaze::column<3>(spherical_harmonics);
    c0 = w0 * blaze::column<0>(normals);
    c1 = w0 * blaze::column<1>(normals);
    c2 = w0 * blaze::column<2>(normals);
    c3 = w3;

    if (harmo_order != ho_low) {
        auto c4 = blaze::column<4>(spherical_harmonics);
        auto c5 = blaze::column<5>(spherical_harmonics);
        auto c6 = blaze::column<6>(spherical_harmonics);
        auto c7 = blaze::column<7>(spherical_harmonics);
        auto c8 = blaze::column<8>(spherical_harmonics);
        T w4 = 3*sqrt(5/(12*M_PI));
        T w5 = 3*sqrt(5/(12*M_PI));
        T w7 = 0.5*w5; // 3/2.0*sqrt(5/(12*M_PI));
        T w8 = 0.5*sqrt(5/(4*M_PI));
        c4 = w4 * blaze::column<0>(normals) * blaze::column<1>(normals); // elementwise multiplication
        c5 = w5 * blaze::column<0>(normals) * blaze::column<2>(normals);
        c6 = w5 * blaze::column<1>(normals) * blaze::column<2>(normals);
        c7 = w7 * (blaze::pow(blaze::column<0>(normals), 2) - blaze::pow(blaze::column<1>(normals), 2));
        c8 = w8 * (3 * blaze::pow(blaze::column<2>(normals), 2) - 1);
    }

    //std::cout << "sph harm:\n" << spherical_harmonics << "\n\n";
    return spherical_harmonics;
}

// */

template <typename T>
std::vector<blaze::CompressedMatrix<T>>
//std::vector<blaze::DynamicMatrix<T>>
calcJacobianWrtNormals(
        const blaze::DynamicMatrix<T> & normals,
        const int harmo_order,
        const blaze::CompressedMatrix<T> & J_n_0,
        const blaze::CompressedMatrix<T> & J_n_1,
        const blaze::CompressedMatrix<T> & J_n_2
        //const blaze::DynamicMatrix<T> & J_n_0,
        //const blaze::DynamicMatrix<T> & J_n_1,
        //const blaze::DynamicMatrix<T> & J_n_2
        )
{
    int nb_harmo = 9;
    if (harmo_order == ho_low)
        nb_harmo = 4;

    T w0 = sqrt(3/(4*M_PI));
    T w3 = sqrt(1/(4*M_PI));

    blaze::CompressedMatrix<T> J_sh_0 = w0 * J_n_0;
    blaze::CompressedMatrix<T> J_sh_1 = w0 * J_n_1;
    blaze::CompressedMatrix<T> J_sh_2 = w0 * J_n_2;
    blaze::CompressedMatrix<T> J_sh_3 (J_n_0.rows(), J_n_0.columns());
    //blaze::DynamicMatrix<T> J_sh_0 = w0 * J_n_0;
    //blaze::DynamicMatrix<T> J_sh_1 = w0 * J_n_1;
    //blaze::DynamicMatrix<T> J_sh_2 = w0 * J_n_2;
    //blaze::DynamicMatrix<T> J_sh_3 (J_n_0.rows(), J_n_0.columns(), 0);

    std::vector<blaze::CompressedMatrix<T>> J = {J_sh_0, J_sh_1, J_sh_2, J_sh_3};
    //std::vector<blaze::DynamicMatrix<T>> J = {J_sh_0, J_sh_1, J_sh_2, J_sh_3};

    if (harmo_order != ho_low) {
        T w4 = 3*sqrt(5/(12*M_PI));
        T w5 = 3*sqrt(5/(12*M_PI));
        T w7 = 3/2*sqrt(5/(12*M_PI));
        T w8 = 0.5*sqrt(5/(4*M_PI));

        // add diag
        blaze::CompressedMatrix<T> N0 (normals.rows(), normals.rows());
        blaze::CompressedMatrix<T> N1 (normals.rows(), normals.rows());
        blaze::CompressedMatrix<T> N2 (normals.rows(), normals.rows());
        //blaze::DynamicMatrix<T> N0 (normals.rows(), normals.rows(), 0);
        //blaze::DynamicMatrix<T> N1 (normals.rows(), normals.rows(), 0);
        //blaze::DynamicMatrix<T> N2 (normals.rows(), normals.rows(), 0);
        blaze::diagonal(N0) = blaze::column(normals, 0);
        blaze::diagonal(N1) = blaze::column(normals, 1);
        blaze::diagonal(N2) = blaze::column(normals, 2);
        blaze::CompressedMatrix<T> J_sh_4 = w4 * (N1 * J_n_0 + N0 * J_n_1);
        blaze::CompressedMatrix<T> J_sh_5 = w4 * (N2 * J_n_0 + N0 * J_n_2);
        blaze::CompressedMatrix<T> J_sh_6 = w5 * (N2 * J_n_1 + N1 * J_n_2);
        blaze::CompressedMatrix<T> J_sh_7 = 2 * w7 * (N0 * J_n_0 - N1 * J_n_1);
        blaze::CompressedMatrix<T> J_sh_8 = 6 * w8 * N2 * J_n_2;
        //blaze::DynamicMatrix<T> J_sh_4 = w4 * (N1 * J_n_0 + N0 * J_n_1);
        //blaze::DynamicMatrix<T> J_sh_5 = w4 * (N2 * J_n_0 + N0 * J_n_2);
        //blaze::DynamicMatrix<T> J_sh_6 = w5 * (N2 * J_n_1 + N1 * J_n_2);
        //blaze::DynamicMatrix<T> J_sh_7 = 2 * w7 * (N0 * J_n_0 - N1 * J_n_1);
        //blaze::DynamicMatrix<T> J_sh_8 = 6 * w8 * N2 * J_n_2;
        J.push_back(J_sh_4);
        J.push_back(J_sh_5);
        J.push_back(J_sh_6);
        J.push_back(J_sh_7);
        J.push_back(J_sh_8);
    }

    return J;
}


#endif // _UPS_NORMALS_TO_SH_HPP
