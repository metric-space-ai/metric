// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// mtrc::modify aggregates the Level-2 components that construct modified or
// derived finite metric spaces:
//   represent - representative subspace selection (lossless)
//   reduce    - record-set coarsening / dimension reduction (map::reduce)
//   expand    - interpolation-like record generation
//   resample  - uneven-sampling correction (denoise)
//   map       - coordinate projections, embeddings, autoencoder/PHATE maps
//   dynamics  - diffusion / Markov-process evolution
//   compose   - composed mapping workflows (PHATE-AE)
// Every component returns a result carrying source lineage, metric status and
// validity bounds.

#ifndef _METRIC_MODIFY_HPP
#define _METRIC_MODIFY_HPP

#include "modify/represent/represent.hpp"
#include "modify/reduce/compress.hpp"
#include "modify/expand/expand.hpp"
#include "modify/resample/denoise.hpp"

#include "modify/map/clustered_space.hpp"
#include "modify/map/diagnostics.hpp"
#include "modify/map/embed.hpp"
#include "modify/map/koc.hpp"
#include "modify/map/map.hpp"
#include "modify/map/mapping.hpp"
#include "modify/map/native_autoencoder.hpp"
#include "modify/map/native_phate_autoencoder.hpp"
#include "modify/map/options.hpp"
#include "modify/map/pcfa.hpp"
#include "modify/map/reduce.hpp"

#include "modify/dynamics/dynamics.hpp"

#include "modify/compose/native_phate_autoencoder.hpp"
#include "modify/compose/pipeline.hpp"

#endif
