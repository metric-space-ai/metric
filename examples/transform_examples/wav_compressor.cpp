/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <chrono>
#ifdef __GNUC__
  #if __GNUC_PREREQ(8,0)
	#include <filesystem>
        namespace fs = std::filesystem;
  #else
	#include <experimental/filesystem>
        namespace fs = std::experimental::filesystem;
  #endif
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif
//#include <experimental/filesystem> // edited by Max F

#include <boost/iostreams/filter/zlib.hpp>

#include "assets/AudioFile.h"

#include "../../modules/distance/k-related/Standards.hpp"

#include "../../modules/utils/pattern_compressor.hpp"


template <typename Compressor>
void compress_file(const std::string& file_name, std::string out_name, Compressor compressor)
{
    AudioFile<double> file;
    file.load(file_name);
    std::vector<double> buffer;
    buffer.reserve(compressor.get_batch_size());
    std::size_t count = 0;
    for(auto & v : file.samples[0]) {
        // accumulate enough data from input
        buffer.push_back(v);
        if (buffer.size() == compressor.get_batch_size()) {
            // process slice
            compressor.process_batch(buffer);
            std::cout << "push slice #" << count << std::endl;
            buffer.clear();
            count++;
        }
    }
    if (!buffer.empty()) {
        if(buffer.size() < compressor.get_batch_size()) {
            buffer.resize(compressor.get_batch_size(), 0.0);
        }
        compressor.process_batch(buffer);
        std::cout << "push slice #" << count << std::endl;
        count++;
    }

    // compress decomposed time series
    boost::iostreams::zlib_params zp(boost::iostreams::zlib::default_compression, boost::iostreams::zlib::deflated, 15,
        8, boost::iostreams::zlib::huffman_only);
    std::vector<char> v = compressor.compress(boost::iostreams::zlib_compressor(zp));
    if(out_name.empty()) {
        out_name = fs::path(file_name).filename().string() + ".z";
        //out_name = std::experimental::filesystem::path(file_name).filename().string() + ".z"; // edited by Max F, Jan, 9, 2020
    }

    // save compressed data
    std::ofstream ostr(out_name.c_str(), std::ios::binary);
    ostr.write(v.data(), v.size());
    ostr.flush();
    ostr.close();
}

template <typename Compressor>
void decompress_file(const std::string& file_name, const std::string& outfile, Compressor compressor)
{
    std::ifstream istr(file_name.c_str(), std::ios::binary);
    std::vector<char> input { std::istreambuf_iterator<char>(istr), std::istreambuf_iterator<char>() };

    compressor.decompress(input, boost::iostreams::zlib_decompressor());
    istr.close();

    AudioFile<double> audio;
    std::vector<std::vector<double>> buffer(1);

    for (std::size_t i = 0; i < compressor.get_band_size(); i++) {
        auto batch = compressor.get_batch(i);
        buffer[0].insert(buffer[0].end(), batch.begin(), batch.end());
    }
    audio.setAudioBuffer(buffer);
    audio.save(outfile);
}

int main(int argc, char** argv)
{

	if ((argc < 5) or (argc > 6)) {
		std::cout << "Usage: wav_compressor input_file slice_size[4096] threshold[0.05] threads [output_file]" << std::endl;
		return -1;
	}

	std::string file_name = argv[1];
	std::size_t slice_size = std::atol(argv[2]);
	double similarity_threshold = std::atof(argv[3]);
	std::size_t threads = std::atol(argv[4]);

	std::string output_file;
	bool is_decompress = false;
	if (argc == 6) {
		output_file = argv[5];
		is_decompress = true;
	}


    // create compressor for double time series with rate = 4096 and wavelet_type = 5
    auto cf = metric::make_compressor_factory<double, metric::TWED<double>>(5, slice_size, 1, 0);
    auto wcomp = cf(std::vector<double>(cf.get_subbands_size(), similarity_threshold), threads);
    if (is_decompress) {
        decompress_file(file_name, output_file, std::move(wcomp));
    } else {
        compress_file(file_name, output_file, std::move(wcomp));
    }

    return 0;
}
