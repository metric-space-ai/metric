// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Smoke test for the native mtrc::record layer: record ids, duplicates, empty
// record sets, composed records, conversion, grouping, printing, and the
// binding-facing buffers that language bindings import and export through.
// Included via metric/engine.hpp to also exercise engine-level header wiring.

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/engine.hpp"

template <typename Callable> static auto throws_invalid_argument(Callable &&call) -> bool
{
	try {
		call();
	} catch (const std::invalid_argument &) {
		return true;
	}
	return false;
}

template <typename Callable> static auto throws_out_of_range(Callable &&call) -> bool
{
	try {
		call();
	} catch (const std::out_of_range &) {
		return true;
	}
	return false;
}

static auto test_record_ids_and_duplicates() -> void
{
	using mtrc::RecordId;

	const std::vector<RecordId> ids{RecordId::from_index(0), RecordId::from_index(1), RecordId::from_index(0),
									RecordId::from_index(2), RecordId::from_index(1)};

	assert(mtrc::has_duplicate_record_ids(ids));
	assert((mtrc::duplicate_record_ids(ids) == std::vector<RecordId>{RecordId::from_index(0), RecordId::from_index(1)}));
	assert((mtrc::unique_record_ids(ids) ==
			std::vector<RecordId>{RecordId::from_index(0), RecordId::from_index(1), RecordId::from_index(2)}));
	assert(mtrc::count_record_id(ids, RecordId::from_index(1)) == 2);
	assert(mtrc::count_record_id(ids, RecordId::from_index(9)) == 0);

	// Unique-ified ids are duplicate-free.
	assert(!mtrc::has_duplicate_record_ids(mtrc::unique_record_ids(ids)));

	const auto report = mtrc::inspect_record_ids(ids);
	assert(report.count == 5);
	assert(report.unique_count == 3);
	assert(report.duplicate_count == 2);
	assert(report.has_duplicates);
	assert(!report.empty);

	// Empty record-id set: the standalone duplicate/count primitives too.
	const auto empty_report = mtrc::inspect_record_ids(std::vector<RecordId>{});
	assert(empty_report.empty);
	assert(empty_report.count == 0 && empty_report.unique_count == 0 && empty_report.duplicate_count == 0);
	assert(!empty_report.has_duplicates);
	assert(!mtrc::has_duplicate_record_ids(std::vector<RecordId>{}));
	assert(mtrc::count_record_id(std::vector<RecordId>{}, RecordId::from_index(0)) == 0);

	// Validation gates.
	assert(throws_invalid_argument([&] { mtrc::validate_unique_record_ids(ids); }));
	mtrc::validate_unique_record_ids(mtrc::unique_record_ids(ids)); // does not throw
}

static auto test_id_set_helpers() -> void
{
	using mtrc::RecordId;
	const std::vector<RecordId> ids{RecordId::from_index(0), RecordId::from_index(1), RecordId::from_index(2)};

	// contains / position lookups (position_of_record_id throws out_of_range, not invalid_argument).
	assert(mtrc::contains_record_id(ids, RecordId::from_index(1)));
	assert(!mtrc::contains_record_id(ids, RecordId::from_index(9)));
	assert(!mtrc::contains_record_id(std::vector<RecordId>{}, RecordId::from_index(0)));
	assert(mtrc::position_of_record_id(ids, RecordId::from_index(2), "missing") == 2);
	assert(throws_out_of_range([&] { (void)mtrc::position_of_record_id(ids, RecordId::from_index(9), "missing"); }));

	// overlap counting: full, partial, and zero overlap.
	const std::vector<RecordId> candidates{RecordId::from_index(1), RecordId::from_index(2), RecordId::from_index(7)};
	assert(mtrc::record_id_overlap_count(ids, candidates) == 2);
	assert(mtrc::record_id_overlap_count(ids, std::vector<RecordId>{RecordId::from_index(7)}) == 0);
	assert(mtrc::record_id_overlap_count(ids, ids) == 3);
}

static auto test_empty_record_sets() -> void
{
	assert(throws_invalid_argument([] { mtrc::validate_records_non_empty(std::vector<int>{}); }));
	assert(throws_invalid_argument([] { mtrc::validate_records_non_empty(std::vector<std::string>{}); }));
	mtrc::validate_records_non_empty(std::vector<int>{1}); // does not throw

	// Empty record collections convert/export to empty buffers without error.
	const auto empty_buffer = mtrc::export_records_to_buffer(std::vector<std::vector<double>>{});
	assert(empty_buffer.empty());
	assert(empty_buffer.record_count == 0 && empty_buffer.record_dim == 0 && empty_buffer.consistent());

	const auto empty_records = mtrc::import_records_from_buffer(std::vector<double>{}, 0, 4);
	assert(empty_records.empty());

	assert(mtrc::validate_uniform_record_dimension(std::vector<std::vector<double>>{}) == 0);
}

