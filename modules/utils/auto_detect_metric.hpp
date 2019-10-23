/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 PANDA Team
*/


namespace metric {


	/**
	 * @class MetricAutoDetector
	 * @brief
	 * 
	 */
	class MetricAutoDetector {

	public:

		/**
		 * @brief Construct a new MetricAutoDetector object
		 *
		 */
		explicit MetricAutoDetector();

		/**
		 * @brief
		 *
		 */
		template <typename Record, typename Graph>
		std::string detect(Graph &graph, std::vector<Record> dataset, bool isEstimate = true);

		/**
		 * @brief
		 *
		 */
		void set_verbose(bool isVerbose)
		{
			verbose = isVerbose;
		}

	private:

		bool verbose = false;
	};



} // end namespace metric


#include "auto_detect_metric/auto_detect_metric.cpp"
