// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// mtrc::modify aggregates the Level-2 components that construct modified or
// derived finite metric spaces:
//   represent - representative subspace selection (lossless)
//   reduce    - record-set coarsening / dimension reduction (map::reduce)
//   expand    - interpolation-like record generation
//   resample  - uneven-sampling correction (density_filter, thin, equalize)
//   map       - coordinate projections, embeddings, parametric diffusion maps
//   dynamics  - diffusion / Markov-process evolution, Redif noise treatment
//   compose   - composed metric-space coordinate workflows
// Every component returns a result carrying source lineage, metric status and
// validity bounds.

#ifndef _METRIC_MODIFY_HPP
#define _METRIC_MODIFY_HPP

#include "modify/represent/represent.hpp"
#include "modify/reduce/compress.hpp"
#include "modify/expand/expand.hpp"
#include "modify/expand/generated.hpp"
#include "modify/resample/density_filter.hpp"
#include "modify/resample/thin.hpp"

#include "modify/map/clustered_space.hpp"
#include "modify/map/diagnostics.hpp"
#include "modify/map/embed.hpp"
#include "modify/map/map.hpp"
#include "modify/map/mapping.hpp"
#include "modify/map/parametric_diffusion_coordinates.hpp"
#include "modify/map/options.hpp"
#include "modify/map/pcfa.hpp"
#include "modify/map/reduce.hpp"

#include "modify/dynamics/dynamics.hpp"
#include "modify/dynamics/diffusion.hpp"
#include "modify/dynamics/finite_dynamics.hpp"
#include "modify/dynamics/redif.hpp"

#include "modify/compose/parametric_diffusion_coordinates.hpp"
#include "modify/compose/pipeline.hpp"

#endif
