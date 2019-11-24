#include <iostream>
#include <fstream>

#include <tuple>

#include "../../../../3rdparty/cereal/types/vector.hpp"
#include "../../../../3rdparty/cereal/archives/binary.hpp"


class Datasets {
	public:
		std::tuple<std::vector<uint8_t>, std::vector<uint32_t>, std::vector<uint8_t>> getMnist(const std::string filename);
};


std::tuple<std::vector<uint8_t>, std::vector<uint32_t>, std::vector<uint8_t>> Datasets::getMnist(const std::string filename)
{
	std::ifstream dataFile(filename, std::ifstream::binary);

	if (dataFile) {
		cereal::BinaryInputArchive ia(dataFile);
		std::vector<uint8_t> labels;
		std::vector<uint32_t> shape;
		std::vector<uint8_t> features;

		ia(labels, shape, features);

		return {labels, shape, features};
	} else {
		return {{}, {}, {}};
	}

}

