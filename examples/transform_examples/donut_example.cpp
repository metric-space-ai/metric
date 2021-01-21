
#include "../../modules/transform/distance_potential_minimization.hpp"

//#include "donuts_test_io.hpp"
#include "assets/helpers.cpp"
#include "donuts.hpp"

#include <iostream>
#include <filesystem>

//#include "assets/helpers.cpp" // only for csv reader called in debug f() function
//#include "modules/utils/visualizer.hpp" // for only blaze2bmp_norm



//void filter_donut(std::string filename) {
//    auto donut = read_png_donut<double>(filename);
//    vector2bmp(matrix2vv(donut), filename + ".input.bmp");
//    std::cout << "processing " << filename  << "\n";
//    donut = radial_diff(donut);

//    auto mask = weightingMask<double>(donut.rows(), donut.columns(), donut.columns()/3, 6);
//    donut = mask % donut;
//    vector2bmp(matrix2vv(donut), filename + ".filtered.bmp");
//}



void fit_donut(std::string filename)
{

    //auto donut = read_png_donut<double>("assets/donuts/crop/crop_2020-07-27_16_23_01_776_donut1_128.png");
    //auto donut = read_png_donut<double>(filename);
    auto donut = read_csv_blaze<double>(filename, ",");
    vector2bmp(matrix2vv(donut), filename + ".input.bmp");

    std::cout << "processing " << filename  << ": " << donut.columns() << "*" << donut.rows() << "\n";

    //blaze::DynamicMatrix<double> debug_out = blaze::submatrix(donut, 0, 0, 10, 10);
    //std::cout << "input:\n" << debug_out << "\n";

    auto donut_input = donut;

    donut = radial_diff(donut);
    vector2bmp(matrix2vv(donut), filename + ".filtered.bmp");

    //debug_out = blaze::submatrix(donut, 0, 0, 10, 10);
    //std::cout << "filtered:\n" << debug_out << "\n";

    //auto mask = weightingMask<double>(donut.rows(), donut.columns(), donut.columns()/3, 6);
    auto mask = weightingMask<double>(donut.rows(), donut.columns(), donut.columns()*0.4, 15); // new donuts
    donut = mask % donut;

    vector2bmp(matrix2vv(mask), filename + ".mask_outer.bmp");

    //mask = weightingMask<double>(128, 128, 10, 2);
    //vector2bmp(matrix2vv(mask), "mask_inner.bmp");

    vector2bmp(matrix2vv(donut), filename + ".mask_applied.bmp");

    auto [u, v] = metric::DPM_detail::gvf(donut, 0.1, 1, 10);
    vector2bmp(matrix2vv(u), filename + ".u.bmp");
    vector2bmp(matrix2vv(v), filename + ".v.bmp");


    //auto donut = read_png_donut<double>("assets/donuts/crop/crop_2020-07-27_16_23_01_776_donut1.png");

    //auto donut = read_png_donut<double>("assets/donuts/crop/donut_6_radial_outer_128.png");
    //auto donut = read_png_donut<double>("assets/donuts/crop/donut_6_radial_outer_256.png");

    //size_t steps = 200;
    //std::vector<double> sigma = {50,30,15,5};
    size_t steps = 50; //1000; // 20;
    std::vector<double> sigma = {2}; //{1.75}; //{2, 1.25}; //{5, 2}; // {15, 2}

//    double init_x = donut.columns() / 2;
//    double init_y = donut.rows() / 2;
//    double init_r = donut.columns() / 3;
    double init_x = donut.columns() / 2;
    double init_y = donut.rows() / 2;
    double init_r = donut.columns() / 2 * 0.85;



    auto blurred = metric::DPM_detail::gaussianBlur(donut, sigma[0]);
    vector2bmp(matrix2vv(blurred), filename + ".blurred.bmp");



    auto t1 = std::chrono::steady_clock::now();

    auto result = metric::fit_hysteresis(donut, init_x, init_y, init_r, steps, sigma);//, 0.2, 1e-8);

    auto t2 = std::chrono::steady_clock::now();

    std::cout << "fitting result:\n xc = " << result[0] << " yc = " << result[1] << " a = " << result[2] << " b = " << result[3]
              << " phi = " << result[4] << std::endl;
    std::cout << "initial guess:\n xc = " << init_x << " yc = " << init_y << " a = " << init_r << " b = " << init_r
              << " phi = " << 0 << std::endl;
    std::cout << " (Overall time: " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
              << " s)" << std::endl;


    //vector2bmp(matrix2vv(donut), filename + "input_filtered.bmp");

    blaze::DynamicMatrix<double> donut_painted = donut;

    auto points = metric::DPM_detail::ellipse2grid(
            donut_painted.rows(), donut_painted.columns(),
            result[0], result[1],
            result[2], result[3],
            result[4]
            );
    for (size_t i = 0; i < points[0].size(); ++i) {
        donut_painted(points[1][i], points[0][i]) = -1;
        donut_input(points[1][i], points[0][i]) = -1;
    }

    auto init_points = metric::DPM_detail::ellipse2grid(
            donut_painted.rows(), donut_painted.columns(),
            init_x, init_y,
            init_r, init_r,
            0
            );
    for (size_t i = 0; i < init_points[0].size(); ++i) {
        if (init_points[0][i] < donut_input.columns() && init_points[1][i] < donut_input.rows()) {
            donut_painted(init_points[1][i], init_points[0][i]) = -0.5;
            donut_input(init_points[1][i], init_points[0][i]) = -0.5;
        } else {
            std::cout << init_points[0][i] << " " << init_points[1][i] << "\n";
        }

    }

    vector2bmp(matrix2vv(donut_painted), filename + ".fitting_result.bmp");
    vector2bmp(matrix2vv(donut_input), filename + ".fitting_result_on_filtered.bmp");


}


