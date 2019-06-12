/* This Source Code Form is subject to the terms of the PANDA GmbH
License. You are not allowed to use or edit the code without license. */

#include <type_traits>
#include <algorithm>

#include "../distance.hpp" // back reference for header only use

#include "details/k-related/Standards.hpp"
#include "details/k-structured/SSIM.hpp"
#include "details/k-random/TWED.hpp"
#include "details/k-structured/EMD.hpp"
#include "details/k-random/Edit.hpp"

#include "details/k-related/L1.hpp"
