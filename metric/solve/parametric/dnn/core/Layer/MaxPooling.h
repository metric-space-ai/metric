#ifndef LAYER_MAXPOOLING_H_
#define LAYER_MAXPOOLING_H_

#include "../Layer.h"

namespace mtrc::solve::parametric::dnn {

namespace internal {
// Dependent always-false predicate: keeps the static_assert below from firing when the header is
// merely included (e.g. from dnn-includes.h); it only triggers on an actual instantiation.
template <typename> inline constexpr bool maxpooling_unsupported = false;
} // namespace internal

///
/// \ingroup Layers
///
/// MaxPooling is intentionally NOT provided by the native solver.
///
/// Two independent reasons:
///   1. Layout: the original pooling kernel walked the raw matrix buffer assuming a column-major,
///      single-image (channels x height x width) memory layout. This engine standardised on a
///      row-major (observations x features) convention (see FullyConnected and Conv2d, which lay
///      each observation out as one row, channel-major and row-major within a channel). Under that
///      convention the old kernel pooled across observation boundaries and produced incorrect
///      results, and the class never even overrode Layer's pure-virtual init(map)/getOutputShape,
///      so it could not be instantiated at all.
///   2. Scope: spatial pooling serves no Metric/Space/Stats/Modify objective. The native DNN exists
///      to optimise metric-space objectives (autoencoder embeddings); none require pooling. Shipping
///      a parallel ML pooling stack (with its own serialization and JSON round-trip) would expand the
///      solver surface for no consumer.
///
/// Rather than ship a silently-broken layer or a fake, any instantiation is a hard compile error.
/// For spatial downsampling, use a strided Conv2d (e.g. Conv2d(W, H, C, C, k, k, /*stride=*/2)).
///
template <typename Scalar, typename Activation> class MaxPooling : public Layer<Scalar> {
	static_assert(internal::maxpooling_unsupported<Scalar>,
				  "MaxPooling is not supported by the native mtrc::solve::parametric::dnn solver. Its "
				  "pooling kernel assumes a column-major single-image memory layout that is incompatible "
				  "with this engine's row-major (observations x features) convention, and spatial pooling "
				  "serves no metric-space objective. Use a strided Conv2d for spatial downsampling.");
};

} // namespace mtrc::solve::parametric::dnn

#endif /* LAYER_MAXPOOLING_H_ */
