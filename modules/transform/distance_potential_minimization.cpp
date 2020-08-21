/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Michael Welsch
*/

//#define MEASURE
//#define DEBUG_OUTPUT
//#define BLAZE_DEBUG_MODE

#include "distance_potential_minimization.hpp"
//#include "../../modules/utils/image_processing/image_filter.hpp"
#include "../../modules/transform/wavelet2d.hpp" // for only Convolution2dCustom

#include <iostream>
#include <tuple>
#include <set>
#include <iterator>
#include <algorithm>
#include <cmath>

#ifdef MEASURE
#include <chrono>
#endif

namespace metric {

namespace DPM_detail {

    blaze::DynamicMatrix<double> addPad(const blaze::DynamicMatrix<double>& A)
    {
        size_t m = A.rows();
        size_t n = A.columns();
        blaze::DynamicMatrix<double> B((m + 2), (n + 2));

        for (size_t i = 0; i < m + 2; ++i) {
            for (size_t j = 0; j < n + 2; ++j) {
                B(i, j) = -1;
            }
        }

        //first row
        B(0, 0) = A(1, 1);
        for (size_t j = 0; j < n; ++j) {
            B(0, j + 1) = A(1, j);
        }
        B(0, n + 1) = A(1, n - 2);

        //inner rows
        for (size_t i = 0; i < m; ++i) {
            B(i + 1, 0) = A(i, 1);
            for (size_t j = 0; j < n; ++j) {
                B(i + 1, j + 1) = A(i, j);
            }
            B(i + 1, n + 1) = A(i, n - 2);
        }

        // //last row
        B(m + 1, 0) = A(m - 2, 1);
        for (size_t j = 0; j < n; ++j) {
            B(m + 1, j + 1) = A(m - 2, j);
        }
        B(m + 1, n + 1) = A(m - 2, n - 2);

        return B;
    }

    blaze::DynamicMatrix<double> removePad(const blaze::DynamicMatrix<double>& A)
    {

        size_t m = A.rows();
        size_t n = A.columns();
        blaze::DynamicMatrix<double> B((m - 2), (n - 2));

        for (size_t i = 0; i < m - 2; i++) {
            for (size_t j = 0; j < n - 2; j++) {
                B(i, j) = A(i + 1, (j + 1));
            }
        }
        return B;
    }

    void updatePad(blaze::DynamicMatrix<double>& A)
    {
        size_t m = A.rows();
        size_t n = A.columns();

        // first row
        A(0, 0) = A(2, 2);
        for (size_t j = 1; j < n - 1; j++) {
            A(0, j) = A(2, j);
        }
        //A(0, n) = A(2, n - 3);
        A(0, n - 1) = A(2, n - 3);

        // middle rows
        for (size_t i = 1; i < m - 1; ++i) {
            A(i, 0) = A(i, 2);
            //A(i, n) = A(i, n - 3);
            A(i, n - 1) = A(i, n - 3);
        }

        // last row
        //A(m, 0) = A(m - 2, 2);
        A(m - 1, 0) = A(m - 3, 2);
        for (size_t j = 1; j < n - 1; j++) {
            //A(m, j) = A(m - 3, j);
            A(m - 1, j) = A(m - 3, j);
        }
        //A(m, n) = A(m - 3, n - 3);
        A(m - 1, n - 1) = A(m - 3, n - 3);
    }

    std::tuple<blaze::DynamicMatrix<double>, blaze::DynamicMatrix<double>> gradient(
        const blaze::DynamicMatrix<double>& f)
    {
        size_t m = f.rows();
        size_t n = f.columns();

        blaze::DynamicMatrix<double> fx(m, n);
        blaze::DynamicMatrix<double> fy(m, n);

        for (size_t i = 0; i < m; i++) {
            for (size_t j = 0; j < n; j++) {
                // fx
                if (j == 0) {
                    fx(i, j) = f(i, j + 1) - f(i, j);
                } else {
                    if (j == (n - 1)) {
                        fx(i, j) = f(i, j) - f(i, j - 1);
                    } else {
                        fx(i, j) = (f(i, j + 1) - f(i, j - 1)) / 2;
                    }
                }

                // fy
                if (i == 0) {
                    fy(i, j) = f(i + 1, j) - f(i, j);
                } else {
                    if (i == (m - 1)) {
                        fy(i, j) = f(i, j) - f(i - 1, j);
                    } else {
                        fy(i, j) = (f(i + 1, j) - f(i - 1, j)) / 2;
                    }
                }
            }
        }
        //return std::make_tuple(fx, fy);
        return std::make_tuple(fy, fx); // replaced by Max F
    }


