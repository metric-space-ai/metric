// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_CORE_ERRORS_HPP
#define _METRIC_CORE_ERRORS_HPP

#include <stdexcept>

namespace mtrc::core {

class MetricError : public std::invalid_argument {
  public:
	using std::invalid_argument::invalid_argument;
};

class MetricInputError : public MetricError {
  public:
	using MetricError::MetricError;
};

class UnsupportedOperationError : public MetricError {
  public:
	using MetricError::MetricError;
};

class StrategyUnavailableError : public UnsupportedOperationError {
  public:
	using UnsupportedOperationError::UnsupportedOperationError;
};

class InvalidRuntimePolicyError : public StrategyUnavailableError {
  public:
	using StrategyUnavailableError::StrategyUnavailableError;
};

class RepresentationError : public MetricError {
  public:
	using MetricError::MetricError;
};

class StaleRepresentationError : public RepresentationError {
  public:
	using RepresentationError::RepresentationError;
};

class PipelineValidationError : public MetricError {
  public:
	using MetricError::MetricError;
};

} // namespace mtrc::core

namespace mtrc {
using core::InvalidRuntimePolicyError;
using core::MetricError;
using core::MetricInputError;
using core::PipelineValidationError;
using core::RepresentationError;
using core::StaleRepresentationError;
using core::StrategyUnavailableError;
using core::UnsupportedOperationError;
} // namespace mtrc

#endif
