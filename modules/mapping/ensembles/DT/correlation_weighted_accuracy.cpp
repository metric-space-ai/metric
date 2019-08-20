/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch, Signal Empowering Technology
*/
#ifndef METRIC_MAPPING_ENSEMBLES_DT_CORRELATION_WEIGHTED_ACCURACY_CPP
#define METRIC_MAPPING_ENSEMBLES_DT_CORRELATION_WEIGHTED_ACCURACY_CPP
#include "correlation_weighted_accuracy.hpp"
#include <assert.h>
#include <cmath>
#include <vector>

#include <algorithm>

namespace metric {

namespace CWA_details {

    template <typename Container>
    std::vector<std::vector<typename Container::value_type>> confusion_matrix(Container a, Container b)
    {
        typedef typename Container::value_type T;
        assert(a.size() == b.size());

        std::vector<T> ab;  // build unique vector ( a.size()+b.size());
        ab.reserve(a.size() + a.size());  // preallocate memory
        ab.insert(ab.end(), a.begin(), a.end());
        ab.insert(ab.end(), b.begin(), b.end());
        std::sort(ab.begin(), ab.end());
        ab.erase(std::unique(ab.begin(), ab.end()), ab.end());

        auto k = ab.size();  // unique counts
        std::vector<std::vector<T>> CM(k, std::vector<T>(k, 0));  // matrix with zeros

        for (int i = 0; i < a.size(); ++i) {
            auto pos_g = std::distance(ab.begin(), std::find(ab.begin(), ab.end(), a[i]));
            auto pos_q = std::distance(ab.begin(), std::find(ab.begin(), ab.end(), b[i]));
            CM[pos_g][pos_q] += 1;
        }

        return CM;
    }

    template <typename T>
    double accuracy(std::vector<std::vector<T>> CM)
    {
        T hits = 0;
        T sum = 0;
        for (int i = 0; i < CM.size(); ++i) {
            for (int j = 0; j < CM.size(); ++j) {
                if (i == j) {
                    hits += CM[i][j];
                }
                sum += CM[i][j];
            }
        }
        return double(hits) / double(sum);
    }

    /*
Matthews correlation coefficient for multiclasses

Function to compute the K-category correlation coefficient based on the confusion matrix
(generalization of the Matthews correlation coefficient for multiclasses
https://en.wikipedia.org/wiki/Matthews_correlation_coefficient)

Reference
Comparing two K-category assignments by a K-category correlation coefficient
J. Gorodkin, Computational Biology and Chemistry, 28:367-374, 2004.
*/
    template <typename T>
    double MCC(std::vector<std::vector<T>> CM)
    {

        int cols = CM.size();

        T MCC_numerator = 0;
        for (int k = 0; k < cols; ++k) {
            for (int l = 0; l < cols; ++l) {
                for (int m = 0; m < cols; ++m) {
                    MCC_numerator = MCC_numerator + (CM[k][k] * CM[m][l]) - (CM[l][k] * CM[k][m]);
                }
            }
        }

        T MCC_denominator_1 = 0;
        for (int k = 0; k < cols; ++k) {
            T MCC_den_1_part1 = 0;
            for (int l = 0; l < cols; ++l) {
                MCC_den_1_part1 = MCC_den_1_part1 + CM[l][k];
            }
            T MCC_den_1_part2 = 0;

            for (int f = 0; f < cols; ++f) {
                if (f != k) {

                    for (int g = 0; g < cols; ++g) {

                        MCC_den_1_part2 = MCC_den_1_part2 + CM[g][f];
                    }
                }
            }
            MCC_denominator_1 = (MCC_denominator_1 + (MCC_den_1_part1 * MCC_den_1_part2));
        }
        T MCC_denominator_2 = 0;
        for (int k = 0; k < cols; ++k) {
            T MCC_den_2_part1 = 0;
            for (int l = 0; l < cols; ++l) {
                MCC_den_2_part1 = MCC_den_2_part1 + CM[k][l];
            }
            T MCC_den_2_part2 = 0;

            for (int f = 0; f < cols; ++f) {
                if (f != k) {
                    for (int g = 0; g < cols; ++g) {
                        MCC_den_2_part2 = MCC_den_2_part2 + CM[f][g];
                    }
                }
            }
            MCC_denominator_2 = (MCC_denominator_2 + (MCC_den_2_part1 * MCC_den_2_part2));
        }

        double MCC
            = (double(MCC_numerator)) / (std::sqrt(double(MCC_denominator_1)) * std::sqrt(double(MCC_denominator_2)));

        return MCC;
    }

}  // namespace CWA_details

template <typename Container>
double correlation_weighted_accuracy(Container a, Container b)
{
    auto CM = CWA_details::confusion_matrix(a, b);
    auto accu = CWA_details::accuracy(CM);
    auto corr = CWA_details::MCC(CM);

    if (corr < 0) {
        corr = 0;
    }

    return std::sqrt(accu * corr);
}

}  // namespace metric
#endif