    blaze::DynamicMatrix<double> laplacian(const blaze::DynamicMatrix<double>& A)
    {
        size_t m = A.rows();
        size_t n = A.columns();
        blaze::DynamicMatrix<double> B(m, n);


        for (size_t i = 0; i < m; i++) {
            for (size_t j = 0; j < n; j++) {
                if (i == 0 || i == m - 1 || j == 0 || j == n - 1) {

                    B(i, j) = 0;
                } else {
                    B(i, j) = A(i + 1, j) + A(i - 1, j) + A(i, j + 1) + A(i, j - 1) - double(4) * A(i, j);
                }
            }
        }
        return B;

    }
    std::tuple<double, double, double> initialCircle(const blaze::DynamicMatrix<double>& A)
    {
        size_t m = A.rows();
        size_t n = A.columns();

        double xc = 0;
        double yc = 0;
        double r = 0;
        double sumI = 0;

        for (size_t i = 0; i < m; i++) {
            for (size_t j = 0; j < n; j++) {
                xc += (j * A(i, j));
                yc += (i * A(i, j));
                sumI += A(i, j);
            }
        }
        xc /= sumI;
        yc /= sumI;

        for (size_t i = 0; i < m; i++) {
            for (size_t j = 0; j < n; j++) {
                r += std::sqrt((i - yc) * (i - yc) + (j - xc) * (j - xc)) * A(i, j);
            }
        }
        r /= sumI;

        return std::make_tuple(xc, yc, r);
    }

    std::vector<double> linspace(double a, double b, size_t n)
    {
        std::vector<double> array;
        if (n > 1) {
            double step = (b - a) / double(n - 1);
            int count = 0;
            while (count < n) {
                array.push_back(a + count * step);
                ++count;
            }
        } else {
            array.push_back(b);
        }
        return array;
    }

    /*  Return a 2d-grid of given ellipse parameter
        the ellipse is represented by the following equation set
        x=xc+a*cos(theta)*cos(phi)-b*sin(theta)*sin(phi)
        y=yc+a*cos(theta)*sin(phi)+b*sin(theta)*cos(phi)
    m: number of rows
    n: number of columns
    yx: center point of x-axis
    yc: center of y-axis
    a: elliptic parameter a
    b: elliptic parameter b
    phi: arc
*/
    std::vector<blaze::DynamicVector<double>> ellipse2grid(
        size_t m, size_t n, double xc, double yc, double a, double b, double phi)
    {

        auto theta = linspace(0, 2 * M_PI, std::round(2 * M_PI * std::max(a, b)));

        blaze::DynamicVector<double> x(theta.size());
        blaze::DynamicVector<double> y(theta.size());

//#ifdef MEASURE
//        auto t1 = std::chrono::steady_clock::now();
//#endif
        for (size_t i = 0; i < theta.size(); ++i) {
            x[i] = std::round(xc + a * std::cos(theta[i]) * std::cos(phi) - b * std::sin(theta[i]) * std::sin(phi));
            y[i] = std::round(yc + a * std::cos(theta[i]) * std::sin(phi) + b * std::sin(theta[i]) * std::cos(phi));
        }
//#ifdef MEASURE
//        auto t2 = std::chrono::steady_clock::now();
//        auto seconds = double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
//        std::cout << "---- in ellipse2grid: theta loop of " << theta.size() << " iterations took " << seconds << " s\n";
//#endif

        // filter our non unique pairs.
        std::vector<int> liste(theta.size(), 1);
        size_t sum = theta.size();
        for (size_t i = 0; i < theta.size(); ++i) {
            for (size_t j = i + 1; j < theta.size(); ++j) {
                double d = std::abs(x[i] - x[j]) + std::abs(y[i] - y[j]);
                if (d == 0) {
                    liste[i] = 0;
                    sum -= 1;
                }
            }
        }

        std::vector<double> x1;
        std::vector<double> y1;
        std::vector<double> theta1;

        for (size_t i = 0; i < theta.size(); ++i) {
            if (liste[i] == 1) {
                x1.push_back(x[i]);
                y1.push_back(y[i]);
                theta1.push_back(theta[i]);
            }
        }

        // sort grid points by arc parameter
        std::vector<size_t> idx(theta1.size());
        std::iota(idx.begin(), idx.end(), 0);
        stable_sort(idx.begin(), idx.end(), [&theta1](size_t i1, size_t i2) { return theta1[i1] < theta1[i2]; });

        blaze::DynamicVector<double> x2(theta1.size());
        blaze::DynamicVector<double> y2(theta1.size());
        blaze::DynamicVector<double> theta2(theta1.size());
        for (size_t i = 0; i < theta1.size(); ++i) {
            x2[i] = x1[idx[i]];
            y2[i] = y1[idx[i]];
            theta2[i] = theta1[idx[i]];
        }

        // check boudaries
        if ((blaze::min(x2) < 1) || (blaze::min(y2) < 1) || (blaze::max(x2) > n) || (blaze::max(y2) > m)) {
            std::cout << "Error: Contour out of image" << std::endl;
        }

        std::vector<blaze::DynamicVector<double>> result = { x2, y2, theta2 };
        return result;
    }

