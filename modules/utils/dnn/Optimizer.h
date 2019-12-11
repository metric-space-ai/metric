#ifndef OPTIMIZER_H_
#define OPTIMIZER_H_

#include "../../../3rdparty/blaze/Math.h"

namespace MiniDNN
{


///
/// \defgroup Optimizers Optimization Algorithms
///

///
/// \ingroup Optimizers
///
/// The interface of optimization algorithms
///
template<typename Scalar>
class Optimizer
{
	protected:
		using AlignedMapVec = blaze::CustomVector<Scalar, blaze::aligned, blaze::unpadded>;
		using ConstAlignedMapVec = const blaze::CustomVector<Scalar, blaze::aligned, blaze::unpadded>;

	public:
        virtual ~Optimizer() {}

        ///
        /// Reset the optimizer to clear all historical information
        ///
        virtual void reset() {};

        ///
        /// Update the parameter vector using its gradient
        ///
        /// It is assumed that the memory addresses of `dvec` and `vec` do not
        /// change during the training process. This is used to implement optimization
        /// algorithms that have "memories". See the AdaGrad algorithm for an example.
        ///
        /// \param dvec The gradient of the parameter. Read-only
        /// \param vec  On entering, the current parameter vector. On exit, the
        ///             updated parameters.
        ///
        virtual void update(ConstAlignedMapVec& dvec, AlignedMapVec& vec) = 0;
};


} // namespace MiniDNN


#endif /* OPTIMIZER_H_ */
