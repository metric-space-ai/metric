// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_DMATDVECMULT_THRESHOLD
#define METRIC_NUMERIC_DMATDVECMULT_THRESHOLD 4000000UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Column-major dense matrix/dense vector multiplication threshold.
// \ingroup config
//
// This setting specifies the threshold between the application of the custom Metric numeric kernels
// and the BLAS kernels for the column-major dense matrix/dense vector multiplication. In case
// the number of elements in the dense matrix is equal or higher than this value, the BLAS
// kernels are preferred over the custom Metric numeric kernels. In case the number of elements in the
// dense matrix is smaller, the Metric numeric kernels are used.
//
// The default setting for this threshold is 62500 (which for instance corresponds to a matrix
// size of \f$ 250 \times 250 \f$). Note that in case the Metric numeric debug mode is active, this
// threshold will be replaced by the mtrc::numeric::TDMATDVECMULT_DEBUG_THRESHOLD value.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_TDMATDVECMULT_THRESHOLD=62500 ...
   \endcode

   \code
   #define METRIC_NUMERIC_TDMATDVECMULT_THRESHOLD 62500UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_TDMATDVECMULT_THRESHOLD
#define METRIC_NUMERIC_TDMATDVECMULT_THRESHOLD 62500UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Dense Vector/row-major dense matrix multiplication threshold.
// \ingroup config
//
// This setting specifies the threshold between the application of the custom Metric numeric kernels
// and the BLAS kernels for the dense vector/row-major dense matrix multiplication. In case
// the number of elements in the dense matrix is equal or higher than this value, the BLAS
// kernels are preferred over the custom Metric numeric kernels. In case the number of elements in the
// dense matrix is smaller, the Metric numeric kernels are used.
//
// The default setting for this threshold is 62500 (which for instance corresponds to a matrix
// size of \f$ 250 \times 250 \f$). Note that in case the Metric numeric debug mode is active, this
// threshold will be replaced by the mtrc::numeric::TDVECDMATMULT_DEBUG_THRESHOLD value.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_TDVECDMATMULT_THRESHOLD=62500 ...
   \endcode

   \code
   #define METRIC_NUMERIC_TDVECDMATMULT_THRESHOLD 62500UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_TDVECDMATMULT_THRESHOLD
#define METRIC_NUMERIC_TDVECDMATMULT_THRESHOLD 62500UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Dense Vector/column-major dense matrix multiplication threshold.
// \ingroup config
//
// This setting specifies the threshold between the application of the custom Metric numeric kernels
// and the BLAS kernels for the dense vector/column-major dense matrix multiplication. In case
// the number of elements in the dense matrix is equal or higher than this value, the BLAS
// kernels are preferred over the custom Metric numeric kernels. In case the number of elements in the
// dense matrix is smaller, the Metric numeric kernels are used.
//
// The default setting for this threshold is 4000000 (which for instance corresponds to a matrix
// size of \f$ 2000 \times 2000 \f$). Note that in case the Metric numeric debug mode is active, this
// threshold will be replaced by the mtrc::numeric::TDVECTDMATMULT_DEBUG_THRESHOLD value.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_TDVECTDMATMULT_THRESHOLD=4000000 ...
   \endcode

   \code
   #define METRIC_NUMERIC_TDVECTDMATMULT_THRESHOLD 4000000UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_TDVECTDMATMULT_THRESHOLD
#define METRIC_NUMERIC_TDVECTDMATMULT_THRESHOLD 4000000UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Row-major dense matrix/row-major dense matrix multiplication threshold.
// \ingroup config
//
// This setting specifies the threshold between the application of the custom Metric numeric kernels and
// the BLAS kernels for the row-major dense matrix/row-major dense matrix multiplication. In
// case the number of elements of the target matrix is equal or higher than this value, the
// BLAS kernels are preferred over the custom Metric numeric kernels. In case the number of elements in
// the target matrix is smaller, the Metric numeric kernels are used.
//
// The default setting for this threshold is 4900 (which for instance corresponds to a matrix
// size of \f$ 70 \times 70 \f$). Note that in case the Metric numeric debug mode is active, this
// threshold will be replaced by the mtrc::numeric::DMATDMATMULT_DEBUG_THRESHOLD value.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_DMATDMATMULT_THRESHOLD=4900 ...
   \endcode

   \code
   #define METRIC_NUMERIC_DMATDMATMULT_THRESHOLD 4900UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_DMATDMATMULT_THRESHOLD
