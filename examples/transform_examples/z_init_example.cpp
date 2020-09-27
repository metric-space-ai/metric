
#include "assets/helpers.cpp"
#include "donuts.hpp"

#include "../../modules/transform/distance_potential_minimization.hpp"  // for only ellipse2grid, TODO remove
#include <cmath>  // for only M_PI


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
//     double phi_o = 0;

    double xc_i = 250;
    double yc_i = 200;
    double a_i = 40;
    double b_i = 20;
    double phi_i = 2*M_PI/3;

    double xc_o = 250;
    double yc_o = 220;
    double a_o = 170;
    double b_o = 130;
    double phi_o = M_PI/3;

    auto z = z_init(
                xc_i, yc_i, a_i, b_i, phi_i,
                xc_o, yc_o, a_o, b_o, phi_o,
                400, 500,
                0.3333
                );

    z = z / blaze::max(z);

    auto inner = metric::DPM_detail::ellipse2grid(400, 500, xc_i, yc_i, a_i, b_i, phi_i);
    auto outer = metric::DPM_detail::ellipse2grid(400, 500, xc_o, yc_o, a_o, b_o, phi_o);
    for (size_t i = 0; i < inner[0].size(); ++i) {
        z(inner[1][i], inner[0][i]) = -1;
    }
    for (size_t i = 0; i < outer[0].size(); ++i) {
        z(outer[1][i], outer[0][i]) = -1;
    }



    vector2bmp(matrix2vv(z), "z_init.bmp");

    return 0;
}
