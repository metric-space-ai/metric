#include "metric/mapping/ensembles.hpp"

#include <functional>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <variant>
#include <vector>

namespace py = pybind11;

template <typename Record, typename WeakLearner, typename Subsampler> void register_wrapper_Boosting(py::module &m)
{
	using Mapping = mtrc::Boosting<Record, WeakLearner, Subsampler>;
	using Container = std::vector<Record>;
	using Features = std::vector<std::function<double(Record)>>;
	using Callback = std::function<bool(Record)>;

	void (Mapping::*train)(Container &, Features &, Callback &, bool) = &Mapping::train;
	void (Mapping::*predict)(Container &, Features &, std::vector<bool> &) = &Mapping::predict;

	py::class_<Mapping>(m, "Boosting")
		.def(py::init<int, double, double, WeakLearner>(), py::arg("ensemble_size_"), py::arg("share_overall"),
			 py::arg("share_minor"), py::arg("weak_classifier"))
		.def("train", train)
		.def("predict", predict);
}

template <typename Record, typename WeakLearnerVariant, typename Subsampler>
void register_wrapper_Bagging(py::module &m)
{
	using Mapping = mtrc::Bagging<Record, WeakLearnerVariant, Subsampler>;
	using Container = std::vector<Record>;
	using Features = std::vector<std::function<double(Record)>>;
	using Callback = std::function<bool(Record)>;

	void (Mapping::*train)(Container &, Features &, Callback &, bool) = &Mapping::train;
	void (Mapping::*predict)(Container &, Features &, std::vector<bool> &) = &Mapping::predict;

	py::class_<Mapping>(m, "Bagging")
		.def(py::init<int, double, double, std::vector<double>, std::vector<WeakLearnerVariant>>(),
			 py::arg("ensemble_size"), py::arg("share_overall"), py::arg("share_minor"), py::arg("type_weight"),
			 py::arg("weak_classifiers"))
		.def("train", train)
		.def("predict", predict);
}

// TODO: implement WeakLearner interface class to provide in python
template <class Record> class PythonWeekLearner {
  public:
	/**
	 * @brief train model on test dataset
	 *
	 * @param payments test dataset
	 * @param features
	 * @param response
	 */
	virtual void train(py::array &payments, std::vector<std::function<double(Record)>> &features,
					   std::function<bool(Record)> &response);

	/**
	 * @brief use model to classify input data
	 *
	 * @param data input data
	 * @param features
	 * @param predictions[out] prediction result
	 */
	virtual void predict(py::array &data, std::vector<std::function<double(Record)>> &features,
						 std::vector<bool> &predictions);

	/**
	 * @brief clone object
	 */
	virtual std::shared_ptr<PythonWeekLearner<Record>> clone();
};

void export_metric_ensembles(py::module &m)
{
	using Record = std::vector<double>;
	// Native weak learner. The libedm-backed C4.5/SVM weak learners were removed
	// together with the metric/3rdparty/libedm island; the ensembles bind to the
	// METRIC-owned mtrc::TestCl weak learner.
	using WeakLearner = mtrc::TestCl<Record>;
	using WeakLearnerVariant = std::variant<mtrc::TestCl<Record>>;
	register_wrapper_Boosting<Record, WeakLearner, mtrc::SubsampleRUS<Record>>(m);
	register_wrapper_Bagging<Record, WeakLearnerVariant, mtrc::SubsampleRUS<Record>>(m);
}

PYBIND11_MODULE(ensembles, m) { export_metric_ensembles(m); }