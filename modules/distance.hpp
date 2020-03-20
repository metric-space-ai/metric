/*
                       
PANDA presents

███╗   ███╗███████╗████████╗██████╗ ██╗ ██████╗    ██████╗ ██╗███████╗████████╗ █████╗ ███╗   ██╗ ██████╗███████╗    
████╗ ████║██╔════╝╚══██╔══╝██╔══██╗██║██╔════╝    ██╔══██╗██║██╔════╝╚══██╔══╝██╔══██╗████╗  ██║██╔════╝██╔════╝    
██╔████╔██║█████╗     ██║   ██████╔╝██║██║         ██║  ██║██║███████╗   ██║   ███████║██╔██╗ ██║██║     █████╗      
██║╚██╔╝██║██╔══╝     ██║   ██╔══██╗██║██║         ██║  ██║██║╚════██║   ██║   ██╔══██║██║╚██╗██║██║     ██╔══╝      
██║ ╚═╝ ██║███████╗   ██║   ██║  ██║██║╚██████╗    ██████╔╝██║███████║   ██║   ██║  ██║██║ ╚████║╚██████╗███████╗    
╚═╝     ╚═╝╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝ ╚═════╝    ╚═════╝ ╚═╝╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝ ╚═════╝╚══════╝   
                                                                                            Licensed under MPL 2.0.

a library for metrics / distance functions


This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) PANDA Team
*/

#ifndef _METRIC_DISTANCE_HPP
#define _METRIC_DISTANCE_HPP

// include the implementation
#include "distance/k-related/Standards.hpp"
#include "distance/k-structured/SSIM.hpp"
#include "distance/k-structured/TWED.hpp"
#include "distance/k-structured/EMD.hpp"
#include "distance/k-structured/Edit.hpp"
#include "distance/k-structured/kohonen_distance.hpp"

#include "distance/k-related/L1.hpp"

#include "distance/k-random/VOI.hpp"
#include "distance/k-random/KolmogorovSmirnov.hpp"
#include "distance/k-random/RandomEMD.hpp"
#include "distance/k-random/CramervonNises.hpp"

#endif //_METRIC_DISTANCE_HPP