static auto test_binding_buffers() -> void
{
	// Row-major binding buffer: 3 records of dimension 2.
	const std::vector<double> buffer{1, 2, 3, 4, 5, 6};

	mtrc::validate_buffer_shape(buffer.size(), 3, 2);
	assert(throws_invalid_argument([&] { mtrc::validate_buffer_shape(buffer.size(), 4, 2); }));

	// Pathological dimensions whose product overflows std::size_t are rejected,
	// not silently wrapped into a passing (and unsafe) shape.
	const std::size_t huge = (std::numeric_limits<std::size_t>::max)() / 2 + 1;
	assert(throws_invalid_argument([&] { mtrc::validate_buffer_shape(0, 4, huge); }));
	assert(throws_invalid_argument([&] { (void)mtrc::import_records_from_buffer(std::vector<double>{}, 4, huge); }));
	assert(throws_invalid_argument([&] { (void)mtrc::make_record_buffer(std::vector<double>{}, 4, huge); }));

	const auto records = mtrc::import_records_from_buffer(buffer, 3, 2);
	assert(records.size() == 3);
	assert((records[0] == std::vector<double>{1, 2}));
	assert((records[2] == std::vector<double>{5, 6}));

	// Mismatched shape is rejected.
	assert(throws_invalid_argument([&] { (void)mtrc::import_records_from_buffer(buffer, 5, 2); }));

	// Round-trip: records -> buffer -> records.
	const auto exported = mtrc::export_records_to_buffer(records);
	assert(exported.record_count == 3 && exported.record_dim == 2 && exported.consistent());
	assert(exported.data == buffer);
	assert(mtrc::import_records_from_buffer(exported) == records);

	// Typed RecordBuffer factory validates its own shape.
	const auto typed = mtrc::make_record_buffer(std::vector<int>{1, 2, 3, 4}, 2, 2);
	assert(typed.consistent() && typed.element_count() == 4);
	assert(throws_invalid_argument([] { (void)mtrc::make_record_buffer(std::vector<int>{1, 2, 3}, 2, 2); }));

	// Ragged records flatten with recoverable shape but reject uniform export.
	const std::vector<std::vector<double>> ragged{{1, 2, 3}, {4}};
	assert((mtrc::record_dimensions(ragged) == std::vector<std::size_t>{3, 1}));
	assert((mtrc::flatten_records(ragged) == std::vector<double>{1, 2, 3, 4}));
	assert(throws_invalid_argument([&] { (void)mtrc::export_records_to_buffer(ragged); }));
	assert(throws_invalid_argument([&] { (void)mtrc::validate_uniform_record_dimension(ragged); }));

	// Generic container import copies records as-is.
	const auto imported = mtrc::import_records(std::vector<std::string>{"a", "b", "c"});
	assert(imported.size() == 3 && imported[1] == "b");
}

static auto test_delimited_file_io() -> void
{
	{
		std::istringstream csv("name,x,y\n\"pump, one\",0,1\nvalve,2,3\n\n");
		mtrc::CsvReadOptions options;
		options.has_header = true;
		const auto rows = mtrc::read_csv<std::string>(csv, options);
		assert(rows.size() == 2);
		assert((rows[0] == std::vector<std::string>{"pump, one", "0", "1"}));
		assert((rows[1] == std::vector<std::string>{"valve", "2", "3"}));
	}

	{
		std::istringstream csv("x,y\n0, 1.5\n2,3\n");
		mtrc::CsvReadOptions options;
		options.has_header = true;
		const auto rows = mtrc::read_csv<double>(csv, options);
		assert(rows.size() == 2);
		assert((rows[0] == std::vector<double>{0.0, 1.5}));
		assert((rows[1] == std::vector<double>{2.0, 3.0}));
	}

	{
		std::istringstream tsv("0\t1\n2\t3\n");
		const auto rows = mtrc::read_tsv<int>(tsv);
		assert((rows == std::vector<std::vector<int>>{{0, 1}, {2, 3}}));
	}

	{
		std::istringstream ragged("1,2,3\n4\n");
		mtrc::CsvReadOptions options;
		options.require_uniform_dimension = false;
		const auto rows = mtrc::read_csv<int>(ragged, options);
		assert((rows == std::vector<std::vector<int>>{{1, 2, 3}, {4}}));
	}

	assert(throws_invalid_argument([] {
		std::istringstream csv("1,2\n3\n");
		(void)mtrc::read_csv<double>(csv);
	}));
	assert(throws_invalid_argument([] {
		std::istringstream csv("1,\n");
		(void)mtrc::read_csv<double>(csv);
	}));
	assert(throws_invalid_argument([] {
		std::istringstream csv("1,nan\n");
		(void)mtrc::read_csv<double>(csv);
	}));
	assert(throws_invalid_argument([] {
		std::istringstream csv("\"unterminated\n");
		(void)mtrc::read_csv<std::string>(csv);
	}));

	{
		std::ostringstream output;
		mtrc::CsvWriteOptions options;
		options.header = {"name", "note"};
		mtrc::write_csv(output, std::vector<std::vector<std::string>>{{"pump, one", "steady"}, {"valve", "he said \"ok\""}},
						options);
		assert(output.str() == "name,note\n\"pump, one\",steady\nvalve,\"he said \"\"ok\"\"\"\n");
	}

	{
		std::ostringstream output;
		mtrc::write_tsv(output, std::vector<std::vector<int>>{{1, 2}, {3, 4}});
		assert(output.str() == "1\t2\n3\t4\n");
	}

	const std::string path = "record_layer_smoke_vectors.csv";
	{
		mtrc::CsvWriteOptions options;
		options.header = {"x", "y"};
		mtrc::write_csv(path, std::vector<std::vector<double>>{{0.0, 1.0}, {2.5, 3.5}}, options);
	}
	{
		mtrc::CsvReadOptions options;
		options.has_header = true;
		const auto rows = mtrc::read_csv<double>(path, options);
		assert((rows == std::vector<std::vector<double>>{{0.0, 1.0}, {2.5, 3.5}}));
	}
	std::remove(path.c_str());
}

