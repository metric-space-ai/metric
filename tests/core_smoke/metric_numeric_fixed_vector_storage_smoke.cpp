#include <cassert>
#include <cmath>
#include <memory>

#include <metric/numeric/Math.h>
#include <metric/numeric/util/policies/Deallocate.h>

#include "numeric_test_helpers.hpp"

int main()
{
	using mtrc::test::close_to;

	mtrc::numeric::StaticVector<double, 2> image_shape{5.0, 7.0};
	const auto half_shape = (image_shape - 1.0) / 2.0;
	assert(half_shape.size() == 2);
	assert(close_to(half_shape[0], 2.0));
	assert(close_to(half_shape[1], 3.0));
	assert(close_to(mtrc::numeric::prod(image_shape), 35.0));
	assert(!mtrc::numeric::isDefault(image_shape));

	mtrc::numeric::HybridVector<double, 4> hybrid_values{1.0, -2.0, 3.0};
	assert(hybrid_values.size() == 3);
	assert(close_to(mtrc::numeric::sum(hybrid_values), 2.0));
	assert(close_to(mtrc::numeric::l2Norm(hybrid_values), std::sqrt(14.0)));
	hybrid_values.resize(4);
	hybrid_values[3] = 4.0;
	assert(close_to(mtrc::numeric::dot(hybrid_values, hybrid_values), 30.0));

	mtrc::numeric::DynamicMatrix<double, mtrc::numeric::columnMajor> generated_matrix =
		mtrc::numeric::generate<mtrc::numeric::columnMajor>(
			2UL, 3UL, [](std::size_t row, std::size_t column) { return 10.0 * row + column; });
	assert(generated_matrix.rows() == 2);
	assert(generated_matrix.columns() == 3);
	assert(close_to(generated_matrix(1, 2), 12.0));

	mtrc::numeric::DynamicVector<double, mtrc::numeric::rowVector> generated_row =
		mtrc::numeric::generate<mtrc::numeric::rowVector>(4UL, [](std::size_t index) { return 0.5 * index; });
	assert(generated_row.size() == 4);
	assert(close_to(generated_row[3], 1.5));

	mtrc::numeric::DynamicMatrix<double, mtrc::numeric::columnMajor> empty_matrix;
	assert(mtrc::numeric::isDefault(empty_matrix));

	mtrc::numeric::DynamicMatrix<double, mtrc::numeric::columnMajor> zero_matrix =
		mtrc::numeric::zero<double, mtrc::numeric::columnMajor>(2UL, 3UL);
	assert(zero_matrix.rows() == 2);
	assert(zero_matrix.columns() == 3);
	assert(close_to(mtrc::numeric::sum(zero_matrix), 0.0));
	zero_matrix(1, 2) = 4.0;
	assert(close_to(mtrc::numeric::sum(zero_matrix), 4.0));

	mtrc::numeric::DynamicVector<double, mtrc::numeric::rowVector> empty_row;
	assert(mtrc::numeric::isDefault(empty_row));

	mtrc::numeric::DynamicVector<double, mtrc::numeric::rowVector> zero_row =
		mtrc::numeric::zero<double, mtrc::numeric::rowVector>(4UL);
	assert(zero_row.size() == 4);
	assert(close_to(mtrc::numeric::sum(zero_row), 0.0));
	zero_row = generated_row;
	assert(close_to(mtrc::numeric::sum(zero_row), 3.0));

	using AlignedVector = mtrc::numeric::CustomVector<double, mtrc::numeric::aligned, mtrc::numeric::unpadded,
														mtrc::numeric::columnVector>;
	std::unique_ptr<double[], mtrc::numeric::Deallocate> parameter_memory(mtrc::numeric::allocate<double>(4UL));
	std::unique_ptr<double[], mtrc::numeric::Deallocate> gradient_memory(mtrc::numeric::allocate<double>(4UL));
	AlignedVector parameters(parameter_memory.get(), 4UL);
	AlignedVector gradient(gradient_memory.get(), 4UL);

	for (std::size_t index = 0; index < 4UL; ++index) {
		parameters[index] = 10.0 + index;
		gradient[index] = static_cast<double>(index + 1UL);
	}

	parameters -= 0.1 * gradient / mtrc::numeric::sqrt(mtrc::numeric::pow(gradient, 2.0) + 1.0e-6);

	assert(parameters.size() == 4);
	assert(!mtrc::numeric::isDefault(parameters));
	for (std::size_t index = 0; index < 4UL; ++index) {
		const double expected =
			10.0 + index - 0.1 * gradient[index] / std::sqrt(gradient[index] * gradient[index] + 1.0e-6);
		assert(close_to(parameter_memory[index], expected));
	}

	return 0;
}