//* donut images

int main() {

    //fit_donut("assets/donuts/crop/crop_2020-07-27_16_23_01_776_donut1_128.png");
    //fit_donut("ring.png");

    //fit_donut("assets/donuts/crop/donut1.png");
    //fit_donut("assets/donuts/crop/donut2.png");

    fit_donut("assets/donuts/crop/donut1.csv");
    fit_donut("assets/donuts/crop/donut2.csv");

    return 0;
}

//*/




/* ellipse2grid test

int main() {
    auto ellipse = metric::DPM_detail::ellipse2grid(50, 50, 24, 15.7, 5.3, 10.5, -1.1);
    for (auto v : ellipse) {
        std::cout << v << "\n length: " << v.size() << "\n";
    }
    return 0;
}

//*/



/* gvf & torsioin test, fits ok

int main() {

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

    auto [u, v] = metric::DPM_detail::gvf(I, 0.1, 1, 10);

    std::cout << "u:\n" << u << "v:\n" << v << "\n";

    std::vector<blaze::DynamicVector<double>> x_y_theta = metric::DPM_detail::ellipse2grid(10, 10, 5, 5, 2, 3, 0.1);

    std::cout << "x:\n" << x_y_theta[0] << "y:\n" << x_y_theta[1] << "theta:\n" <<x_y_theta[2] << "\n";

    double torsion = metric::DPM_detail::torsion_moment(u, v, x_y_theta[0], x_y_theta[1], x_y_theta[2], 5, 5, 0.1);

    std::cout << "torsion moment: " << torsion << "\n";

    return 0;
}


// */