static auto test_conversion() -> void
{
	assert(mtrc::convert_record_value<int>(3.9) == 3);
	assert((mtrc::convert_record<int>(std::vector<double>{1.2, 2.8, 3.5}) == std::vector<int>{1, 2, 3}));

	const auto promoted = mtrc::convert_records<double>(std::vector<std::vector<int>>{{1, 2}, {3, 4}});
	assert(promoted.size() == 2);
	assert((promoted[0] == std::vector<double>{1.0, 2.0}));
	assert(promoted[1][1] == 4.0);
}

static auto test_grouping() -> void
{
	using mtrc::RecordId;
	const std::vector<RecordId> ids{RecordId::from_index(0), RecordId::from_index(1), RecordId::from_index(2),
									RecordId::from_index(3), RecordId::from_index(4)};

	// Group by parity of the id index (an explicit caller key, not a metric).
	const auto grouping = mtrc::group_record_ids_by_key(ids, [](RecordId id) { return id.index() % 2; });
	assert(grouping.group_count() == 2);
	assert(grouping.keys[0] == 0 && grouping.keys[1] == 1); // first-seen order
	assert(grouping.groups[0].size() == 3);                 // 0, 2, 4
	assert(grouping.groups[1].size() == 2);                 // 1, 3
	assert(grouping.groups[0][1] == RecordId::from_index(2));

	// Group by a parallel key vector.
	const auto labelled =
		mtrc::group_record_ids(ids, std::vector<std::string>{"x", "y", "x", "y", "x"});
	assert(labelled.group_count() == 2);
	assert(labelled.keys[0] == "x" && labelled.groups[0].size() == 3);
	assert(throws_invalid_argument([&] { (void)mtrc::group_record_ids(ids, std::vector<int>{1, 2}); }));
}

