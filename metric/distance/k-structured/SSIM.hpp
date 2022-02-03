/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/

#ifndef _METRIC_DISTANCE_K_STRUCTURED_SSIM_HPP
#define _METRIC_DISTANCE_K_STRUCTURED_SSIM_HPP

namespace metric {

/**
 * @class SSIM
 *
 * @brief structural similarity (for images)
 *
 */
template <typename D, typename V> // added D as distance_type (to get f.e. double distance for int values of pixels) ->
								  // Stepan Mamontov
								  struct SSIM {
	using value_type = V;
	using distance_type = D;

	explicit SSIM() = default;

	/**
	 * @brief Construct a new SSIM object
	 *
	 * @param dynamic_range_  dynamic range of the pixel values
	 * @param masking_
	 */
	SSIM(const typename V::value_type dynamic_range_, const typename V::value_type masking_)
		: dynamic_range(dynamic_range_), masking(masking_)
	{
	}

	/**
	 * @brief Calculate structural similarity for images in given containers
	 *
	 * @param img1 first image
	 * @param img2 second image
	 * @return  structural similarity
	 */
	template <typename Container> distance_type operator()(const Container &img1, const Container &img2) const;

	typename V::value_type dynamic_range = 255.0;
	typename V::value_type masking = 2.0;
};

} // namespace metric

#include "SSIM.cpp"

#endif // Header Guard