/*

int main() { // convert single file to csv, png reader fails!

    auto donut = read_png_donut<double>("test01.png");
    blaze::DynamicMatrix<double> fragm = blaze::submatrix(donut, 0, 0, 20, 20);
    std::cout << fragm;
    blaze_dm_to_csv(fragm, "fragm01.csv");
    blaze_dm_to_csv(donut, "test01.csv");

//    auto donut = read_png_donut<double>("assets/donuts/crop/donut1.png");
//    blaze_dm_to_csv(donut, "assets/donuts/crop/donut1.csv");
//    donut = read_png_donut<double>("assets/donuts/crop/donut2.png");
//    blaze_dm_to_csv(donut, "assets/donuts/crop/donut2.csv");
//    donut = read_png_donut<double>("assets/donuts/crop/donut_6_radial_outer_128.png");
//    blaze_dm_to_csv(donut, "assets/donuts/crop/donut_6_radial_outer_128.csv");
//    donut = read_png_donut<double>("assets/donuts/crop/donut_6_radial_outer_256.png");
//    blaze_dm_to_csv(donut, "assets/donuts/crop/donut_6_radial_outer_256.csv");

    return 0;
}


// */




/* batch

int main() {

    //std::string path = "assets/donuts/crop/crop256";
    std::string path = "assets/donuts/crop/crop128";
    for (const auto & entry : std::filesystem::directory_iterator(path)) {
        //std::cout << entry.path() << std::endl;
        if (entry.path().extension() == ".png")
            fit_donut(entry.path());
            //filter_donut(entry.path());
    }

    return 0;
}

// */



//// fit_ellipse by parts - for debugging

//int f() { // function for debug purpose
////int main() {

////    auto donut = read_png_donut<double>("assets/donuts/crop/crop_2020-07-27_16_23_01_776_donut1_128.png");
////    donut = radial_diff(donut);

////    auto mask = weightingMask<double>(donut.rows(), donut.columns(), donut.columns()/3, 6);
////    donut = mask % donut;

//    /* // generate & save ring
//    auto ring = weightingMask<double>(128 - 7, 128 - 7, 40, 6);
//    blaze::DynamicMatrix<double> donut (128, 128, 0);
//    blaze::submatrix(donut, 7, 7, 128 - 7, 128 - 7) = ring;
//    blaze_dm_to_csv(donut, "ring.csv");
//    vector2bmp(matrix2vv(donut), "ring.bmp");
//    // */

//    auto donut = read_csv_blaze<double>("test01.csv", ",");


//    //vector2bmp(matrix2vv(mask), "mask_outer.bmp");

//    //mask = weightingMask<double>(128, 128, 10, 2);
//    //vector2bmp(matrix2vv(mask), "mask_inner.bmp");

//    //vector2bmp(matrix2vv(donut), "mask_applied.bmp");


//    //auto donut = read_png_donut<double>("assets/donuts/crop/crop_2020-07-27_16_23_01_776_donut1.png");

//    //auto donut = read_png_donut<double>("assets/donuts/crop/donut_6_radial_outer_128.png");
//    //auto donut = read_png_donut<double>("assets/donuts/crop/donut_6_radial_outer_256.png");

//    //size_t steps = 200;
//    //std::vector<double> sigma = {50,30,15,5};
//    //size_t steps = 1000; // 20;
//    std::vector<double> sigma = {2}; //{1.75}; //{2, 1.25}; //{5, 2}; // {15, 2}

//    double init_x = donut.columns() / 2;
//    double init_y = donut.rows() / 2;
//    double init_r = donut.columns() / 3;


////    std::cout << "initial ellipse position:\n xc = " << init_x << " yc = " << init_y << " a = " << init_r << " b = " << init_r
////              << " phi = " << 0 << std::endl;

//    //* // padding & gradient tests
////    blaze::DynamicMatrix<double> A {{1, 2, 3, 4, 5}, {6, 7, 8, 9, 10}, {11, 12, 13, 14, 15}, {16, 17, 18, 19, 20}};
////    //blaze::DynamicMatrix<double> A {{0, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}};

