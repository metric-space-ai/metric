#ifndef ZEROINITIALIZER_H
#define ZEROINITIALIZER_H

#include "../Initializer.h"


namespace metric::dnn {
	template<typename Scalar>
	class ZeroInitializer : public Initializer<Scalar>
	{
	protected:
		using typename Initializer<Scalar>::ColumnMatrix;
		using typename Initializer<Scalar>::RowVector;

	public:
		void init(const size_t rows, const size_t columns, ColumnMatrix &matrix) const
		{
			matrix = blaze::zero<Scalar>(rows, columns);
		}

		void init(const size_t size, RowVector &vector) const
		{
			vector = blaze::zero<Scalar, blaze::rowVector>(size);
		}

	};
}

#endif // ZEROINITIALIZER_H