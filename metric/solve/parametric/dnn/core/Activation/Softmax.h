#ifndef ACTIVATION_SOFTMAX_H_
#define ACTIVATION_SOFTMAX_H_

namespace mtrc::solve::parametric::dnn {

namespace internal {
// Dependent always-false predicate: the static_assert below only fires on an actual instantiation,
// not on mere inclusion.
template <typename> inline constexpr bool softmax_unsupported = false;
} // namespace internal

///
/// \ingroup Activations
///
/// Softmax is intentionally NOT supported by the native solver.
///
/// Its apply_jacobian was written against an Eigen-style API (cwiseProduct / colwise / rowwise) that
/// this numeric backend does not provide, so the activation never compiled; it is correspondingly
/// excluded from dnn-includes.h. Softmax is a classification activation and serves no
/// Metric/Space/Stats/Modify objective, so rather than port a dead classification path we block it.
/// Any instantiation is a hard compile error. For a probability-like final layer use Sigmoid.
///
template <typename Scalar> class Softmax {
	static_assert(internal::softmax_unsupported<Scalar>,
				  "Softmax is not supported by the native mtrc::solve::parametric::dnn solver: its Jacobian "
				  "was written against an Eigen-style API this numeric backend does not provide, and softmax "
				  "serves no metric-space objective. It is excluded from dnn-includes.h.");
};

} // namespace mtrc::solve::parametric::dnn

#endif /* ACTIVATION_SOFTMAX_H_ */