////    auto B = metric::DPM_detail::addPad(A);
////    std::cout << A << "\n";
////    std::cout << B << "\n";
////    //auto G = metric::DPM_detail::gradient(A); //B);
////    //std::cout << std::get<0>(G) << "\n" << std::get<1>(G) << "\n";
////    metric::DPM_detail::updatePad(A);
////    std::cout << A << "\n";
////    metric::DPM_detail::updatePad(B);
////    std::cout << B << "\n";
////    blaze::submatrix(B, 1, 1, 4, 5) = {{51, 52, 53, 54, 55}, {56, 57, 58, 59, 60}, {61, 62, 63, 64, 65}, {66, 67, 68, 69, 70}};
////    std::cout << B << "\n";
////    metric::DPM_detail::updatePad(B);
////    std::cout << B << "\n";
////    auto C = metric::DPM_detail::removePad(B);
////    std::cout << C << "\n";
//    // */

//    /* // GVF test

//    blaze::DynamicMatrix<double> I = {
//        {1, 0, 0, 0, 0 ,0, 0, 0, 0, 1},
//        {0, 8, 2, 0, 0 ,0, 0, 0, 7, 0},
//        {0, 0, 2, 0, 0 ,0, 0, 0, 0, 0},
//        {0, 0, 2, 5, 0 ,0, 0, 0, 0, 0},
//        {0, 0, 3, 6, 2 ,0, 0, 0, 0, 0},
//        {1, 0, 2, 4, 1 ,0, 0, 1, 0, 0},
//        {0, 0, 2, 0, 0 ,0, 1, 2, 1, 0},
//        {0, 0, 1, 0, 0 ,1, 2, 3, 2, 1},
//        {0, 5, 0, 0, 0 ,0, 1, 2, 6, 0},
//        {1, 0, 2, 0, 0 ,0, 0, 1, 0, 1}
//    };

//    auto [h_1, v_1] = metric::DPM_detail::gvf(I, 0.1, 1, 10);

//    std::cout << h_1 << "\n" << v_1 << "\n";
//    return 0;

////            // original code
////            blaze::DynamicMatrix<double> f(I);

////            size_t m = f.rows();
////            size_t n = f.columns();

////            // normalization
////            auto fmin = blaze::min(f);
////            auto fmax = blaze::max(f);

////            if (fmax <= fmin) {
////                std::cout << "Error: constant Input Matrix." << std::endl;
////            }
////            for (size_t i = 0; i < m; i++) {
////                for (size_t j = 0; j < n; j++) {
////                    f(i, j) = (f(i, j) - fmin) / (fmax - fmin);
////                }
////            }

////    auto f2 = metric::DPM_detail::addPad(f);

////    //std::cout << f2 << "\n";

////    auto [fx, fy] = metric::DPM_detail::gradient(f2);

////    //std::cout << fx << "\n" << fy << "\n";

////            // originl code
////            // square of magnitude
////            blaze::DynamicMatrix<double> fxy_square((m + 2), (n + 2));
////            for (size_t i = 0; i < m + 2; i++) {
////                for (size_t j = 0; j < n + 2; ++j) {
////                    fxy_square(i, j) = fx(i, j) * fx(i, j) + fy(i, j) * fy(i, j);
////                }
////            }

////            blaze::DynamicMatrix<double> u1(fx);
////            blaze::DynamicMatrix<double> v1(fy);
////            //blaze::DynamicMatrix<double> Lu1((m + 2), (n + 2));
////            //blaze::DynamicMatrix<double> Lv1((m + 2), (n + 2));
////            blaze::DynamicMatrix<double> Lu1;//((m + 2), (n + 2), 0);
////            blaze::DynamicMatrix<double> Lv1;//((m + 2), (n + 2), 0);

////            //for (size_t it = 0; it < iter; it++) {
////            for (size_t it = 0; it < 1; it++) {
////                metric::DPM_detail::updatePad(u1);
////                metric::DPM_detail::updatePad(v1);
////                Lu1 = metric::DPM_detail::laplacian(u1);
////                Lv1 = metric::DPM_detail::laplacian(v1);

