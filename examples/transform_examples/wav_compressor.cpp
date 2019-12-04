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
#include <filesystem>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/program_options.hpp>

#include "assets/AudioFile.h"
#include "../../modules/utils/pattern_compressor.hpp"
#include "../../modules/distance/k-related/Standards.hpp"

namespace po = boost::program_options;

template <typename Compressor>
void compress_file(const std::string& file_name, const std::string& out_name, Compressor compressor)
{
    AudioFile<double> file;
    file.load(file_name);
    std::vector<double> buffer;
    buffer.reserve(compressor.get_rate());
    std::size_t count = 0;
    for (std::size_t i = 0; i < file.samples[0].size(); i++) {
        // accumulate enough data from input
        buffer.push_back(file.samples[0][i]);
        if (buffer.size() == compressor.get_rate()) {

            // process slice
            compressor.process_batch(std::move(buffer));
            std::cout << "push slice #" << count << std::endl;
            buffer.clear();
            count++;
        }
    }
    if (!buffer.empty()) {
        compressor.process_batch(std::move(buffer));
        std::cout << "push slice #" << count << std::endl;
        count++;
    }

    // compress decomposed time series
    boost::iostreams::zlib_params zp(boost::iostreams::zlib::default_compression, boost::iostreams::zlib::deflated, 15,
        8, boost::iostreams::zlib::huffman_only);
    std::vector<char> v = compressor.compress(boost::iostreams::zlib_compressor(zp));

    // save compressed data
    std::string out_file = std::filesystem::path(file_name).filename().string() + ".z";
    std::ofstream ostr(out_file.c_str(), std::ios::binary);
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
        auto slice = compressor.get_slice(i);
        buffer[0].insert(buffer[0].end(), slice.begin(), slice.end());
    }
    audio.setAudioBuffer(buffer);
    audio.save(outfile);
}

void usage(const po::options_description& desc)
{
    std::cout << "Usage: wav_compressor [-h] [-d] [-o output_file] input_file \n";
    std::cout << desc;
}

int main(int argc, char** argv)
{

    bool is_decompress = false;
    std::string file_name;
    std::string output_file;
    std::size_t threads = 1;
    try {
        po::options_description desc("wav_compressor");
        desc.add_options()("decompress,d", "Decompress")("threads,t", po::value<std::size_t>()->default_value(2),
            "Number of threads to use while compressing")("help,h", "produce help message")("output-file,o",
            po::value<std::string>(),
            "output file, default == 'input-file'.z")("input-file", po::value<std::string>(), "input file");

        po::positional_options_description p;
        p.add("input-file", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            usage(desc);
            return 0;
        }
        if (vm.count("threads")) {
            threads = vm["threads"].as<std::size_t>();
        }
        if (vm.count("input-file")) {
            file_name = vm["input-file"].as<std::string>();
        } else {
            usage(desc);
            return 1;
        }

        if (vm.count("output-file")) {
            output_file = vm["output-file"].as<std::string>();
        } else {
            output_file = file_name + ".z";
        }

        if (vm.count("decompress")) {
            if (!vm.count("output-file")) {
                usage(desc);
                return 1;
            }
            is_decompress = true;
        }

    } catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return 1;
    }

    // create compressor for double time series with rate = 4096 and wavelet_type = 5
    auto cf = metric::make_compressor_factory<double, metric::TWED<double>>(5, 4096, 1, 0);
    auto wcomp = cf(std::vector<double>(cf.get_subbands_size(), 0), threads);
    if (is_decompress) {
        decompress_file(file_name, output_file, std::move(wcomp));
    } else {
        compress_file(file_name, output_file, std::move(wcomp));
    }

    return 0;
}