#define METRIC_NUMERIC_DMATDMATMULT_THRESHOLD 4900UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Row-major dense matrix/column-major dense matrix multiplication threshold.
// \ingroup config
//
// This setting specifies the threshold between the application of the custom Metric numeric kernels and
// the BLAS kernels for the row-major dense matrix/column-major dense matrix multiplication. In
// case the number of elements of the target matrix is equal or higher than this value, the
// BLAS kernels are preferred over the custom Metric numeric kernels. In case the number of elements in
// the target matrix is smaller, the Metric numeric kernels are used.
//
// The default setting for this threshold is 4900 (which for instance corresponds to a matrix
// size of \f$ 70 \times 70 \f$). Note that in case the Metric numeric debug mode is active, this
// threshold will be replaced by the mtrc::numeric::DMATTDMATMULT_DEBUG_THRESHOLD value.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_DMATTDMATMULT_THRESHOLD=4900 ...
   \endcode

   \code
   #define METRIC_NUMERIC_DMATTDMATMULT_THRESHOLD 4900UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_DMATTDMATMULT_THRESHOLD
#define METRIC_NUMERIC_DMATTDMATMULT_THRESHOLD 4900UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Column-major dense matrix/row-major dense matrix multiplication threshold.
// \ingroup config
//
// This setting specifies the threshold between the application of the custom Metric numeric kernels and
// the BLAS kernels for the column-major dense matrix/row-major dense matrix multiplication. In
// case the number of elements of the target matrix is equal or higher than this value, the
// BLAS kernels are preferred over the custom Metric numeric kernels. In case the number of elements in
// the target matrix is smaller, the Metric numeric kernels are used.
//
// The default setting for this threshold is 4900 (which for instance corresponds to a matrix
// size of \f$ 70 \times 70 \f$). Note that in case the Metric numeric debug mode is active, this
// threshold will be replaced by the mtrc::numeric::TDMATDMATMULT_DEBUG_THRESHOLD value.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_TDMATDMATMULT_THRESHOLD=4900 ...
   \endcode

   \code
   #define METRIC_NUMERIC_TDMATDMATMULT_THRESHOLD 4900UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_TDMATDMATMULT_THRESHOLD
#define METRIC_NUMERIC_TDMATDMATMULT_THRESHOLD 4900UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Column-major dense matrix/column-major dense matrix multiplication threshold.
// \ingroup config
//
// This setting specifies the threshold between the application of the custom Metric numeric kernels and
// the BLAS kernels for the column-major dense matrix/column-major dense matrix multiplication.
// In case the number of elements of the target matrix is equal or higher than this value, the
// BLAS kernels are preferred over the custom Metric numeric kernels. In case the number of elements in
// the target matrix is smaller, the Metric numeric kernels are used.
//
// The default setting for this threshold is 4900 (which for instance corresponds to a matrix
// size of \f$ 70 \times 70 \f$). Note that in case the Metric numeric debug mode is active, this
// threshold will be replaced by the mtrc::numeric::TDMATTDMATMULT_DEBUG_THRESHOLD value.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_TDMATTDMATMULT_THRESHOLD=4900 ...
   \endcode

   \code
   #define METRIC_NUMERIC_TDMATTDMATMULT_THRESHOLD 4900UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_TDMATTDMATMULT_THRESHOLD
#define METRIC_NUMERIC_TDMATTDMATMULT_THRESHOLD 4900UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Row-major dense matrix/row-major sparse matrix multiplication threshold.
// \ingroup config
//
// This setting specifies the threshold between the application of the Metric numeric kernels for small
// and for large row-major dense matrix/row-major sparse matrix multiplications. In case the
// number of elements of the target matrix is equal or higher than this value, the kernel for
// large matrices is preferred over the kernel for small matrices. In case the number of elements
// in the target matrix is smaller, the kernel for small matrices is used.
//
// The default setting for this threshold is 2500 (which for instance corresponds to a matrix
// size of \f$ 50 \times 50 \f$). Note that in case the Metric numeric debug mode is active, this
// threshold will be replaced by the mtrc::numeric::DMATSMATMULT_DEBUG_THRESHOLD value.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_DMATSMATMULT_THRESHOLD=2500 ...
   \endcode

   \code
   #define METRIC_NUMERIC_DMATSMATMULT_THRESHOLD 2500UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_DMATSMATMULT_THRESHOLD
#define METRIC_NUMERIC_DMATSMATMULT_THRESHOLD 2500UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Column-major dense matrix/row-major sparse matrix multiplication threshold.
// \ingroup config
//
// This setting specifies the threshold between the application of the Metric numeric kernels for small
// and for large column-major dense matrix/row-major sparse matrix multiplications. In case the
// number of elements of the target matrix is equal or higher than this value, the kernel for
// large matrices is preferred over the kernel for small matrices. In case the number of elements
// in the target matrix is smaller, the kernel for small matrices is used.
//
// The default setting for this threshold is 2500 (which for instance corresponds to a matrix
// size of \f$ 50 \times 50 \f$). Note that in case the Metric numeric debug mode is active, this
// threshold will be replaced by the mtrc::numeric::TDMATSMATMULT_DEBUG_THRESHOLD value.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_TDMATSMATMULT_THRESHOLD=2500 ...
   \endcode

   \code
   #define METRIC_NUMERIC_TDMATSMATMULT_THRESHOLD 2500UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_TDMATSMATMULT_THRESHOLD
