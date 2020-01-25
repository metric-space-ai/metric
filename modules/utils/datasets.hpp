#include <iostream>
#include <fstream>

#include <tuple>
#include <regex>

#include "3rdparty/cereal/types/vector.hpp"
#include "3rdparty/cereal/archives/binary.hpp"

#include "3rdparty/blaze/Math.h"

namespace metric {
	class Datasets
	{
	public:
		std::tuple<std::vector<uint8_t>, std::vector<uint32_t>, std::vector<uint8_t>>
		getMnist(const std::string filename);

		template<typename T>
		static blaze::DynamicMatrix<T> readDenseMatrixFromFile(const std::string filepath);

	private:
		static std::regex getSeparator(std::string string);
		template<typename T>
		static blaze::DynamicVector<T, blaze::rowVector> getRowFromStrings(std::vector<std::string> stringElements);
	};


	std::tuple<std::vector<uint8_t>, std::vector<uint32_t>, std::vector<uint8_t>>
	Datasets::getMnist(const std::string filename)
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
			std::cout << "Could not open " << filename << std::endl;
			return {{},
			        {},
			        {}};
		}

	}

	std::regex Datasets::getSeparator(std::string string)
	{
		std::regex delimiter;
		std::regex decimal;

		std::regex r("\\s*;\\s*");
		if (std::regex_search(string, r)) {
			return r;
		}

		if (std::regex_search(string, std::regex(R"(\s+,\s+)"))) {
			return std::regex(R"(\s*,\s*)");
		} else {
			return std::regex(R"(\s+)");
		}
	}

	template<typename T>
	blaze::DynamicMatrix<T> Datasets::readDenseMatrixFromFile(const std::string filepath)
	{
		/* Open file */
		std::ifstream file(filepath);

		std::string line;
		std::vector<blaze::DynamicVector<T, blaze::rowVector>> rows;
		while (std::getline(file, line)) {
			auto delimiter = getSeparator(line);

			std::sregex_token_iterator first{line.begin(), line.end(), delimiter, -1};
			std::vector<std::string> row{first, {}};

			rows.push_back(getRowFromStrings<T>(row));
		}

		/* Construct matrix */
		blaze::DynamicMatrix<T> matrix(rows.size(), rows[0].size());
		for (size_t i = 0; i < matrix.rows(); ++i) {
			blaze::row(matrix, i) = rows[i];
		}

		//if (delimiter == std::regex("")) {
		//	std::cout << "nothing" << std::endl;
		//}

		return matrix;
	}

	template<typename T>
	blaze::DynamicVector<T, blaze::rowVector> Datasets::getRowFromStrings(std::vector<std::string> stringElements)
	{
		blaze::DynamicVector<T, blaze::rowVector> row(stringElements.size());
		for (size_t i = 0; i < row.size(); ++i) {
			row[i] = std::stod(stringElements[i]);
		}
		return row;
	}
}
