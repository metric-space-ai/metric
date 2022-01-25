/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

/*
based on Olivier Mallets implementation https://github.com/olmallet81/GALGO-2.0
*/

#ifndef _METRIC_GENETIC_HPP
#define _METRIC_GENETIC_HPP

#include <iostream>

namespace metric {

template <typename P, typename T> class Genetic;

namespace genetic_details {

template <typename T, int N = 16> class Parameter;

template <typename P, typename T> class Chromosome;

template <typename P, typename T> class Population;

// convenient typedefs
template <typename P, typename T> using CHR = std::shared_ptr<Chromosome<P, T>>;

template <typename T> using PAR = std::unique_ptr<Parameter<T>>;

} // namespace genetic_details
} // namespace metric

#include "genetic/Chromosome.hpp"
#include "genetic/Parameter.hpp"
#include "genetic/Population.hpp"
#include "genetic/evolution_methods.hpp"
#include "genetic/genetic_impl.hpp"

#endif // header guard
