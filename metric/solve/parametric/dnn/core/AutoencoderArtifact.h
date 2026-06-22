#ifndef AUTOENCODER_ARTIFACT_H_
#define AUTOENCODER_ARTIFACT_H_

#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <metric/core/metadata.hpp>

#include "AutoencoderModel.h"
#include "Loss.h"
#include "Network.h"

namespace mtrc::solve::parametric::dnn {

template <class Scalar> struct NativeAutoencoderArtifact {
	mtrc::core::Metadata manifest;
	std::string network_binary;
	mtrc::core::Metadata diagnostics;
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
									  const CompositeLoss<Scalar> &objective, const TrainingSpec<Scalar> &training_spec,
									  std::size_t source_record_count, std::uint64_t source_space_version = 0,
									  std::string artifact_format = "metric.native_autoencoder_artifact",
									  mtrc::core::Metadata additional_manifest = mtrc::core::Metadata::object())
	-> NativeAutoencoderArtifact<Scalar>
{
	std::stringstream network_stream;
	model.network().save(network_stream);

	NativeAutoencoderArtifact<Scalar> artifact;
	artifact.network_binary = network_stream.str();
	artifact.diagnostics = training_report_to_json(model.training_report());
	artifact.manifest = {
		{"format", std::move(artifact_format)},
		{"format_version", 1},
		{"backend", model.backend_name()},
		{"scalar_type", scalar_type_name<Scalar>()},
		{"network", {{"serialization", "metric_native_binary"}, {"byte_count", artifact.network_binary.size()}}},
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
	if (!additional_manifest.empty()) {
		artifact.manifest.update(std::move(additional_manifest));
	}
	return artifact;
}

template <class Scalar>
auto load_native_autoencoder_model(const NativeAutoencoderArtifact<Scalar> &artifact) -> AutoencoderModel<Scalar>
{
	const auto format = artifact.manifest.at("format").template get<std::string>();
	if (format != "metric.native_autoencoder_artifact" && format != "metric.native_phate_autoencoder_artifact") {
		throw std::invalid_argument("unexpected native autoencoder artifact format");
	}
	if (artifact.manifest.at("format_version").template get<int>() != 1) {
		throw std::invalid_argument("unsupported native autoencoder artifact version");
	}
	if (artifact.manifest.at("backend").template get<std::string>() != "native_dnn") {
		throw std::invalid_argument("unsupported native autoencoder artifact backend");
	}
	// The network bytes are a raw scalar dump (see Network::read_scalar); loading a payload written
	// with a different scalar width into this Scalar would silently reinterpret the bytes. Reject a
	// mismatch up front instead of corrupting the deserialized weights.
	if (artifact.manifest.at("scalar_type").template get<std::string>() != scalar_type_name<Scalar>()) {
		throw std::invalid_argument("native autoencoder artifact scalar_type does not match the requested model");
	}
	if (artifact.network_binary.empty()) {
		throw std::invalid_argument("native autoencoder artifact is missing network bytes");
	}

	std::stringstream network_stream(artifact.network_binary);
	Network<Scalar> network;
	network.load(network_stream);
	return AutoencoderModel<Scalar>(std::move(network),
									autoencoder_topology_from_json(artifact.manifest.at("autoencoder_topology")));
}

} // namespace mtrc::solve::parametric::dnn

#endif /* AUTOENCODER_ARTIFACT_H_ */
