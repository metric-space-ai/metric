
#include "assets/helpers.cpp"
#include "donuts.hpp"

#include "../../modules/transform/distance_potential_minimization.hpp"  // for only ellipse2grid, TODO remove
//#include <cmath>  // for only M_PI
#include <iostream>
#include <chrono>


int main() {

//     double xc_i = 250;
//     double yc_i = 200;
//     double a_i = 20;
//     double b_i = 40;
//     double phi_i = 0.5;

//     double xc_o = 270;
//     double yc_o = 180;
//     double a_o = 170;
//     double b_o = 130;
//     double phi_o = 1;

//    double xc_i = 250;
//    double yc_i = 200;
//    double a_i = 30;
//    double b_i = 20;
//    double phi_i = 1;

//    double xc_o = 290;
//    double yc_o = 270;
//    double a_o = 180;
//    double b_o = 120;
//    double phi_o = -0.5;

//    size_t m = 501;
//    size_t n = 601;


//    double xc_i = 120;
//    double yc_i = 130;
//    double a_i = 20;
//    double b_i = 40;
//    double phi_i = 0.5;

//    double xc_o = 135;
//    double yc_o = 125;
//    double a_o = 100;
//    double b_o = 80;
//    double phi_o = 1;

//    size_t m = 256;
//    size_t n = 256;


    double xc_i = 120;
    double yc_i = 130;
    double a_i = 20;
    double b_i = 40;
    double phi_i = 0.5;

    double xc_o = 135;
    double yc_o = 125;
    double a_o = 100;
    double b_o = 80;
    double phi_o = 1;

    size_t m = 256;
    size_t n = 256;



    double arc = 0.8; //0.3; // 0.9;

    auto t1 = std::chrono::steady_clock::now();

    auto z = z_init_s(
                xc_i, yc_i, a_i, b_i, phi_i,
                xc_o, yc_o, a_o, b_o, phi_o,
                m, n,
                arc
                );

//    auto z = z_init(
//                xc_i, yc_i, a_i, b_i, phi_i,
//                xc_o, yc_o, a_o, b_o, phi_o,
//                m, n,
//                arc,
//                2 // set this coeff to 1 for no internal scaling
//                );

    auto t2 = std::chrono::steady_clock::now();

    std::cout << "time: " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
              << " s" << std::endl;

    write_obj(z, "z.obj");

    std::cout << "max z: " << max(z) << "\n\n";
    z = z / blaze::max(z);

    // draw ellipses on the output
    auto inner = metric::DPM_detail::ellipse2grid(m, n, xc_i, yc_i, a_i, b_i, phi_i);
    auto outer = metric::DPM_detail::ellipse2grid(m, n, xc_o, yc_o, a_o, b_o, phi_o);
    for (size_t i = 0; i < inner[0].size(); ++i) {
        z(inner[1][i], inner[0][i]) = -1;
    }
    for (size_t i = 0; i < outer[0].size(); ++i) {
        z(outer[1][i], outer[0][i]) = -1;
    }

    vector2bmp(matrix2vv(z), "z_init.bmp");


    return 0;
}
