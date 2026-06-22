#ifndef ZEROINITIALIZER_H
#define ZEROINITIALIZER_H

#include "../Initializer.h"

namespace mtrc::solve::parametric::dnn {
template <typename Scalar> class ZeroInitializer : public Initializer<Scalar> {
  protected:
	using typename Initializer<Scalar>::ColumnMatrix;
	using typename Initializer<Scalar>::RowVector;

  public:
	void init(const size_t rows, const size_t columns, ColumnMatrix &matrix)
	{
		matrix = mtrc::numeric::zero<Scalar, mtrc::numeric::columnMajor>(rows, columns);
	}

	void init(const size_t size, RowVector &vector)
	{
		vector = mtrc::numeric::zero<Scalar, mtrc::numeric::rowVector>(size);
	}
};
} // namespace mtrc::solve::parametric::dnn

#endif // ZEROINITIALIZER_H