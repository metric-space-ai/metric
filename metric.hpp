/*
                       
PANDA presents

███╗   ███╗███████╗████████╗██████╗ ██╗ ██████╗
████╗ ████║██╔════╝╚══██╔══╝██╔══██╗██║██╔════╝   
██╔████╔██║█████╗     ██║   ██████╔╝██║██║       
██║╚██╔╝██║██╔══╝     ██║   ██╔══██╗██║██║       
██║ ╚═╝ ██║███████╗   ██║   ██║  ██║██║╚██████╗  
                        Licensed under MPL 2.0. 
                        Michael Welsch (c) 2018.
                                                                                                   
a framework to process abritrary data

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/


/*
blaze need additional linking to blas.
clang++ -std=c++17 main.cpp -o run.o -Ofast -L/usr/local/Cellar/openblas/0.3.5/lib -L/usr/local/Cellar/openblas/0.3.5/include -lopenblas
 */

#ifndef _PANDA_METRIC_HPP
#define _PANDA_METRIC_HPP

#include "metric/distance.hpp"
#include "metric/space.hpp"
#include "metric/correlation.hpp"
#include "metric/transform.hpp"
#include "metric/mapping.hpp"

#endif