/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 M.Welsch <michael@welsch.one> */


#include "clustering.hpp"


namespace clustering 
{


namespace distance_functions 
{
    /*
    distance measure between two data points.
    */
    std::string 
    default_measure(void){
        std::string defaultmeasure = "euclidian";
        return defaultmeasure;
    }

    template <typename T>
    T 
    euclidian(const std::vector<T> &datapoint_a,
                       const std::vector<T> &datapoint_b){
    
       
       
        T value = T(0);
        for (int i = 0; i < datapoint_a.size(); ++i)
        {
            T delta = datapoint_a[i] - datapoint_b[i];
            value += delta * delta;
        }
        return std::sqrt(value);
    }

    template <typename T>
    T 
    rms(const std::vector<T> &datapoint_a,
                       const std::vector<T> &datapoint_b){
    
       
       
        T value = T(0);
        for (int i = 0; i < datapoint_a.size(); ++i)
        {
            T delta = datapoint_a[i] - datapoint_b[i];
            value += delta * delta;
        }
        return value;
    }

    template <typename T>
    T 
    manhatten(const std::vector<T> &datapoint_a,
                       const std::vector<T> &datapoint_b){
    
       
        T value = T(0);
        for (int i = 0; i < datapoint_a.size(); ++i)
        {
            value += std::abs(datapoint_a[i] - datapoint_b[i]);
        }
        return value;
    }

    template <typename T>
    T 
    distance(const std::vector<T> &a,
                        const std::vector<T> &b,
                        std::string distance_measure){
    

        assert(a.size() == b.size()); // data vectors have not the same length
        if (distance_measure.compare("euclidian") == 0)
            return euclidian(a,b);
        else if (distance_measure.compare("rms") == 0)
            return rms(a,b);
        else if (distance_measure.compare("manhatten") == 0)
            return manhatten(a,b);
        else {
            std::cout << "distance measure not found, using default (euclidian)" << std::endl;
            return euclidian(a,b);

    }
}

    

}

namespace kmeans_functions{
  
    
    
    /*
    closest distance between datapoints and means.
    */
    template <typename T>
    std::vector<T>
    closest_distance(
        const std::vector<std::vector<T>> &means,
        const std::vector<std::vector<T>> &datapoints,
        int k,
        std::string distance_measure)
    {
        std::vector<T> distances;
        distances.reserve(k);
        for (auto &d : datapoints)
        {
            T closest = distance_functions::distance(d, means[0], distance_measure);
            for (auto &m : means)
            {
                T distance = distance_functions::distance(d, m, distance_measure);
                if (distance < closest)
                    closest = distance;
            }
            distances.push_back(closest);
        }
        return distances;
    }
    
    /*
    means initialization based on the [kmeans++](https://en.wikipedia.org/wiki/K-means%2B%2B) algorithm.
    */
    template <typename T>
    std::vector<std::vector<T>>
    random_init(const std::vector<std::vector<T>> &data,
                int k,std::string distance_measure)
    {
        assert(k > 0);
        using input_size_t = typename std::vector<T>::size_type;
        std::vector<std::vector<T>> means;
        // Using a very simple PRBS generator, parameters selected according to
        // https://en.wikipedia.org/wiki/Linear_congruential_generator#Parameters_in_common_use
        std::random_device rand_device;
        std::linear_congruential_engine<uint64_t, 6364136223846793005, 1442695040888963407, UINT64_MAX> rand_engine(
            rand_device());
    
        // Select first mean at random from the set
        {
            std::uniform_int_distribution<input_size_t> uniform_generator(0, data.size() - 1);
            means.push_back(data[uniform_generator(rand_engine)]);
        }
    
        for (int count = 1; count < k; ++count)
        {
            // Calculate the distance to the closest mean for each data point
            auto distances = closest_distance(means, data, k,distance_measure);
            // Pick a random point weighted by the distance from existing means
            // TODO: This might convert floating point weights to ints, distorting the distribution for small weights
            std::discrete_distribution<size_t> generator(distances.begin(), distances.end());
            means.push_back(data[generator(rand_engine)]);
        }
        return means;
    }
    
