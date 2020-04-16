/*
This Source Code Form is subject to the terms of the Mozilla Public License,
v.2.0. If a copy of the MPL was not distributed with this file,
You can obtain one at http :  //mozilla.org/MPL/2.0/.

Copyright(c) 2019 PANDA Team
*/

#ifndef _METRIC_UTILS_PATTERN_COMPRESSOS_HPP
#define _METRIC_UTILS_PATTERN_COMPRESSOS_HPP

#include <atomic>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
#include <thread>
#include <mutex>

#include <boost/iostreams/filter/zlib.hpp>

#include "../..//3rdparty/blaze/math/CompressedVector.h"
#include "../..//3rdparty/blaze/util/ThreadPool.h"
#include "../../modules/distance/k-structured/TWED.hpp"
#include "../../modules/space/tree.hpp"
#include "../../modules/transform/wavelet.hpp"
#include "../../modules/transform/helper_functions.cpp" // added by Max F, Jan, 29, 2020

namespace metric {

/**
     * \class pattern_compressor
     * \brief compressor/decompressor using wavelet decomposition
     *
     *
     */
template <typename T, typename Metric>
class pattern_compressor {
    using StdThreadPool
        = blaze::ThreadPool<std::thread, std::mutex, std::unique_lock<std::mutex>, std::condition_variable>;

    using band_variant_t = std::variant<typename blaze::CompressedVector<T>, std::size_t>;

    template <typename T1, typename Metric1>
    struct recMetric_Blaze {
        Metric1 metric;

        recMetric_Blaze() = default;

        explicit recMetric_Blaze(Metric1 m)
            : metric(m)
        {
        }
        T1 operator()(const blaze::CompressedVector<T1>& p, const blaze::CompressedVector<T1>& q) const
        {
            auto v = metric(metric::TWED_details::addZeroPadding(p), metric::TWED_details::addZeroPadding(q));
            return v;
        }
    };

    using metric_t = recMetric_Blaze<T, Metric>;
    using Tree = metric::Tree<typename blaze::CompressedVector<T>, metric_t>;
    using band_index_t = std::unordered_map<std::size_t, std::size_t>;

    std::size_t batch_size;
    int wavelet_type;
    std::vector<std::unique_ptr<Tree>> trees;
    std::vector<band_index_t> bands_index_map;
    std::vector<T> noise_threshold;
    std::vector<T> similarity_threshold;

    std::vector<std::unordered_map<std::size_t, band_variant_t>> join_map;
    std::atomic<std::size_t> counter = 0;
    std::deque<std::mutex> mutex;
    std::unique_ptr<StdThreadPool> thread_pool;

    void process_band(std::vector<T>&& band, std::size_t band_index, std::size_t slice_index);

    void make_task(std::vector<T>&& v, std::size_t band_index, std::size_t slice_index);

    std::vector<T> to_vector(const blaze::CompressedVector<T>& v) const;

    void print_vec(const std::vector<T>& v) const;

    void print_vec(const blaze::CompressedVector<T>& v, std::size_t real_index) const;

    pattern_compressor(const pattern_compressor<T, Metric> &) = delete;
    pattern_compressor<T, Metric> & operator=(const pattern_compressor<T, Metric> &) = delete;

public:
    /** \brief Constructor
     *
     * @param noise_threshold_ vector of threshold values used to remove noise from input data
     * @param similarity_threshold_ vector of threshold values used to similarity check
     * @param batch_size_ size of input data batch
     * @param wavelet_type type of wavelet used to decompose input data
     * @param metric Metric object used to similarity check
     * @param threads number of thread in internal thread_pool
     */
    pattern_compressor(std::vector<T>&& similarity_threshold_, std::vector<T>&& noise_threshold_,
                       std::size_t batch_size_, int wavelet_type_, Metric metric, int threads);

    /** \brief Move constructor
     *
     */
    pattern_compressor(pattern_compressor<T, Metric>&& pc) noexcept;

    /**
     * \brief moving operator=
     */
    pattern_compressor<T,Metric> & operator=(pattern_compressor<T, Metric> && pc)  noexcept;

    ~pattern_compressor() = default;
    
    /** \briefProcess next slice of time series
     *
     * @param buffer next slice of input data, size of buffer should be equal to get_batch_size()
     * @exception std::runtime_error throws when size of bufffer is not equalt to get_batch_size()
     */
    void process_batch(const std::vector<T>& buffer);

    /** \brief Compress decomposed data
     * @param compressor compression filter, may be boost::iostreams::zlib_compressor
     * @return array of cahr with compressed data
     */
    template <typename Compressor>
    std::vector<char> compress(const Compressor & compressor = boost::iostreams::zlib_compressor()) const;

    /**
     * \brief Decompress data
     * @param input vector of compressed data
     * @param decompression filter, may be boost::iostreams::zlib_decompressor
     */
    template <typename Decompressor>
    void decompress(const std::vector<char>& input, const Decompressor & decompressor = boost::iostreams::zlib_decompressor());

    /**
     * \brief Restore data batch
     *
     * @param index index of the batch in time series
     * @return restored batch of data
     */
    std::vector<T> get_batch(std::size_t index) const;

