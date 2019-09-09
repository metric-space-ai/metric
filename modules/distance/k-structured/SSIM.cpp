/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/
#ifndef _METRIC_DISTANCE_K_STRUCTURED_SSIM_CPP
#define _METRIC_DISTANCE_K_STRUCTURED_SSIM_CPP
#include "SSIM.hpp"
#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159326
#endif

namespace metric {

namespace SSIM_details {
    // gaussian_blur filter
    inline std::vector<std::vector<double>> gaussian_blur(size_t n)
    {
        std::vector<std::vector<double>> gauss(n, std::vector<double>(n));
        size_t d = n / 2;
        size_t x, y;
        double Norm = 0.0;

        for (x = 0; x < n; x++) {
            for (y = 0; y < n; y++) {
                double g = std::exp(-(((x - d) * (x - d) + (y - d) * (y - d)) / 2.25));
                Norm += g;
                gauss[x][y] = g;
            }
        }
        for (x = 0; x < n; x++) {
            for (y = 0; y < n; y++) {
                gauss[x][y] = gauss[x][y] / Norm;
            }
        }
        return gauss;
    }
}  // namespace SSIM_details

namespace detail {
    template <class>
    struct sfinae_true : std::true_type {
    };
    template <typename T>
    static auto test_vec_of_vec(int) -> sfinae_true<decltype(std::declval<T>()[0][0])>;
    template <typename>
    static auto test_vec_of_vec(long) -> std::false_type;
}  // namespace detail
template <typename T>
struct is_vec_of_vec : decltype(detail::test_vec_of_vec<T>(0)) {
};

/*** distance measure for imagges by structural similarity. ***/
template <typename D, typename V>
template <typename Container>
auto SSIM<D, V>::operator()(const Container& img1, const Container& img2) const -> distance_type
{
    if constexpr (is_vec_of_vec<Container>() != true) {
        static_assert(true, "container should be 2D");
    } else {
        double sum = 0.0;
        bool is_visibility = (masking < 2.0);  // use stabilizer

        // create gaussian filter matrix
        size_t n = 11;
        auto gauss = SSIM_details::gaussian_blur(n);

        double C1 = std::pow(0.01 /*K1*/ * dynamic_range, 2);
        double C2 = std::pow(0.03 /*K2*/ * dynamic_range, 2);

        for (size_t i = 0; i < img1.size() - n + 1; ++i) {
            for (size_t j = 0; j < img1[0].size() - n + 1; ++j) {
                // initialize values
                double mu1 = 0.0, mu2 = 0.0;
                double sigma1 = 0.0, sigma2 = 0;
                double corr = 0.0, sigma12 = 0;
                double S1 = 0;
                double S2 = 0;

                for (size_t y = 0; y < n; y++) {
                    for (size_t x = 0; x < n; x++) {
                        double k1 = img1[i + y][j + x];
                        double k2 = img2[i + y][j + x];
                        double valv = gauss[y][x];
                        mu1 += k1 * valv;
                        mu2 += k2 * valv;
                        sigma1 += k1 * k1 * valv;
                        sigma2 += k2 * k2 * valv;
                        corr += k1 * k2 * valv;
                    }
                }

                double visibility = 1;  // default
                if (is_visibility) {
                    double l2norm1 = 0.0;
                    double l2norm2 = 0.0;
                    double lpnorm1 = 0.0;
                    double lpnorm2 = 0.0;
                    double sscale = n * n;
                    double C3 = C2 * std::pow(sscale, 2.0 / masking - 1.0);  // scaling
                    for (size_t y = 0; y < n; y++) {
                        for (size_t x = 0; x < n; x++) {
                            double k1 = img1[i + y][j + x];
                            double k2 = img2[i + y][j + x];
                            double valv = gauss[y][x] * sscale;
                            double v1 = k1 - mu1;
                            double v2 = k2 - mu2;
                            l2norm1 += v1 * v1 * valv;
                            l2norm2 += v2 * v2 * valv;
                            lpnorm1 += std::pow(std::abs(v1), masking) * valv;
                            lpnorm2 += std::pow(std::abs(v2), masking) * valv;
                        }
                    }
                    lpnorm1 = std::pow(lpnorm1, 2.0 / masking);
                    lpnorm2 = std::pow(lpnorm2, 2.0 / masking);
                    visibility = (l2norm1 + l2norm2 + C3) / (lpnorm1 + lpnorm2 + C3);
                    visibility = std::pow(visibility, masking / 2.0);

                    if (visibility > 1) {
                        visibility = 1;
                    } else if (visibility < 0) {
                        visibility = 0;
                    }
                }

                sigma1 -= mu1 * mu1;
                sigma2 -= mu2 * mu2;
                corr -= mu1 * mu2;

                if (sigma1 < 0) {
                    sigma1 = 0;
                }
                if (sigma2 < 0) {
                    sigma2 = 0;
                }

                sigma12 = std::sqrt(sigma1 * sigma2);

                // Structural Indicies
                S1 = (2.0 * mu1 * mu2 + C1) / (mu1 * mu1 + mu2 * mu2 + C1);
                S2 = (2.0 * sigma12 + C2) / (sigma1 + sigma2 + C2);

                // sum up the local ssim_distance
                sum += std::sqrt(2 - S1 - S2);
            }
        }
		
        return sum / ((img1.size() - n + 1) * (img1[0].size() - n + 1));  // normalize the sum
    }
    return distance_type {};
}

}  // namespace metric

#endif
