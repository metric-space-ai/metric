#ifndef OPTIMIZER_H_
#define OPTIMIZER_H_

#include <metric/numeric/Math.h>

namespace mtrc::solve::parametric::dnn {

///
/// \defgroup Optimizers Optimization Algorithms
///

///
/// \ingroup Optimizers
///
/// The interface of optimization algorithms
///
template <typename Scalar> class Optimizer {
  protected:
	using AlignedMapVec = mtrc::numeric::CustomVector<Scalar, mtrc::numeric::aligned, mtrc::numeric::unpadded>;
	using ConstAlignedMapVec =
		const mtrc::numeric::CustomVector<Scalar, mtrc::numeric::aligned, mtrc::numeric::unpadded>;
	using RowVector = mtrc::numeric::DynamicVector<Scalar, mtrc::numeric::rowVector>;
	using ColumnMatrix = mtrc::numeric::DynamicMatrix<Scalar, mtrc::numeric::columnMajor>;

  public:
	virtual ~Optimizer() {}

	virtual mtrc::core::Metadata toJson() = 0;
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
	virtual void update(ConstAlignedMapVec &dvec, AlignedMapVec &vec) = 0;

	virtual void update(const RowVector &dvec, RowVector &vec) = 0;

	virtual void update(const ColumnMatrix &dvec, ColumnMatrix &vec) = 0;
};

} // namespace mtrc::solve::parametric::dnn

#endif /* OPTIMIZER_H_ */
