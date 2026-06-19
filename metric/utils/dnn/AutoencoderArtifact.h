#ifndef AUTOENCODER_ARTIFACT_H_
#define AUTOENCODER_ARTIFACT_H_

#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <nlohmann/json.hpp>

#include "AutoencoderModel.h"
#include "Loss.h"
#include "Network.h"

namespace metric::dnn {

template <class Scalar> struct NativeAutoencoderArtifact {
	nlohmann::json manifest;
	std::string network_cereal;
	nlohmann::json diagnostics;
};

template <class Scalar> auto scalar_type_name() -> std::string
{
	if constexpr (std::is_same<Scalar, float>::value) {
		return "float";
	} else if constexpr (std::is_same<Scalar, double>::value) {
		return "double";
	} else if constexpr (std::is_same<Scalar, long double>::value) {
		return "long_double";
	} else {
		return "custom";
	}
}

template <class Scalar, class Codec>
auto make_native_autoencoder_artifact(const AutoencoderModel<Scalar> &model, const Codec &codec,
									  const CompositeLoss<Scalar> &objective,
									  const TrainingSpec<Scalar> &training_spec,
									  std::size_t source_record_count,
									  std::uint64_t source_space_version = 0)
	-> NativeAutoencoderArtifact<Scalar>
{
	std::stringstream network_stream;
	model.network().save(network_stream);

	NativeAutoencoderArtifact<Scalar> artifact;
	artifact.network_cereal = network_stream.str();
	artifact.diagnostics = training_report_to_json(model.training_report());
	artifact.manifest = {
		{"format", "metric.native_autoencoder_artifact"},
		{"format_version", 1},
		{"backend", model.backend_name()},
		{"scalar_type", scalar_type_name<Scalar>()},
		{"network", {{"serialization", "cereal_binary"}, {"byte_count", artifact.network_cereal.size()}}},
		{"network_json", model.network().toJson()},
		{"autoencoder_topology", autoencoder_topology_to_json(model.topology())},
		{"codec", codec.to_json()},
		{"loss", objective.to_json()},
		{"training_spec", training_spec_to_json(training_spec)},
		{"diagnostics", {{"serialization", "json"}, {"epoch_count", model.training_report().epochs.size()}}},
		{"source",
		 {{"record_count", source_record_count},
		  {"feature_count", codec.feature_count()},
		  {"space_version", source_space_version}}}};
	return artifact;
}

template <class Scalar>
auto load_native_autoencoder_model(const NativeAutoencoderArtifact<Scalar> &artifact) -> AutoencoderModel<Scalar>
{
	if (artifact.manifest.at("format").template get<std::string>() != "metric.native_autoencoder_artifact") {
		throw std::invalid_argument("unexpected native autoencoder artifact format");
	}
	if (artifact.manifest.at("format_version").template get<int>() != 1) {
		throw std::invalid_argument("unsupported native autoencoder artifact version");
	}
	if (artifact.manifest.at("backend").template get<std::string>() != "native_dnn") {
		throw std::invalid_argument("unsupported native autoencoder artifact backend");
	}
	if (artifact.network_cereal.empty()) {
		throw std::invalid_argument("native autoencoder artifact is missing network bytes");
	}

	std::stringstream network_stream(artifact.network_cereal);
	Network<Scalar> network;
	network.load(network_stream);
	return AutoencoderModel<Scalar>(std::move(network),
									autoencoder_topology_from_json(artifact.manifest.at("autoencoder_topology")));
}

} // namespace metric::dnn

#endif /* AUTOENCODER_ARTIFACT_H_ */
