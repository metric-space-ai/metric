
#include "assets/helpers.cpp"
#include "donuts.hpp"

#include "../../modules/transform/distance_potential_minimization.hpp"  // for only ellipse2grid, TODO remove


int main() {

    auto z = z_init(
                250, 200, 10, 20, 0.5,
                260, 190, 160, 140, 1,
                400, 500,
                0.3333
                );

    z = z / blaze::max(z);

    auto inner = metric::DPM_detail::ellipse2grid(400, 500, 250, 200, 10, 20, 0.5);
    auto outer = metric::DPM_detail::ellipse2grid(400, 500, 260, 190, 160, 140, 1);
    for (size_t i = 0; i < inner[0].size(); ++i) {
        z(inner[1][i], inner[0][i]) = -1;
    }
    for (size_t i = 0; i < outer[0].size(); ++i) {
        z(outer[1][i], outer[0][i]) = -1;
    }



    vector2bmp(matrix2vv(z), "z_init.bmp");

    return 0;
}
