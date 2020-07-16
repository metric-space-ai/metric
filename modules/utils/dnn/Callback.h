#ifndef CALLBACK_H_
#define CALLBACK_H_


namespace metric::dnn
{

template <typename Scalar>
class Network;

///
/// \defgroup Callbacks Callback Functions
///

///
/// \ingroup Callbacks
///
/// The interface and default implementation of the callback function during
/// model fitting. The purpose of this class is to allow users printing some
/// messages in each epoch or mini-batch training, for example the time spent,
/// the loss function values, etc.
///
/// This default implementation is a silent version of the callback function
/// that basically does nothing. See the VerboseCallback class for a verbose
/// version that prints the loss function value in each mini-batch.
///
template <typename Scalar>
class Callback
{
    protected:
		using Matrix = blaze::DynamicMatrix<Scalar>;
		using IntegerVector = blaze::DynamicMatrix<int, blaze::columnMajor>;

    public:
        // Public members that will be set by the network during the training process
        int batchesNumber;   // Number of total batches
        int batchId; // The index for the current mini-batch (0, 1, ..., m_nbatch-1)
        int epochsNumber;   // Total number of epochs (one run on the whole data set) in the training process
        int epochId; // The index for the current epoch (0, 1, ..., m_nepoch-1)

        Callback() :
		        batchesNumber(0), batchId(0), epochsNumber(0), epochId(0)
        {}

        virtual ~Callback() {}

		virtual void preTrainingEpoch(const Network<Scalar> *net) {};

		virtual void postTrainingEpoch(const Network<Scalar> *net) {};

		// Before training a mini-batch
        virtual void preTrainingBatch(const Network<Scalar>* net, const Matrix& x,
                                      const Matrix& y) {}
        //virtual void preTrainingBatch(const Network<Scalar>* net, const Matrix& x,
                                      //const IntegerVector& y) {}

        // After a mini-batch is trained
        virtual void postTrainingBatch(const Network<Scalar>* net, const Matrix& x,
                                       const Matrix& y) {}
        //virtual void postTrainingBatch(const Network<Scalar>* net, const Matrix& x,
                                       //const IntegerVector& y) {}
};


} // namespace metric::dnn


#endif /* CALLBACK_H_ */