    /*
    find closest mean for a data point
    */
    template <typename T>
    int
    findClosestMean(const std::vector<T> &datapoint,
                    const std::vector<std::vector<T>> &means,
                    std::string distance_measure)
    {
        assert(!means.empty());
    
        T smallest_distance = distance_functions::distance(datapoint, means[0],distance_measure);
        //typename std::vector<T>::size_type index = 0;
        int index = 0;
        T distance;
        for (int i = 1; i < means.size(); ++i)
        {
            distance = distance_functions::distance(datapoint, means[i],distance_measure);
            if (distance < smallest_distance)
            {
                smallest_distance = distance;
                index = i;
            }
        }
        return index;
    }
    
    /*
    index of the closest means
    */
    template <typename T>
    void
    update_assignments(
        std::vector<int> &assignments,
        const std::vector<std::vector<T>> &data,
            const std::vector<std::vector<T>> &means,
            std::string distance_measure)
    {
        for (int i=0;i<data.size();++i){
            assignments[i]=findClosestMean(data[i], means, distance_measure);
        }
    }
    
    /*
    means based on datapoints and their cluster assignments.
    */
    template <typename T>
    std::tuple<std::vector<int>,int>
    update_means(std::vector<std::vector<T>> &means,
        const std::vector<std::vector<T>> &data,
          const std::vector<int> &assignments,
          const int &k)
    {
        
    
        std::vector<std::vector<T>> old_means = means;

        std::vector<int> count(k, int(0));
        for (int i = 0; i < std::min(assignments.size(), data.size()); ++i)
        {
            count[assignments[i]] += 1;
            for (int j = 0; j < std::min(data[i].size(), means[assignments[i]].size()); ++j)
            {
                means[assignments[i]][j] += data[i][j];
            }
        }
        int updated = 0;
        for (int i = 0; i < k; ++i)
        {
            if (count[i] == 0)
            {
                means[i] = old_means[i];
            }
            else
            {
                for (int j = 0; j < means[i].size(); ++j)
                {
                    means[i][j] /= double(count[i]);
                    if (means[i][j] != old_means[i][j])
                    updated += 1;
                }
            }
        }

    
        return {count,updated};
    }

    
    void
    rearrange_assignments(std::vector<int> &assignments){
        std::vector<int> from_list; 
        std::vector<int> to_list;
        from_list.push_back(assignments[0]);
        to_list.push_back(int(0));

        for (int i= 1; i< assignments.size(); ++i){
            bool hit = false;
            for (int j=0; j<from_list.size(); ++j){
                if (from_list[j] == assignments[i]){
                hit = true;
                }
            }
            if (!hit) {
                from_list.push_back(assignments[i]);
                to_list.push_back(from_list.size()-1);
                }
            
        }

        for (int i= 0; i< assignments.size();++i){
            int old_indx;
            for (int j=0; j< to_list.size();++j){
                if (from_list[j] == assignments[i])
                    old_indx = j;
            }    
            assignments[i]=to_list[old_indx];
        }
        


    }
    
} // end namespace kmeans_functions
    
/*
    A k-means implementation with optimized seeding.
    Input (vector of fixed-size vector, clustersize)
    for example:
    std::vector<std::vector<float, 5>> data{
            {0, 0, 0, 0, 0},
            {1.74120000000000, 4.07812000000000, -0.0927036000000, 41.7888000000000, 41.7888000000000},
            {7.75309000000000, 16.2466000000000, 3.03956000000000, 186.074000000000, 186.074000000000},
            {2.85493000000000, 3.25380000000000, 2.50559000000000, 68.5184000000000, 68.5184000000000},
            {5.81414000000000, 8.14015000000000, 3.22950000000000, 139.539000000000, 139.539000000000},
            {2.57927000000000, 2.63399000000000, 2.46802000000000, 61.9026000000000, 61.9026000000000}};
    auto [means, idx] = kmeans(data, 4); // clusters the data in 4 groups.
    means: A vector holding the means (same type as input data)
    idx: A vector containing the cluster index
    */
    template <typename T>
    std::tuple<std::vector<int>,std::vector<std::vector<T>>,std::vector<int>>
    kmeans(
        const std::vector<std::vector<T>> &data,
        const int &k,
        const int &maxiter = 200,
        std::string distance_measure = distance_functions::default_measure())
    {
        static_assert(std::is_arithmetic<T>::value && std::is_signed<T>::value,
                      "kmeans_lloyd requires the template parameter T to be a signed arithmetic type (e.g. float, double, int)");
        assert(k > 0);            // k must be greater than zero
        assert(data.size() >= k); // there must be at least k data points
        
        std::vector<std::vector<T>> means = kmeans_functions::random_init(data, k, distance_measure);
    
        //std::vector<std::vector<T>> old_means;
        std::vector<int> assignments(data.size());
        // Calculate new meansData until convergence is reached
        int t = 0;
        int updated_number_of_means = 0;
        std::vector<int> counts(k, int(0));
        do
        {
            kmeans_functions::update_assignments(assignments, data, means, distance_measure);
            auto [updated_counts, updated_number_of_means] = kmeans_functions::update_means(means, data, assignments, k);
            counts = updated_counts;
            ++t;
        } 
        while (updated_number_of_means != int(0) && t < maxiter);
    
        kmeans_functions::rearrange_assignments(assignments);
        return {assignments,means,counts};
    }

// --------------------------------------------------------------
// DBSCAN
// --------------------------------------------------------------
namespace dbscan_functions
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



} //namespace dbscan_functions

