
#include "../../modules/transform/distance_potential_minimization.hpp"
#include "donuts.hpp"
#include <iostream>

//#include "../../modules/utils/image_processing/image_filter.hpp" // for only test call of imfilter,TODO remove
#include "assets/helpers.cpp"
#include "modules/utils/visualizer.hpp" // for only blaze2bmp_norm

int main()
{

    auto donut = read_png_donut<double>("assets/donuts/crop/crop_2020-07-27_16_23_01_776_donut1_128.png");
    vector2bmp(matrix2vv(donut), "input.bmp");
    donut = radial_diff(donut);

    auto mask = weightingMask<double>(donut.rows(), donut.columns(), donut.columns()/3, 6);
    vector2bmp(matrix2vv(mask), "mask_outer.bmp");
    //mask = weightingMask<double>(128, 128, 10, 2);
    //vector2bmp(matrix2vv(mask), "mask_inner.bmp");

    donut = mask % donut;
    vector2bmp(matrix2vv(donut), "mask_applied.bmp");


    //auto donut = read_png_donut<double>("assets/donuts/crop/crop_2020-07-27_16_23_01_776_donut1.png");

    //auto donut = read_png_donut<double>("assets/donuts/crop/donut_6_radial_outer_128.png");
    //auto donut = read_png_donut<double>("assets/donuts/crop/donut_6_radial_outer_256.png");

    //size_t steps = 200;
    //std::vector<double> sigma = {50,30,15,5};
    size_t steps = 20;
    std::vector<double> sigma = {5, 2}; // {15, 2}

    double init_x = donut.columns() / 2;
    double init_y = donut.rows() / 2;
    double init_r = donut.columns() / 3;

    auto t1 = std::chrono::steady_clock::now();

    auto result = metric::fit_hysteresis(donut, init_x, init_y, init_r, steps, sigma); //, 0.2, 1e-8); // TODO remove non-default thresold

    auto t2 = std::chrono::steady_clock::now();

    std::cout << " xc = " << result[0] << " yc = " << result[1] << " a = " << result[2] << " b = " << result[3]
              << " phi = " << result[4] << std::endl;
    std::cout << " (Overall time: " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
              << " s)" << std::endl;

    vector2bmp(matrix2vv(donut), "input_filtered.bmp");

    blaze::DynamicMatrix<double> donut_painted = donut;

    auto points = metric::DPM_detail::ellipse2grid(
            donut_painted.rows(), donut_painted.columns(),
            result[0], result[1],
            result[2], result[3],
            result[4]
            );
    for (size_t i = 0; i < points[0].size(); ++i) {
        donut_painted(points[0][i], points[1][i]) = -1;
    }

    auto init_points = metric::DPM_detail::ellipse2grid(
            donut_painted.rows(), donut_painted.columns(),
            init_x, init_y,
            init_r, init_r,
            0
            );
    for (size_t i = 0; i < init_points[0].size(); ++i) {
        donut_painted(init_points[0][i], init_points[1][i]) = -0.5;
    }

    vector2bmp(matrix2vv(donut_painted), "fitting_result.bmp");

    std::cout << "initial ellipse position:\n xc = " << init_x << " yc = " << init_y << " a = " << init_r << " b = " << init_r
              << " phi = " << 0 << std::endl;

    //* // padding & gradient tests, TODO delete
//    blaze::DynamicMatrix<double> A {{1, 2, 3, 4, 5}, {6, 7, 8, 9, 10}, {11, 12, 13, 14, 15}, {16, 17, 18, 19, 20}};
//    //blaze::DynamicMatrix<double> A {{0, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}};

//    auto B = metric::DPM_detail::addPad(A);
//    std::cout << A << "\n";
//    std::cout << B << "\n";
//    //auto G = metric::DPM_detail::gradient(A); //B);
//    //std::cout << std::get<0>(G) << "\n" << std::get<1>(G) << "\n";
//    metric::DPM_detail::updatePad(A);
//    std::cout << A << "\n";
//    metric::DPM_detail::updatePad(B);
//    std::cout << B << "\n";
//    blaze::submatrix(B, 1, 1, 4, 5) = {{51, 52, 53, 54, 55}, {56, 57, 58, 59, 60}, {61, 62, 63, 64, 65}, {66, 67, 68, 69, 70}};
//    std::cout << B << "\n";
//    metric::DPM_detail::updatePad(B);
//    std::cout << B << "\n";
//    auto C = metric::DPM_detail::removePad(B);
//    std::cout << C << "\n";
    // */

    /* // GVF test, TODO remove

    blaze::DynamicMatrix<double> I = {
        {1, 0, 0, 0, 0 ,0, 0, 0, 0, 1},
        {0, 8, 2, 0, 0 ,0, 0, 0, 7, 0},
        {0, 0, 2, 0, 0 ,0, 0, 0, 0, 0},
        {0, 0, 2, 5, 0 ,0, 0, 0, 0, 0},
        {0, 0, 3, 6, 2 ,0, 0, 0, 0, 0},
        {1, 0, 2, 4, 1 ,0, 0, 1, 0, 0},
        {0, 0, 2, 0, 0 ,0, 1, 2, 1, 0},
        {0, 0, 1, 0, 0 ,1, 2, 3, 2, 1},
        {0, 5, 0, 0, 0 ,0, 1, 2, 6, 0},
        {1, 0, 2, 0, 0 ,0, 0, 1, 0, 1}
    };

    auto [h_1, v_1] = metric::DPM_detail::gvf(I, 0.1, 1, 10);

    std::cout << h_1 << "\n" << v_1 << "\n";

//            // original code
//            blaze::DynamicMatrix<double> f(I);

//            size_t m = f.rows();
//            size_t n = f.columns();

//            // normalization
//            auto fmin = blaze::min(f);
//            auto fmax = blaze::max(f);

//            if (fmax <= fmin) {
//                std::cout << "Error: constant Input Matrix." << std::endl;
//            }
//            for (size_t i = 0; i < m; i++) {
//                for (size_t j = 0; j < n; j++) {
//                    f(i, j) = (f(i, j) - fmin) / (fmax - fmin);
//                }
//            }

//    auto f2 = metric::DPM_detail::addPad(f);

//    //std::cout << f2 << "\n";

//    auto [fx, fy] = metric::DPM_detail::gradient(f2);

//    //std::cout << fx << "\n" << fy << "\n";

//            // originl code
//            // square of magnitude
//            blaze::DynamicMatrix<double> fxy_square((m + 2), (n + 2));
//            for (size_t i = 0; i < m + 2; i++) {
//                for (size_t j = 0; j < n + 2; ++j) {
//                    fxy_square(i, j) = fx(i, j) * fx(i, j) + fy(i, j) * fy(i, j);
//                }
//            }

//            blaze::DynamicMatrix<double> u1(fx);
//            blaze::DynamicMatrix<double> v1(fy);
//            //blaze::DynamicMatrix<double> Lu1((m + 2), (n + 2));
//            //blaze::DynamicMatrix<double> Lv1((m + 2), (n + 2));
//            blaze::DynamicMatrix<double> Lu1;//((m + 2), (n + 2), 0);
//            blaze::DynamicMatrix<double> Lv1;//((m + 2), (n + 2), 0);

//            //for (size_t it = 0; it < iter; it++) {
//            for (size_t it = 0; it < 1; it++) {
//                metric::DPM_detail::updatePad(u1);
//                metric::DPM_detail::updatePad(v1);
//                Lu1 = metric::DPM_detail::laplacian(u1);
//                Lv1 = metric::DPM_detail::laplacian(v1);

//                for (size_t i = 0; i < (m + 2); i++) {
//                    for (size_t j = 0; j < (n + 2); ++j) {
//                        u1(i, j) = u1(i, j) + alpha * (mu * Lu1(i, j) - fxy_square(i, j) * (u1(i, j) - fx(i, j)));
//                        v1(i, j) = v1(i, j) + alpha * (mu * Lv1(i, j) - fxy_square(i, j) * (v1(i, j) - fy(i, j)));

//                    }
//                }
//            }

//    //std::cout << fxy_square << "\n";
//    std::cout << u1 << "\n";
//    std::cout << v1 << "\n";

    // */


    /* // mask test, TODO delete
    auto mask = weightingMask<double>(donut.rows(), donut.columns(), donut.columns()/3, 6);
    vector2bmp(matrix2vv(mask), "mask_outer.bmp");
    //mask = weightingMask<double>(128, 128, 10, 2);
    //vector2bmp(matrix2vv(mask), "mask_inner.bmp");

    blaze::DynamicMatrix<double> applied = mask % donut;
    vector2bmp(matrix2vv(applied), "mask_applied.bmp");

    //std::cout << mask << "\n";
    // */


    /* // fit_ellipse & forces test, TODO delete

    init_x = donut.columns() / 2;
    init_y = donut.rows() / 2;
    init_r = donut.columns() / 3;

    //size_t filtersize = round(sigma[0] * 6); // 3 sigma
    //metric::imfilter<double, 1, metric::FilterType::GAUSSIAN, metric::PadDirection::BOTH, metric::PadType::CONST> f(filtersize, filtersize, sigma[0]);
    //auto I1 = f(donut);
    //auto I1 = metric::DPM_detail::gaussianBlur(donut, sigma[0]);


    auto gk = metric::DPM_detail::gaussianKernel(sigma[0]);
    blaze::DynamicMatrix<double> I1 = metric::DPM_detail::blackPaddedConv(donut, gk);
    vector2bmp(matrix2vv(I1), "blurred.bmp");
    mat2bmp::blaze2bmp_norm(I1, "blurred_norm.bmp");
    //I1 = blaze::submatrix(I1, (I1.rows() - donut.rows()) / 2, (I1.columns() - donut.columns()) / 2, donut.rows(), donut.columns());
    //vector2bmp(matrix2vv(I1), "blurred_cropped.bmp");

    std::cout << "blur input: min: " << blaze::min(donut) << ", max: " << blaze::max(donut) << "\n";
    std::cout << "GVF input: min: " << blaze::min(I1) << ", max: " << blaze::max(I1) << "\n";

    auto [u1, v1] = metric::DPM_detail::gvf(I1, 0.1, 1, 10);
    vector2bmp(matrix2vv(u1), "u1.bmp");
    vector2bmp(matrix2vv(v1), "v1.bmp");

    //std::cout << "\ninput: \n" << donut << "\n";
    //std::cout << "\nblurred: \n" << I1 << "\n";
    //std::cout << "\nforse field: \n" << u1 << "\n" << v1 << "\n";

    std::vector<double> ep = { init_x, init_y, init_r, init_r, 0 };  // initial parameter guess
    double thresh = 1e-6;
    double incr = 0.2;
    blaze::DynamicVector<double> increment = { incr, incr, incr, incr, M_PI / 180 * incr };  // increment in each iteration
    blaze::DynamicVector<double> threshold = { thresh, thresh, thresh, thresh, thresh };  // threshold for forces/torsinal moments
    double half_min_size = (donut.rows() < donut.columns() ? donut.rows() : donut.columns()) / 2.0;
    std::vector<double> bound = { 5, half_min_size, 5, half_min_size };  // the lower/upper bounds of a and b

    std::cout << "\nfit_ellipse test:\n";

//    ep = metric::DPM_detail::fit_ellipse(ep, sigma[0] / 5 * increment, sigma[0] / 5 * threshold, bound, u1, v1, steps / sigma.size());
//    for (size_t i = 0; i < ep.size(); ++i) {
//        std::cout << ep[i] << "\t";
//    }
//    std::cout << "\n  by parts:\n";


    size_t m = u1.rows();
    size_t n = u1.columns();

    double xc = init_x;
    double yc = init_y;
    double a = init_r;
    double b = init_r;
    double phi = 0;

    size_t local_n_iter = 1;

    for (size_t it = 0; it < local_n_iter; ++it) {

        // compute grid points from ellipse parameter
        std::vector<blaze::DynamicVector<double>> x_y_theta = metric::DPM_detail::ellipse2grid(m, n, xc, yc, a, b, phi);

        // torsion along the ellpise about center
        double torsion = metric::DPM_detail::torsion_moment(u1, v1, x_y_theta[0], x_y_theta[1], x_y_theta[2], xc, yc, phi);

        //std::cout << "theta: \n" << x_y_theta[0] << "\n" << x_y_theta[1] << "\n" << x_y_theta[2] << "\n";
        std::cout << "gvf_x: min: " << blaze::min(u1) << ", max: " << blaze::max(u1) << "\n";
        std::cout << "gvf_y: min: " << blaze::min(v1) << ", max: " << blaze::max(v1) << "\n";


        // update phi
        if (torsion > threshold[4]) {
            phi = phi + increment[4];
        }
        if (torsion < -threshold[4]) {
            phi = phi - increment[4];
        }

        std::cout << "torsion_moment: \n" << torsion << "\n";

        // F_around
        blaze::DynamicMatrix<double> iresult = metric::DPM_detail::contourForces(u1, v1, x_y_theta[0], x_y_theta[1]);
        blaze::DynamicVector<double, blaze::rowVector> F_round = blaze::sum<blaze::columnwise>(iresult);

        for (size_t i = 0; i < F_round.size(); ++i) {
            F_round[i] = F_round[i] / double(x_y_theta[2].size());
        }

        std::vector<double> Fround(F_round.size());
        for (size_t i = 0; i < F_round.size(); ++i) {
            Fround[i] = F_round[i];
        }

        std::cout << "f_round: \n" << F_round << "\n";

    }



    // */


    return 0;
}