    double torsion_moment(const blaze::DynamicMatrix<double>& u, const blaze::DynamicMatrix<double>& v,
        const blaze::DynamicVector<double>& x, const blaze::DynamicVector<double>& y,
        const blaze::DynamicVector<double>& theta, double xc, double yc, double phi)
    {

        size_t m = u.rows();
        size_t n = u.columns();
        size_t N = x.size();

        double torsional_moment = 0;
        double d;
        size_t r, c;
        double fx, fy;

        for (size_t i = 0; i < N; i++) {
            r = (size_t)y[i] - 1;
            c = (size_t)x[i] - 1;
            fx = u(r, c);
            fy = v(r, c);
            d = std::sqrt((x[i] - xc) * (x[i] - xc) + (y[i] - yc) * (y[i] - yc));

            torsional_moment += ((-fx * std::sin(theta[i] + phi) + fy * std::cos(theta[i] + phi)) * d);
        }
        torsional_moment /= (N * (double)N);

        return torsional_moment;
    }

    blaze::DynamicMatrix<double> contourForces(const blaze::DynamicMatrix<double>& u,
        const blaze::DynamicMatrix<double>& v, const blaze::DynamicVector<double>& x,
        const blaze::DynamicVector<double>& y)
    {
        size_t m = u.rows();
        size_t n = u.columns();
        size_t N = x.size();
        size_t r, c;

        blaze::DynamicMatrix<double> F(N, 2);
        for (size_t i = 0; i < N; i++) {
            r = (size_t)y[i] - 1;
            c = (size_t)x[i] - 1;
            F(i, 0) = u(r, c);
            F(i, 1) = v(r, c);
        }
        return F;
    }

