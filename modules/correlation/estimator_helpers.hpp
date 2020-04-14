/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _ESTIMATOR_HELPERS_HPP
#define _ESTIMATOR_HELPERS_HPP

#include <vector>

namespace metric {

// averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing

std::vector<double> linspace(double a, double b, int n);

double polyeval(const std::vector<double>& poly, const double z);

double erfinv_imp(const double p, const double q);

double erfcinv(const double z);

std::vector<double> icdf(const std::vector<double>& prob, const double mu, const double sigma);

double variance(const std::vector<double>& data, const double mean);

double mean(const std::vector<double>& data);

double peak2ems(const std::vector<double>& data);

}  // namespace metric

#include "estimator_helpers.cpp"

#endif
