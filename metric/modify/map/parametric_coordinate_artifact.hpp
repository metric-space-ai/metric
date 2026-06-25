// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MODIFY_MAP_PARAMETRIC_COORDINATE_ARTIFACT_HPP
#define _METRIC_MODIFY_MAP_PARAMETRIC_COORDINATE_ARTIFACT_HPP

#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/numeric/Math.h>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/record/id.hpp>
#include <metric/core/result.hpp>
#include <metric/metric/catalog/vector/Standards.hpp>
#include <metric/modify/compose/pipeline.hpp>
#include <metric/solve/parametric/dnn/coordinate_solver.hpp>

namespace mtrc::modify::map {

template <typename Record, typename Scalar, typename Codec = solve::parametric::CoordinateRecordCodec<Record, Scalar>>
class ParametricCoordinateMapArtifact {
  public:
	using record_type = Record;
	using scalar_type = Scalar;
	using latent_record_type = std::vector<scalar_type>;
	using latent_metric_type = Euclidean<scalar_type>;
	using space_type = MetricSpace<latent_record_type, latent_metric_type>;
	using result_type = MappingResult<space_type>;
	using codec_type = Codec;
	using matrix_type = mtrc::numeric::DynamicMatrix<scalar_type>;

	ParametricCoordinateMapArtifact(solve::parametric::CoordinateSolver<scalar_type> coordinate_solver,
									codec_type codec, std::size_t source_record_count,
									std::string mapping_name = "parametric_coordinates",
									std::string strategy_name = "native_coordinate_solver",
									modify::compose::PipelinePlan pipeline_plan = modify::compose::PipelinePlan{})
		: coordinate_solver_(std::move(coordinate_solver)), codec_(std::move(codec)), source_record_count_(source_record_count),
		  mapping_name_(std::move(mapping_name)), strategy_name_(std::move(strategy_name)),
		  pipeline_plan_(std::move(pipeline_plan))
	{
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto transform(const Space &space) const -> result_type
	{
		static_assert(std::is_same<typename Space::record_type, record_type>::value,
					  "ParametricCoordinateMapArtifact can only transform spaces with the derived record type");
		if (space.empty()) {
			throw std::invalid_argument("cannot transform an empty space with a parametric coordinate artifact");
		}

		const auto coordinates = codec_.encode_batch(space.records());
		const auto derived_coordinates = coordinate_solver_.encode(coordinates);
		auto derived_records = numeric::matrix_to_row_vectors(derived_coordinates);
		space_type derived_space(std::move(derived_records), latent_metric_type{});
		auto lineage = mtrc::one_to_one_lineage(space);

		// The derived Euclidean coordinate space is a true metric on the calibrated
		// coordinates. The coordinates approximate, but do not reproduce, the source
		// geometry, so callers must not treat the derived metric as isometric to the
		// source metric.
		return core::make_mapping_result(
			std::move(derived_space), std::move(lineage.source_records),
			std::move(lineage.representative_records), space.size(), codec_.inverse_supported(), mapping_name_,
			strategy_name_, "metric_space", core::metric_traits<latent_metric_type>::law, true,
			"parametric coordinates; derived Euclidean coordinate space approximating source geometry; "
			"valid for records matching the calibrated coordinate input shape");
	}

	auto inverse_transform(const space_type &latent_space) const -> std::vector<record_type>
	{
		return inverse_transform(latent_space.records());
	}

	auto inverse_transform(const result_type &result) const -> std::vector<record_type>
	{
		return inverse_transform(result.space);
	}

	auto inverse_transform(const std::vector<latent_record_type> &latent_records) const -> std::vector<record_type>
	{
		const auto derived_coordinates = numeric::row_vectors_to_matrix<scalar_type>(latent_records);
		const auto source_coordinates = coordinate_solver_.decode(derived_coordinates);
		return codec_.decode_batch(source_coordinates);
	}

	auto calibration_report() const -> const solve::parametric::CoordinateCalibrationReport<scalar_type> &
	{
		return solve::parametric::coordinate_calibration_report(coordinate_solver_);
	}
	auto source_record_count() const -> std::size_t { return source_record_count_; }
	auto latent_dimension() const -> std::size_t { return coordinate_solver_.latent_dimension(); }
	auto coordinate_solver() const -> const solve::parametric::CoordinateSolver<scalar_type> & { return coordinate_solver_; }
	auto codec() const -> const codec_type & { return codec_; }
	auto mapping_name() const -> const std::string & { return mapping_name_; }
	auto strategy_name() const -> const std::string & { return strategy_name_; }
	auto has_pipeline_plan() const -> bool { return !pipeline_plan_.name().empty(); }
	auto pipeline_plan() const -> const modify::compose::PipelinePlan & { return pipeline_plan_; }

  private:
	mutable solve::parametric::CoordinateSolver<scalar_type> coordinate_solver_;
	codec_type codec_;
	std::size_t source_record_count_{};
	std::string mapping_name_;
	std::string strategy_name_;
	modify::compose::PipelinePlan pipeline_plan_;
};

} // namespace mtrc::modify::map

#endif