/*
A DBSCAN implementation based on distance matrix.
*/


//   References:
//
//       Martin Ester, Hans-peter Kriegel, Jörg S, and Xiaowei Xu
//       A density-based algorithm for discovering clusters
//       in large spatial databases with noise. 1996.



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

namespace affprop_functions {

    //build similarity matrix
    template <typename T>
    std::vector<std::vector<T>> 
    similarity_matrix(const std::vector<std::vector<T>> &data,
                        const T preference,
                        std::string distance_measure){
        int n = data.size();
        T pmin = 0;
        T pmax= -2e21;
        std::vector<std::vector<T>> matrix(n, std::vector<T>(n)); //initialize
        for (int i=0;i<n;++i){
            for (int j=i;j<n;++j){
         
                T distance = distance_functions::distance(data[i],data[j], distance_measure);
                T similarity = -distance;
                if (similarity < pmin)
                    pmin=similarity;
                    if (similarity > pmax)
                    pmax = similarity;
                matrix[i][j]= similarity;
                matrix[j][i]= similarity;
            }
        }

        
        for (int i=0;i<n;++i){
            matrix[i][i] = preference * pmax + (1-preference)*pmin; 
        }



        return matrix;
    }
 
    // compute responsibilities
    template <typename T>
    T
    update_responsibilities(std::vector<std::vector<T>> &R,const std::vector<std::vector<T>> &S,const std::vector<std::vector<T>> &A, const T &damp){
        int n = S.size();
        T maxabs = 0;
    
        std::vector<int> I1(n);  // I1[i] is the column index of the maximum element in (A+S) vector
        std::vector<T> Y1(n);    // Y1[i] is the maximum element in (A+S) vector
        std::vector<T> Y2(n);     // Y2[i] is the second maximum element in (A+S) vector
            
        // Find the first and second maximum elements along each row
        for (int i = 0 ;i<n; ++i){
            T v1 = A[i][0] + S[i][0];
            T v2 = A[i][1] + S[i][1];
            if (v1 > v2){
                I1[i] = 0;
                Y1[i] = v1;
                Y2[i] = v2;
            }
            else{
                I1[i] = 1;
                Y1[i] = v2;
                Y2[i] = v1;
            }
        }
        for (int j = 2; j<n;++j){
            for (int i = 0; i<n; ++i){
                T v = A[i][j] + S[i][j];
                if (v > Y2[i]){
                    if (v > Y1[i]){
                        Y2[i] = Y1[i];
                        I1[i] = j;
                        Y1[i] = v;
                    }
                    else
                        Y2[i] = v;

                }
            }
        }

    
        // update R values
        for (int j=0; j<n; ++j){
            for (int i=0; i<n; ++i){
                T Rij_old=R[i][j];
                T mv = (j == I1[i] ? Y2[i] : Y1[i]);
                T Rij_new = S[i][j] - mv;

                // update
                R[i][j] = damp * Rij_old + (1-damp) * Rij_new;

                // compute convergenze criteria
                T abs_ij = std::abs(Rij_old-Rij_new);
                if (abs_ij > maxabs)
                    maxabs = abs_ij;
        }
    }
    
       return maxabs;
    }
    