    double force(const blaze::DynamicMatrix<double>& u, const blaze::DynamicMatrix<double>& v,
        const std::vector<double>& x, const std::vector<double>& y, double p_cos, double p_sin)
    {
        size_t m = u.rows();
        size_t n = u.columns();
        size_t N = x.size();

        double force = 0;
        size_t r, c;
        double fx, fy;
        for (size_t i = 0; i < N; i++) {
            r = (size_t)y[i] - 1;
            c = (size_t)x[i] - 1;
            fx = u(r, c);
            fy = v(r, c);
            force += (fx * p_cos + fy * p_sin);
        }
        return force / N;
    }

/*  searching for the equliribirum of the vector partial differential equation. 
taking two pictures, where one picture is static and one is based on a parametric model (an ellipse).
the algorithm finds the best model parameters to minimimize the distance potential aka gradient vector field.
gvf field is implemented using an explicit finite difference scheme.
v(x,y,t+􏰀t)= v(x,y,t)+ 􏰀t/(􏰀x􏰀y) g (|∇f|) L * v(x,y,t) −􏰀th(|∇f |) [v(x, y, t) − ∇f ].
*/ 
    std::vector<double> fit_ellipse(const std::vector<double>& init, const blaze::DynamicVector<double>& increment,
        const blaze::DynamicVector<double>& threshold, const std::vector<double>& bound,
        const blaze::DynamicMatrix<double>& gvf_x, const blaze::DynamicMatrix<double>& gvf_y, size_t iter)
    {
        size_t m = gvf_x.rows();
        size_t n = gvf_x.columns();

        double xc = init[0];
        double yc = init[1];
        double a = init[2];
        double b = init[3];
        double phi = init[4];

//#ifdef MEASURE
//        auto t1 = std::chrono::steady_clock::now();
//#endif
        //std::cout << xc << " " << yc << " " << a << " " << b << " " << phi << "\n";  // TODO remove

        for (size_t it = 0; it < iter; ++it) {

            // compute grid points from ellipse parameter
            std::vector<blaze::DynamicVector<double>> x_y_theta = ellipse2grid(m, n, xc, yc, a, b, phi);

            // torsion along the ellpise about center
            double torsion = torsion_moment(gvf_x, gvf_y, x_y_theta[0], x_y_theta[1], x_y_theta[2], xc, yc, phi);
#ifdef DEBUG_OUTPUT
            //std::cout << "theta: \n" << x_y_theta[0] << "\n" << x_y_theta[1] << "\n" << x_y_theta[2] << "\n";
            std::cout << "iteration " << it << ":\n";
            std::cout << "gvf_x: min: " << blaze::min(gvf_x) << ", max: " << blaze::max(gvf_x) << "\n";
            std::cout << "gvf_y: min: " << blaze::min(gvf_y) << ", max: " << blaze::max(gvf_y) << "\n";
#endif
            // update phi
            if (torsion > threshold[4]) {
                phi = phi + increment[4];
            }
            if (torsion < -threshold[4]) {
                phi = phi - increment[4];
            }

            // F_around
            blaze::DynamicMatrix<double> iresult = contourForces(gvf_x, gvf_y, x_y_theta[0], x_y_theta[1]);
            blaze::DynamicVector<double, blaze::rowVector> F_round = blaze::sum<blaze::columnwise>(iresult);

            for (size_t i = 0; i < F_round.size(); ++i) {
                F_round[i] = F_round[i] / double(x_y_theta[2].size());
            }

            std::vector<double> Fround(F_round.size());
            for (size_t i = 0; i < F_round.size(); ++i) {
                Fround[i] = F_round[i];
            }

            std::vector<double> x_index1;
            std::vector<double> y_index1;
            std::vector<double> x_index2;
            std::vector<double> y_index2;
            std::vector<double> x_index3;
            std::vector<double> y_index3;
            std::vector<double> x_index4;
            std::vector<double> y_index4;

            for (size_t i = 0; i < x_y_theta[0].size(); ++i) {
                if (x_y_theta[2][i] > M_PI * 3 / 4 && x_y_theta[2][i] < M_PI * 5 / 4) {
                    x_index1.push_back(x_y_theta[0][i]);
                    y_index1.push_back(x_y_theta[1][i]);
                }
                if (x_y_theta[2][i] < M_PI / 4 || x_y_theta[2][i] > M_PI * 7 / 4) {
                    x_index2.push_back(x_y_theta[0][i]);
                    y_index2.push_back(x_y_theta[1][i]);
                }
                if (x_y_theta[2][i] > M_PI / 4 && x_y_theta[2][i] < M_PI * 3 / 4) {
                    x_index3.push_back(x_y_theta[0][i]);
                    y_index3.push_back(x_y_theta[1][i]);
                }
                if (x_y_theta[2][i] > M_PI * 5 / 4 && x_y_theta[2][i] < M_PI * 7 / 4) {
                    x_index4.push_back(x_y_theta[0][i]);
                    y_index4.push_back(x_y_theta[1][i]);
                }
            }

            double F_left = force(gvf_x, gvf_y, x_index1, y_index1, std::cos(phi), std::sin(phi));
            double F_right = force(gvf_x, gvf_y, x_index2, y_index2, -std::cos(phi), -std::sin(phi));
            double F_up = force(gvf_x, gvf_y, x_index3, y_index3, std::sin(phi), -std::cos(phi));
            double F_down = force(gvf_x, gvf_y, x_index4, y_index4, -std::sin(phi), std::cos(phi));

            // std::cout << "it=" << it << " --> " << F_left << " " << F_right << " " << F_up << " " << F_down
            //           << std::endl;

            // update xc and yc
            double F_left_right = F_round[0] * 1 + F_round[1] * 0;
            if (F_left_right > threshold[0]) {
                xc = xc + increment[0];
                ;
            } else if (F_left_right < -threshold[0]) {
                xc = xc - increment[0];
            }

            double F_down_up = F_round[0] * 0 + F_round[1] * 1;
            if (F_down_up > threshold[1]) {
                yc = yc + increment[1];
            } else if (F_down_up < -threshold[1]) {
                yc = yc - increment[1];
            }

            // update xc and yc again according to diagonal force
            double F_diag1 = F_round[0] * 0.7071 + F_round[1] * 0.7071;
            if (F_diag1 > threshold[0] + threshold[1]) {
                xc = xc + increment[0];
                yc = yc + increment[1];
            } else if (F_diag1 < -threshold[0] - threshold[1]) {
                xc = xc - increment[0];
                yc = yc - increment[1];
            }

            double F_diag2 = F_round[0] * (-0.7071) + F_round[1] * 0.7071;
            if (F_diag2 > threshold[0] + threshold[1]) {
                xc = xc - increment[0];
                yc = yc + increment[1];
            } else if (F_diag2 < -threshold[0] - threshold[1]) {
                xc = xc + increment[0];
                yc = yc - increment[1];
            }

            // update a and b

            if (F_left + F_right > threshold[2])
                a = a - increment[2];
            else if (F_left + F_right < -threshold[2])
                a = a + increment[2];

            if (F_up + F_down > threshold[3])
                b = b - increment[3];
            else if (F_up + F_down < -threshold[3])
                b = b + increment[3];

            if (b > a) {
                std::swap(a, b);
                phi = std::fmod(phi + M_PI / 2, M_PI);
            }

            // restrict a and b using lower and upper bounds
            if (a > bound[1])
                a = bound[1];

            if (a < bound[0])
                a = bound[0];

            if (b > bound[3])
                b = bound[3];

            if (b < bound[2])
                b = bound[2];

            //std::cout << xc << " " << yc << " " << a << " " << b << " " << phi << " | " << it << "\n";  // TODO remove
        }

//#ifdef MEASURE
//        auto t2 = std::chrono::steady_clock::now();
//        auto seconds = double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
//        std::cout << "---- in fit_ellipse: " << iter << " iterations completed in " << seconds << " s\n";
//#endif

        std::vector<double> result = { xc, yc, a, b, phi };
        return result;
    }