////                for (size_t i = 0; i < (m + 2); i++) {
////                    for (size_t j = 0; j < (n + 2); ++j) {
////                        u1(i, j) = u1(i, j) + alpha * (mu * Lu1(i, j) - fxy_square(i, j) * (u1(i, j) - fx(i, j)));
////                        v1(i, j) = v1(i, j) + alpha * (mu * Lv1(i, j) - fxy_square(i, j) * (v1(i, j) - fy(i, j)));

////                    }
////                }
////            }

////    //std::cout << fxy_square << "\n";
////    std::cout << u1 << "\n";
////    std::cout << v1 << "\n";

//    // */


//    /* // mask test
//    auto mask = weightingMask<double>(donut.rows(), donut.columns(), donut.columns()/3, 6);
//    vector2bmp(matrix2vv(mask), "mask_outer.bmp");
//    //mask = weightingMask<double>(128, 128, 10, 2);
//    //vector2bmp(matrix2vv(mask), "mask_inner.bmp");

//    blaze::DynamicMatrix<double> applied = mask % donut;
//    vector2bmp(matrix2vv(applied), "mask_applied.bmp");

//    //std::cout << mask << "\n";
//    // */


//    //* // fit_ellipse & forces test

//    init_x = donut.columns() / 2;
//    init_y = donut.rows() / 2;
//    init_r = donut.columns() / 3;

//    //size_t filtersize = round(sigma[0] * 6); // 3 sigma
//    //metric::imfilter<double, 1, metric::FilterType::GAUSSIAN, metric::PadDirection::BOTH, metric::PadType::CONST> f(filtersize, filtersize, sigma[0]);
//    //auto I1 = f(donut);
//    //auto I1 = metric::DPM_detail::gaussianBlur(donut, sigma[0]);


////    auto gk = metric::DPM_detail::gaussianKernel(sigma[0]);
////    blaze::DynamicMatrix<double> I1 = metric::DPM_detail::blackPaddedConv(donut, gk);
////    vector2bmp(matrix2vv(I1), "blurred.bmp");
////    mat2bmp::blaze2bmp_norm(I1, "blurred_norm.bmp");
////    //I1 = blaze::submatrix(I1, (I1.rows() - donut.rows()) / 2, (I1.columns() - donut.columns()) / 2, donut.rows(), donut.columns());
////    //vector2bmp(matrix2vv(I1), "blurred_cropped.bmp");

////    std::cout << "blur input: min: " << blaze::min(donut) << ", max: " << blaze::max(donut) << "\n";
////    std::cout << "GVF input: min: " << blaze::min(I1) << ", max: " << blaze::max(I1) << "\n";


//    auto I1 = donut;

//    auto [u1, v1] = metric::DPM_detail::gvf(I1, 0.1, 1, 10);
//    vector2bmp(matrix2vv(u1), "u1.bmp");
//    vector2bmp(matrix2vv(v1), "v1.bmp");

//    u1 = read_csv_blaze<double>("u.csv", ","); // TODO remove
//    v1 = read_csv_blaze<double>("v.csv", ",");
//    vector2bmp(matrix2vv(u1), "u2.bmp");
//    vector2bmp(matrix2vv(v1), "v2.bmp");

//    std::cout << "input: " << blaze::min(I1) << ".." << blaze::max(I1) << "\n";
//    std::cout << "u1: " << blaze::min(u1) << ".." << blaze::max(u1) << "\n";
//    std::cout << "v1: " << blaze::min(v1) << ".." << blaze::max(v1) << "\n";

//    //blaze_dm_to_csv(I1, "test01_read.csv");
//    //blaze_dm_to_csv(u1, "u1.csv");
//    //blaze_dm_to_csv(v1, "v1.csv");

//    //std::cout << "\ninput: \n" << donut << "\n";
//    //std::cout << "\nblurred: \n" << I1 << "\n";
//    //std::cout << "\nforse field: \n" << u1 << "\n" << v1 << "\n";

