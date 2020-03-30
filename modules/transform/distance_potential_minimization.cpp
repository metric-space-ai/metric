/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Michael Welsch
*/

#include "distance_potential_minimization.hpp"
#include <iostream>
#include <tuple>
#include <set>
#include <iterator>
#include <algorithm>

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
            B(m + 1, j + 1) = A(n - 2, j);
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
        A(0, n) = A(2, n - 3);

        // middle rows
        for (size_t i = 1; i < m - 1; ++i) {
            A(i, 0) = A(i, 2);
            A(i, n) = A(i, n - 3);
        }

        // last row
        A(m, 0) = A(m - 2, 2);
        for (size_t j = 1; j < n - 1; j++) {
            A(m, j) = A(m - 3, j);
        }
        A(m, n) = A(m - 3, n - 3);
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
                } else if (j == (n - 1)) {
                    fx(i, j) = f(i, j) - f(i, j - 1);
                } else {
                    fx(i, j) = (f(i, j + 1) - f(i, j - 1)) / 2;
                }

                // fy
                if (i == 0) {
                    fy(i, j) = f(i + 1, j) - f(i, j);
                } else if (i == (m - 1)) {
                    fy(i, j) = f(i, j) - f(i - 1, j);
                } else {
                    fy(i, j) = (f(i + 1, j) - f(i - 1, j)) / 2;
                }
            }
        }
        return std::make_tuple(fx, fy);
    }

    blaze::DynamicMatrix<double> diffuseStep(const blaze::DynamicMatrix<double>& f)
    {
        size_t m = f.rows();
        size_t n = f.columns();
        blaze::DynamicMatrix<double> diffused_f(m, n);

        for (size_t i = 0; i < m; i++) {
            for (size_t j = 0; j < n; j++) {
                if (i == 0 || i == m - 1 || j == 0 || j == n - 1) {
                    diffused_f(i, j) = 0;
                } else {
                    diffused_f(i, j) = f(i + 1, j) + f(i - 1, j) + f(i, j + 1) + f(i, j - 1) - double(4) * f(i, j);
                }
            }
        }
        return diffused_f;
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

        for (size_t i = 0; i < theta.size(); ++i) {
            x[i] = std::round(xc + a * std::cos(theta[i]) * std::cos(phi) - b * std::sin(theta[i]) * std::sin(phi));
            y[i] = std::round(yc + a * std::cos(theta[i]) * std::sin(phi) + b * std::sin(theta[i]) * std::cos(phi));
        }

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

        for (size_t it = 0; it < iter; ++it) {

            // compute grid points from ellipse parameter
            std::vector<blaze::DynamicVector<double>> x_y_theta = ellipse2grid(m, n, xc, yc, a, b, phi);

            std::vector<double> x(x_y_theta[0].size());
            std::vector<double> theta(x_y_theta[0].size());
            std::vector<double> y(x_y_theta[0].size());
            for (size_t i = 0; i < x_y_theta[0].size(); ++i) {
                x[i] = x_y_theta[0][i];
                y[i] = x_y_theta[1][i];
                theta[i] = x_y_theta[2][i];
            }

            // torsion along the ellpise about center
            double torsion = torsion_moment(gvf_x, gvf_y, x_y_theta[0], x_y_theta[1], x_y_theta[2], xc, yc, phi);

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
        }

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
        auto f2 = metric::DPM_detail::addPad(f);

        // compute the gradient field
        auto [fx, fy] = metric::DPM_detail::gradient(f2);

        // square of magnitude
        blaze::DynamicMatrix<double> fxy_square((m + 2), (n + 2));
        for (size_t i = 0; i < m + 2; i++) {
            for (size_t j = 0; j < n + 2; ++j) {
                fxy_square(i, j) = fx(i, j) * fx(i, j) + fy(i, j) * fy(i, j);
            }
        }

        blaze::DynamicMatrix<double> u1(fx);
        blaze::DynamicMatrix<double> v1(fy);
        blaze::DynamicMatrix<double> diffuseu1((m + 2), (n + 2));
        blaze::DynamicMatrix<double> diffusev1((m + 2), (n + 2));
        for (size_t it = 0; it < iter; it++) {
            metric::DPM_detail::updatePad(u1);
            metric::DPM_detail::updatePad(v1);
            diffuseu1 = metric::DPM_detail::diffuseStep(u1);
            diffusev1 = metric::DPM_detail::diffuseStep(v1);

            for (size_t i = 0; i < (m + 2); i++) {
                for (size_t j = 0; j < (n + 2); ++j) {
                    u1(i, j) = u1(i, j) + alpha * (mu * diffuseu1(i, j) - fxy_square(i, j) * (u1(i, j) - fx(i, j)));
                    v1(i, j) = v1(i, j) + alpha * (mu * diffusev1(i, j) - fxy_square(i, j) * (v1(i, j) - fy(i, j)));
                }
            }
        }

        auto u2 = metric::DPM_detail::removePad(u1);
        auto v2 = metric::DPM_detail::removePad(v1);

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

}  // end namespace DPM_detail

std::vector<double> fit_hysteresis(blaze::DynamicVector<double> x, blaze::DynamicVector<double> y, size_t rows,
    size_t cols, size_t steps, std::vector<double> sigma)
{
    // size_t rows = 400;
    // size_t cols = 500;
    // generate 2d Grid
    blaze::DynamicMatrix<double> I = blaze::zero<double>(rows, cols);
    for (size_t i = 0; i < x.size(); ++i) {
        I(y[i], x[i]) = 100;
    }

    auto [xc0, yc0, r0] = DPM_detail::initialCircle(I);  // initial guess
    std::vector<double> ep = { xc0, yc0, r0, r0, 0 };  // initial parameter guess
    blaze::DynamicVector<double> increment = { 0.2, 0.2, 0.2, 0.2, M_PI / 180 * 0.2 };  // increment in each iteration
    blaze::DynamicVector<double> threshold = { 1e-6, 1e-6, 1e-6, 1e-6, 1e-6 };  // threshold for forces/torsinal moments
    std::vector<double> bound = { 10, 200, 10, 200 };  // the lower/upper bounds of a and b

    for (size_t i = 0; i < sigma.size(); ++i) {

        blaze::DynamicMatrix<double> I1 = I;  // TODO: replace with gaussian filter
        //auto I1=gaussianBlur(I,sigma[i]);
        auto [h1, v1] = DPM_detail::gvf(I1, 1, 0.1, 10);
        ep = DPM_detail::fit_ellipse(
            ep, sigma[i] / 5 * increment, sigma[i] / 5 * threshold, bound, h1, v1, steps / sigma.size());
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