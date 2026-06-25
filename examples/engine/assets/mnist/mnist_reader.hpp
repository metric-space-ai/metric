// Minimal MNIST IDX reader for engine examples.
// Original reader: Baptiste Wicht, MIT License.

#ifndef METRIC_ENGINE_MNIST_READER_HPP
#define METRIC_ENGINE_MNIST_READER_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "mnist_reader_common.hpp"

namespace mnist {

template <template <typename...> class Container = std::vector, typename Image, typename Functor>
auto read_mnist_image_file(Container<Image> &images, const std::string &path, std::size_t limit, Functor make_image)
	-> void
{
	auto buffer = read_mnist_file(path, 0x803);
	if (!buffer) {
		return;
	}

	auto count = read_header(buffer, 1);
	const auto rows = read_header(buffer, 2);
	const auto columns = read_header(buffer, 3);
	auto image_buffer = reinterpret_cast<unsigned char *>(buffer.get() + 16);

	if (limit > 0 && count > limit) {
		count = static_cast<unsigned int>(limit);
	}

	images.reserve(count);
	for (std::size_t image_index = 0; image_index < count; ++image_index) {
		images.push_back(make_image());
		for (std::size_t pixel_index = 0; pixel_index < rows * columns; ++pixel_index) {
			const auto pixel = *image_buffer++;
			images[image_index][pixel_index] = static_cast<typename Image::value_type>(pixel);
		}
	}
}

template <template <typename...> class Container = std::vector, typename Label = std::uint8_t>
auto read_mnist_label_file(Container<Label> &labels, const std::string &path, std::size_t limit = 0) -> void
{
	auto buffer = read_mnist_file(path, 0x801);
	if (!buffer) {
		return;
	}

	auto count = read_header(buffer, 1);
	auto label_buffer = reinterpret_cast<unsigned char *>(buffer.get() + 8);
	if (limit > 0 && count > limit) {
		count = static_cast<unsigned int>(limit);
	}

	labels.resize(count);
	for (std::size_t label_index = 0; label_index < count; ++label_index) {
		labels[label_index] = static_cast<Label>(*label_buffer++);
	}
}

template <template <typename...> class Container = std::vector, typename Image, typename Functor>
auto read_calibration_images(const std::string &folder, std::size_t limit, Functor make_image) -> Container<Image>
{
	Container<Image> images;
	read_mnist_image_file<Container, Image>(images, folder + "/train-images-idx3-ubyte", limit, make_image);
	return images;
}

template <template <typename...> class Container = std::vector, typename Label = std::uint8_t>
auto read_calibration_labels(const std::string &folder, std::size_t limit) -> Container<Label>
{
	Container<Label> labels;
	read_mnist_label_file<Container, Label>(labels, folder + "/train-labels-idx1-ubyte", limit);
	return labels;
}

} // namespace mnist

#endif
