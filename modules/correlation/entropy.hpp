/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

// TODO continue refactoring!!

#ifndef _METRIC_CORRELATION_ENTROPY_HPP
#define _METRIC_CORRELATION_ENTROPY_HPP

#include "../distance/k-related/Standards.hpp"

#include "../../3rdparty/blaze/Blaze.h"


namespace metric {



template <typename MT> // TODO add default metric::Chebyshev<double>
class Entropy {
public:
    Entropy(MT metric = MT(), size_t k = 7, size_t p = 70, bool exp = false) :
        metric(metric),
        k(k),
        p(p),
        exp(exp) {}

    template <typename Container>
    double operator()(const Container& data) const;

    template <typename Container>
    double estimate(
            const Container & a,
            const size_t sampleSize = 250,
            const double threshold = 0.05,
            size_t maxIterations = 1000
    ) const;

private:
    size_t k;
    size_t p;
    MT metric;
    bool exp;
};




template <typename MT>
class EntropySimple { // averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing
public:
    EntropySimple(MT metric = MT(), size_t k = 3, bool exp = false) :
        metric(metric),
        k(k),
        exp(exp),
        logbase(2) {} // TODO remove (?)

    template <typename Container>
    double operator()(const Container& data) const;

    template <typename Container>
    double estimate(
            const Container & a,
            const size_t sampleSize = 250,
            const double threshold = 0.05,
            size_t maxIterations = 1000
    ) const;

private:
    size_t k;
    size_t p;
    MT metric;
    double logbase;
    bool exp;
};




} // namespace metric

#include "entropy.cpp"
#endif
