// Minimal MNIST IDX reader helpers for engine examples.
// Original reader: Baptiste Wicht, MIT License.

#ifndef METRIC_ENGINE_MNIST_READER_COMMON_HPP
#define METRIC_ENGINE_MNIST_READER_COMMON_HPP

#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace mnist {

inline auto read_header(const std::unique_ptr<char[]> &buffer, std::size_t position) -> std::uint32_t
{
	auto header = reinterpret_cast<std::uint32_t *>(buffer.get());
	auto value = *(header + position);
	return (value << 24) | ((value << 8) & 0x00FF0000) | ((value >> 8) & 0X0000FF00) | (value >> 24);
}

inline auto read_mnist_file(const std::string &path, std::uint32_t key) -> std::unique_ptr<char[]>
{
	std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
	if (!file) {
		std::cout << "Error opening file: " << path << std::endl;
		return {};
	}

	const auto size = file.tellg();
	std::unique_ptr<char[]> buffer(new char[static_cast<std::size_t>(size)]);
	file.seekg(0, std::ios::beg);
	file.read(buffer.get(), size);

	const auto magic = read_header(buffer, 0);
	if (magic != key) {
		std::cout << "Invalid magic number, probably not a MNIST file" << std::endl;
		return {};
	}

	const auto count = read_header(buffer, 1);
	if (magic == 0x803) {
		const auto rows = read_header(buffer, 2);
		const auto columns = read_header(buffer, 3);
		if (size < static_cast<std::streamoff>(count * rows * columns + 16)) {
			std::cout << "The file is not large enough to hold all MNIST images" << std::endl;
			return {};
		}
	} else if (magic == 0x801 && size < static_cast<std::streamoff>(count + 8)) {
		std::cout << "The file is not large enough to hold all MNIST labels" << std::endl;
		return {};
	}

	return buffer;
}

} // namespace mnist

#endif