    /**
     * \brief Get the expected size of input slice
     *
     */
    [[nodiscard]] std::size_t get_batch_size() const noexcept { return batch_size; }

    /**
     * \brief Get number of decomposed subbands
     *
     */
    [[nodiscard]] std::size_t get_subbands_size() const noexcept { return join_map.size(); }

    /**
     *  \brief Get number of elements of one subband
     */
    [[nodiscard]] std::size_t get_band_size() const noexcept
    {
        if (join_map.empty())
            return 0;
        return join_map.begin()->size();
    }
};

/**
     * @class pattern_compressor_factory
     *
     * @tparam RecType  type of input data
     * @tparam Metric metric used for similarity check
     * @tparam Ts types of arguments for Metric constructor
     */
template <typename RecType, typename Metric, typename... Ts>
class pattern_compressor_factory {
    const int wavelet_type;
    const std::size_t batch_size;
    const std::tuple<Ts...> metric_args;
    const std::size_t subbands_size;

    template <typename... Args, std::size_t... Is>
    Metric make_metric(const std::tuple<Args...>& tup, std::index_sequence<Is...>) const
    {
        return Metric(std::get<Is>(tup)...);
    }

    template <typename... Args>
    Metric make_metric(const std::tuple<Args...>& tup) const
    {
        return make_metric(tup, std::index_sequence_for<Args...> {});
    }

public:
    /**
         * \brief Constructor
         *
         * @param wavelet_type_ wavelet type number
         * @param rate_ size of incoming batch of data
         * @param args arguments for Metric constructor
         */
    pattern_compressor_factory(int wavelet_type_, std::size_t batch_size_, Ts&&... args)
        : wavelet_type(wavelet_type_)
        , batch_size(batch_size_)
        , metric_args(std::make_tuple(std::forward<Ts>(args)...))
        , subbands_size(wavelet::wmaxlev(batch_size, wavelet_type) + 2)
    {
    }

    /**
         * \brief Return a number of decomposed subbands
         *
         */
    [[nodiscard]] std::size_t get_subbands_size() const { return subbands_size; }

    /**
         * \@brief make vector with default values of threshold to remove noise from input data.
         *
         * @return vector with default values of threshold for each subband
         */

    std::vector<RecType> make_noise_threshold() const
    {
        std::vector<RecType> threshold;
        RecType factor = std::sqrt(RecType { 2.0 });
        for (std::size_t i = 0; i < subbands_size; ++i) {
            if (i < 2) {
                threshold.push_back(0.01);
            } else {
                threshold.push_back(threshold[i - 1] * factor);
            }
        }
        return threshold;
    }

    /**
         * \brief Constructor of pattern_compressor objects based on factory parameters
         *
         * @param metric_threshold vector of threshold values to check similarity between subbands
         * @param noise_threshold vector of threshold values to remove noise from input data
         * @param threads number of thread in internal thread_pool, default value is 2
         * @return pattern_compressor object
         */
    pattern_compressor<RecType, Metric> operator()(
        std::vector<RecType>&& similarity_threshold, std::vector<RecType>&& noise_threshold, int threads = 2) const
    {
        if(similarity_threshold.size() != get_subbands_size()) {
            throw std::runtime_error("size of similarity_threshold should be equal to get_subbands_size()");
        }
        if (noise_threshold.size() != get_subbands_size()) {
            throw std::runtime_error("size of similarity_threshold should be equal to get_subbands_size()");
        }

        return pattern_compressor<RecType, Metric>(std::forward<std::vector<RecType>>(similarity_threshold),
                                                   std::forward<std::vector<RecType>>(noise_threshold),
                                                   batch_size, wavelet_type, make_metric(metric_args), threads);
    }

    /**
         * \brief Constructor of pattern_compressor objects based on factory parameters
         *
         * @param similarity_threshold vector of threshold values to check similarity between subbands
         * @param threads number of thread in internal thread_pool, default value is 2
         * @return pattern_compressor object
         */
    pattern_compressor<RecType, Metric> operator()(std::vector<RecType>&& similarity_threshold, int threads = 2) const
    {
        if (similarity_threshold.size() != get_subbands_size()) {
            throw std::runtime_error("size of similarity_threshold should be equal to get_subbands_size()");
        }

        return pattern_compressor<RecType, Metric>(std::forward<std::vector<RecType>>(similarity_threshold),
                                                   make_noise_threshold(),
                                                   batch_size, wavelet_type, make_metric(metric_args), threads);
    }
};

/**
     * \brief Create compressor factory
     *
     * @tparam RecType type of input data
     * @tparam Metric metric class used to similarity check
     *
     * @param wavelet_type type of wavelet used for decomposition
     * @param batch_size size of batch of input data
     * @param args arguments for Metric constructor
     */
template <typename RecType, typename Metric, typename... Ts>
inline pattern_compressor_factory<RecType, Metric, Ts...> make_compressor_factory(
    int wavelet_type, std::size_t batch_size, Ts&&... args)
{
    return pattern_compressor_factory<RecType, Metric, Ts...>(wavelet_type, batch_size, std::forward<Ts>(args)...);
}
}  // namespace metric

#include "pattern_compressor.cpp"
#endif
