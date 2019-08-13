/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 M.Welsch <michael@welsch.one> 
*/

#ifndef _METRIC_DBSCAN_HPP
#define _METRIC_DBSCAN_HPP

/*
A DBSCAN implementation based on distance matrix.
*/


//   References:
//
//       Martin Ester, Hans-peter Kriegel, Jörg S, and Xiaowei Xu
//       A density-based algorithm for discovering clusters
//       in large spatial databases with noise. 1996.


namespace metric 
{

// --------------------------------------------------------------
// DBSCAN
// --------------------------------------------------------------
namespace dbscan_details
{
// computes the distance matrix (pairwaise)
    template <typename T>
    std::vector<std::vector<T>> 
    distance_matrix(const std::vector<std::vector<T>> &data,
        std::string distance_measure){

        std::vector<std::vector<T>> matrix(data.size(), std::vector<T>(data.size())); //initialize
        for (int i=0;i<data.size();++i){
            for (int j=i;j<data.size();++j){
                T distance = distance_functions::distance(data[i],data[j], distance_measure);
                matrix[i][j]= distance;
                matrix[j][i]= distance;
            }
        }
        return matrix;
    }

// key steps
template <typename T>
std::deque<int>
region_query(std::vector<std::vector<T>> D, int p, T eps){

    std::deque<int> nbs;
    for (int i = 0; i<D.size(); ++i){
        if (D[p][i] < eps){
            nbs.push_back(i);
        }
    }
    return nbs;
}

// a changing arguments function
template <typename T>
int 
update_cluster(const std::vector<std::vector<T>> &D,                      // distance matrix
                                       const int &k,                      // the index of current cluster
                                       const int &p,                      // the index of seeding point
                                       const T &eps,                      // radius of neighborhood
                                       const int &minpts,                 // minimum number of neighbors of a density point
                                       std::deque<int> &nbs,              // eps-neighborhood of p
                                       std::vector<int> &assignments,     // assignment vector
                                       std::vector<bool> &visited){       // visited indicators
    assignments[p] = k;
    int cnt = 1;
    while (!std::empty(nbs)){
        //q = shift!(nbs)
        int q = nbs[0];
        nbs.pop_front();
        if (!visited[q]){
            visited[q] = true;
            auto qnbs = region_query(D, q, eps);
            if (qnbs.size() >= minpts){
                for (auto x : qnbs){
                    if (assignments[x] == 0)
                        nbs.push_back(x);
                    
                }
            }
        }
        if (assignments[q] == 0){
            assignments[q] = k;
            cnt += 1;
        }
    }
    return cnt;
}



} //namespace dbscan_details





// main algorithm
template <typename T>
std::tuple<std::vector<int>,std::vector<int>,std::vector<int>>
dbscan(const std::vector<std::vector<T>> &data, 
    T eps, 
    int minpts,
    std::string distance_measure = distance_functions::default_measure()){

        // check arguments
        int n = data.size();
        
        assert(n >= 2); // error("There must be at least two points.")
        assert(eps > 0); // error("eps must be a positive real value.")
        assert(minpts >= 1); // error("minpts must be a positive integer.")
        
        // build the (pairwaise) distance matrix
        auto D = dbscan_functions::distance_matrix(data,distance_measure);
    
    // initialize
    std::vector<int> seeds;
    std::vector<int> counts;
    std::vector<int> assignments(n,int(0));
    std::vector<bool> visited(n,false);
    std::vector<int> visitseq(n);
    std::iota(visitseq.begin(), visitseq.end(), 0);  // (generates a linear index vector [0, 1, 2, ...])
    
    // main loop
    int k = 0;
    for (int p : visitseq){
        if (assignments[p] == 0 && !visited[p]){
            visited[p] = true;
            auto nbs = dbscan_functions::region_query(D, p, eps);
            if (nbs.size() >= minpts){
                k += 1;
                auto cnt = dbscan_functions::update_cluster(D, k, p,  eps, minpts,nbs, assignments, visited);
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