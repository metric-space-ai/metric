/*
This Source Code Form is subject to the terms of the Mozilla Public License,
v.2.0. If a copy of the MPL was not distributed with this file,
You can obtain one at http :  //mozilla.org/MPL/2.0/.

Copyright(c) 2019 PANDA Team
*/

#ifndef _METRIC_UTILS_PATTERN_COMPRESSOR_CPP
#define _METRIC_UTILS_PATTERN_COMPRESSOR_CPP
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include <blaze/math/CompressedVector.h>

#include "../../3rdparty/cereal/archives/binary.hpp"
#include "../../3rdparty/cereal/types/unordered_map.hpp"
#include "../../3rdparty/cereal/types/vector.hpp"
#include "../../3rdparty/cereal/specialize.hpp"

#include "pattern_compressor.hpp"

namespace metric {
template <typename T, typename Metric>
inline pattern_compressor<T, Metric>::pattern_compressor(std::vector<T>&& noise_threshold_,
    std::vector<T>&& metric_threshold_, std::size_t rate_, int wavelet_type_, Metric metric, int threads)
    : rate(rate_)
    , wavelet_type(wavelet_type_)
    , thread_pool(std::make_unique<StdThreadPool>(threads))
{
    bands_index_map.resize(noise_threshold_.size());
    join_map.resize(noise_threshold_.size());
    trees.reserve(noise_threshold_.size());
    mutex.resize(noise_threshold_.size());
    noise_threshold = noise_threshold_;
    similarity_threshold = metric_threshold_;
    for (std::size_t i = 0; i < noise_threshold.size(); i++) {
        trees.emplace_back(std::make_unique<Tree>(-1, recMetric_Blaze<T, Metric>(metric)));
    }
}
template <typename T, typename Metric>
inline pattern_compressor<T, Metric>::pattern_compressor(pattern_compressor<T, Metric>&& pc)
    : rate(pc.rate)
    , wavelet_type(pc.wavelet_type)
    , bands_index_map(std::move(pc.bands_index_map))
    , join_map(std::move(pc.join_map))
    , trees(std::move(pc.trees))
    , mutex(std::move(pc.mutex))
    , noise_threshold(std::move(pc.noise_threshold))
    , similarity_threshold(std::move(pc.similarity_threshold))
    , thread_pool(std::move(pc.thread_pool))
{
}

template <typename T, typename Metric>
inline void pattern_compressor<T, Metric>::process_band(
    std::vector<T>&& band, std::size_t band_index, std::size_t slice_index)
{
    blaze::CompressedVector<T> vc = wavelet::smoothDenoise(band, noise_threshold[band_index]);
    std::lock_guard<std::mutex> lk(mutex[band_index]);
    auto res = trees[band_index]->insert_if(vc, similarity_threshold[band_index]);
    if (std::get<1>(res) == false) {
        auto index = std::get<0>(res);
        join_map[band_index].insert({ slice_index, band_variant_t(bands_index_map[band_index][index]) });
    } else {
        bands_index_map[band_index][std::get<0>(res)] = slice_index;
        join_map[band_index].insert({ slice_index, vc });
    }
}

template <typename T, typename Metric>
inline void pattern_compressor<T, Metric>::make_task(
    std::vector<T>&& v, std::size_t band_index, std::size_t slice_index)
{
    thread_pool->schedule([this, v = std::move(v), band_index, slice_index]() mutable {
        process_band(std::forward<std::vector<T>>(v), band_index, slice_index);
    });
}

template <typename T, typename Metric>
inline std::vector<T> pattern_compressor<T, Metric>::to_vector(const blaze::CompressedVector<T>& v)
{
    std::vector<T> res(v.size(), T { 0 });
    for (auto p = v.cbegin(); p != v.cend(); ++p) {
        res[p->index()] = p->value();
    }
    return res;
}
template <typename T, typename Metric>
inline void pattern_compressor<T, Metric>::print_vec(const std::vector<T>& v)
{
    for (std::size_t i = 0; i < v.size(); i++) {
        std::cout << v[i] << " ";
    }
    std::cout << std::endl;
}
template <typename T, typename Metric>
inline void pattern_compressor<T, Metric>::print_vec(const blaze::CompressedVector<T>& v, std::size_t real_index)
{
    std::cout << "(" << v.size() << ", " << v.nonZeros() << ", " << real_index << ") ";
    for (std::size_t i = 0; i < v.size(); i++) {
        std::cout << v[i] << " ";
    }
    std::cout << std::endl;
}

template <typename T, typename Metric>
inline void pattern_compressor<T, Metric>::process_batch(std::vector<T>&& buffer)
{
    if (buffer.size() < rate) {
        buffer.resize(rate, T {});
    }
    std::size_t slice_index = counter++;
    std::vector<T> x_tmp = buffer;

    make_task(std::vector<T>(buffer.size(), 0), trees.size() - 1, slice_index);

    for (int k = 0; k < trees.size() - 2; ++k) {
        auto [x_tmp2, d] = wavelet::dwt(x_tmp, wavelet_type);
        x_tmp.swap(x_tmp2);
        make_task(std::move(d), trees.size() - 2 - k, slice_index);
    }
    make_task(std::move(x_tmp), 0, slice_index);
}

template <typename T, typename Metric>
template <typename Compressor>
inline std::vector<char> pattern_compressor<T, Metric>::compress(Compressor compressor)
{
    thread_pool->wait();
    std::vector<char> result;
    {
        boost::iostreams::filtering_ostream os;
        boost::iostreams::back_insert_device<std::vector<char>> inserter(result);
        os.push(compressor);
        os.push(inserter);
        cereal::BinaryOutputArchive oar(os);
        oar(join_map);
    }
    return result;
}

template <typename T, typename Metric>
template <typename Decompressor>
inline void pattern_compressor<T, Metric>::decompress(const std::vector<char>& input, Decompressor decompressor)
{
    join_map.clear();
    bands_index_map.clear();
    {
        boost::iostreams::filtering_istream is;
        is.push(std::forward<Decompressor>(decompressor));
        is.push(boost::make_iterator_range(input));
        cereal::BinaryInputArchive iar(is);
        iar(join_map);
    }
}

template <typename T, typename Metric>
inline std::vector<T> pattern_compressor<T, Metric>::get_slice(std::size_t index)
{
    if (join_map.empty())
        throw std::runtime_error("index out of bounds");
    if (join_map[0].size() < index)
        throw std::runtime_error("index out of bounds");
    std::deque<std::vector<T>> band_sample(join_map.size());
    for (std::size_t j = 0; j < join_map.size(); j++) {
        const band_variant_t& v = join_map[j][index];
        if (v.index() == 0) {
            band_sample[j] = to_vector(std::get<0>(v));
        } else {
            std::size_t i = std::get<std::size_t>(v);
            band_sample[j] = to_vector(std::get<0>(join_map[j][i]));
        }
    }
    return wavelet::waverec(band_sample, wavelet_type);
}

}
namespace cereal {
template <typename Archive, typename T>
void save(Archive& ar, const blaze::CompressedVector<T>& v)
{

    ar(v.size());
    ar(v.nonZeros());
    for (auto p = v.cbegin(); p != v.cend(); ++p) {
        ar(p->index());
        ar(p->value());
    }
}

template <typename Archive, typename T>
void load(Archive& ar, blaze::CompressedVector<T>& v)
{
    std::size_t size, nonZeros;
    ar(size);
    ar(nonZeros);
    v.resize(size);
    v.reserve(nonZeros);
    for (std::size_t i = 0; i < nonZeros; i++) {
        std::size_t index;
        double e;
        ar(index, e);
        v.set(index, e);
    }
}

template <typename Archive, typename T>
void save(Archive& ar, const std::variant<blaze::CompressedVector<T>, std::size_t>& v)
{
    ar(v.index());
    if (v.index() == 0) {
        ar(std::get<0>(v));
    } else {
        ar(std::get<1>(v));
    }
}

template <typename Archive, typename T>
void load(Archive& ar, std::variant<blaze::CompressedVector<T>, std::size_t>& v)
{
    std::size_t index;
    ar(index);
    if (index == 0) {
        blaze::CompressedVector<T> vec;
        ar(vec);
        v = vec;
    } else {
        std::size_t i;
        ar(i);
        v = i;
    }
}

template <class Archive, typename T>
struct specialize<Archive, blaze::CompressedVector<T>, cereal::specialization::non_member_load_save> {
};

template <class Archive, typename T>
struct specialize<Archive, std::variant<blaze::CompressedVector<T>, size_t>,
    cereal::specialization::non_member_load_save> {
};
}

#endif
