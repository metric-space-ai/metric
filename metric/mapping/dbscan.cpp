/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 M.Welsch
*/
#ifndef _METRIC_MAPPING_DBSCAN_CPP
#define _METRIC_MAPPING_DBSCAN_CPP
#include "../distance/k-related/Standards.hpp"
#include <cassert>
#include <deque>
#include <numeric>
#include <string>
#include <vector>
namespace metric {

// --------------------------------------------------------------
// DBSCAN
// --------------------------------------------------------------
namespace dbscan_details {
// key steps
template <typename T, typename DistanceMatrix> std::deque<int> region_query(const DistanceMatrix &D, int p, T eps)
{
	std::deque<int> nbs;
	for (std::size_t i = 0; i < D.size(); ++i) {
		if (D(p, i) < eps) {
			nbs.push_back(i);
		}
	}
	return nbs;
}

// a changing arguments function
template <typename T, typename DistanceMatrix>
int update_cluster(const DistanceMatrix &D,		  // distance matrix
				   const int &k,				  // the index of current cluster
				   const int &p,				  // the index of seeding point
				   const T &eps,				  // radius of neighborhood
				   const int &minpts,			  // minimum number of neighbors of a density point
				   std::deque<int> &nbs,		  // eps-neighborhood of p
				   std::vector<int> &assignments, // assignment vector
				   std::vector<bool> &visited)
{ // visited indicators
	assignments[p] = k;
	int cnt = 1;
	while (!std::empty(nbs)) {
		// q = shift!(nbs)
		int q = nbs[0];
		nbs.pop_front();
		if (!visited[q]) {
			visited[q] = true;
			auto qnbs = region_query(D, q, eps);
			if (qnbs.size() >= minpts) {
				for (auto x : qnbs) {
					if (assignments[x] == 0)
						nbs.push_back(x);
				}
			}
		}
		if (assignments[q] == 0) {
			assignments[q] = k;
			cnt += 1;
		}
	}
	return cnt;
}

} // namespace dbscan_details

// main algorithm
template <typename RecType, typename Metric, typename T>
std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> dbscan(const Matrix<RecType, Metric> &DM, T eps,
																		std::size_t minpts)
{

	// check arguments
	auto n = DM.size();

	assert(n >= 2);		 // error("There must be at least two points.")
	assert(eps > 0);	 // error("eps must be a positive real value.")
	assert(minpts >= 1); // error("minpts must be a positive integer.")

	// initialize
	std::vector<int> seeds;
	std::vector<int> counts;
	std::vector<int> assignments(n, int(0));
	std::vector<bool> visited(n, false);
	std::vector<int> visitseq(n);
	std::iota(visitseq.begin(), visitseq.end(), 0); // (generates a linear index vector [0, 1, 2, ...])

	// main loop
	int k = 0;
	for (int p : visitseq) {
		if (assignments[p] == 0 && !visited[p]) {
			visited[p] = true;
			auto nbs = dbscan_details::region_query(DM, p, eps);
			if (nbs.size() >= minpts) {
				k += 1;
				auto cnt = dbscan_details::update_cluster(DM, k, p, eps, minpts, nbs, assignments, visited);
				seeds.push_back(p);
				counts.push_back(cnt);
			}
		}
	}

	// make output
	return {assignments, seeds, counts};
}

} // namespace metric

#endif
