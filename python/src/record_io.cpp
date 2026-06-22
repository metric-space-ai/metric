#include <metric/record.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace py = pybind11;

namespace {

auto single_char_option(const std::string &value, const char *name) -> char
{
	if (value.size() != 1) {
		throw std::invalid_argument(std::string(name) + " must be exactly one character");
	}
	return value.front();
}

auto read_options(char delimiter, const std::string &quote, bool has_header, bool trim_fields,
				  bool allow_empty_lines, bool require_uniform_dimension, bool require_finite)
	-> mtrc::record::DelimitedReadOptions
{
	mtrc::record::DelimitedReadOptions options;
	options.delimiter = delimiter;
	options.quote = single_char_option(quote, "quote");
	options.has_header = has_header;
	options.trim_fields = trim_fields;
	options.allow_empty_lines = allow_empty_lines;
	options.require_uniform_dimension = require_uniform_dimension;
	options.require_finite = require_finite;
	return options;
}

auto write_options(char delimiter, const std::string &quote, bool newline_at_end, std::vector<std::string> header)
	-> mtrc::record::DelimitedWriteOptions
{
	mtrc::record::DelimitedWriteOptions options;
	options.delimiter = delimiter;
	options.quote = single_char_option(quote, "quote");
	options.newline_at_end = newline_at_end;
	options.header = std::move(header);
	return options;
}

template <typename Value>
auto read_csv_records(const std::string &path, bool has_header, bool trim_fields, bool allow_empty_lines,
					  bool require_uniform_dimension, bool require_finite, const std::string &quote)
	-> std::vector<std::vector<Value>>
{
	auto options = read_options(',', quote, has_header, trim_fields, allow_empty_lines, require_uniform_dimension,
								require_finite);
	return mtrc::record::read_csv<Value>(path, options);
}

template <typename Value>
auto read_tsv_records(const std::string &path, bool has_header, bool trim_fields, bool allow_empty_lines,
					  bool require_uniform_dimension, bool require_finite, const std::string &quote)
	-> std::vector<std::vector<Value>>
{
	auto options = read_options('\t', quote, has_header, trim_fields, allow_empty_lines, require_uniform_dimension,
								require_finite);
	return mtrc::record::read_tsv<Value>(path, options);
}

template <typename Value>
auto write_csv_records(const std::string &path, const std::vector<std::vector<Value>> &records,
					   bool newline_at_end, std::vector<std::string> header, const std::string &quote) -> void
{
	auto options = write_options(',', quote, newline_at_end, std::move(header));
	mtrc::record::write_csv(path, records, options);
}

template <typename Value>
auto write_tsv_records(const std::string &path, const std::vector<std::vector<Value>> &records,
					   bool newline_at_end, std::vector<std::string> header, const std::string &quote) -> void
{
	auto options = write_options('\t', quote, newline_at_end, std::move(header));
	mtrc::record::write_tsv(path, records, options);
}

} // namespace

void export_record_io(py::module &m)
{
	m.def("read_csv_double_records", &read_csv_records<double>, py::arg("path"), py::arg("has_header") = false,
		  py::arg("trim_fields") = true, py::arg("allow_empty_lines") = true,
		  py::arg("require_uniform_dimension") = true, py::arg("require_finite") = true, py::arg("quote") = "\"");
	m.def("read_csv_string_records", &read_csv_records<std::string>, py::arg("path"), py::arg("has_header") = false,
		  py::arg("trim_fields") = true, py::arg("allow_empty_lines") = true,
		  py::arg("require_uniform_dimension") = true, py::arg("require_finite") = true, py::arg("quote") = "\"");
	m.def("read_tsv_double_records", &read_tsv_records<double>, py::arg("path"), py::arg("has_header") = false,
		  py::arg("trim_fields") = true, py::arg("allow_empty_lines") = true,
		  py::arg("require_uniform_dimension") = true, py::arg("require_finite") = true, py::arg("quote") = "\"");
	m.def("read_tsv_string_records", &read_tsv_records<std::string>, py::arg("path"), py::arg("has_header") = false,
		  py::arg("trim_fields") = true, py::arg("allow_empty_lines") = true,
		  py::arg("require_uniform_dimension") = true, py::arg("require_finite") = true, py::arg("quote") = "\"");

	m.def("write_csv_double_records", &write_csv_records<double>, py::arg("path"), py::arg("records"),
		  py::arg("newline_at_end") = true, py::arg("header") = std::vector<std::string>{}, py::arg("quote") = "\"");
	m.def("write_csv_string_records", &write_csv_records<std::string>, py::arg("path"), py::arg("records"),
		  py::arg("newline_at_end") = true, py::arg("header") = std::vector<std::string>{}, py::arg("quote") = "\"");
	m.def("write_tsv_double_records", &write_tsv_records<double>, py::arg("path"), py::arg("records"),
		  py::arg("newline_at_end") = true, py::arg("header") = std::vector<std::string>{}, py::arg("quote") = "\"");
	m.def("write_tsv_string_records", &write_tsv_records<std::string>, py::arg("path"), py::arg("records"),
		  py::arg("newline_at_end") = true, py::arg("header") = std::vector<std::string>{}, py::arg("quote") = "\"");
}