    /* Gradient Vector Flow (GVF) */
    std::tuple<blaze::DynamicMatrix<double>, blaze::DynamicMatrix<double>> gvf(
        const blaze::DynamicMatrix<double>& f0, double alpha, double mu, size_t iter)
    {
        blaze::DynamicMatrix<double> f(f0);

        size_t m = f.rows();
        size_t n = f.columns();

        // normalization
        auto fmin = blaze::min(f);
        auto fmax = blaze::max(f);

        if (fmax <= fmin) {
            std::cout << "Error: constant Input Matrix." << std::endl;
        }
        for (size_t i = 0; i < m; i++) {
            for (size_t j = 0; j < n; j++) {
                f(i, j) = (f(i, j) - fmin) / (fmax - fmin);
            }
        }

        // add pads around the grid for processing
#ifdef MEASURE
        auto t1 = std::chrono::steady_clock::now();
#endif
        auto f2 = metric::DPM_detail::addPad(f);
#ifdef MEASURE
        auto t2 = std::chrono::steady_clock::now();
        auto seconds = double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
        std::cout << "--- in gvf: call of addPad took " << seconds << " s\n";
#endif

        // compute the gradient field
#ifdef MEASURE
        t1 = std::chrono::steady_clock::now();
#endif
        auto [fx, fy] = metric::DPM_detail::gradient(f2);
#ifdef MEASURE
        t2 = std::chrono::steady_clock::now();
        seconds = double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
        std::cout << "--- in gvf: call of gradient took " << seconds << " s\n";
#endif

        // square of magnitude
        blaze::DynamicMatrix<double> fxy_square((m + 2), (n + 2));
        for (size_t i = 0; i < m + 2; i++) {
            for (size_t j = 0; j < n + 2; ++j) {
                fxy_square(i, j) = fx(i, j) * fx(i, j) + fy(i, j) * fy(i, j);
            }
        }

        blaze::DynamicMatrix<double> u1(fx);
        blaze::DynamicMatrix<double> v1(fy);
        //blaze::DynamicMatrix<double> Lu1((m + 2), (n + 2));
        //blaze::DynamicMatrix<double> Lv1((m + 2), (n + 2));
        blaze::DynamicMatrix<double> Lu1;//((m + 2), (n + 2), 0);
        blaze::DynamicMatrix<double> Lv1;//((m + 2), (n + 2), 0);

#ifdef MEASURE
        seconds = 0;
        auto useconds2 = 0;
#endif
        for (size_t it = 0; it < iter; it++) {
#ifdef MEASURE
            t1 = std::chrono::steady_clock::now();
#endif
            metric::DPM_detail::updatePad(u1);
            metric::DPM_detail::updatePad(v1);
#ifdef MEASURE
            t2 = std::chrono::steady_clock::now();
            seconds += double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
            t1 = std::chrono::steady_clock::now();
#endif
            Lu1 = metric::DPM_detail::laplacian(u1);
            Lv1 = metric::DPM_detail::laplacian(v1);
#ifdef MEASURE
            t2 = std::chrono::steady_clock::now();
            useconds2 += double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
#endif
            for (size_t i = 0; i < (m + 2); i++) {
                for (size_t j = 0; j < (n + 2); ++j) {
                    u1(i, j) = u1(i, j) + alpha * (mu * Lu1(i, j) - fxy_square(i, j) * (u1(i, j) - fx(i, j)));
                    v1(i, j) = v1(i, j) + alpha * (mu * Lv1(i, j) - fxy_square(i, j) * (v1(i, j) - fy(i, j)));

                }
            }
        }
#ifdef MEASURE
        std::cout << "--- in gvf: " << iter << "*2 calls of updatePad took " << seconds << " s\n";
        std::cout << "--- in gvf: " << iter << "*2 calls of laplacian took " << useconds2 / 1000000 << " s\n";
        t1 = std::chrono::steady_clock::now();
#endif

        auto u2 = metric::DPM_detail::removePad(u1);
        auto v2 = metric::DPM_detail::removePad(v1);
#ifdef MEASURE
        t2 = std::chrono::steady_clock::now();
        seconds = double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
        std::cout << "--- in gvf: 2 calls of removePad took " << seconds << " s\n";
#endif
        //std::cout << "u:\n" << u2 << "v:\n" << v2 << "\n"; // TODO remove

        return std::make_tuple(u2, v2);
    }

