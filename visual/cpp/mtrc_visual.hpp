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

#include <string>
#include <vector>
#include <sstream>
#include <cmath>
#include <cstdio>

namespace mtrc {
namespace visual {

inline std::string escape(const std::string& s) {
  std::string out;
  out.reserve(s.size() + 8);
  for (char c : s) {
    switch (c) {
      case '"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        if (static_cast<unsigned char>(c) < 0x20) {
          char buf[8];
          std::snprintf(buf, sizeof(buf), "\\u%04x", c);
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
  ss.precision(6);
  ss << v;
  return ss.str();
}

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

// Build a JSON object payload for a record. Callers may compose their own.
inline std::string vector_payload(const std::vector<double>& values) {
  return "{\"kind\":\"vector\",\"values\":" + number_array(values) + "}";
}

inline std::string series_payload(const std::vector<double>& series, double sample_rate_hz = 0) {
  std::string out = "{\"kind\":\"time_series\",\"series\":" + number_array(series);
  if (sample_rate_hz > 0) out += ",\"sample_rate_hz\":" + num(sample_rate_hz);
  out += "}";
  return out;
}

inline std::string string_payload(const std::string& text) {
  return "{\"kind\":\"string\",\"text\":" + quote(text) + "}";
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
    datasets_.push_back("{\"id\":" + quote(id) + ",\"title\":" + quote(title) +
                        ",\"description\":" + quote(description) + ",\"source\":" + quote(source) +
                        ",\"license\":" + quote(license) + "}");
    return *this;
  }

  Document& record(const std::string& id, const std::string& dataset_id, const std::string& record_type,
                   const std::string& label, const std::string& payload_json) {
    records_.push_back("{\"id\":" + quote(id) + ",\"dataset_id\":" + quote(dataset_id) +
                       ",\"record_type\":" + quote(record_type) + ",\"label\":" + quote(label) +
                       ",\"payload\":" + payload_json + "}");
    return *this;
  }

  Document& metric_relation(const std::string& id, const std::string& dataset_id, const std::string& name,
                            const std::vector<std::string>& record_ids, const std::vector<Edge>& edges,
                            const std::string& storage = "sparse_edge_list") {
    std::vector<std::string> values;
    values.reserve(edges.size());
    for (const auto& edge : edges) {
      values.push_back("{\"row_id\":" + quote(edge.row_id) + ",\"column_id\":" + quote(edge.column_id) +
                       ",\"value\":" + num(edge.value) + "}");
    }
    relations_.push_back("{\"id\":" + quote(id) + ",\"dataset_id\":" + quote(dataset_id) +
                         ",\"name\":" + quote(name) + ",\"relation_type\":\"metric\",\"value_type\":\"scalar\"" +
                         ",\"record_ids\":" + string_array(record_ids) + ",\"storage\":" + quote(storage) +
                         ",\"values\":" + array_of(values) + "}");
    return *this;
  }

  Document& space(const std::string& id, const std::string& dataset_id, const std::vector<std::string>& record_ids,
                  const std::string& primary_relation_id, const std::string& space_type) {
    spaces_.push_back("{\"id\":" + quote(id) + ",\"dataset_id\":" + quote(dataset_id) +
                      ",\"record_ids\":" + string_array(record_ids) + ",\"primary_relation_id\":" +
                      quote(primary_relation_id) + ",\"space_type\":" + quote(space_type) + ",\"metadata\":{}}");
    return *this;
  }

  Document& scalar_property(const std::string& id, const std::string& dataset_id, const std::string& name,
                            const std::vector<ScalarValue>& values) {
    std::vector<std::string> entries;
    entries.reserve(values.size());
    for (const auto& value : values) {
      entries.push_back("{\"record_id\":" + quote(value.record_id) + ",\"value\":" + num(value.value) + "}");
    }
    properties_.push_back("{\"id\":" + quote(id) + ",\"dataset_id\":" + quote(dataset_id) +
                          ",\"name\":" + quote(name) + ",\"target_type\":\"record\",\"value_type\":\"scalar\"" +
                          ",\"values\":" + array_of(entries) + "}");
    return *this;
  }

  Document& categorical_property(const std::string& id, const std::string& dataset_id, const std::string& name,
                                 const std::vector<CategoricalValue>& values) {
    std::vector<std::string> entries;
    entries.reserve(values.size());
    for (const auto& value : values) {
      entries.push_back("{\"record_id\":" + quote(value.record_id) + ",\"value\":" + quote(value.value) + "}");
    }
    properties_.push_back("{\"id\":" + quote(id) + ",\"dataset_id\":" + quote(dataset_id) +
                          ",\"name\":" + quote(name) + ",\"target_type\":\"record\",\"value_type\":\"categorical\"" +
                          ",\"values\":" + array_of(entries) + "}");
    return *this;
  }

  Document& coordinates(const std::string& id, const std::string& dataset_id, const std::string& space_id,
                        const std::string& name, std::size_t dimension, const std::vector<Position>& positions) {
    std::vector<std::string> entries;
    entries.reserve(positions.size());
    for (const auto& p : positions) {
      entries.push_back("{\"record_id\":" + quote(p.record_id) + ",\"position\":" + number_array(p.position) + "}");
    }
    coordinates_.push_back("{\"id\":" + quote(id) + ",\"dataset_id\":" + quote(dataset_id) +
                           ",\"space_id\":" + quote(space_id) + ",\"name\":" + quote(name) +
                           ",\"dimension\":" + std::to_string(dimension) + ",\"record_positions\":" +
                           array_of(entries) + "}");
    return *this;
  }

  Document& coordinates3(const std::string& id, const std::string& dataset_id, const std::string& space_id,
                         const std::string& name, const std::vector<Position>& positions) {
    return coordinates(id, dataset_id, space_id, name, 3, positions);
  }

  Document& graph(const std::string& id, const std::string& dataset_id, const std::vector<std::string>& node_record_ids,
                  const std::string& edge_relation_id, const std::string& graph_type,
                  const std::vector<GraphEdge>& edges) {
    std::vector<std::string> entries;
    entries.reserve(edges.size());
    for (const auto& edge : edges) {
      entries.push_back("{\"source_id\":" + quote(edge.source_id) + ",\"target_id\":" + quote(edge.target_id) +
                        ",\"weight\":" + num(edge.weight) + "}");
    }
    graphs_.push_back("{\"id\":" + quote(id) + ",\"dataset_id\":" + quote(dataset_id) +
                      ",\"node_record_ids\":" + string_array(node_record_ids) + ",\"edge_relation_id\":" +
                      quote(edge_relation_id) + ",\"graph_type\":" + quote(graph_type) + ",\"edges\":" +
                      array_of(entries) + "}");
    return *this;
  }

  Document& diagnostic(const std::string& id, const std::string& dataset_id, const std::string& kind,
                       const std::string& payload_json) {
    diagnostics_.push_back("{\"id\":" + quote(id) + ",\"dataset_id\":" + quote(dataset_id) +
                           ",\"diagnostic_type\":" + quote(kind) + ",\"payload\":" + payload_json + "}");
    return *this;
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
    out += ",\"timelines\":[]";
    out += ",\"events\":[]";
    out += ",\"views\":" + array_of(views_);
    out += ",\"diagnostics\":" + array_of(diagnostics_);
    out += "}";
    return out;
  }

 private:
  std::vector<std::string> datasets_, records_, relations_, spaces_, properties_, graphs_, coordinates_, views_,
      diagnostics_;
};

}  // namespace visual
}  // namespace mtrc
