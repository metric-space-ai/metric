/*

PANDA presents

███╗   ███╗███████╗████████╗██████╗ ██╗ ██████╗     ███╗   ███╗ █████╗ ██████╗ ██████╗ ██╗███╗   ██╗ ██████╗
████╗ ████║██╔════╝╚══██╔══╝██╔══██╗██║██╔════╝     ████╗ ████║██╔══██╗██╔══██╗██╔══██╗██║████╗  ██║██╔════╝
██╔████╔██║█████╗     ██║   ██████╔╝██║██║          ██╔████╔██║███████║██████╔╝██████╔╝██║██╔██╗ ██║██║  ███╗
██║╚██╔╝██║██╔══╝     ██║   ██╔══██╗██║██║          ██║╚██╔╝██║██╔══██║██╔═══╝ ██╔═══╝ ██║██║╚██╗██║██║   ██║
██║ ╚═╝ ██║███████╗   ██║   ██║  ██║██║╚██████╗     ██║ ╚═╝ ██║██║  ██║██║     ██║     ██║██║ ╚████║╚██████╔╝
╚═╝     ╚═╝╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝ ╚═════╝     ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝     ╚═╝╚═╝  ╚═══╝ ╚═════╝
																					Licensed under MPL 2.0.

a library for metric spacing converting (dimension reduction and expanding)


This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) PANDA Team
*/

#ifndef _METRIC_MAPPING_HPP
#define _METRIC_MAPPING_HPP

// DEPRECATED legacy umbrella. This header aggregates the pre-revival mapping/
// clustering implementations (ESN, KOC, SOM, ensembles, kmeans, hierarchical
// clustering, ...). It is NOT the supported surface and is not covered by the
// core test gate. New code should use the promoted, tested surfaces instead:
//
//   clustering / outliers / representatives -> <metric/stats.hpp>
//                                              (mtrc::stats::structural_analysis)
//   embeddings / derived coordinate spaces  -> <metric/modify.hpp> / <metric/engine.hpp>
//                                              (PCFA, native autoencoder, PHATE-AE)
//
// See docs/supported-surface.md for what is promoted vs. legacy.

#include "mapping/ESN.hpp"
#include "mapping/KOC.hpp"
#include "mapping/PCFA.hpp"
#include "mapping/SOM.hpp"
#include "mapping/affprop.hpp"
#include "mapping/dbscan.hpp"
#include "mapping/ensembles.hpp"
#include "mapping/hierarchClustering.hpp"
#include "mapping/kmeans.hpp"
#include "mapping/kmedoids.hpp"

#endif