//    std::vector<double> ep = { init_x, init_y, init_r, init_r, 0 };  // initial parameter guess
//    double thresh = 1e-6;
//    double incr = 0.2;
//    blaze::DynamicVector<double> increment = { incr, incr, incr, incr, M_PI / 180 * incr };  // increment in each iteration
//    blaze::DynamicVector<double> threshold = { thresh, thresh, thresh, thresh, thresh };  // threshold for forces/torsinal moments
//    double half_min_size = (donut.rows() < donut.columns() ? donut.rows() : donut.columns()) / 2.0;
//    std::vector<double> bound = { 5, half_min_size, 5, half_min_size };  // the lower/upper bounds of a and b

//    std::cout << "\nfit_ellipse test:\n";

////    ep = metric::DPM_detail::fit_ellipse(ep, sigma[0] / 5 * increment, sigma[0] / 5 * threshold, bound, u1, v1, steps / sigma.size());
////    for (size_t i = 0; i < ep.size(); ++i) {
////        std::cout << ep[i] << "\t";
////    }
////    std::cout << "\n  by parts:\n";


//    size_t m = u1.rows();
//    size_t n = u1.columns();

//    double xc = init_x;
//    double yc = init_y;
//    double a = init_r;
//    double b = init_r;
//    double phi = 0;

//    increment = sigma[0] / 5 * increment;
//    threshold = sigma[0] / 5 * threshold;

//    size_t local_n_iter = 2000;

//    for (size_t it = 0; it < local_n_iter; ++it) {

//        // compute grid points from ellipse parameter
//        std::vector<blaze::DynamicVector<double>> x_y_theta = metric::DPM_detail::ellipse2grid(m, n, xc, yc, a, b, phi);

//        // torsion along the ellpise about center
//        double torsion = metric::DPM_detail::torsion_moment(u1, v1, x_y_theta[0], x_y_theta[1], x_y_theta[2], xc, yc, phi);

//        //std::cout << "theta: \n" << x_y_theta[0] << "\n" << x_y_theta[1] << "\n" << x_y_theta[2] << "\n";
//        //std::cout << "gvf_x: min: " << blaze::min(u1) << ", max: " << blaze::max(u1) << "\n";
//        //std::cout << "gvf_y: min: " << blaze::min(v1) << ", max: " << blaze::max(v1) << "\n";


//        // update phi
//        if (torsion > threshold[4]) {
//            phi = phi + increment[4];
//        }
//        if (torsion < -threshold[4]) {
//            phi = phi - increment[4];
//        }

//        std::cout << "torsion_moment: \n" << torsion << "\n";

//        // F_around
//        blaze::DynamicMatrix<double> iresult = metric::DPM_detail::contourForces(u1, v1, x_y_theta[0], x_y_theta[1]);
//        blaze::DynamicVector<double, blaze::rowVector> F_round = blaze::sum<blaze::columnwise>(iresult);

//        for (size_t i = 0; i < F_round.size(); ++i) {
//            F_round[i] = F_round[i] / double(x_y_theta[2].size());
//        }

//        std::vector<double> Fround(F_round.size());
//        for (size_t i = 0; i < F_round.size(); ++i) {
//            Fround[i] = F_round[i];
//        }

//        std::cout << "f_round: \n" << F_round << "\n";


//        std::vector<double> x_index1;
//        std::vector<double> y_index1;
//        std::vector<double> x_index2;
//        std::vector<double> y_index2;
//        std::vector<double> x_index3;
//        std::vector<double> y_index3;
//        std::vector<double> x_index4;
//        std::vector<double> y_index4;

