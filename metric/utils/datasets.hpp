#include <fstream>
#include <iostream>

#include <regex>
#include <tuple>

#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>

#include <blaze/Math.h>

namespace metric {

}
class Datasets {
  public:
	std::tuple<std::vector<uint8_t>, std::vector<uint32_t>, std::vector<uint8_t>> getMnist(const std::string filename);

	template <typename T> static blaze::DynamicMatrix<T> readDenseMatrixFromFile(const std::string filepath);

	static std::tuple<std::vector<uint32_t>, std::vector<uint8_t>> loadImages(const std::string imagesListFilename);

  private:
	static std::regex getDelimiterAndSetDecimal(std::string &string);

	template <typename T>
	static blaze::DynamicVector<T, blaze::rowVector> getRowFromStrings(const std::vector<std::string> &stringElements);
};

std::tuple<std::vector<uint8_t>, std::vector<uint32_t>, std::vector<uint8_t>> inline Datasets::getMnist(
	const std::string filename)
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
		return {{}, {}, {}};
	}
}

inline std::regex Datasets::getDelimiterAndSetDecimal(std::string &string)
{
	std::regex delimiter;
	std::regex decimal;

	bool dotFound = (string.find('.') != std::string::npos);
	bool commaFound = (string.find(',') != std::string::npos);

	std::regex r("\\s*;\\s*");
	if (std::regex_search(string, r)) {
		if (commaFound) {
			string = std::regex_replace(string, std::regex(","), ".");
		}
		return r;
	}

	if (commaFound) {
		if (dotFound) {
			return std::regex(R"(\s*,\s*)");
		} else {
			if (std::regex_search(string, std::regex(R"(\s+)"))) {
				if (std::regex_search(string, std::regex(R"(\s+,\s+|,\S+,)"))) {
					return std::regex(R"(\s*,\s*)");
				} else {
					string = std::regex_replace(string, std::regex(","), ".");
					return std::regex(R"(\s+)");
				}
			} else {
				return std::regex(R"(\s*,\s*)");
			}
		}
	} else {
		return std::regex(R"(\s+)");
	}
}

template <typename T> blaze::DynamicMatrix<T> Datasets::readDenseMatrixFromFile(const std::string filepath)
{
	/* Open file */
	std::ifstream file(filepath);

	std::string line;

	/* Count rows */
	auto rowsNumber = std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), '\n');
	file.seekg(0);

	/* Reserve rows */
	std::vector<blaze::DynamicVector<T, blaze::rowVector>> rows;
	rows.reserve(rowsNumber);

	while (std::getline(file, line)) {
		/* Trim */
		line = std::regex_replace(line, std::regex(R"(^\s+|\s+$)"), "");
		line = std::regex_replace(line, std::regex(R"(^\[|\]$)"), "");

		/* Get delimiter and replace decimal char to dot (if needs) */
		auto delimiter = getDelimiterAndSetDecimal(line);

		/* Split */
		std::sregex_token_iterator first{line.begin(), line.end(), delimiter, -1};
		std::vector<std::string> row{first, {}};

		rows.push_back(getRowFromStrings<T>(row));
	}

	/* Construct matrix */
	blaze::DynamicMatrix<T> matrix(rows.size(), rows[0].size());
	for (size_t i = 0; i < matrix.rows(); ++i) {
		blaze::row(matrix, i) = rows[i];
	}

	return matrix;
}

template <typename T>
blaze::DynamicVector<T, blaze::rowVector> Datasets::getRowFromStrings(const std::vector<std::string> &stringElements)
{
	blaze::DynamicVector<T, blaze::rowVector> row(stringElements.size());
	for (size_t i = 0; i < row.size(); ++i) {
		row[i] = std::stod(stringElements[i]);
	}
	return row;
}

inline std::tuple<std::vector<uint32_t>, std::vector<uint8_t>>
Datasets::loadImages(const std::string imagesListFilename)
{
	/* Open imagesList file */
	std::ifstream imagesListFile(imagesListFilename);
	if (imagesListFile.is_open()) {

		/* Init shape */
		std::vector<uint32_t> shape = {0, 0, 0, 1};

		std::vector<uint8_t> data;

		std::string line;
		while (getline(imagesListFile, line)) {
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
			imageFile.read(reinterpret_cast<char *>(imageData.data()), dataSize);

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