#define METRIC_NUMERIC_TDMATSMATMULT_THRESHOLD 2500UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Column-major sparse matrix/row-major dense matrix multiplication threshold.
// \ingroup config
//
// This setting specifies the threshold between the application of the Metric numeric kernels for small
// and for large column-major sparse matrix/row-major dense matrix multiplications. In case the
// number of elements of the target matrix is equal or higher than this value, the kernel for
// large matrices is preferred over the kernel for small matrices. In case the number of elements
// in the target matrix is smaller, the kernel for small matrices is used.
//
// The default setting for this threshold is 10000 (which for instance corresponds to a matrix
// size of \f$ 100 \times 100 \f$). Note that in case the Metric numeric debug mode is active, this
// threshold will be replaced by the mtrc::numeric::TSMATDMATMULT_DEBUG_THRESHOLD value.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_TSMATDMATMULT_THRESHOLD=10000 ...
   \endcode

   \code
   #define METRIC_NUMERIC_TSMATDMATMULT_THRESHOLD 10000UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_TSMATDMATMULT_THRESHOLD
#define METRIC_NUMERIC_TSMATDMATMULT_THRESHOLD 10000UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Column-major sparse matrix/column-major dense matrix multiplication threshold.
// \ingroup config
//
// This setting specifies the threshold between the application of the Metric numeric kernels for small
// and for large column-major sparse matrix/column-major dense matrix multiplications. In case
// the number of elements of the target matrix is equal or higher than this value, the kernel for
// large matrices is preferred over the kernel for small matrices. In case the number of elements
// in the target matrix is smaller, the kernel for small matrices is used.
//
// The default setting for this threshold is 22500 (which for instance corresponds to a matrix
// size of \f$ 150 \times 150 \f$). Note that in case the Metric numeric debug mode is active, this
// threshold will be replaced by the mtrc::numeric::TSMATTDMATMULT_DEBUG_THRESHOLD value.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_TSMATTDMATMULT_THRESHOLD=22500 ...
   \endcode

   \code
   #define METRIC_NUMERIC_TSMATTDMATMULT_THRESHOLD 22500UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_TSMATTDMATMULT_THRESHOLD
#define METRIC_NUMERIC_TSMATTDMATMULT_THRESHOLD 22500UL
#endif
//*************************************************************************************************

