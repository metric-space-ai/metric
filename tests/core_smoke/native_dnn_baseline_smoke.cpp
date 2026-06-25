#include <cassert>
#include <cmath>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <metric/numeric/Math.h>

#include "metric/solve/parametric/dnn.hpp"

namespace {

using Matrix = mtrc::numeric::DynamicMatrix<double>;
using RowVector = mtrc::numeric::DynamicVector<double, mtrc::numeric::rowVector>;

auto close(double lhs, double rhs, double tolerance = 1.0e-9) -> bool { return std::abs(lhs - rhs) <= tolerance; }

auto assert_same_shape(const Matrix &matrix, std::size_t rows, std::size_t columns) -> void
{
	assert(matrix.rows() == rows);
	assert(matrix.columns() == columns);
}

template <typename Callable> auto assert_invalid_argument(Callable &&call) -> void
{
	bool rejected = false;
	try {
		call();
	} catch (const std::invalid_argument &) {
		rejected = true;
	}
	assert(rejected);
}

} // namespace

int main()
{
	static_assert(std::is_base_of<mtrc::solve::parametric::dnn::Optimizer<double>, mtrc::solve::parametric::dnn::RMSProp<double>>::value);

	{
		mtrc::solve::parametric::dnn::RegressionMSE<double> mse;
		const Matrix predicted{{1.0, 2.0}, {3.0, 4.0}};
		const Matrix target{{1.0, 1.0}, {5.0, 4.0}};

		mse.evaluate(predicted, target);
		assert_same_shape(mse.backprop_data(), 2, 2);
		assert(close(mse.backprop_data()(0, 0), 0.0));
		assert(close(mse.backprop_data()(0, 1), 2.0));
		assert(close(mse.backprop_data()(1, 0), -4.0));
		assert(close(mse.backprop_data()(1, 1), 0.0));
		assert(close(mse.loss(), 2.5));
	}

	{
		mtrc::solve::parametric::dnn::RMSProp<double> optimizer(0.0001, 1.0e-8, 0.99);
		RowVector weights{-0.021, 1.03, -0.05, -0.749, 0.009};
		const RowVector gradients{1.0, -3.24, -0.60, 2.79, 1.82};

		optimizer.update(gradients, weights);
		assert(close(weights[0], -0.0220, 1.0e-4));
		assert(close(weights[1], 1.0310, 1.0e-4));
		assert(close(weights[2], -0.0490, 1.0e-4));
		assert(close(weights[3], -0.7500, 1.0e-4));
		assert(close(weights[4], 0.0080, 1.0e-4));

		optimizer.update(gradients, weights);
		assert(close(weights[0], -0.0227, 1.0e-4));
		assert(close(weights[1], 1.0317, 1.0e-4));
		assert(close(weights[2], -0.0483, 1.0e-4));
		assert(close(weights[3], -0.7507, 1.0e-4));
		assert(close(weights[4], 0.0073, 1.0e-4));
	}

	{
		mtrc::solve::parametric::dnn::Network<double> network;
		network.addLayer(mtrc::solve::parametric::dnn::FullyConnected<double, mtrc::solve::parametric::dnn::Identity<double>>(2, 2));
		network.setOutput(mtrc::solve::parametric::dnn::RegressionMSE<double>());
		network.setOptimizer(mtrc::solve::parametric::dnn::RMSProp<double>(0.001, 1.0e-8, 0.99));
		network.init(0.0, 0.01, 7);

		const Matrix samples{{0.0, 0.0}, {1.0, 1.0}, {2.0, 2.0}};
		const auto before = network.predict(samples);
		assert_same_shape(before, 3, 2);

		const auto fit_result = network.fit(samples, samples, 2, 2, 11);
		assert(fit_result);
		const auto after = network.predict(samples);
		assert_same_shape(after, 3, 2);
	}

	{
		const Matrix samples{{0.0, 10.0}, {1.0, 11.0}, {2.0, 12.0}, {3.0, 13.0}};
		const Matrix targets{{100.0}, {101.0}, {102.0}, {103.0}};
		std::mt19937 rng(17);
		std::vector<Matrix> sample_batches;
		std::vector<Matrix> target_batches;

		const int batch_count =
			mtrc::solve::parametric::dnn::internal::create_shuffled_batches(samples, targets, 2, rng, sample_batches, target_batches);
		assert(batch_count == 2);
		assert(sample_batches.size() == 2);
		assert(target_batches.size() == 2);

		std::vector<bool> seen(samples.rows(), false);
		for (std::size_t batch = 0; batch < sample_batches.size(); ++batch) {
			assert_same_shape(sample_batches[batch], 2, 2);
			assert_same_shape(target_batches[batch], 2, 1);
			for (std::size_t row = 0; row < sample_batches[batch].rows(); ++row) {
				const auto source_index = static_cast<std::size_t>(sample_batches[batch](row, 0));
				assert(source_index < seen.size());
				assert(!seen[source_index]);
				seen[source_index] = true;
				assert(close(sample_batches[batch](row, 1), static_cast<double>(source_index + 10)));
				assert(close(target_batches[batch](row, 0), static_cast<double>(source_index + 100)));
			}
		}
		for (const bool visited : seen) {
			assert(visited);
		}

		const Matrix empty_samples(0, 2);
		const Matrix empty_targets(0, 1);
		const int empty_batch_count = mtrc::solve::parametric::dnn::internal::create_shuffled_batches(
			empty_samples, empty_targets, 2, rng, sample_batches, target_batches);
		assert(empty_batch_count == 0);
		assert(sample_batches.empty());
		assert(target_batches.empty());

		assert_invalid_argument([&samples, &targets, &rng, &sample_batches, &target_batches]() {
			(void)mtrc::solve::parametric::dnn::internal::create_shuffled_batches(samples, targets, 0, rng, sample_batches,
																 target_batches);
		});
	}

	return 0;
}
