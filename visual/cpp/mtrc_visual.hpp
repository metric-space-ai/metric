// mtrc::visual — header-only, dependency-free writer for metric.visual.v1.
//
// METRIC C++ computes evidence; this header serializes it into the visual
// document that METRIC Visual loads directly. It pulls in nothing beyond the
// standard library and never computes a metric — callers pass values they have
// already computed.
//
// Usage:
//   mtrc::visual::Document doc;
//   doc.dataset("sensor", "Sensor", "desc", "synthetic", "CC0-1.0");
//   doc.record("r0", "sensor", "vector", "obs 0", mtrc::visual::vector_payload({0.1,0.2}));
//   doc.metric_relation("m", "sensor", "metric", {"r0","r1"}, edges);
//   doc.space("space", "sensor", {"r0","r1"}, "m", "finite_metric_space");
//   doc.coordinates3("c", "sensor", "space", "3D", positions);
//   std::string json = doc.to_json();
#pragma once

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

namespace mtrc {
namespace visual {

inline std::string escape(const std::string& s) {
  std::string out;
  out.reserve(s.size() + 8);
  for (char c : s) {
    const unsigned char uc = static_cast<unsigned char>(c);
    switch (c) {
      case '"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        if (uc < 0x20) {
          char buf[8];
          std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned int>(uc));
          out += buf;
        } else {
          out += c;
        }
    }
  }
  return out;
}

inline std::string quote(const std::string& s) { return "\"" + escape(s) + "\""; }

inline std::string num(double v) {
  if (!std::isfinite(v)) return "0";
  std::ostringstream ss;
  ss.imbue(std::locale::classic());
  ss << std::setprecision(std::numeric_limits<double>::max_digits10) << v;
  return ss.str();
}

inline std::string integer(long long v) { return std::to_string(v); }

inline std::string size(std::size_t v) { return std::to_string(v); }

inline std::string boolean(bool v) { return v ? "true" : "false"; }

inline std::string array_of(const std::vector<std::string>& items) {
  std::string out = "[";
  for (size_t i = 0; i < items.size(); ++i) {
    if (i) out += ",";
    out += items[i];
  }
  out += "]";
  return out;
}

inline std::string string_array(const std::vector<std::string>& items) {
  std::vector<std::string> quoted;
  quoted.reserve(items.size());
  for (const auto& item : items) quoted.push_back(quote(item));
  return array_of(quoted);
}

inline std::string number_array(const std::vector<double>& values) {
  std::vector<std::string> nums;
  nums.reserve(values.size());
  for (double v : values) nums.push_back(num(v));
  return array_of(nums);
}

inline std::string integer_array(const std::vector<long long>& values) {
  std::vector<std::string> nums;
  nums.reserve(values.size());
  for (long long v : values) nums.push_back(integer(v));
  return array_of(nums);
}

inline std::string size_array(const std::vector<std::size_t>& values) {
  std::vector<std::string> nums;
  nums.reserve(values.size());
  for (std::size_t v : values) nums.push_back(size(v));
  return array_of(nums);
}

inline std::string bool_array(const std::vector<bool>& values) {
  std::vector<std::string> bools;
  bools.reserve(values.size());
  for (bool v : values) bools.push_back(boolean(v));
  return array_of(bools);
}

struct Field {
  std::string name;
  std::string json;
};

inline Field field(const std::string& name, const std::string& raw_json) { return Field{name, raw_json}; }

inline Field string_field(const std::string& name, const std::string& value) { return field(name, quote(value)); }

inline Field number_field(const std::string& name, double value) { return field(name, num(value)); }

inline Field integer_field(const std::string& name, long long value) { return field(name, integer(value)); }

inline Field size_field(const std::string& name, std::size_t value) { return field(name, size(value)); }

inline Field bool_field(const std::string& name, bool value) { return field(name, boolean(value)); }

inline Field string_array_field(const std::string& name, const std::vector<std::string>& values) {
  return field(name, string_array(values));
}

inline Field number_array_field(const std::string& name, const std::vector<double>& values) {
  return field(name, number_array(values));
}

inline Field integer_array_field(const std::string& name, const std::vector<long long>& values) {
  return field(name, integer_array(values));
}

inline Field size_array_field(const std::string& name, const std::vector<std::size_t>& values) {
  return field(name, size_array(values));
}

inline Field bool_array_field(const std::string& name, const std::vector<bool>& values) {
  return field(name, bool_array(values));
}