    // compute availabilities
    template <typename T>
    T
    update_availabilities(std::vector<std::vector<T>> &A, const std::vector<std::vector<T>> &R, const T &damp){
        int n = R.size();
        T maxabs = 0;
        for (int j = 0; j<n; ++j){
            T rjj = R[j][j];
    
            // compute sum
            T sum = 0;
            for (int i = 0;i<n;++i){
                if (i != j){
                    T r = R[i][j];
                    if (r > 0)
                        sum += r;
                }
            }
    
            for (int i = 0; i<n; ++i){
                T Aij_old = A[i][j];
                T Aij_new;
                if (i == j)
                    Aij_new = sum;
                else{
                    T r = R[i][j];
                    T u = rjj + sum;
                    if (r > 0)
                        u -= r;
                    Aij_new = (u < 0 ? u : 0);
                }
                
                // update
                A[i][j] = damp * Aij_old + (1-damp) * Aij_new;

                // compute convergenze criteria
                T abs_ij = std::abs(Aij_old-Aij_new);
                if (abs_ij > maxabs)
                    maxabs = abs_ij;
            }
        }
       return maxabs;
    }
        

    
    // extract all exemplars
    template <typename T>
    std::vector<int>
    extract_exemplars(const std::vector<std::vector<T>> &A, const std::vector<std::vector<T>> &R){
        int n = A.size();
        std::vector<int> r;
        for (int i = 0;i<n;++i){
            if (A[i][i] + R[i][i] > 0)
                r.push_back(i);
        }
        return r;
      }
    
    // get assignments
    template <typename T>
    std::tuple<std::vector<int>,std::vector<int>>
    get_assignments(const std::vector<std::vector<T>> &S, const std::vector<int> &exemplars){
        int n = S.size();
        int k = exemplars.size();
        std::vector<std::vector<T>> Se;
        for (int i=0;i<k;++i){
            Se.push_back(S[exemplars[i]]);
        }
        std::vector<int> a(n);
        std::vector<int> cnts(k,0);
        
        for (int j = 0; j<n;++j){
            int p = 0;
            T v = Se[0][j];
            for (int i = 1;i<k;++i){
                T s = Se[i][j];

                if (s > v){
                    v = s;
                    p = i;
                }
            }
            a[j] = p;
        }
       
        for (int i = 0;i<k;++i){
            a[exemplars[i]] = i;
        }

        for (int i = 0;i<n;++i){
            cnts[a[i]] += 1;
        }

        return {a, cnts};
    }    


} // end namespace affprop_functions