//=================================================================================================
//
//  SMP THRESHOLDS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief SMP dense vector assignment threshold.
// \ingroup config
//
// This threshold specifies when an assignment of a simple dense vector can be executed in
// parallel. In case the number of elements of the target vector is larger or equal to this
// threshold, the operation is executed in parallel. If the number of elements is below this
// threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 38000. In case the threshold is set to 0, the
// operation is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DVECASSIGN_THRESHOLD=38000 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DVECASSIGN_THRESHOLD 38000UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DVECASSIGN_THRESHOLD
#define METRIC_NUMERIC_SMP_DVECASSIGN_THRESHOLD 38000UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/scalar multiplication/division threshold.
// \ingroup config
//
// This threshold specifies when a dense vector/scalar multiplication/division can be executed
// in parallel. In case the number of elements of the target vector is larger or equal to this
// threshold, the operation is executed in parallel. If the number of elements is below this
// threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 51000. In case the threshold is set to 0, the
// operation is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DVECSCALARMULT_THRESHOLD=51000 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DVECSCALARMULT_THRESHOLD 51000UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DVECSCALARMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_DVECSCALARMULT_THRESHOLD 51000UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/dense vector addition threshold.
// \ingroup config
//
// This threshold specifies when a dense vector/dense vector addition can be executed in parallel.
// In case the number of elements of the target vector is larger or equal to this threshold, the
// operation is executed in parallel. If the number of elements is below this threshold the
// operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 38000. In case the threshold is set to 0, the
// operation is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DVECDVECADD_THRESHOLD=38000 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DVECDVECADD_THRESHOLD 38000UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DVECDVECADD_THRESHOLD
#define METRIC_NUMERIC_SMP_DVECDVECADD_THRESHOLD 38000UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/dense vector subtraction threshold.
// \ingroup config
//
// This threshold specifies when a dense vector/dense vector subtraction can be executed in
// parallel. In case the number of elements of the target vector is larger or equal to this
// threshold, the operation is executed in parallel. If the number of elements is below this
// threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 38000. In case the threshold is set to 0, the
// operation is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DVECDVECSUB_THRESHOLD=38000 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DVECDVECSUB_THRESHOLD 38000UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DVECDVECSUB_THRESHOLD
#define METRIC_NUMERIC_SMP_DVECDVECSUB_THRESHOLD 38000UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/dense vector multiplication threshold.
// \ingroup config
//
// This threshold specifies when a dense vector/dense vector multiplication can be executed
// in parallel. In case the number of elements of the target vector is larger or equal to this
// threshold, the operation is executed in parallel. If the number of elements is below this
// threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 38000. In case the threshold is set to 0, the
// operation is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DVECDVECMULT_THRESHOLD=38000 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DVECDVECMULT_THRESHOLD 38000UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DVECDVECMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_DVECDVECMULT_THRESHOLD 38000UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/dense vector division threshold.
// \ingroup config
//
// This threshold specifies when a dense vector/dense vector division can be executed in
// parallel. In case the number of elements of the target vector is larger or equal to this
// threshold, the operation is executed in parallel. If the number of elements is below this
// threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 38000. In case the threshold is set to 0, the
// operation is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DVECDVECDIV_THRESHOLD=38000 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DVECDVECDIV_THRESHOLD 38000UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DVECDVECDIV_THRESHOLD
#define METRIC_NUMERIC_SMP_DVECDVECDIV_THRESHOLD 38000UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/dense vector outer product threshold.
// \ingroup config
//
// This threshold specifies when a dense vector/dense vector outer product can be executed in
// parallel. In case the number of elements of the target matrix is larger or equal to this
// threshold, the operation is executed in parallel. If the number of elements is below this
// threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 84100 (which corresponds to a matrix size of
// \f$ 290 \times 290 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DVECDVECOUTER_THRESHOLD=84100 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DVECDVECOUTER_THRESHOLD 84100UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DVECDVECOUTER_THRESHOLD
#define METRIC_NUMERIC_SMP_DVECDVECOUTER_THRESHOLD 84100UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/dense vector multiplication threshold.
// \ingroup config
//
// This threshold specifies when a row-major dense matrix/dense vector multiplication can be
// executed in parallel. In case the number of elements of the target vector is larger or equal
// to this threshold, the operation is executed in parallel. If the number of elements is below
// this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 330. In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DMATDVECMULT_THRESHOLD=330 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DMATDVECMULT_THRESHOLD 330UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DMATDVECMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_DMATDVECMULT_THRESHOLD 330UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major dense matrix/dense vector multiplication threshold.
// \ingroup config
//
// This threshold specifies when a column-major dense matrix/dense vector multiplication can be
// executed in parallel. In case the number of elements of the target vector is larger or equal
// to this threshold, the operation is executed in parallel. If the number of elements is below
// this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 360. In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TDMATDVECMULT_THRESHOLD=360 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TDMATDVECMULT_THRESHOLD 360UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TDMATDVECMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TDMATDVECMULT_THRESHOLD 360UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/row-major dense matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a dense vector/row-major dense matrix multiplication can be
// executed in parallel. In case the number of elements of the target vector is larger or equal
// to this threshold, the operation is executed in parallel. If the number of elements is below
// this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 370. In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TDVECDMATMULT_THRESHOLD=370 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TDVECDMATMULT_THRESHOLD 370UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TDVECDMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TDVECDMATMULT_THRESHOLD 370UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/column-major dense matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a dense vector/column-major dense matrix multiplication can be
// executed in parallel. In case the number of elements of the target vector is larger or equal
// to this threshold, the operation is executed in parallel. If the number of elements is below
// this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 340. In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TDVECTDMATMULT_THRESHOLD=340 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TDVECTDMATMULT_THRESHOLD 340UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TDVECTDMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TDVECTDMATMULT_THRESHOLD 340UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/sparse vector multiplication threshold.
// \ingroup config
//
// This threshold specifies when a row-major dense matrix/sparse vector multiplication can be
// executed in parallel. In case the number of elements of the target vector is larger or equal
// to this threshold, the operation is executed in parallel. If the number of elements is below
// this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 480. In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DMATSVECMULT_THRESHOLD=480 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DMATSVECMULT_THRESHOLD 480UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DMATSVECMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_DMATSVECMULT_THRESHOLD 480UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major dense matrix/sparse vector multiplication threshold.
// \ingroup config
//
// This threshold specifies when a column-major dense matrix/sparse vector multiplication can be
// executed in parallel. In case the number of elements of the target vector is larger or equal
// to this threshold, the operation is executed in parallel. If the number of elements is below
// this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 910. In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TDMATSVECMULT_THRESHOLD=910 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TDMATSVECMULT_THRESHOLD 910UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TDMATSVECMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TDMATSVECMULT_THRESHOLD 910UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP sparse vector/row-major dense matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a sparse vector/row-major dense matrix multiplication can be
// executed in parallel. In case the number of elements of the target vector is larger or equal
// to this threshold, the operation is executed in parallel. If the number of elements is below
// this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 910. In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TSVECDMATMULT_THRESHOLD=910 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TSVECDMATMULT_THRESHOLD 910UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TSVECDMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TSVECDMATMULT_THRESHOLD 910UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP sparse vector/column-major dense matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a sparse vector/column-major dense matrix multiplication can be
// executed in parallel. In case the number of elements of the target vector is larger or equal
// to this threshold, the operation is executed in parallel. If the number of elements is below
// this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 480. In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TSVECTDMATMULT_THRESHOLD=480 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TSVECTDMATMULT_THRESHOLD 480UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TSVECTDMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TSVECTDMATMULT_THRESHOLD 480UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major sparse matrix/dense vector multiplication threshold.
// \ingroup config
//
// This threshold specifies when a row-major sparse matrix/dense vector multiplication can be
// executed in parallel. In case the number of elements of the target vector is larger or equal
// to this threshold, the operation is executed in parallel. If the number of elements is below
// this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 600. In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_SMATDVECMULT_THRESHOLD=600 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_SMATDVECMULT_THRESHOLD 600UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_SMATDVECMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_SMATDVECMULT_THRESHOLD 600UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major sparse matrix/dense vector multiplication threshold.
// \ingroup config
//
// This threshold specifies when a column-major sparse matrix/dense vector multiplication can be
// executed in parallel. In case the number of elements of the target vector is larger or equal
// to this threshold, the operation is executed in parallel. If the number of elements is below
// this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 1250. In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TSMATDVECMULT_THRESHOLD=1250 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TSMATDVECMULT_THRESHOLD 1250UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TSMATDVECMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TSMATDVECMULT_THRESHOLD 1250UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/row-major sparse matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a dense vector/row-major sparse matrix multiplication can be
// executed in parallel. In case the number of elements of the target vector is larger or equal
// to this threshold, the operation is executed in parallel. If the number of elements is below
// this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 1190. In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TDVECSMATMULT_THRESHOLD=1190 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TDVECSMATMULT_THRESHOLD 1190UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TDVECSMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TDVECSMATMULT_THRESHOLD 1190UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/column-major sparse matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a dense vector/column-major sparse matrix multiplication can be
// executed in parallel. In case the number of elements of the target vector is larger or equal
// to this threshold, the operation is executed in parallel. If the number of elements is below
// this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 530. In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TDVECTSMATMULT_THRESHOLD=530 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TDVECTSMATMULT_THRESHOLD 530UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TDVECTSMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TDVECTSMATMULT_THRESHOLD 530UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major sparse matrix/sparse vector multiplication threshold.
// \ingroup config
//
// This threshold specifies when a row-major sparse matrix/sparse vector multiplication can be
// executed in parallel. In case the number of elements of the target vector is larger or equal
// to this threshold, the operation is executed in parallel. If the number of elements is below
// this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 260. In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_SMATSVECMULT_THRESHOLD=260 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_SMATSVECMULT_THRESHOLD 260UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_SMATSVECMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_SMATSVECMULT_THRESHOLD 260UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major sparse matrix/sparse vector multiplication threshold.
// \ingroup config
//
// This threshold specifies when a column-major sparse matrix/sparse vector multiplication can be
// executed in parallel. In case the number of elements of the target vector is larger or equal
// to this threshold, the operation is executed in parallel. If the number of elements is below
// this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 2160. In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TSMATSVECMULT_THRESHOLD=2160 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TSMATSVECMULT_THRESHOLD 2160UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TSMATSVECMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TSMATSVECMULT_THRESHOLD 2160UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP sparse vector/row-major sparse matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a sparse vector/row-major sparse matrix multiplication can be
// executed in parallel. In case the number of elements of the target vector is larger or equal
// to this threshold, the operation is executed in parallel. If the number of elements is below
// this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 2160. In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TSVECSMATMULT_THRESHOLD=2160 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TSVECSMATMULT_THRESHOLD 2160UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TSVECSMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TSVECSMATMULT_THRESHOLD 2160UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP sparse vector/column-major sparse matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a sparse vector/column-major sparse matrix multiplication can be
// executed in parallel. In case the number of elements of the target vector is larger or equal
// to this threshold, the operation is executed in parallel. If the number of elements is below
// this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 260. In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TSVECTSMATMULT_THRESHOLD=260 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TSVECTSMATMULT_THRESHOLD 260UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TSVECTSMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TSVECTSMATMULT_THRESHOLD 260UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense matrix assignment threshold.
// \ingroup config
//
// This threshold specifies when an assignment with a simple dense matrix can be executed in
// parallel. In case the number of elements of the target matrix is larger or equal to this
// threshold, the operation is executed in parallel. If the number of elements is below this
// threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 48400 (which corresponds to a matrix size of
// \f$ 220 \times 220 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DMATASSIGN_THRESHOLD=48400 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DMATASSIGN_THRESHOLD 48400UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DMATASSIGN_THRESHOLD
#define METRIC_NUMERIC_SMP_DMATASSIGN_THRESHOLD 48400UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense matrix/scalar multiplication/division threshold.
// \ingroup config
//
// This threshold specifies when a dense matrix/scalar multiplication or division can be executed
// in parallel. In case the number of elements of the target matrix is larger or equal to this
// threshold, the operation is executed in parallel. If the number of elements is below this
// threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 48400 (which corresponds to a matrix size of
// \f$ 220 \times 220 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DMATSCALARMULT_THRESHOLD=48400 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DMATSCALARMULT_THRESHOLD 48400UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DMATSCALARMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_DMATSCALARMULT_THRESHOLD 48400UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/row-major dense matrix addition threshold.
// \ingroup config
//
// This threshold specifies when a row-major dense matrix/row-major dense matrix addition can
// be executed in parallel. This threshold affects both additions between two row-major matrices
// or two column-major dense matrices. In case the number of elements of the target matrix is
// larger or equal to this threshold, the operation is executed in parallel. If the number of
// elements is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 36100 (which corresponds to a matrix size of
// \f$ 190 \times 190 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DMATDMATADD_THRESHOLD=36100 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DMATDMATADD_THRESHOLD 36100UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DMATDMATADD_THRESHOLD
#define METRIC_NUMERIC_SMP_DMATDMATADD_THRESHOLD 36100UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/column-major dense matrix addition threshold.
// \ingroup config
//
// This threshold specifies when a row-major dense matrix/column-major dense matrix addition can
// be executed in parallel. This threshold affects both additions between a row-major matrix and
// a column-major matrix and a column-major matrix and a row-major matrix. In case the number of
// elements of the target matrix is larger or equal to this threshold, the operation is executed
// in parallel. If the number of elements is below this threshold the operation is executed
// single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 30625 (which corresponds to a matrix size of
// \f$ 175 \times 175 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DMATTDMATADD_THRESHOLD=30625 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DMATTDMATADD_THRESHOLD 30625UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DMATTDMATADD_THRESHOLD
#define METRIC_NUMERIC_SMP_DMATTDMATADD_THRESHOLD 30625UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/row-major dense matrix subtraction threshold.
// \ingroup config
//
// This threshold specifies when a row-major dense matrix/row-major dense matrix subtraction
// can be executed in parallel. This threshold affects both subtractions between two row-major
// matrices or two column-major dense matrices. In case the number of elements of the target
// matrix is larger or equal to this threshold, the operation is executed in parallel. If the
// number of elements is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 36100 (which corresponds to a matrix size of
// \f$ 190 \times 190 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DMATDMATSUB_THRESHOLD=36100 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DMATDMATSUB_THRESHOLD 36100UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DMATDMATSUB_THRESHOLD
#define METRIC_NUMERIC_SMP_DMATDMATSUB_THRESHOLD 36100UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/column-major dense matrix subtraction threshold.
// \ingroup config
//
// This threshold specifies when a row-major dense matrix/column-major dense matrix subtraction
// can be executed in parallel. This threshold affects both subtractions between a row-major
// matrix and a column-major matrix and a column-major matrix and a row-major matrix. In case
// the number of elements of the target matrix is larger or equal to this threshold, the
// operation is executed in parallel. If the number of elements is below this threshold the
// operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 30625 (which corresponds to a matrix size of
// \f$ 175 \times 175 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DMATTDMATSUB_THRESHOLD=30625 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DMATTDMATSUB_THRESHOLD 30625UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DMATTDMATSUB_THRESHOLD
#define METRIC_NUMERIC_SMP_DMATTDMATSUB_THRESHOLD 30625UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/row-major dense matrix Schur product threshold.
// \ingroup config
//
// This threshold specifies when a row-major dense matrix/row-major dense matrix Schur product
// can be executed in parallel. This threshold affects both Schur products between two row-major
// matrices or two column-major dense matrices. In case the number of elements of the target
// matrix is larger or equal to this threshold, the operation is executed in parallel. If the
// number of elements is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 36100 (which corresponds to a matrix size of
// \f$ 190 \times 190 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DMATDMATSCHUR_THRESHOLD=36100 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DMATDMATSCHUR_THRESHOLD 36100UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DMATDMATSCHUR_THRESHOLD
#define METRIC_NUMERIC_SMP_DMATDMATSCHUR_THRESHOLD 36100UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/column-major dense matrix Schur product threshold.
// \ingroup config
//
// This threshold specifies when a row-major dense matrix/column-major dense matrix Schur product
// can be executed in parallel. This threshold affects both Schur products between a row-major
// matrix and a column-major matrix and a column-major matrix and a row-major matrix. In case
// the number of elements of the target matrix is larger or equal to this threshold, the
// operation is executed in parallel. If the number of elements is below this threshold the
// operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 30625 (which corresponds to a matrix size of
// \f$ 175 \times 175 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DMATTDMATSCHUR_THRESHOLD=30625 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DMATTDMATSCHUR_THRESHOLD 30625UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DMATTDMATSCHUR_THRESHOLD
#define METRIC_NUMERIC_SMP_DMATTDMATSCHUR_THRESHOLD 30625UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/row-major dense matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a row-major dense matrix/row-major dense matrix multiplication
// can be executed in parallel. In case the number of elements of the target matrix is larger
// or equal to this threshold, the operation is executed in parallel. If the number of elements
// is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 3025 (which corresponds to a matrix size of
// \f$ 55 \times 55 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DMATDMATMULT_THRESHOLD=3025 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DMATDMATMULT_THRESHOLD 3025UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DMATDMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_DMATDMATMULT_THRESHOLD 3025UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/column-major dense matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a row-major dense matrix/column-major dense matrix multiplication
// can be executed in parallel. In case the number of elements of the target matrix is larger
// or equal to this threshold, the operation is executed in parallel. If the number of elements
// is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 3025 (which corresponds to a matrix size of
// \f$ 55 \times 55 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DMATTDMATMULT_THRESHOLD=3025 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DMATTDMATMULT_THRESHOLD 3025UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DMATTDMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_DMATTDMATMULT_THRESHOLD 3025UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major dense matrix/row-major dense matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a column-major dense matrix/row-major dense matrix multiplication
// can be executed in parallel. In case the number of elements of the target matrix is larger
// or equal to this threshold, the operation is executed in parallel. If the number of elements
// is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 3025 (which corresponds to a matrix size of
// \f$ 55 \times 55 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TDMATDMATMULT_THRESHOLD=3025 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TDMATDMATMULT_THRESHOLD 3025UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TDMATDMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TDMATDMATMULT_THRESHOLD 3025UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major dense matrix/column-major dense matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a column-major dense matrix/column-major dense matrix multiplication
// can be executed in parallel. In case the number of elements of the target matrix is larger
// or equal to this threshold, the operation is executed in parallel. If the number of elements
// is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 3025 (which corresponds to a matrix size of
// \f$ 55 \times 55 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TDMATTDMATMULT_THRESHOLD=3025 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TDMATTDMATMULT_THRESHOLD 3025UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TDMATTDMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TDMATTDMATMULT_THRESHOLD 3025UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/row-major sparse matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a row-major dense matrix/row-major sparse matrix multiplication
// can be executed in parallel. In case the number of elements of the target matrix is larger
// or equal to this threshold, the operation is executed in parallel. If the number of elements
// is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 4096 (which corresponds to a matrix size of
// \f$ 64 \times 64 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DMATSMATMULT_THRESHOLD=4096 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DMATSMATMULT_THRESHOLD 4096UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DMATSMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_DMATSMATMULT_THRESHOLD 4096UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/column-major sparse matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a row-major dense matrix/column-major sparse matrix multiplication
// can be executed in parallel. In case the number of elements of the target matrix is larger
// or equal to this threshold, the operation is executed in parallel. If the number of elements
// is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 4624 (which corresponds to a matrix size of
// \f$ 68 \times 68 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DMATTSMATMULT_THRESHOLD=4624 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DMATTSMATMULT_THRESHOLD 4624UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DMATTSMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_DMATTSMATMULT_THRESHOLD 4624UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major dense matrix/row-major sparse matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a column-major dense matrix/row-major sparse matrix multiplication
// can be executed in parallel. In case the number of elements of the target matrix is larger
// or equal to this threshold, the operation is executed in parallel. If the number of elements
// is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 8100 (which corresponds to a matrix size of
// \f$ 90 \times 90 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TDMATSMATMULT_THRESHOLD=8100 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TDMATSMATMULT_THRESHOLD 8100UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TDMATSMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TDMATSMATMULT_THRESHOLD 8100UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major dense matrix/column-major sparse matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a column-major dense matrix/column-major sparse matrix multiplication
// can be executed in parallel. In case the number of elements of the target matrix is larger
// or equal to this threshold, the operation is executed in parallel. If the number of elements
// is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 8100 (which corresponds to a matrix size of
// \f$ 90 \times 90 \f$). In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TDMATTSMATMULT_THRESHOLD=8100 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TDMATTSMATMULT_THRESHOLD 8100UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TDMATTSMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TDMATTSMATMULT_THRESHOLD 8100UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major sparse matrix/row-major dense matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a row-major sparse matrix/row-major dense matrix multiplication
// can be executed in parallel. In case the number of elements of the target matrix is larger
// or equal to this threshold, the operation is executed in parallel. If the number of elements
// is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 7744 (which corresponds to a matrix size of
// \f$ 88 \times 88 \f$). In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_SMATDMATMULT_THRESHOLD=7744 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_SMATDMATMULT_THRESHOLD 7744UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_SMATDMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_SMATDMATMULT_THRESHOLD 7744UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major sparse matrix/column-major dense matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a row-major sparse matrix/column-major dense matrix multiplication
// can be executed in parallel. In case the number of elements of the target matrix is larger
// or equal to this threshold, the operation is executed in parallel. If the number of elements
// is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 5184 (which corresponds to a matrix size of
// \f$ 72 \times 72 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_SMATTDMATMULT_THRESHOLD=5184 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_SMATTDMATMULT_THRESHOLD 5184UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_SMATTDMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_SMATTDMATMULT_THRESHOLD 5184UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major sparse matrix/row-major dense matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a column-major sparse matrix/row-major dense matrix multiplication
// can be executed in parallel. In case the number of elements of the target matrix is larger
// or equal to this threshold, the operation is executed in parallel. If the number of elements
// is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 4356 (which corresponds to a matrix size of
// \f$ 66 \times 66 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TSMATDMATMULT_THRESHOLD=4356 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TSMATDMATMULT_THRESHOLD 4356UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TSMATDMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TSMATDMATMULT_THRESHOLD 4356UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major sparse matrix/column-major dense matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a column-major sparse matrix/column-major dense matrix multiplication
// can be executed in parallel. In case the number of elements of the target matrix is larger
// or equal to this threshold, the operation is executed in parallel. If the number of elements
// is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 4356 (which corresponds to a matrix size of
// \f$ 66 \times 66 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TSMATTDMATMULT_THRESHOLD=4356 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TSMATTDMATMULT_THRESHOLD 4356UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TSMATTDMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TSMATTDMATMULT_THRESHOLD 4356UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major sparse matrix/row-major sparse matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a row-major sparse matrix/row-major sparse matrix multiplication
// can be executed in parallel. In case the number of elements of the target matrix is larger
// or equal to this threshold, the operation is executed in parallel. If the number of elements
// is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 22500 (which corresponds to a matrix size of
// \f$ 150 \times 150 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_SMATSMATMULT_THRESHOLD=22500 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_SMATSMATMULT_THRESHOLD 22500UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_SMATSMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_SMATSMATMULT_THRESHOLD 22500UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major sparse matrix/column-major sparse matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a row-major sparse matrix/column-major sparse matrix multiplication
// can be executed in parallel. In case the number of elements of the target matrix is larger
// or equal to this threshold, the operation is executed in parallel. If the number of elements
// is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 19600 (which corresponds to a matrix size of
// \f$ 140 \times 140 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_SMATTSMATMULT_THRESHOLD=19600 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_SMATTSMATMULT_THRESHOLD 19600UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_SMATTSMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_SMATTSMATMULT_THRESHOLD 19600UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major sparse matrix/row-major sparse matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a column-major sparse matrix/row-major sparse matrix multiplication
// can be executed in parallel. In case the number of elements of the target matrix is larger
// or equal to this threshold, the operation is executed in parallel. If the number of elements
// is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 19600 (which corresponds to a matrix size of
// \f$ 140 \times 140 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TSMATSMATMULT_THRESHOLD=19600 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TSMATSMATMULT_THRESHOLD 19600UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TSMATSMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TSMATSMATMULT_THRESHOLD 19600UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major sparse matrix/column-major sparse matrix multiplication threshold.
// \ingroup config
//
// This threshold specifies when a column-major sparse matrix/column-major sparse matrix
// multiplication can be executed in parallel. In case the number of elements of the target
// matrix is larger or equal to this threshold, the operation is executed in parallel. If the
// number of elements is below this threshold the operation is executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 22500 (which corresponds to a matrix size of
// \f$ 150 \times 150 \f$). In case the threshold is set to 0, the operation is unconditionally
// executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_TSMATTSMATMULT_THRESHOLD=22500 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_TSMATTSMATMULT_THRESHOLD 22500UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_TSMATTSMATMULT_THRESHOLD
#define METRIC_NUMERIC_SMP_TSMATTSMATMULT_THRESHOLD 22500UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense matrix reduction threshold.
// \ingroup config
//
// This threshold specifies when a dense matrix reduction can be executed in parallel. In case
// the number of elements of the target vector is larger or equal to this threshold, the operation
// is executed in parallel. If the number of elements is below this threshold the operation is
// executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 180. In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_DMATREDUCE_THRESHOLD=180 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_DMATREDUCE_THRESHOLD 180UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_DMATREDUCE_THRESHOLD
#define METRIC_NUMERIC_SMP_DMATREDUCE_THRESHOLD 180UL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP sparse matrix reduction threshold.
// \ingroup config
//
// This threshold specifies when a sparse matrix reduction can be executed in parallel. In case
// the number of elements of the target vector is larger or equal to this threshold, the operation
// is executed in parallel. If the number of elements is below this threshold the operation is
// executed single-threaded.
//
// Please note that this threshold is highly sensitiv to the used system architecture and the
// shared memory parallelization technique. Therefore the default value cannot guarantee maximum
// performance for all possible situations and configurations. It merely provides a reasonable
// standard for the current generation of CPUs. Also note that the provided default has been
// determined using the OpenMP parallelization and requires individual adaption for the C++11
// and other METRIC-supported parallelization strategies.
//
// The default setting for this threshold is 180. In case the threshold is set to 0, the operation
// is unconditionally executed in parallel.
//
// \note It is possible to specify this threshold via command line or by defining this symbol
// manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_SMP_SMATREDUCE_THRESHOLD=180 ...
   \endcode

   \code
   #define METRIC_NUMERIC_SMP_SMATREDUCE_THRESHOLD 180UL
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_SMP_SMATREDUCE_THRESHOLD
#define METRIC_NUMERIC_SMP_SMATREDUCE_THRESHOLD 180UL
#endif
//*************************************************************************************************
