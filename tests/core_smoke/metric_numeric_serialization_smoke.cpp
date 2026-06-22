#include <cassert>
#include <cmath>
#include <sstream>

#include <metric/numeric.hpp>

#include "numeric_test_helpers.hpp"

int main()
{
	using mtrc::test::close_to;

	mtrc::numeric::DynamicMatrix<double> input_weights(2, 3);
	input_weights(0, 0) = 1.0;
	input_weights(0, 1) = -2.0;
	input_weights(0, 2) = 3.0;
	input_weights(1, 0) = 4.0;
	input_weights(1, 1) = 5.0;
	input_weights(1, 2) = -6.0;

	mtrc::numeric::CompressedMatrix<double> reservoir(3, 3);
	reservoir.reserve(4);
	reservoir.set(0, 1, 0.25);
	reservoir.set(1, 0, -0.5);
	reservoir.set(1, 2, 1.5);
	reservoir.set(2, 2, 2.0);

	mtrc::numeric::DynamicMatrix<double> output_weights(3, 2);
	output_weights(0, 0) = 0.1;
	output_weights(0, 1) = 0.2;
	output_weights(1, 0) = -0.3;
	output_weights(1, 1) = 0.4;
	output_weights(2, 0) = 0.5;
	output_weights(2, 1) = -0.6;

	mtrc::numeric::DynamicVector<double> params{0.7, 1.1, 4.0};

	std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
	{
		mtrc::numeric::Archive<std::stringstream> writer(stream);
		writer << input_weights << reservoir << output_weights << params;
		assert(writer);
	}

	stream.clear();
	stream.seekg(0);

	mtrc::numeric::DynamicMatrix<double> loaded_input_weights;
	mtrc::numeric::CompressedMatrix<double> loaded_reservoir;
	mtrc::numeric::DynamicMatrix<double> loaded_output_weights;
	mtrc::numeric::DynamicVector<double> loaded_params;
	{
		mtrc::numeric::Archive<std::stringstream> reader(stream);
		reader >> loaded_input_weights >> loaded_reservoir >> loaded_output_weights >> loaded_params;
		assert(reader);
	}

	assert(loaded_input_weights.rows() == input_weights.rows());
	assert(loaded_input_weights.columns() == input_weights.columns());
	for (std::size_t row = 0; row < input_weights.rows(); ++row) {
		for (std::size_t column = 0; column < input_weights.columns(); ++column) {
			assert(close_to(loaded_input_weights(row, column), input_weights(row, column)));
		}
	}

	assert(loaded_reservoir.rows() == reservoir.rows());
	assert(loaded_reservoir.columns() == reservoir.columns());
	assert(loaded_reservoir.nonZeros() == reservoir.nonZeros());
	for (std::size_t row = 0; row < reservoir.rows(); ++row) {
		for (std::size_t column = 0; column < reservoir.columns(); ++column) {
			assert(close_to(loaded_reservoir(row, column), reservoir(row, column)));
		}
	}

	assert(loaded_output_weights.rows() == output_weights.rows());
	assert(loaded_output_weights.columns() == output_weights.columns());
	for (std::size_t row = 0; row < output_weights.rows(); ++row) {
		for (std::size_t column = 0; column < output_weights.columns(); ++column) {
			assert(close_to(loaded_output_weights(row, column), output_weights(row, column)));
		}
	}

	assert(loaded_params.size() == params.size());
	for (std::size_t index = 0; index < params.size(); ++index) {
		assert(close_to(loaded_params[index], params[index]));
	}

	mtrc::numeric::DynamicMatrix<double> dense_from_sparse;
	stream.clear();
	stream.seekp(0);
	stream.str(std::string{});
	{
		mtrc::numeric::Archive<std::stringstream> writer(stream);
		writer << reservoir;
		assert(writer);
	}
	stream.clear();
	stream.seekg(0);
	{
		mtrc::numeric::Archive<std::stringstream> reader(stream);
		reader >> dense_from_sparse;
		assert(reader);
	}
	assert(dense_from_sparse.rows() == reservoir.rows());
	assert(dense_from_sparse.columns() == reservoir.columns());
	assert(close_to(dense_from_sparse(1, 2), 1.5));
	assert(close_to(dense_from_sparse(0, 0), 0.0));

	return 0;
}
