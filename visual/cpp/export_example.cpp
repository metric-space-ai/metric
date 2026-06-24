// Minimal example exporter for metric.visual.v1 using mtrc::visual.
//
// Builds a tiny finite metric space (already-computed synthetic values) and
// prints the document to stdout. The output loads directly in METRIC Visual and
// passes the JS schema validator (see visual/tools/check-cpp-export.mjs).
//
// Build & run:
//   c++ -std=c++17 -Ivisual/cpp visual/cpp/export_example.cpp -o /tmp/mtrc_export
//   /tmp/mtrc_export
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "mtrc_visual.hpp"

int main() {
  using namespace mtrc::visual;

  const int n = 8;
  const std::string ds = "demo";
  std::vector<std::string> ids;
  std::vector<std::vector<double>> features;

  Document doc;
  doc.dataset(ds, "Header-only export demo", "A tiny finite metric space exported from C++.",
              "synthetic", "CC0-1.0");

  for (int i = 0; i < n; ++i) {
    std::string id = "r" + std::to_string(i);
    ids.push_back(id);
    double a = std::sin(i * 0.7);
    double b = std::cos(i * 0.5);
    std::vector<double> feature = {a, b, static_cast<double>(i) / n};
    features.push_back(feature);
    doc.record(id, ds, "vector", "obs " + std::to_string(i), vector_payload(feature));
  }

  // k-nearest edges over the already-computed feature vectors (k = 3).
  std::vector<Edge> edges;
  for (int i = 0; i < n; ++i) {
    std::vector<std::pair<double, int>> dist;
    for (int j = 0; j < n; ++j) {
      if (i == j) continue;
      double s = 0;
      for (size_t d = 0; d < features[i].size(); ++d) {
        double delta = features[i][d] - features[j][d];
        s += delta * delta;
      }
      dist.push_back({std::sqrt(s), j});
    }
    std::sort(dist.begin(), dist.end());
    for (int k = 0; k < 3 && k < static_cast<int>(dist.size()); ++k) {
      edges.push_back(Edge{ids[i], ids[dist[k].second], dist[k].first});
    }
  }

  doc.metric_relation("metric", ds, "feature metric", ids, edges);
  doc.space("space", ds, ids, "metric", "finite_metric_space");

  std::vector<Position> positions;
  std::vector<ScalarValue> entropy;
  for (int i = 0; i < n; ++i) {
    positions.push_back(Position{ids[i], {features[i][0], features[i][2], features[i][1]}});
    entropy.push_back(ScalarValue{ids[i], 0.4 + 0.5 * std::fabs(features[i][0])});
  }
  doc.coordinates3("landmark-3d", ds, "space", "metric-space lift", positions);
  doc.scalar_property("entropy", ds, "local entropy", entropy);

  std::printf("%s\n", doc.to_json().c_str());
  return 0;
}