/*
A Affinity Propagation implementation based on a similarity matrix.
*/
    //   Reference:
    //       Clustering by Passing Messages Between Data Points.
    //       Brendan J. Frey and Delbert Dueck
    //       Science, vol 315, pages 972-976, 2007.
    //

    template <typename T>
    std::tuple<std::vector<int>,std::vector<int>,std::vector<int>>
    affprop(const std::vector<std::vector<T>> &data,
                        T preference = 0.5,
                        std::string distance_measure = distance_functions::default_measure(),
                        int maxiter =200,
                        T tol =1.0e-6,
                        T damp = 0.5){

                            // check arguments
                            int n = data.size();
                            
                            assert(n >= 2); //the number of samples must be at least 2.
                            assert(tol > 0); //tol must be a positive value.
                            assert (0 <= damp && damp < 1); // damp must be between 0 and 1.
                            assert (0 <= preference && preference < 1); // preference must be between 0 and 1.
        
        // build similarity matrix with preference                   
        std::vector<std::vector<T>> S = affprop_functions::similarity_matrix(data,preference,distance_measure);
    
        // initialize messages
        std::vector<std::vector<T>> R(n, std::vector<T>(n, 0));
        std::vector<std::vector<T>> A(n, std::vector<T>(n, 0));
    
        // main loop
        int t = 0;
        bool isConverged = false;
        while (!isConverged && t < maxiter){
            t += 1;
    
            // compute new messages
            T maxabsR = affprop_functions::update_responsibilities(R, S, A, damp);        
            T maxabsA = affprop_functions::update_availabilities(A, R, damp);
               
            // determine convergence
            T ch = std::max(maxabsA , maxabsR ) / (1 - damp);
            isConverged = (ch < tol);
    
        }
        // extract exemplars and assignments
        auto exemplars = affprop_functions::extract_exemplars(A, R);
        auto [assignments, counts] = affprop_functions::get_assignments(S, exemplars);
        
        return {assignments, exemplars, counts};
      }
    
    
      namespace kmedoids_functions {
        
        // computes the (pairwaise) distance matrix
            template <typename T>
            std::tuple<std::vector<std::vector<T>> ,T>
            distance_matrix(const std::vector<std::vector<T>> &data,
                std::string distance_measure){
                
                    T sum = 0;
                std::vector<std::vector<T>> matrix(data.size(), std::vector<T>(data.size())); //initialize
                for (int i=0;i<data.size();++i){
                    for (int j=i;j<data.size();++j){
                        T distance = distance_functions::distance(data[i],data[j], distance_measure);
                        matrix[i][j]= distance;
                        matrix[j][i]= distance;
                        if (i != j)
                        sum += 2 * distance;
                        else
                        sum += distance;
                    }
                }
                return {matrix,sum};
            }
            template <typename T>
            T 
            update_cluster(const std::vector<std::vector<T>> &D,
                std::vector<int> &seeds,
                std::vector<int> &assignments, 
                std::vector<int> &sec_nearest,
                std::vector<int> &counts) {

                if (sec_nearest.size() != assignments.size()) {
                    sec_nearest.resize(assignments.size());
                }
    
                // go through and assign each object to nearest medoid, keeping track of total distance.
                T total_distance = 0;
    
                
                for (int i=0; i < assignments.size(); i++) {
                    T    d1, d2;  // smallest, second smallest distance to medoid, respectively
                    int m1, m2;  // index of medoids with distances d1, d2 from object i, respectively
    
                    d1 = d2 = std::numeric_limits<T>::max();
                    m1 = m2 = seeds.size();
                    for (int m=0; m < seeds.size(); m++) {
                        T d = D[i][seeds[m]];
                        if (d < d1 || seeds[m] == i) {  // prefer the medoid in case of ties.
                            d2 = d1;  m2 = m1;
                            d1 = d;   m1 = m;
                        } else if (d < d2) {
                            d2 = d;   m2 = m;
                        }
                    }
                    counts[m1] +=1;
                    assignments[i] = m1;
                    sec_nearest[i] = m2;
                    total_distance += d1;
                }
    
                return total_distance;
            }

            template <typename T>
            void 
            init_medoids(int k, const std::vector<std::vector<T>> &D,     
                std::vector<int> &seeds,
                std::vector<int> &assignments, 
                std::vector<int> &sec_nearest,
                std::vector<int> &counts) {
                seeds.clear();
                
                // find first object: object minimum distance to others
                int first_medoid = 0;
                T min_dissim = std::numeric_limits<T>::max();
                for (int i=0; i < D[0].size(); i++) {
                    T total = 0;
                    for (int j=0; j < D.size(); j++) {
                        total += D[i][j];
                    }
                    if (total < min_dissim) {
                        min_dissim   = total;
                        first_medoid = i;
                    }
                }
                // add first object to medoids and compute medoid ids.
                seeds.push_back(first_medoid);
                kmedoids_functions::update_cluster(D,seeds,assignments,sec_nearest,counts);
    
                // now select next k-1 objects according to KR's BUILD algorithm
                for (int cur_k = 1; cur_k < k; cur_k++) {
                    int best_obj = 0;
                    T max_gain = 0;
                    for (int i=0; i < D[0].size(); i++) {
                        if (seeds[assignments[i]] == i) continue;
    
                        T gain = 0;
                        
                        for (int j=0; j < D[0].size(); j++) {
                            T Dj = D[j][seeds[assignments[j]]];  // D from j to its medoid
                            gain += std::max(Dj - D[i][j], T(0));                 // gain from selecting i  
                        }
    
                        if (gain >= max_gain) {   // set the next medoid to the object that 
                            max_gain = gain;        // maximizes the gain function.
                            best_obj = i;
                        }
                    }
    
                    seeds.push_back(best_obj);
                    kmedoids_functions::update_cluster(D,seeds,assignments,sec_nearest,counts);
                }
            }

            template <typename T>
            T 
            cost(int i, int h, const std::vector<std::vector<T>> &D,
                                std::vector<int> &seeds,
                                std::vector<int> &assignments, 
                                std::vector<int> &sec_nearest){
                
                                    T total = 0;
                for (int j = 0; j < assignments.size(); j++) {
                    int mi  = seeds[i];                // object id of medoid i
                    T    dhj = D[h][j];               // distance between object h and object j
    
                    int mj1 = seeds[assignments[j]];   // object id of j's nearest medoid
                    T    dj1 = D[mj1][j];             // distance to j's nearest medoid
    
                    // check if D bt/w medoid i and j is same as j's current nearest medoid.
                    if (D[mi][j] == dj1) {
                        T dj2 = std::numeric_limits<T>::max();
                        if (seeds.size() > 1) {   // look at 2nd nearest if there's more than one medoid.
                            int mj2 = seeds[sec_nearest[j]];  // object id of j's 2nd-nearest medoid
                            dj2 = D[mj2][j];                      // D to j's 2nd-nearest medoid
                        }
                        total += std::min(dj2, dhj) - dj1;
    
                    } else if (dhj < dj1) {
                        total += dhj - dj1;
                    }
                }
                return total;
            }
 
    
    } // namespace kmedoids_functions

    template <typename T>
    std::tuple<std::vector<int>,std::vector<int>,std::vector<int>>
    kmedoids(const std::vector<std::vector<T>> &data, 
        int k,
        std::string distance_measure = distance_functions::default_measure()){


        // check arguments
        int n = data.size();
        
        assert(n >= 2); // error("There must be at least two points.")
        assert(k <= n); // Attempt to run PAM with more clusters than data.
        
        
        // build the (pairwaise) distance matrix
        auto [D,Dsum] = kmedoids_functions::distance_matrix(data,distance_measure);
        
        std::vector<int> seeds(k);
        std::vector<int> counts(k,0);
        std::vector<int> assignments(n,0); 
        std::vector<int> sec_nearest(n,0);      /// Index of second closest medoids.  Used by PAM.
        T total_distance;              /// Total distance tp their medoid
        T epsilon =  1e-15;                          /// Normalized sensitivity for convergence


        // set initianl medoids
        kmedoids_functions::init_medoids(k, D,seeds,assignments,sec_nearest,counts);

        T tolerance = epsilon * Dsum / (D[0].size() * D.size());

        while (true) {
            // initial cluster
            for (int i=0; i<counts.size();++i){
                counts[i]=0;
            }
            total_distance = kmedoids_functions::update_cluster(D,seeds,assignments,sec_nearest,counts);

            //vars to keep track of minimum
            T minTotalCost = std::numeric_limits<T>::max();
            int minMedoid = 0;
            int minObject = 0;

            //iterate over each medoid
            for (int i=0; i < k; i++) {
                //iterate over all non-medoids
                for (int h = 0; h < assignments.size(); h++) {
                    if (seeds[assignments[h]] == h) continue;

                    //see if the total cost of swapping i & h was less than min
                    T curCost = kmedoids_functions::cost(i, h, D,seeds,assignments,sec_nearest);
                    if (curCost < minTotalCost) {
                        minTotalCost = curCost;
                        minMedoid = i;
                        minObject = h;
                    }
                }
            }

            // convergence check
            if (minTotalCost >= -tolerance) break;

            // install the new medoid if we found a beneficial swap
            seeds[minMedoid] = minObject;
            assignments[minObject] = minMedoid;
        }


        return {assignments, seeds, counts};
    }

        

} // namespace clustering

