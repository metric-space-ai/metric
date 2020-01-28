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
//#include <filesystem>
#include <experimental/filesystem> // temporary
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/program_options.hpp>

#include "assets/AudioFile.h"

#include "../../modules/distance/k-related/Standards.hpp"

#include "../../modules/utils/pattern_compressor.hpp"
namespace po = boost::program_options;

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
        out_name = std::filesystem::path(file_name).filename().string() + ".z";
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
    double similarity_threshold = 0.05;
    std::size_t slice_size = 4096;
    try {
        po::options_description desc("wav_compressor");
        desc.add_options()
            ("help,h", "produce help message")
            ("decompress,d", "Decompress")
            ("threshold,r", po::value<double>()->default_value(0.05), "Similarity threshold")
            ("slice-size,s", po::value<std::size_t>()->default_value(4096), "Size of input data slices")
            ("threads,t", po::value<std::size_t>()->default_value(2), "Number of threads to use while compressing")
            ("output-file,o", po::value<std::string>(), "output file, default == 'input-file'.z")
            ("input-file", po::value<std::string>(), "input file");

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
        } 

        if(vm.count("threshold")) {
            similarity_threshold = vm["threshold"].as<double>();
        }
        if (vm.count("slice-size")) {
            slice_size = vm["slice-size"].as<std::size_t>();
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
    auto cf = metric::make_compressor_factory<double, metric::TWED<double>>(5, slice_size, 1, 0);
    auto wcomp = cf(std::vector<double>(cf.get_subbands_size(), similarity_threshold), threads);
    if (is_decompress) {
        decompress_file(file_name, output_file, std::move(wcomp));
    } else {
        compress_file(file_name, output_file, std::move(wcomp));
    }

    return 0;
}
