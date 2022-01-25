/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 M.Welsch
*/

#ifndef _METRIC_MAPPING_AFFPROP_HPP
#define _METRIC_MAPPING_AFFPROP_HPP

#include "../space/matrix.hpp"

#include <algorithm>
#include <tuple>
#include <type_traits>
#include <vector>

namespace metric {

/*
 A Affinity Propagation implementation based on a similarity matrix.
*/
//   Reference:
//       Clustering by Passing Messages Between Data Points.
//       Brendan J. Frey and Delbert Dueck
//       Science, vol 315, pages 972-976, 2007.
//
template <typename RecType, typename Metric> class AffProp {
	using Value = typename std::invoke_result<Metric, const RecType &, const RecType &>::type;

  public:
	AffProp() = default;

	AffProp(Value preference, int maxiter = 200, Value tol = 1.0e-6, Value damp = 0.5)
		: preference(preference), maxiter(maxiter), tol(tol), damp(damp)
	{
	}

	/**
	 * @brief
	 *
	 * @param data distance matrix
	 * @param preference
	 * @param maxiter
	 * @param tol
	 * @param damp
	 * @return
	 */
	auto operator()(const Matrix<RecType, Metric> &DM) const
		-> std::tuple<std::vector<std::size_t>, std::vector<std::size_t>, std::vector<std::size_t>>;

  private:
	Value preference = 0.5;
	int maxiter = 200;
	Value tol = 1.0e-6;
	Value damp = 0.5;
};

} // namespace metric

#include "affprop.cpp"

#endif
