#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include <metric/workflow.hpp>

namespace metric_template {

using record_type = mtrc::record::ComposedRecord<std::string, std::vector<double>, int>;

struct FleetMetric {
	mtrc::Edit<char> name_metric{};
	mtrc::Wasserstein<double> distribution_metric{mtrc::Wasserstein<double>::on_line(4)};
	mtrc::DiscreteMetric<double> state_metric{1.0};

	auto operator()(const record_type &lhs, const record_type &rhs) const -> double
	{
		return static_cast<double>(name_metric(lhs.template field<0>(), rhs.template field<0>())) +
			   distribution_metric(lhs.template field<1>(), rhs.template field<1>()) +
			   state_metric(lhs.template field<2>(), rhs.template field<2>());
	}
};

} // namespace metric_template

namespace mtrc::core {

template <> struct metric_traits<::metric_template::FleetMetric> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::structured;
	static constexpr bool thread_safe = true;

	static auto cache_key(const ::metric_template::FleetMetric &) -> std::string
	{
		return "metric_template::FleetMetric:edit+wass-line4+discrete";
	}
};

} // namespace mtrc::core

int main()
{
	const std::vector<std::string> event_codes = {"pump_ok", "pump_warn", "valve_ok", "valve_warn"};
	const std::vector<std::vector<double>> spectral_mass = {
		{0.70, 0.20, 0.10, 0.00},
		{0.64, 0.24, 0.12, 0.00},
		{0.00, 0.10, 0.25, 0.65},
		{0.00, 0.08, 0.28, 0.64},
	};
	const std::vector<int> machine_state = {0, 1, 0, 1};

	const auto imported = mtrc::record::import_mixed_records(event_codes, spectral_mass, machine_state);
	assert(imported.report.row_count == imported.records.size());
	assert(imported.report.field_count == 3);

	auto space = mtrc::space::build_checked(imported.records, metric_template::FleetMetric{});
	const auto probe = mtrc::record::compose_record(std::string("valve_warning"), std::vector<double>{0.0, 0.08, 0.27, 0.65}, 1);
	const auto nearest = mtrc::space::query::nearest(space, probe);
	assert(nearest.id == space.id(3));

	const auto diagnosis = mtrc::stats::diagnose_space(space);
	assert(diagnosis.discoverable_metric);

	std::cout << "mixed rows=" << imported.report.row_count << "\n";
	std::cout << "nearest mixed record id=" << nearest.id.index() << "\n";
	return 0;
}