    double ln_gamma(double x)
    {
        double g;
        if (x < 0) {
            std::cout << "error: ln_gamma(), undefined for z <= 0" << std::endl;
        } else if (x < 10.0) {
            // Use recursive formula:
            return ln_gamma(x + 1.0) - std::log(x);

        } else {
            // high value approximation
            g = 0.5 * (std::log(2 * M_PI) - log(x));
            g += x * (std::log(x + (1 / (12.0 * x - 0.1 / x))) - 1);
        }
        return g;
    }

    double ln_besseli(double _nu, double _z)
    {
        double t0 = _nu * std::log(0.5 * _z);
        double t1 = 0.0;
        double t2 = 0.0;
        double t3 = 0.0;
        double y = 0.0;

        size_t iterations = 64;
        for (size_t k = 0; k < iterations; ++k) {
            // compute log( (z^2/4)^k )
            t1 = 2.0 * k * std::log(double(0.5) * _z);

            // compute: log( k! * Gamma(nu + k +1) )
            t2 = ln_gamma((double)k + 1.0);
            t3 = ln_gamma(_nu + (double)k + 1.0);

            // accumulate y
            y += std::exp(t1 - t2 - t3);
        }

        return t0 + std::log(y);
    }

    // I_v(z) : Modified Bessel function of the first kind
    double besseli(double nu, double z) { return std::exp(ln_besseli(nu, z)); }

    /* Correct for the radius of curvature
   input: 
   r: radius
   sigma: std of Gaussian PSF
   iter: number of iterations
   output:
   R: corrected radius
*/
    double correctCurve(double r, double sigma, size_t iter)
    {
        double var = sigma * sigma;
        double R = r;
        double x;
        for (size_t it = 0; it < iter; ++it) {
            x = r * R / (var);
            if (x < 100) {
                R = (r + var / r) * besseli(1, x) / besseli(0, x);
            } else {
                R = (r + var / r) * (128 * x * x - 48 * x - 15) / (128 * x * x + 16 * x + 9);
            }
        }
        return R;
    }



    // added by Max F




    template <typename T>

    T gauss(T x, T mu, T sigma) {
        T expVal = -1 * (pow(x - mu, 2) / pow(2 * sigma, 2));
        return exp(expVal) / (sqrt(2 * M_PI * pow(sigma, 2)));
    }



    template <typename T>
    blaze::DynamicMatrix<T> gaussianKernel(T sigma) {
        size_t sz = round(sigma * 6) + 2;
        if (sz % 2 != 0)
            ++sz;
        T center = T(sz) / 2.0;
        size_t c = center;
        auto kernel = blaze::DynamicMatrix<T>(sz, sz);
        T r, value;
        for (size_t i = 0; i < c; ++i) {
            for (size_t j = 0; j < c; ++j) {
                r = sqrt(pow(i - center, 2) + pow(j - center, 2));
                value = gauss(r, 0.0, sigma);
                kernel(i, j) = value;
                kernel(sz - 1 - i, j) = value;
                kernel(i, sz - 1- j) = value;
                kernel(sz - 1 - i, sz - 1 - j) = value;
            }
        }
        return kernel;
    }



    // using Convolurion2d.hpp

    template <typename T, size_t Channels>
    class Convolution2dCustomStride1 : public metric::Convolution2d<T, Channels> {