static auto test_composed_records() -> void
{
	// A mixed record: a text part, a histogram-like vector part, and a scalar.
	auto record = mtrc::compose_record(std::string("sensor"), std::vector<double>{0.1, 0.2, 0.7}, 5);
	using Composed = mtrc::ComposedRecord<std::string, std::vector<double>, int>;
	assert((Composed::field_count == 3));
	assert(record.field<0>() == "sensor");
	assert((record.field<1>() == std::vector<double>{0.1, 0.2, 0.7}));
	assert(record.field<2>() == 5);

	auto same = mtrc::compose_record(std::string("sensor"), std::vector<double>{0.1, 0.2, 0.7}, 5);
	assert(record == same);
	record.field<2>() = 6;
	assert(record != same);

	// Column-wise assembly of a collection of composed records.
	const auto records =
		mtrc::compose_records(std::vector<std::string>{"a", "b"}, std::vector<int>{1, 2});
	assert(records.size() == 2);
	assert(records[0].field<0>() == "a" && records[0].field<1>() == 1);
	assert(records[1].field<0>() == "b" && records[1].field<1>() == 2);

	const std::vector<std::string> labels{"pump", "valve"};
	const std::vector<std::vector<double>> spectra{{0.2, 0.8}, {0.7, 0.3}};
	const std::vector<int> states{1, 2};
	const auto column_report = mtrc::inspect_record_columns(labels, spectra, states);
	assert(column_report.ok());
	assert(column_report.field_count == 3);
	assert(column_report.row_count == 2);
	assert((column_report.field_sizes == std::vector<std::size_t>{2, 2, 2}));

	const auto imported_mixed = mtrc::import_mixed_records(labels, spectra, states);
	assert(imported_mixed.size() == 2);
	assert(imported_mixed.report.ok());
	assert(imported_mixed.records[0].field<0>() == "pump");
	assert((imported_mixed.records[1].field<1>() == std::vector<double>{0.7, 0.3}));
	assert(imported_mixed.records[1].field<2>() == 2);

	const auto bad_report =
		mtrc::inspect_record_columns(labels, std::vector<double>{1.0}, std::vector<int>{1, 2, 3});
	assert(!bad_report.ok());
	assert(bad_report.issue_count() == 2);
	assert(bad_report.issues[0].field_index == 1);
	assert(bad_report.issues[0].expected_count == 2);
	assert(bad_report.issues[0].actual_count == 1);
	assert(bad_report.issues[1].field_index == 2);
	assert(bad_report.issues[1].actual_count == 3);
	assert(throws_invalid_argument([&] {
		(void)mtrc::validate_record_columns(labels, std::vector<double>{1.0}, std::vector<int>{1, 2, 3});
	}));
	assert(throws_invalid_argument([&] {
		(void)mtrc::import_mixed_records(labels, std::vector<double>{1.0}, std::vector<int>{1, 2, 3});
	}));

	// Composed records can live in a metric space; the metric stays in
	// mtrc::metric, not in mtrc::record. We only check the records are usable
	// and deduplicate by value here.
	std::vector<Composed> collection{same, same};
	assert(collection[0] == collection[1]);

	// Mismatched column lengths are rejected.
	assert(throws_invalid_argument(
		[] { (void)mtrc::compose_records(std::vector<int>{1, 2, 3}, std::vector<int>{1}); }));
}

static auto test_printing() -> void
{
	using mtrc::RecordId;

	assert(mtrc::record_to_string(RecordId::from_index(7)) == "#7");
	assert(mtrc::record_to_string(std::vector<int>{1, 2, 3}) == "[1, 2, 3]");
	assert(mtrc::record_to_string(std::vector<std::vector<int>>{{1, 2}, {3}}) == "[[1, 2], [3]]");
	assert(mtrc::record_to_string(std::string("text")) == "text");

	auto composed = mtrc::compose_record(std::string("x"), std::vector<int>{1, 2}, 9);
	assert(mtrc::record_to_string(composed) == "(x | [1, 2] | 9)");

	const std::vector<RecordId> ids{RecordId::from_index(0), RecordId::from_index(3)};
	assert(mtrc::record_ids_to_string(ids) == "[#0, #3]");
	assert(mtrc::records_to_string(std::vector<int>{4, 5}) == "[4, 5]");

	// Cover the direct ostream-writing overloads, not only the string wrappers.
	{
		std::ostringstream stream;
		mtrc::write_record(stream, std::vector<int>{1, 2});
		assert(stream.str() == "[1, 2]");
	}
	{
		std::ostringstream stream;
		mtrc::print_records(stream, std::vector<int>{4, 5});
		assert(stream.str() == "[4, 5]");
	}
	{
		std::ostringstream stream;
		mtrc::print_record_ids(stream, ids);
		assert(stream.str() == "[#0, #3]");
	}
	{
		std::ostringstream stream;
		mtrc::print_record(stream, RecordId::from_index(7));
		assert(stream.str() == "#7");
	}
}

static auto test_record_layer_in_metric_space() -> void
{
	// The record layer is independent of metric selection: records imported
	// from a binding buffer can be placed in a space with a metric chosen
	// elsewhere. No metric logic lives in mtrc::record itself.
	struct AbsoluteDistance {
		auto operator()(const std::vector<double> &lhs, const std::vector<double> &rhs) const -> double
		{
			double total = 0.0;
			for (std::size_t index = 0; index < lhs.size(); ++index) {
				total += lhs[index] > rhs[index] ? lhs[index] - rhs[index] : rhs[index] - lhs[index];
			}
			return total;
		}
	};

	const auto records = mtrc::import_records_from_buffer(std::vector<double>{0, 0, 1, 0, 0, 1}, 3, 2);
	auto space = mtrc::make_space(records, AbsoluteDistance{});
	assert(space.size() == 3);
	assert(space.distance(space.id(0), space.id(1)) == 1.0);
}

int main()
{
	test_record_ids_and_duplicates();
	test_id_set_helpers();
	test_empty_record_sets();
	test_binding_buffers();
	test_delimited_file_io();
	test_conversion();
	test_grouping();
	test_composed_records();
	test_printing();
	test_record_layer_in_metric_space();
	return 0;
}