inline std::string object(const std::vector<Field>& fields) {
  std::vector<std::string> entries;
  entries.reserve(fields.size());
  for (const auto& f : fields) entries.push_back(quote(f.name) + ":" + f.json);
  const std::string array_json = array_of(entries);
  return "{" + array_json.substr(1, array_json.size() - 2) + "}";
}

inline void append_field(std::vector<Field>& fields, const std::string& name, const std::string& raw_json) {
  fields.push_back(field(name, raw_json));
}

inline std::string metadata_or_empty(const std::string& metadata_json) {
  return metadata_json.empty() ? "{}" : metadata_json;
}

inline bool write_file(const std::filesystem::path& path, const std::string& contents) {
  std::ofstream out(path, std::ios::binary);
  if (!out) return false;
  out << contents;
  return static_cast<bool>(out);
}

// Build a JSON object payload for a record. Callers may compose their own.
inline std::string vector_payload(const std::vector<double>& values) {
  return object({string_field("kind", "vector"), number_array_field("values", values)});
}

inline std::string series_payload(const std::vector<double>& series, double sample_rate_hz = 0) {
  std::vector<Field> fields{string_field("kind", "time_series"), number_array_field("series", series)};
  if (sample_rate_hz > 0) append_field(fields, "sample_rate_hz", num(sample_rate_hz));
  return object(fields);
}

inline std::string string_payload(const std::string& text) {
  return object({string_field("kind", "string"), string_field("text", text)});
}

struct Edge {
  std::string row_id;
  std::string column_id;
  double value;
};

struct ScalarValue {
  std::string record_id;
  double value;
};

struct CategoricalValue {
  std::string record_id;
  std::string value;
};

struct Position {
  std::string record_id;
  std::vector<double> position;
};

struct GraphEdge {
  std::string source_id;
  std::string target_id;
  double weight{1.0};
};

class Document {
 public:
  Document& dataset(const std::string& id, const std::string& title, const std::string& description,
                    const std::string& source, const std::string& license) {
    datasets_.push_back(object({string_field("id", id), string_field("title", title),
                                string_field("description", description), string_field("source", source),
                                string_field("license", license)}));
    return *this;
  }

  Document& record(const std::string& id, const std::string& dataset_id, const std::string& record_type,
                   const std::string& label, const std::string& payload_json,
                   const std::string& metadata_json = "{}") {
    records_.push_back(object({string_field("id", id), string_field("dataset_id", dataset_id),
                               string_field("record_type", record_type), string_field("label", label),
                               field("payload", payload_json), field("metadata", metadata_or_empty(metadata_json))}));
    return *this;
  }

  Document& metric_relation(const std::string& id, const std::string& dataset_id, const std::string& name,
                            const std::vector<std::string>& record_ids, const std::vector<Edge>& edges,
                            const std::string& storage = "sparse_edge_list",
                            const std::string& metadata_json = "{}") {
    std::vector<std::string> values;
    values.reserve(edges.size());
    for (const auto& edge : edges) {
      values.push_back(object({string_field("row_id", edge.row_id), string_field("column_id", edge.column_id),
                               number_field("value", edge.value)}));
    }
    relations_.push_back(object({string_field("id", id), string_field("dataset_id", dataset_id),
                                 string_field("name", name), string_field("relation_type", "metric"),
                                 string_field("value_type", "scalar"), string_array_field("record_ids", record_ids),
                                 string_field("storage", storage), field("values", array_of(values)),
                                 field("metadata", metadata_or_empty(metadata_json))}));
    return *this;
  }

  Document& space(const std::string& id, const std::string& dataset_id, const std::vector<std::string>& record_ids,
                  const std::string& primary_relation_id, const std::string& space_type,
                  const std::string& metadata_json = "{}") {
    spaces_.push_back(object({string_field("id", id), string_field("dataset_id", dataset_id),
                              string_array_field("record_ids", record_ids),
                              string_field("primary_relation_id", primary_relation_id),
                              string_field("space_type", space_type),
                              field("metadata", metadata_or_empty(metadata_json))}));
    return *this;
  }

  Document& scalar_property(const std::string& id, const std::string& dataset_id, const std::string& name,
                            const std::vector<ScalarValue>& values, const std::string& metadata_json = "{}") {
    std::vector<std::string> entries;
    entries.reserve(values.size());
    for (const auto& value : values) {
      entries.push_back(object({string_field("record_id", value.record_id), number_field("value", value.value)}));
    }
    properties_.push_back(object({string_field("id", id), string_field("dataset_id", dataset_id),
                                  string_field("name", name), string_field("target_type", "record"),
                                  string_field("value_type", "scalar"), field("values", array_of(entries)),
                                  field("metadata", metadata_or_empty(metadata_json))}));
    return *this;
  }