      public:
        Convolution2dCustomStride1(
                size_t imageWidth,
                size_t imageHeight,
                size_t kernelWidth,
                size_t kernelHeight
                //const PadDirection pd = PadDirection::POST,
                //const PadType pt = PadType::CIRCULAR,
                //const size_t stride = 1
                )
        {
            //this->padWidth = kernelWidth - 1;
            //this->padHeight = kernelHeight - 1;
    //        metric::Convolution2d<T, Channels>(imageWidth, imageHeight, kernelWidth, kernelHeight); // TODO remove

            this->padWidth = 0;
            this->padHeight = 0;

            metric::PadDirection pd = metric::PadDirection::POST;
            //metric::PadDirection pd = metric::PadDirection::BOTH;
            metric::PadType pt = metric::PadType::CIRCULAR;
            //metric::PadType pt = metric::PadType::REPLICATE;
            //metric::PadType pt = metric::PadType::SYMMETRIC;
            size_t stride = 1;

            this->padModel = std::make_shared<metric::PadModel<T>>(pd, pt, 0);

            //auto t1 = Clock::now();
            this->convLayer = std::make_shared<typename metric::Convolution2d<T, Channels>::ConvLayer2d>(imageWidth + this->padWidth, imageHeight + this->padHeight, 1, 1, kernelWidth, kernelHeight, stride);
            //auto t2 = Clock::now();
            //auto d = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);

        }
    };



    template <typename T>
    blaze::DynamicMatrix<T> gaussianBlur(const blaze::DynamicMatrix<T> & img, T sigma) {
        auto kernel = gaussianKernel(sigma);
        auto conv = Convolution2dCustomStride1<T, 1>(img.columns(), img.rows(), kernel.columns(), kernel.rows());
        auto blurred = conv({img}, kernel)[0];
        blaze::DynamicMatrix<T> padded (img.rows(), img.columns(), 0);
        blaze::submatrix( // padding with black after conv
                    padded,
                    (img.rows() - blurred.rows())/2, (img.columns() - blurred.columns())/2,
                    blurred.rows(), blurred.columns()
                    ) = blurred;
        return padded;
    }


