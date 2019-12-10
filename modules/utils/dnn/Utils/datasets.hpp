#include <iostream>
#include <fstream>

#include <tuple>
#include <tiff.h>

#include "../../../../3rdparty/cereal/types/vector.hpp"
#include "../../../../3rdparty/cereal/archives/binary.hpp"


class Datasets {
	public:
		std::tuple<std::vector<uint8_t>, std::vector<uint32_t>, std::vector<uint8_t>> getMnist(const std::string filename);

		static std::tuple<std::vector<uint32_t>, std::vector<uint8_t>> loadImages(const std::string imagesListFilename);
};


std::tuple<std::vector<uint8_t>, std::vector<uint32_t>, std::vector<uint8_t>> Datasets::getMnist(const std::string filename)
{
	std::ifstream dataFile(filename, std::ifstream::binary);

	if (dataFile.is_open()) {
		cereal::BinaryInputArchive ia(dataFile);
		std::vector<uint8_t> labels;
		std::vector<uint32_t> shape;
		std::vector<uint8_t> features;

		ia(labels, shape, features);

		return {labels, shape, features};
	} else {
		std::cout << "Could not open " << filename << std::endl;
		return {{}, {}, {}};
	}

}

std::tuple<std::vector<uint32_t>, std::vector<uint8_t>> Datasets::loadImages(const std::string imagesListFilename)
{
	/* Open imagesList file */
	std::ifstream imagesListFile(imagesListFilename);
	if (imagesListFile.is_open()) {

		std::vector<uint32_t> shape = {0, 0, 0, 1};
		std::vector<uint8_t> data;

		std::string line;
		while(getline(imagesListFile, line)) {
			std::ifstream imageFile(line);
			if (!imageFile.is_open()) {
				std::cout << "Could not open " << line << std::endl;
				continue;
			}

			/* Check format */
			std::string format;
			imageFile >> format;
			if (format != "P5") {
				std::cout << "Format of " << line << " is not P6" << std::endl;
				continue;
			}

			/* Read and check resolution */
			uint32_t width, height;
			imageFile >> width >> height;

			imageFile.close();

			/* Load binary image data */
			imageFile.open(line, std::ifstream::binary);
			size_t dataSize = width * height * sizeof(uint8_t);
			imageFile.seekg(-dataSize, imageFile.end);

			std::vector<uint8_t> imageData(width * height);
			imageFile.read(reinterpret_cast<char*>(imageData.data()), dataSize);

			/* Copy to general container */
			data.insert(data.end(), imageData.begin(), imageData.end());

			++shape[0];

			shape[1] = width;
			shape[2] = height;
			std::cout << line << std::endl;
		}

		if (shape[0] * shape[1] * shape[2] * shape[3] != data.size()) {
			std::cout << "All images must have the same shape" << std::endl;
			return {{}, {}};
		} else {
			return {shape, data};
		}

	} else {
		std::cout << "Could not open " << imagesListFilename << std::endl;
		return {{}, {}};
	}


}

