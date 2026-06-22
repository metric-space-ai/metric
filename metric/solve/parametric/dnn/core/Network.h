#ifndef NETWORK_H_
#define NETWORK_H_

#include <chrono>
#include <fstream>
#include <istream>
#include <map>
#include <memory>
#include <ostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <metric/numeric/Math.h>

#include "Callback.h"
#include "Layer.h"
#include "Output.h"
#include "Utils/Random.h"

#include "Initializer/NormalInitializer.h"
#include "Initializer/ZeroInitializer.h"

namespace mtrc::solve::parametric::dnn {

///
/// \defgroup Network Neural Network Model
///

///
/// \ingroup Network
///
/// This class represents a neural network model that typically consists of a
/// number of hidden layers and an output layer. It provides functions for
/// network building, model fitting, and prediction, etc.
///
template <typename Scalar> class Network {
  public:
	using Matrix = mtrc::numeric::DynamicMatrix<Scalar>;
	using parameter_table_type = std::map<std::size_t, std::vector<std::vector<Scalar>>>;

  private:
	std::mt19937 randomEngine; // Reference to the std::mt19937 provided by the user,

	std::shared_ptr<Output<Scalar>> outputLayer; // The output layer
	std::shared_ptr<Callback<Scalar>> callback;	 // Points to user-provided callback function,

	std::shared_ptr<Optimizer<Scalar>> opt;

	std::map<std::string, std::shared_ptr<Initializer<Scalar>>> initializers;

	/* Check dimensions of layers */
	void check_unit_sizes() const
	{
		const int nlayer = num_layers();

		if (nlayer <= 1) {
			return;
		}

		for (int i = 1; i < nlayer; i++) {
			if (layers[i]->getInputSize() != layers[i - 1]->getOutputSize()) {
				throw std::invalid_argument("Unit sizes do not match");
			}
		}
	}

	// Let each layer compute its output
	void forward(const Matrix &input)
	{
		const int nlayer = num_layers();

		if (nlayer <= 0) {
			return;
		}

		// First layer
		if (input.columns() != layers[0]->getInputSize()) {
			throw std::invalid_argument("Input data have incorrect dimension");
		}

		layers[0]->forward(input);

		// The following layers
		for (int i = 1; i < nlayer; i++) {
			layers[i]->forward(layers[i - 1]->output());
		}
	}

	void validate_layer_gradient_shape(const Matrix &gradient, size_t layer_index, size_t row_count) const
	{
		if (mtrc::numeric::size(gradient) == 0) {
			return;
		}
		if (gradient.rows() != row_count || gradient.columns() != layers[layer_index]->getOutputSize()) {
			throw std::invalid_argument("Layer gradient shape does not match layer output");
		}
	}

	// Let each layer compute its gradients of the parameters
	// target has two versions: Matrix and RowVectorXi
	// The RowVectorXi version is used in classification problems where each
	// element is a class label
	template <typename TargetType> void backprop(const Matrix &input, const TargetType &target)
	{
		const int nlayer = num_layers();

		if (nlayer <= 0) {
			return;
		}

		auto firstLayer = layers[0];
		auto lastLayer = layers[nlayer - 1];

		// Let output layer compute back-propagation data
		outputLayer->check_target_data(target);
		outputLayer->evaluate(lastLayer->output(), target);

		// If there is only one hidden layer, "prev_layer_data" will be the input data
		if (nlayer == 1) {
			firstLayer->backprop(input, outputLayer->backprop_data());
			return;
		}

		// Compute gradients for the last hidden layer
		lastLayer->backprop(layers[nlayer - 2]->output(), outputLayer->backprop_data());

		// Compute gradients for all the hidden layers except for the first one and the last one
		for (int i = nlayer - 2; i > 0; i--) {
			layers[i]->backprop(layers[i - 1]->output(), layers[i + 1]->backprop_data());
		}

		// Compute gradients for the first layer
		firstLayer->backprop(input, layers[1]->backprop_data());
	}

	// Update parameters
	void update()
	{
		const int nlayer = num_layers();

		if (nlayer <= 0) {
			return;
		}

		for (int i = 0; i < nlayer; i++) {
			layers[i]->update(*opt);
		}
	}

	auto layer_parameters() const -> parameter_table_type
	{
		parameter_table_type layers_parameters;
		for (std::size_t i = 0; i < layers.size(); ++i) {
			layers_parameters[i] = layers[i]->getParameters();
		}
		return layers_parameters;
	}

	auto apply_layer_parameters(const parameter_table_type &layers_parameters) -> void
	{
		for (const auto &entry : layers_parameters) {
			if (entry.first >= layers.size()) {
				throw std::invalid_argument("network artifact layer index exceeds network layer count");
			}
			layers[entry.first]->setParameters(entry.second);
		}
	}

	static auto write_size(std::ostream &stream, std::size_t value) -> void
	{
		stream.write(reinterpret_cast<const char *>(&value), sizeof(value));
		if (!stream) {
			throw std::runtime_error("failed to write native network artifact");
		}
	}

	static auto read_size(std::istream &stream) -> std::size_t
	{
		std::size_t value{};
		stream.read(reinterpret_cast<char *>(&value), sizeof(value));
		if (!stream) {
			throw std::runtime_error("failed to read native network artifact");
		}
		return value;
	}

	static auto write_string(std::ostream &stream, const std::string &value) -> void
	{
		write_size(stream, value.size());
		stream.write(value.data(), static_cast<std::streamsize>(value.size()));
		if (!stream) {
			throw std::runtime_error("failed to write native network artifact string");
		}
	}

	static auto read_string(std::istream &stream) -> std::string
	{
		const auto size = read_size(stream);
		std::string value(size, '\0');
		stream.read(value.data(), static_cast<std::streamsize>(size));
		if (!stream) {
			throw std::runtime_error("failed to read native network artifact string");
		}
		return value;
	}

	static auto write_scalar(std::ostream &stream, Scalar value) -> void
	{
		stream.write(reinterpret_cast<const char *>(&value), sizeof(value));
		if (!stream) {
			throw std::runtime_error("failed to write native network artifact scalar");
		}
	}

	static auto read_scalar(std::istream &stream) -> Scalar
	{
		Scalar value{};
		stream.read(reinterpret_cast<char *>(&value), sizeof(value));
		if (!stream) {
			throw std::runtime_error("failed to read native network artifact scalar");
		}
		return value;
	}

	static auto write_parameters(std::ostream &stream, const parameter_table_type &layers_parameters) -> void
	{
		write_size(stream, layers_parameters.size());
		for (const auto &layer : layers_parameters) {
			write_size(stream, layer.first);
			write_size(stream, layer.second.size());
			for (const auto &parameter_group : layer.second) {
				write_size(stream, parameter_group.size());
				for (const auto &value : parameter_group) {
					write_scalar(stream, value);
				}
			}
		}
	}

	static auto read_parameters(std::istream &stream) -> parameter_table_type
	{
		parameter_table_type layers_parameters;
		const auto layer_count = read_size(stream);
		for (std::size_t layer = 0; layer < layer_count; ++layer) {
			const auto layer_index = read_size(stream);
			const auto group_count = read_size(stream);
			std::vector<std::vector<Scalar>> groups;
			groups.reserve(group_count);
			for (std::size_t group = 0; group < group_count; ++group) {
				const auto value_count = read_size(stream);
				std::vector<Scalar> values;
				values.reserve(value_count);
				for (std::size_t value = 0; value < value_count; ++value) {
					values.push_back(read_scalar(stream));
				}
				groups.push_back(std::move(values));
			}
			layers_parameters.emplace(layer_index, std::move(groups));
		}
		return layers_parameters;
	}

	auto save_native(std::ostream &stream) const -> void
	{
		write_string(stream, toJson().dump());
		write_parameters(stream, layer_parameters());
	}

	auto load_native(std::istream &stream) -> void
	{
		const auto metadata = read_string(stream);
		auto layers_parameters = read_parameters(stream);
		constructFromJsonString(metadata);
		apply_layer_parameters(layers_parameters);
	}

  public:
	/* Layers pointers */
	std::vector<std::shared_ptr<Layer<Scalar>>> layers;

	/* Default constructor that creates an empty neural network */
	Network() : randomEngine{std::random_device()()}, outputLayer(NULL)
	{
		setDefaultCallback();

		setInitializer("normal", NormalInitializer<Scalar>(0, 0.01, randomEngine));
		setInitializer("zero", ZeroInitializer<Scalar>());
	}

	Network(const std::string &jsonString) : Network() { constructFromJsonString(jsonString); }

	void forward_all(const Matrix &input, std::vector<Matrix> *activations)
	{
		forward(input);

		if (activations == nullptr) {
			return;
		}

		activations->clear();
		activations->reserve(layers.size());
		for (const auto &layer : layers) {
			activations->push_back(layer->output());
		}
	}

	void backprop_from_layer_gradients(const Matrix &input, const std::vector<Matrix> &output_gradients_by_layer)
	{
		const size_t nlayer = layers.size();
		if (nlayer == 0) {
			return;
		}
		if (output_gradients_by_layer.size() != nlayer) {
			throw std::invalid_argument("Expected one output-gradient slot per layer");
		}

		std::vector<Matrix> activations;
		forward_all(input, &activations);

		Matrix upstream_gradient;
		for (size_t reverse_index = nlayer; reverse_index > 0; --reverse_index) {
			const size_t layer_index = reverse_index - 1;
			const auto row_count = input.rows();
			Matrix layer_gradient(row_count, layers[layer_index]->getOutputSize());
			layer_gradient = Scalar(0);

			const auto &local_gradient = output_gradients_by_layer[layer_index];
			validate_layer_gradient_shape(local_gradient, layer_index, row_count);
			if (mtrc::numeric::size(local_gradient) != 0) {
				layer_gradient += local_gradient;
			}
			validate_layer_gradient_shape(upstream_gradient, layer_index, row_count);
			if (mtrc::numeric::size(upstream_gradient) != 0) {
				layer_gradient += upstream_gradient;
			}

			const auto &previous_output = layer_index == 0 ? input : activations[layer_index - 1];
			layers[layer_index]->backprop(previous_output, layer_gradient);
			upstream_gradient = layers[layer_index]->backprop_data();
		}
	}

	void apply_optimizer()
	{
		if (!opt) {
			throw std::invalid_argument("Optimizer is not set");
		}
		update();
	}

	void reset_optimizer()
	{
		if (!opt) {
			throw std::invalid_argument("Optimizer is not set");
		}
		opt->reset();
	}

	void constructFromJsonString(const std::string &jsonString)
	{
		/* Parse json */
		auto json = mtrc::core::Metadata::parse(jsonString);
		if (!json.is_object() || !json.contains("train")) {
			throw std::invalid_argument("network JSON must be an object containing a 'train' section");
		}

		/* Construct layers */
		layers.clear();
		// The mandatory 'train' entry guarantees size() >= 1, so the layer count
		// cannot underflow on empty/malformed JSON (json.size() is unsigned).
		const std::size_t layer_count = json.size() - 1;
		for (std::size_t i = 0; i < layer_count; ++i) {
			auto layerJson = json[std::to_string(i)];

			auto activation = layerJson["activation"].get<std::string>();

			auto type = layerJson["type"].get<std::string>();
			// Unknown layer types / activations must fail loudly: silently skipping a layer here
			// produces a network with fewer layers than the JSON declared, which then mismatches
			// shapes (or corrupts results) far from the real cause.
			if (type == "FullyConnected") {
				if (activation == "Identity") {
					addLayer(FullyConnected<Scalar, Identity<Scalar>>(layerJson));
				} else if (activation == "ReLU") {
					addLayer(FullyConnected<Scalar, ReLU<Scalar>>(layerJson));
				} else if (activation == "Sigmoid") {
					addLayer(FullyConnected<Scalar, Sigmoid<Scalar>>(layerJson));
				} else {
					throw std::invalid_argument("unsupported activation '" + activation +
												"' for layer type 'FullyConnected'");
				}
			} else if (type == "Conv2d") {
				if (activation == "Identity") {
					addLayer(Conv2d<Scalar, Identity<Scalar>>(layerJson));
				} else if (activation == "ReLU") {
					addLayer(Conv2d<Scalar, ReLU<Scalar>>(layerJson));
				} else if (activation == "Sigmoid") {
					addLayer(Conv2d<Scalar, Sigmoid<Scalar>>(layerJson));
				} else {
					throw std::invalid_argument("unsupported activation '" + activation +
												"' for layer type 'Conv2d'");
				}

			} else if (type == "Conv2dTranspose") {
				if (activation == "Identity") {
					addLayer(Conv2dTranspose<Scalar, Identity<Scalar>>(layerJson));
				} else if (activation == "ReLU") {
					addLayer(Conv2dTranspose<Scalar, ReLU<Scalar>>(layerJson));
				} else if (activation == "Sigmoid") {
					addLayer(Conv2dTranspose<Scalar, Sigmoid<Scalar>>(layerJson));
				} else {
					throw std::invalid_argument("unsupported activation '" + activation +
												"' for layer type 'Conv2dTranspose'");
				}
			} else {
				throw std::invalid_argument("unknown layer type '" + type + "'");
			}
		}

		/* Create train part */
		auto trainJson = json["train"];

		/* Loss */
		auto loss = trainJson["loss"].get<std::string>();
		if (loss == "RegressionMSE") {
			setOutput(RegressionMSE<Scalar>());
		}

		/* Optimizer */
		auto optimizerJson = trainJson["optimizer"];
		auto optimizerType = optimizerJson["type"].get<std::string>();
		auto learningRate = optimizerJson["learningRate"].get<Scalar>();
		auto eps = optimizerJson["eps"].get<Scalar>();
		auto decay = optimizerJson["decay"].get<Scalar>();
		if (optimizerType == "RMSProp") {
			RMSProp<Scalar> optimizer;
			optimizer.learningRate = learningRate;
			optimizer.m_eps = eps;
			optimizer.m_decay = decay;
			setOptimizer(optimizer);
		}

		/* Init layers */
		init();
	}

	~Network() = default;

	mtrc::core::Metadata toJson() const
	{
		/* Layers */
		mtrc::core::Metadata json;
		for (auto i = 0; i < layers.size(); ++i) {
			json[std::to_string(i)] = layers[i]->toJson();
		}

		/* Loss */
		json["train"]["loss"] = outputLayer->getType();

		/* Optimizer */
		json["train"]["optimizer"] = opt->toJson();

		return json;
	}

	void save(const std::string filename) const
	{
		std::ofstream file(filename);
		save_native(file);
	}

	void save(std::stringstream &ss) const
	{
		save_native(ss);
	}

	void load(const std::string filepath)
	{
		std::ifstream file(filepath);
		load_native(file);
	}

	void load(std::stringstream &ss)
	{
		load_native(ss);
	}

	///
	/// Add a hidden layer to the neural network
	///
	/// \param layer A pointer to a Layer object, typically constructed from
	///              layer classes such as FullyConnected and Convolutional.
	///              **NOTE**: the pointer will be handled and freed by the
	///              network object, so do not delete it manually.
	///
	template <typename T> void addLayer(const T &layer) { layers.push_back(std::make_shared<T>(layer)); }

	///
	/// Set the output layer of the neural network
	///
	/// \param output A pointer to an Output object, typically constructed from
	///               output layer classes such as RegressionMSE and MultiClassEntropy.
	///               **NOTE**: the pointer will be handled and freed by the
	///               network object, so do not delete it manually.
	///
	template <typename T> void setOutput(const T &output) { outputLayer = std::make_shared<T>(output); }

	template <typename T> void setOptimizer(const T &optimizer) { opt = std::make_shared<T>(optimizer); }

	///
	/// Number of hidden layers in the network
	///
	int num_layers() const { return layers.size(); }

	///
	/// Get the list of hidden layers of the network
	///
	/*std::vector<const std::shared_ptr<Layer>> get_layers() const
	{
		const int nlayer = num_layers();
		std::vector<const std::shared_ptr<Layer>> layers(nlayer);
		std::copy(m_layers.begin(), m_layers.end(), layers.begin());
		return layers;
	}*/

	///
	/// Get the output layer
	///
	const Output<Scalar> *get_output() const { return outputLayer.get(); }

	///
	/// Set the callback function that can be called during model fitting
	///
	/// \param callback A user-provided callback function object that inherits
	///                 from the default Callback class.
	///
	template <typename T> void setCallback(const T &_callback) { callback = std::make_shared<T>(_callback); }
	///
	/// Set the default silent callback function
	///
	void setDefaultCallback() { setCallback(Callback<Scalar>()); }

	void setRandomEngineSeed(const unsigned int seed) { randomEngine.seed(seed); }

	template <typename T> void setInitializer(const std::string name, const T &initializer)
	{
		initializers[name] = std::make_shared<T>(initializer);
	}

	///
	/// Initialize layer parameters in the network using normal distribution
	///
	/// \param mu    Mean of the normal distribution.
	/// \param sigma Standard deviation of the normal distribution.
	/// \param seed  Set the random seed of the %std::mt19937 if `seed > 0`, otherwise
	///              use the current random state.
	///
	void init(const Scalar &mu = Scalar(0), const Scalar &sigma = Scalar(0.01), int seed = -1)
	{
		check_unit_sizes();

		if (seed > 0) {
			setRandomEngineSeed(seed);
		}

		// Rebuild the "normal" initializer so the requested mu/sigma AND the (re)seeded engine
		// actually take effect. NormalInitializer keeps its OWN copy of the engine, so without this
		// rebuild both the seed and mu/sigma passed here were silently ignored — the construction-time
		// defaults (0, 0.01, and the original random_device state) were used instead, making seeded
		// initialization non-reproducible.
		setInitializer("normal", NormalInitializer<Scalar>(mu, sigma, randomEngine));

		for (auto layer : layers) {
			layer->init(this->initializers);
		}
	}

	// NOTE: dead get_parameters()/set_parameters() members were removed here. They called
	// layers[i]->get_parameters()/set_parameters(), which the Layer interface does not define
	// (it exposes getParameters()/setParameters()), so they were non-compiling if ever
	// instantiated. The live serialization path is layer_parameters()/apply_layer_parameters()
	// (save_native/load_native), which use the correct getParameters()/setParameters().

	///
	/// Get the serialized derivatives of layer parameters
	///
	std::vector<std::vector<Scalar>> get_derivatives() const
	{
		const int nlayer = num_layers();
		std::vector<std::vector<Scalar>> res;
		res.reserve(nlayer);

		for (int i = 0; i < nlayer; i++) {
			res.push_back(layers[i]->get_derivatives());
		}

		return res;
	}

	// NOTE: a commented-out check_gradient() debugging helper lived here; it depended on the removed
	// get_parameters()/set_parameters() and on layer methods that no longer exist. Gradient
	// correctness is now covered by tests/core_smoke/native_dnn_loss_gradient_check_smoke.

	///
	/// Fit the model based on the given data
	///
	/// \param x          The predictors. Each row is an observation.
	/// \param y          The response variable. Each row is an observation.
	/// \param batch_size Mini-batch size.
	/// \param epoch      Number of epochs of training.
	/// \param seed       Set the random seed of the %std::mt19937 if `seed > 0`, otherwise
	///                   use the current random state.
	///
	template <typename DerivedX, typename DerivedY>
	bool fit(const DerivedX &x, const DerivedY &y, int batch_size, int epoch, int seed = -1)
	{
		// We do not directly use PlainObjectX since it may be row-majored if x is passed as mat.transpose()
		// We want to force XType and YType to be row-majored
		const int nlayer = num_layers();

		if (nlayer <= 0) {
			return false;
		}

		// Reset optimizer
		opt->reset();

		// Create shuffled mini-batches
		if (seed > 0) {
			randomEngine.seed(seed);
		}

		std::vector<DerivedX> x_batches;
		std::vector<DerivedY> y_batches;
		const int nbatch = internal::create_shuffled_batches(x, y, batch_size, randomEngine, x_batches, y_batches);
		// Set up callback parameters
		callback->batchesNumber = nbatch;
		callback->epochsNumber = epoch;

		// Iterations on the whole data set
		for (int k = 0; k < epoch; k++) {
			callback->epochId = k;

			callback->preTrainingEpoch(this);

			// Train on each mini-batch
			for (int i = 0; i < nbatch; i++) {
				callback->batchId = i;

				callback->preTrainingBatch(this, x_batches[i], y_batches[i]);

				this->forward(x_batches[i]);
				this->backprop(x_batches[i], y_batches[i]);
				this->update();

				callback->postTrainingBatch(this, x_batches[i], y_batches[i]);
			}
			callback->postTrainingEpoch(this);
		}

		return true;
	}

	///
	/// Use the fitted model to make predictions
	///
	/// \param x The predictors. Each row is an observation.
	///
	Matrix predict(const Matrix &x)
	{
		const int nlayer = num_layers();

		if (nlayer <= 0) {
			return Matrix();
		}

		this->forward(x);
		return layers[nlayer - 1]->output();
	}
};

} // namespace mtrc::solve::parametric::dnn

#endif /* NETWORK_H_ */