    template <typename T> // very temporary solution, TODO update!!
    blaze::DynamicMatrix<T> blackPaddedConv(const blaze::DynamicMatrix<T> & img, const blaze::DynamicMatrix<T> & kernel) {
        auto conv = Convolution2dCustomStride1<T, 1>(img.columns(), img.rows(), kernel.columns(), kernel.rows());
        auto blurred = conv({img}, kernel)[0];
        blaze::DynamicMatrix<T> padded (img.rows(), img.columns(), 0);
        blaze::submatrix( // padding with black after conv
                    padded,
                    (img.rows() - blurred.rows())/2, (img.columns() - blurred.columns())/2,
                    blurred.rows(), blurred.columns()
                    ) = blurred;
        return padded;
    }



}  // end namespace DPM_detail

std::vector<double> fit_hysteresis(const blaze::DynamicVector<double> & x, const blaze::DynamicVector<double> & y, size_t grid_row,
    size_t grid_column, size_t steps, std::vector<double> sigma)
{

    blaze::DynamicMatrix<double> I = blaze::zero<double>(grid_row, grid_column);


    for (size_t i = 0; i < x.size(); ++i) {
        I((int)y[i], (int)x[i]) = 100;
    }


    return fit_hysteresis(I, steps, sigma);
}



std::vector<double> fit_hysteresis(
        const blaze::DynamicMatrix<double> & I,
        double xc0,
        double yc0,
        double r0,
        size_t steps,
        std::vector<double> sigma,
        double incr,
        double thresh
        )
{

    std::vector<double> ep = { xc0, yc0, r0, r0, 0 };  // initial parameter guess
    blaze::DynamicVector<double> increment = { incr, incr, incr, incr, M_PI / 180 * incr };  // increment in each iteration
    blaze::DynamicVector<double> threshold = { thresh, thresh, thresh, thresh, thresh };  // threshold for forces/torsinal moments

    double half_min_size = (I.rows() < I.columns() ? I.rows() : I.columns()) / 2.0;
    std::vector<double> bound = { 5, half_min_size, 5, half_min_size };  // the lower/upper bounds of a and b

    for (size_t i = 0; i < sigma.size(); ++i) {
        //size_t filtersize = round(sigma[i] * 7 + 2);
        //size_t filtersize = round(sigma[i] * 6); // 3 sigma
#ifdef MEASURE
        auto t1 = std::chrono::steady_clock::now();
#endif
        //imfilter<double, 1, FilterType::GAUSSIAN, PadDirection::BOTH, PadType::SYMMETRIC> f(filtersize, filtersize, sigma[i]);
        auto gk = DPM_detail::gaussianKernel(sigma[i]);
#ifdef MEASURE
        auto t2 = std::chrono::steady_clock::now();
        auto seconds = double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
        std::cout << "-- in fit_hysteresis: construction #" << i << " of gaussian kernel took " << seconds << " s\n";
        t1 = std::chrono::steady_clock::now();
#endif
        //blaze::DynamicMatrix<double> I1 = f(I);
        blaze::DynamicMatrix<double> I1 = DPM_detail::blackPaddedConv(I, gk);
#ifdef MEASURE
        t2 = std::chrono::steady_clock::now();
        seconds = double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
        std::cout << "-- in fit_hysteresis: call #" << i << " gaussian blur took " << seconds << " s\n";
        t1 = std::chrono::steady_clock::now();
#endif
#ifdef DEBUG_OUTPUT
        std::cout << "blur input: min: " << blaze::min(I) << ", max: " << blaze::max(I) << "\n";
        std::cout << "GVF input: min: " << blaze::min(I1) << ", max: " << blaze::max(I1) << "\n";
#endif
        //auto [u1, v1] = DPM_detail::gvf(I1, 1, 0.1, 10);
        auto [u1, v1] = DPM_detail::gvf(I1, 0.1, 1, 10);

#ifdef MEASURE
        t2 = std::chrono::steady_clock::now();
        seconds = double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
        std::cout << "-- in fit_hysteresis: call #" << i << " of gvf took " << seconds << " s, queried 10 iterations\n";
        t1 = std::chrono::steady_clock::now();
#endif
        ep = DPM_detail::fit_ellipse(ep, sigma[i] / 5 * increment, sigma[i] / 5 * threshold, bound, u1, v1, steps / sigma.size());

#ifdef MEASURE
        t2 = std::chrono::steady_clock::now();
        seconds = double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
        std::cout << "-- in fit_hysteresis: call #" << i << " of fit_ellipse took " << seconds << " s, queried " << steps / sigma.size() << " iterations\n";
#endif


    }
    // ep == ellipse parameter [xc, yc, a, b, phi]

    ep[4] = std::fmod(ep[4], M_PI);
    //curve correction
    double r1 = DPM_detail::correctCurve(std::pow(ep[3], 2) / ep[2], sigma[sigma.size() - 1], 100);
    double r2 = DPM_detail::correctCurve(std::pow(ep[2], 2) / ep[3], sigma[sigma.size() - 1], 100);
    ep[2] = std::pow(r1 * r2 * r2, (1.0 / 3.0));
    ep[3] = std::pow(r1 * r1 * r2, (1.0 / 3.0));

    return ep;
}



std::vector<double> fit_hysteresis(const blaze::DynamicMatrix<double> & I, size_t steps, std::vector<double> sigma)
{

    auto [xc0, yc0, r0] = DPM_detail::initialCircle(I);  // initial guess
    std::vector<double> ep = { xc0, yc0, r0, r0, 0 };  // initial parameter guess
    blaze::DynamicVector<double> increment = { 0.2, 0.2, 0.2, 0.2, M_PI / 180 * 0.2 };  // increment in each iteration
    blaze::DynamicVector<double> threshold = { 1e-6, 1e-6, 1e-6, 1e-6, 1e-6 };  // threshold for forces/torsinal moments
    std::vector<double> bound = { 10, 200, 10, 200 };  // the lower/upper bounds of a and b

    for (size_t i = 0; i < sigma.size(); ++i) {

        blaze::DynamicMatrix<double> I1 = I;  // TODO: replace with gaussian filter
        //auto I1=gaussianBlur(I,sigma[i]);
#ifdef MEASURE
        auto t1 = std::chrono::steady_clock::now();
#endif
        auto [u1, v1] = DPM_detail::gvf(I1, 1, 0.1, 10);

#ifdef MEASURE
        auto t2 = std::chrono::steady_clock::now();
        auto seconds = double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
        std::cout << "-- in fit_hysteresis: call #" << i << " of gvf took " << seconds << " s, queried 10 iterations\n";
        t1 = std::chrono::steady_clock::now();
#endif
        ep = DPM_detail::fit_ellipse(ep, sigma[i] / 5 * increment, sigma[i] / 5 * threshold, bound, u1, v1, steps / sigma.size());

#ifdef MEASURE
        t2 = std::chrono::steady_clock::now();
        seconds = double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
        std::cout << "-- in fit_hysteresis: call #" << i << " of fit_ellipse took " << seconds << " s, queried " << steps / sigma.size() << " iterations\n";
#endif


    }
    // ep == ellipse parameter [xc, yc, a, b, phi]

    ep[4] = std::fmod(ep[4], M_PI);
    //curve correction
    double r1 = DPM_detail::correctCurve(std::pow(ep[3], 2) / ep[2], sigma[sigma.size() - 1], 100);
    double r2 = DPM_detail::correctCurve(std::pow(ep[2], 2) / ep[3], sigma[sigma.size() - 1], 100);
    ep[2] = std::pow(r1 * r2 * r2, (1.0 / 3.0));
    ep[3] = std::pow(r1 * r1 * r2, (1.0 / 3.0));

    return ep;
}


}  // end namespace metric
