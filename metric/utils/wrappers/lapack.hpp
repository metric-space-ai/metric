#ifndef PANDA_METRIC_LAPACK_HPP
#define PANDA_METRIC_LAPACK_HPP

#include <blaze/Math.h>

extern "C" {
extern void dsygv_(int *itype, char *jobz, char *uplo, int *n, double *A, int *LDA, double *B, int *LDB, double *W,
				   double *WORK, int *LWORK, int *INFO);
}

namespace metric {

inline void dsygv(int itype, char jobz, char uplo, int n, double *A, int lda, double *B, int ldb, double *W,
				  double *work, int lwork, int& info)
{
	dsygv_(&itype, &jobz, &uplo, &n, A, &lda, B, &ldb, W, work, &lwork, &info);
}

/**
 * Blaze wrapper for dsygv()
 * @tparam MT
 * @tparam VT
 * @param A symmetric matrix A
 * @param B symmetric matrix B
 * @param w eigenvalues in ascending order
 * @return true if info == 0
 */
template <typename MT, typename VT>
bool sygv(blaze::DynamicMatrix<MT, blaze::rowMajor> A, blaze::DynamicMatrix<MT, blaze::rowMajor> B,
		  blaze::DynamicVector<VT, blaze::columnVector> &w)
{
	w.resize(A.rows());
	int lwork = 3 * A.rows() - 1;
	std::vector<double> work(lwork);
	int info = -1;

	dsygv(1, 'N', 'U', A.rows(), A.data(), A.spacing(), B.data(), B.spacing(), w.data(), work.data(), lwork, info);

	if (info == 0) {
		return true;
	} else {
		return false;
	}
}
} // namespace metric

#endif // PANDA_METRIC_LAPACK_HPP