  Document& categorical_property(const std::string& id, const std::string& dataset_id, const std::string& name,
                                 const std::vector<CategoricalValue>& values,
                                 const std::string& metadata_json = "{}") {
    std::vector<std::string> entries;
    entries.reserve(values.size());
    for (const auto& value : values) {
      entries.push_back(object({string_field("record_id", value.record_id), string_field("value", value.value)}));
    }
    properties_.push_back(object({string_field("id", id), string_field("dataset_id", dataset_id),
                                  string_field("name", name), string_field("target_type", "record"),
                                  string_field("value_type", "categorical"), field("values", array_of(entries)),
                                  field("metadata", metadata_or_empty(metadata_json))}));
    return *this;
  }

  Document& coordinates(const std::string& id, const std::string& dataset_id, const std::string& space_id,
                        const std::string& name, std::size_t dimension, const std::vector<Position>& positions,
                        const std::string& metadata_json = "{}") {
    std::vector<std::string> entries;
    entries.reserve(positions.size());
    for (const auto& p : positions) {
      entries.push_back(object({string_field("record_id", p.record_id), number_array_field("position", p.position)}));
    }
    coordinates_.push_back(object({string_field("id", id), string_field("dataset_id", dataset_id),
                                   string_field("space_id", space_id), string_field("name", name),
                                   size_field("dimension", dimension), field("record_positions", array_of(entries)),
                                   field("metadata", metadata_or_empty(metadata_json))}));
    return *this;
  }

  Document& coordinates3(const std::string& id, const std::string& dataset_id, const std::string& space_id,
                         const std::string& name, const std::vector<Position>& positions,
                         const std::string& metadata_json = "{}") {
    return coordinates(id, dataset_id, space_id, name, 3, positions, metadata_json);
  }

  Document& graph(const std::string& id, const std::string& dataset_id, const std::vector<std::string>& node_record_ids,
                  const std::string& edge_relation_id, const std::string& graph_type,
                  const std::vector<GraphEdge>& edges, const std::string& metadata_json = "{}") {
    std::vector<std::string> entries;
    entries.reserve(edges.size());
    for (const auto& edge : edges) {
      entries.push_back(object({string_field("source_id", edge.source_id), string_field("target_id", edge.target_id),
                                number_field("weight", edge.weight)}));
    }
    graphs_.push_back(object({string_field("id", id), string_field("dataset_id", dataset_id),
                              string_array_field("node_record_ids", node_record_ids),
                              string_field("edge_relation_id", edge_relation_id),
                              string_field("graph_type", graph_type), field("edges", array_of(entries)),
                              field("metadata", metadata_or_empty(metadata_json))}));
    return *this;
  }

  Document& diagnostic(const std::string& id, const std::string& dataset_id, const std::string& kind,
                       const std::string& payload_json) {
    diagnostics_.push_back(object({string_field("id", id), string_field("dataset_id", dataset_id),
                                   string_field("diagnostic_type", kind), field("payload", payload_json)}));
    return *this;
  }

  Document& view_json(const std::string& view_json) {
    views_.push_back(view_json);
    return *this;
  }

  Document& timeline_json(const std::string& timeline_json) {
    timelines_.push_back(timeline_json);
    return *this;
  }

  bool write_file(const std::filesystem::path& path) const {
    return mtrc::visual::write_file(path, to_json());
  }

  bool write_file(const std::string& path) const {
    return write_file(std::filesystem::path(path));
  }

  bool write_file(const char* path) const {
    return write_file(std::filesystem::path(path));
  }

  std::string to_json() const {
    std::string out = "{\"schema\":\"metric.visual.v1\",\"provenance\":{\"writer\":\"mtrc::visual\"}";
    out += ",\"datasets\":" + array_of(datasets_);
    out += ",\"records\":" + array_of(records_);
    out += ",\"relations\":" + array_of(relations_);
    out += ",\"spaces\":" + array_of(spaces_);
    out += ",\"properties\":" + array_of(properties_);
    out += ",\"graphs\":" + array_of(graphs_);
    out += ",\"coordinates\":" + array_of(coordinates_);
    out += ",\"timelines\":" + array_of(timelines_);
    out += ",\"events\":[]";
    out += ",\"views\":" + array_of(views_);
    out += ",\"diagnostics\":" + array_of(diagnostics_);
    out += "}";
    return out;
  }

 private:
  std::vector<std::string> datasets_, records_, relations_, spaces_, properties_, graphs_, coordinates_, timelines_,
      views_, diagnostics_;
};

}  // namespace visual
}  // namespace mtrc