//        for (size_t i = 0; i < x_y_theta[0].size(); ++i) {
//            if (x_y_theta[2][i] > M_PI * 3 / 4 && x_y_theta[2][i] < M_PI * 5 / 4) {
//                x_index1.push_back(x_y_theta[0][i]);
//                y_index1.push_back(x_y_theta[1][i]);
//            }
//            if (x_y_theta[2][i] < M_PI / 4 || x_y_theta[2][i] > M_PI * 7 / 4) {
//                x_index2.push_back(x_y_theta[0][i]);
//                y_index2.push_back(x_y_theta[1][i]);
//            }
//            if (x_y_theta[2][i] > M_PI / 4 && x_y_theta[2][i] < M_PI * 3 / 4) {
//                x_index3.push_back(x_y_theta[0][i]);
//                y_index3.push_back(x_y_theta[1][i]);
//            }
//            if (x_y_theta[2][i] > M_PI * 5 / 4 && x_y_theta[2][i] < M_PI * 7 / 4) {
//                x_index4.push_back(x_y_theta[0][i]);
//                y_index4.push_back(x_y_theta[1][i]);
//            }
//        }

//        double F_left = metric::DPM_detail::force(u1, v1, x_index1, y_index1, std::cos(phi), std::sin(phi));
//        double F_right = metric::DPM_detail::force(u1, v1, x_index2, y_index2, -std::cos(phi), -std::sin(phi));
//        double F_up = metric::DPM_detail::force(u1, v1, x_index3, y_index3, std::sin(phi), -std::cos(phi));
//        double F_down = metric::DPM_detail::force(u1, v1, x_index4, y_index4, -std::sin(phi), std::cos(phi));

//        // std::cout << "it=" << it << " --> " << F_left << " " << F_right << " " << F_up << " " << F_down
//        //           << std::endl;

//        // update xc and yc
//        double F_left_right = F_round[0] * 1 + F_round[1] * 0;
//        if (F_left_right > threshold[0]) {
//            xc = xc + increment[0];
//            ;
//        } else if (F_left_right < -threshold[0]) {
//            xc = xc - increment[0];
//        }

//        double F_down_up = F_round[0] * 0 + F_round[1] * 1;
//        if (F_down_up > threshold[1]) {
//            yc = yc + increment[1];
//        } else if (F_down_up < -threshold[1]) {
//            yc = yc - increment[1];
//        }

//        // update xc and yc again according to diagonal force
//        double F_diag1 = F_round[0] * 0.7071 + F_round[1] * 0.7071;
//        if (F_diag1 > threshold[0] + threshold[1]) {
//            xc = xc + increment[0];
//            yc = yc + increment[1];
//        } else if (F_diag1 < -threshold[0] - threshold[1]) {
//            xc = xc - increment[0];
//            yc = yc - increment[1];
//        }

//        double F_diag2 = F_round[0] * (-0.7071) + F_round[1] * 0.7071;
//        if (F_diag2 > threshold[0] + threshold[1]) {
//            xc = xc - increment[0];
//            yc = yc + increment[1];
//        } else if (F_diag2 < -threshold[0] - threshold[1]) {
//            xc = xc + increment[0];
//            yc = yc - increment[1];
//        }

//        // update a and b

//        if (F_left + F_right > threshold[2])
//            a = a - increment[2];
//        else if (F_left + F_right < -threshold[2])
//            a = a + increment[2];

//        if (F_up + F_down > threshold[3])
//            b = b - increment[3];
//        else if (F_up + F_down < -threshold[3])
//            b = b + increment[3];

//        if (b > a) {
//            std::swap(a, b);
//            phi = std::fmod(phi + M_PI / 2, M_PI);
//        }

//        // restrict a and b using lower and upper bounds
//        if (a > bound[1])
//            a = bound[1];

//        if (a < bound[0])
//            a = bound[0];

//        if (b > bound[3])
//            b = bound[3];

//        if (b < bound[2])
//            b = bound[2];

//        std::cout << "iter result: " << xc << " " << yc << " " << a << " " << b << " " << phi << " | " << it << "\n\n";  // TODO remove

//    }

//    // */

//    return 0;
//}



