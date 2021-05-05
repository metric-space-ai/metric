/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 M.Welsch
*/

#ifndef _METRIC_MAPPING_KMEANS_CPP
#define _METRIC_MAPPING_KMEANS_CPP

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

#include <vector>
#include <string>
#include <random>
#include <cassert>
#include <algorithm>
#include "../distance/k-related/Standards.hpp"

namespace metric {

namespace kmeans_details {
    inline std::string default_measure(void) { return "Euclidean"; }
    template <typename T>
    T distance(const std::vector<T>& a, const std::vector<T>& b, std::string distance_measure)
    {

        assert(a.size() == b.size());  // data vectors have not the same length
        if (distance_measure.compare("Euclidean") == 0)
            return metric::Euclidean<T>()(a, b);
        else if (distance_measure.compare("rms") == 0) {
            T val = metric::Euclidean<T>()(a, b);
            return val * val;
        } else if (distance_measure.compare("manhatten") == 0)
            return metric::Manhatten<T>()(a, b);
		else if (distance_measure.compare("cosine_inverted") == 0)
            return metric::CosineInverted<T>()(a, b);
        else {
            return metric::Euclidean<T>()(a, b);
        }
    }

    /*
    closest distance between datapoints and means.
    */
    template <typename T>
    std::vector<T> closest_distance(const std::vector<std::vector<T>>& means,
        const std::vector<std::vector<T>>& datapoints, int k, std::string distance_measure)
    {
        std::vector<T> distances;
        distances.reserve(k);
        for (auto& d : datapoints) {
            T closest = kmeans_details::distance(d, means[0], distance_measure);
            for (auto& m : means) {
                T distance = kmeans_details::distance(d, m, distance_measure);
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
    std::vector<std::vector<T>> random_init(
        const std::vector<std::vector<T>>& data, int k, std::string distance_measure, long long random_seed)
    {
        assert(k > 0);
        using input_size_t = typename std::vector<T>::size_type;
        std::vector<std::vector<T>> means;
        // Using a very simple PRBS generator, parameters selected according to
        // https://en.wikipedia.org/wiki/Linear_congruential_generator#Parameters_in_common_use
        //std::random_device rand_device;
        //std::linear_congruential_engine<uint64_t, 6364136223846793005, 1442695040888963407, UINT64_MAX> rand_engine(
        //    rand_device());
		if (random_seed == -1)
		{
			random_seed = std::chrono::system_clock::now().time_since_epoch().count();
		}
		std::default_random_engine random_generator(random_seed);

        // Select first mean at random from the set
        {
            std::uniform_int_distribution<input_size_t> uniform_generator(0, data.size() - 1);
            means.push_back(data[uniform_generator(random_generator)]);
        }

        for (int count = 1; count < k; ++count) {
            // Calculate the distance to the closest mean for each data point
            auto distances = closest_distance(means, data, k, distance_measure);
            // Pick a random point weighted by the distance from existing means
            // TODO: This might convert floating point weights to ints, distorting the distribution for small weights
            std::discrete_distribution<size_t> generator(distances.begin(), distances.end());
            means.push_back(data[generator(random_generator)]);
        }
        return means;
    }

    /*
    find closest mean for a data point
    */
    template <typename T>
    int findClosestMean(
        const std::vector<T>& datapoint, const std::vector<std::vector<T>>& means, std::string distance_measure)
    {
        assert(!means.empty());

        T smallest_distance = kmeans_details::distance(datapoint, means[0], distance_measure);
        //typename std::vector<T>::size_type index = 0;
        int index = 0;
        T distance;
        for (int i = 1; i < means.size(); ++i) {
            distance = kmeans_details::distance(datapoint, means[i], distance_measure);
            if (distance < smallest_distance) {
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
    void update_assignments(std::vector<int>& assignments, const std::vector<std::vector<T>>& data,
        const std::vector<std::vector<T>>& means, std::string distance_measure)
    {
        for (int i = 0; i < data.size(); ++i) {
            assignments[i] = findClosestMean(data[i], means, distance_measure);
        }
    }

    /*
    means based on datapoints and their cluster assignments.
    */
    template <typename T>
    std::tuple<std::vector<int>, int> update_means(std::vector<std::vector<T>>& means,
        const std::vector<std::vector<T>>& data, const std::vector<int>& assignments, const int& k)
    {

        std::vector<std::vector<T>> old_means = means;

        std::vector<int> count(k, int(0));
        for (int i = 0; i < std::min(assignments.size(), data.size()); ++i) {
            count[assignments[i]] += 1;
            for (int j = 0; j < std::min(data[i].size(), means[assignments[i]].size()); ++j) {
                means[assignments[i]][j] += data[i][j];
            }
        }
        int updated = 0;
        for (int i = 0; i < k; ++i) {
            if (count[i] == 0) {
                means[i] = old_means[i];
            } else {
                for (int j = 0; j < means[i].size(); ++j) {
                    means[i][j] /= double(count[i]);
                    if (means[i][j] != old_means[i][j])
                        updated += 1;
                }
            }
        }

        return { count, updated };
    }

    inline void rearrange_assignments(std::vector<int>& assignments)
    {
        std::vector<int> from_list;
        std::vector<int> to_list;
        from_list.push_back(assignments[0]);
        to_list.push_back(int(0));

        for (int i = 1; i < assignments.size(); ++i) {
            bool hit = false;
            for (int j = 0; j < from_list.size(); ++j) {
                if (from_list[j] == assignments[i]) {
                    hit = true;
                }
            }
            if (!hit) {
                from_list.push_back(assignments[i]);
                to_list.push_back(from_list.size() - 1);
            }
        }

        for (int i = 0; i < assignments.size(); ++i) {
            int old_indx;
            for (int j = 0; j < to_list.size(); ++j) {
                if (from_list[j] == assignments[i])
                    old_indx = j;
            }
            assignments[i] = to_list[old_indx];
        }
    }

}  // end namespace kmeans_details

template <typename T>
std::tuple<std::vector<int>, std::vector<std::vector<T>>, std::vector<int>> kmeans(
    const std::vector<std::vector<T>>& data, int k, int maxiter, std::string distance_measure, long long random_seed)
{
    static_assert(std::is_arithmetic<T>::value && std::is_signed<T>::value,
        "kmeans_lloyd requires the template parameter T to be a signed arithmetic type (e.g. float, double, int)");
    assert(k > 0);  // k must be greater than zero
    assert(data.size() >= k);  // there must be at least k data points

    std::vector<std::vector<T>> means = kmeans_details::random_init(data, k, distance_measure, random_seed);

    std::vector<int> assignments(data.size());
    // Calculate new meansData until convergence is reached
    int t = 0;
    int updated_number_of_means = 0;
    std::vector<int> counts(k, int(0));
	
    do {
        kmeans_details::update_assignments(assignments, data, means, distance_measure);
        auto [updated_counts, updated_number_of_means] = kmeans_details::update_means(means, data, assignments, k);
        counts = updated_counts;
        ++t;
    } while (updated_number_of_means != int(0) && t < maxiter);

    //kmeans_details::rearrange_assignments(assignments); // hide by Stepan Mmaontov 28 10 2019 - rearranging asssignments does not reflect with counts and means
    return { assignments, means, counts };
}

}  // namespace metric

#endif
