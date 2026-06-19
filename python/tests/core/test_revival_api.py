import os
import subprocess
import sys
import unittest
from pathlib import Path

import metric
import numpy as np
from metric.metrics import Edit, available
from metric import mappings, transforms
from metric.operators import (
    GraphDegreeDiagnostics,
    GraphConstructionMetadata,
    GraphConstructionResult,
    coverage_representative_indices,
    coverage_representatives,
    exact_knn_graph,
    exact_knn_graph_edges,
    exact_radius_graph,
    exact_radius_graph_edges,
    graph_degree_diagnostics,
    intrinsic_dimension,
    medoid,
    medoid_index,
    nearest_neighbors,
    pairwise_distance_matrix,
    prune_graph_out_degree,
    range_neighbors,
    representative_indices,
    representatives,
    separated_representative_indices,
    separated_representatives,
    symmetrize_graph,
)
from metric.spaces import FiniteMetricSpace, MatrixSpace, Space


class RevivalApiTest(unittest.TestCase):
    def setUp(self):
        self.records = ["cat", "cot", "coat", "dog"]
        self.metric = Edit()

    def assertMetricContracts(self, records, metric, places=7):
        for lhs in records:
            self.assertAlmostEqual(metric(lhs, lhs), 0.0, places=places)

            for rhs in records:
                lhs_rhs = metric(lhs, rhs)
                rhs_lhs = metric(rhs, lhs)

                self.assertGreaterEqual(lhs_rhs, 0.0)
                self.assertAlmostEqual(lhs_rhs, rhs_lhs, places=places)

                for through in records:
                    self.assertLessEqual(
                        metric(lhs, through),
                        lhs_rhs + metric(rhs, through) + 10 ** -places,
                    )

    def test_metric_concepts_are_importable(self):
        self.assertIn("Edit", available())
        self.assertIs(MatrixSpace, FiniteMetricSpace)
        self.assertIs(metric.metrics.Edit, Edit)
        self.assertIs(metric.spaces.FiniteMetricSpace, FiniteMetricSpace)
        self.assertIs(metric.operators.nearest_neighbors, nearest_neighbors)
        self.assertIs(metric.operators.range_neighbors, range_neighbors)
        self.assertIs(metric.operators.GraphDegreeDiagnostics, GraphDegreeDiagnostics)
        self.assertIs(metric.operators.GraphConstructionMetadata, GraphConstructionMetadata)
        self.assertIs(metric.operators.GraphConstructionResult, GraphConstructionResult)
        self.assertIs(metric.operators.exact_knn_graph, exact_knn_graph)
        self.assertIs(metric.operators.exact_knn_graph_edges, exact_knn_graph_edges)
        self.assertIs(metric.operators.exact_radius_graph, exact_radius_graph)
        self.assertIs(metric.operators.exact_radius_graph_edges, exact_radius_graph_edges)
        self.assertIs(metric.operators.graph_degree_diagnostics, graph_degree_diagnostics)
        self.assertIs(metric.operators.intrinsic_dimension, intrinsic_dimension)
        self.assertIs(metric.operators.prune_graph_out_degree, prune_graph_out_degree)
        self.assertIs(metric.operators.medoid_index, medoid_index)
        self.assertIs(metric.operators.medoid, medoid)
        self.assertIs(metric.operators.representative_indices, representative_indices)
        self.assertIs(metric.operators.representatives, representatives)
        self.assertIs(metric.operators.separated_representative_indices, separated_representative_indices)
        self.assertIs(metric.operators.separated_representatives, separated_representatives)
        self.assertIs(metric.operators.symmetrize_graph, symmetrize_graph)
        self.assertIs(metric.operators.coverage_representative_indices, coverage_representative_indices)
        self.assertIs(metric.operators.coverage_representatives, coverage_representatives)
        self.assertIs(metric.mappings, mappings)
        self.assertIs(metric.transforms, transforms)
        self.assertIs(metric.Edit, Edit)
        self.assertIs(metric.GraphDegreeDiagnostics, GraphDegreeDiagnostics)
        self.assertIs(metric.GraphConstructionMetadata, GraphConstructionMetadata)
        self.assertIs(metric.GraphConstructionResult, GraphConstructionResult)
        self.assertIs(metric.exact_knn_graph, exact_knn_graph)
        self.assertIs(metric.exact_knn_graph_edges, exact_knn_graph_edges)
        self.assertIs(metric.exact_radius_graph, exact_radius_graph)
        self.assertIs(metric.exact_radius_graph_edges, exact_radius_graph_edges)
        self.assertIs(metric.graph_degree_diagnostics, graph_degree_diagnostics)
        self.assertIs(metric.intrinsic_dimension, intrinsic_dimension)
        self.assertIs(metric.prune_graph_out_degree, prune_graph_out_degree)
        self.assertIs(metric.medoid_index, medoid_index)
        self.assertIs(metric.medoid, medoid)
        self.assertIs(metric.range_neighbors, range_neighbors)
        self.assertIs(metric.representative_indices, representative_indices)
        self.assertIs(metric.representatives, representatives)
        self.assertIs(metric.separated_representative_indices, separated_representative_indices)
        self.assertIs(metric.separated_representatives, separated_representatives)
        self.assertIs(metric.symmetrize_graph, symmetrize_graph)
        self.assertIs(metric.coverage_representative_indices, coverage_representative_indices)
        self.assertIs(metric.coverage_representatives, coverage_representatives)
        self.assertIs(metric.FiniteMetricSpace, FiniteMetricSpace)
        self.assertIs(metric.Space, Space)
        self.assertIn("FiniteMetricSpace", metric.__all__)
        self.assertIn("Space", metric.__all__)
        self.assertIn("mappings", metric.__all__)
        self.assertIn("transforms", metric.__all__)
        self.assertIn("GraphDegreeDiagnostics", metric.__all__)
        self.assertIn("GraphConstructionMetadata", metric.__all__)
        self.assertIn("GraphConstructionResult", metric.__all__)
        self.assertIn("exact_knn_graph", metric.__all__)
        self.assertIn("exact_knn_graph_edges", metric.__all__)
        self.assertIn("exact_radius_graph", metric.__all__)
        self.assertIn("exact_radius_graph_edges", metric.__all__)
        self.assertIn("graph_degree_diagnostics", metric.__all__)
        self.assertIn("prune_graph_out_degree", metric.__all__)
        self.assertIn("medoid_index", metric.__all__)
        self.assertIn("medoid", metric.__all__)
        self.assertIn("representative_indices", metric.__all__)
        self.assertIn("representatives", metric.__all__)
        self.assertIn("separated_representative_indices", metric.__all__)
        self.assertIn("separated_representatives", metric.__all__)
        self.assertIn("symmetrize_graph", metric.__all__)
        self.assertIn("coverage_representative_indices", metric.__all__)
        self.assertIn("coverage_representatives", metric.__all__)
        self.assertEqual(mappings.STABILITY, "beta")
        self.assertEqual(transforms.STABILITY, "beta")
        self.assertIsInstance(mappings.available(), tuple)
        self.assertIsInstance(transforms.available(), tuple)

    def test_promoted_metric_space_examples_run(self):
        examples_dir = Path(__file__).resolve().parents[2] / "examples" / "metric_space"
        examples = sorted(examples_dir.glob("*.py"))

        self.assertGreaterEqual(len(examples), 1)
        for example in examples:
            with self.subTest(example=example.name):
                result = subprocess.run(
                    [sys.executable, str(example)],
                    check=False,
                    env={**os.environ, "PYTHONDONTWRITEBYTECODE": "1"},
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    text=True,
                )
                self.assertEqual(result.returncode, 0, result.stdout)

    def test_finite_metric_space_caches_pairwise_distances(self):
        space = FiniteMetricSpace(self.records, self.metric)

        self.assertEqual(len(space), len(self.records))
        self.assertEqual(space[0], "cat")
        self.assertEqual(space(0, 1), 1)
        self.assertEqual(space.distance(0, 2), 1)

        distances = space.pairwise_distances()
        distances[0][1] = 999
        self.assertEqual(space(0, 1), 1)

    def test_nearest_neighbor_helpers_use_record_ids(self):
        space = FiniteMetricSpace(self.records, self.metric)

        self.assertEqual(space.knn("cut", 2), [(0, 1), (1, 1)])
        self.assertEqual(space.nn("cut"), (0, 1))
        self.assertEqual(space.rnn("cut", 1), [(0, 1), (1, 1)])
        self.assertEqual(nearest_neighbors(self.records, self.metric, "cut", 2), [(0, 1), (1, 1)])
        self.assertEqual(range_neighbors(self.records, self.metric, "cut", 1), [(0, 1), (1, 1)])
        self.assertEqual(pairwise_distance_matrix(self.records, self.metric)[0][1], 1)

    def test_intrinsic_dimension_estimates_distance_growth(self):
        records = [0, 1, 2, 3, 4]

        def absolute_distance(lhs, rhs):
            return abs(lhs - rhs)

        self.assertAlmostEqual(intrinsic_dimension(records, absolute_distance), np.log2(5.0 / 3.0))

    def test_representative_selection_uses_farthest_first_traversal(self):
        records = [
            (1.0, 0.0, 0.0, 0.0),
            (0.0, 1.0, 0.0, 0.0),
            (0.0, 0.0, 0.0, 1.0),
            (0.5, 0.5, 0.0, 0.0),
            (0.0, 0.5, 0.5, 0.0),
        ]

        def cumulative_transport_distance(lhs, rhs):
            cumulative_delta = 0.0
            distance = 0.0
            for lhs_mass, rhs_mass in zip(lhs, rhs):
                cumulative_delta += lhs_mass - rhs_mass
                distance += abs(cumulative_delta)

            return distance

        self.assertEqual(representative_indices(records, cumulative_transport_distance, 3), [0, 2, 4])
        self.assertEqual(
            representatives(records, cumulative_transport_distance, 3),
            [records[0], records[2], records[4]],
        )
        self.assertEqual(medoid_index(records, cumulative_transport_distance), 1)
        self.assertEqual(medoid(records, cumulative_transport_distance), records[1])
        self.assertEqual(separated_representative_indices(records, cumulative_transport_distance, 1.5), [0, 2, 4])
        self.assertEqual(
            separated_representatives(records, cumulative_transport_distance, 1.5),
            [records[0], records[2], records[4]],
        )
        self.assertEqual(
            exact_knn_graph_edges(records, cumulative_transport_distance, 1),
            [(0, 3, 0.5), (1, 3, 0.5), (2, 4, 1.5), (3, 0, 0.5), (4, 1, 0.5)],
        )
        knn_graph = exact_knn_graph(records, cumulative_transport_distance, 1)
        self.assertIsInstance(knn_graph, GraphConstructionResult)
        self.assertEqual(list(knn_graph.edges), exact_knn_graph_edges(records, cumulative_transport_distance, 1))
        self.assertIsInstance(knn_graph.metadata, GraphConstructionMetadata)
        self.assertEqual(knn_graph.metadata.strategy, "exact_knn")
        self.assertEqual(knn_graph.metadata.record_count, len(records))
        self.assertEqual(knn_graph.metadata.edge_count, len(knn_graph.edges))
        self.assertTrue(knn_graph.metadata.directed)
        self.assertFalse(knn_graph.metadata.self_loops)
        self.assertTrue(knn_graph.metadata.exact)
        self.assertEqual(knn_graph.metadata.k, 1)
        self.assertIsNone(knn_graph.metadata.radius)
        self.assertEqual(knn_graph.metadata.edge_payload, "metric_distance")
        self.assertEqual(knn_graph.metadata.weighting, "none")
        self.assertIsNone(knn_graph.metadata.max_out_degree)
        self.assertEqual(knn_graph.metadata.sparsification, "none")
        self.assertEqual(knn_graph.metadata.symmetrization, "none")
        self.assertEqual(knn_graph.metadata.normalization, "none")
        self.assertEqual(knn_graph.metadata.tie_break, "distance_then_target_index")

        wide_knn_graph = exact_knn_graph(records, cumulative_transport_distance, 2)
        pruned_graph = prune_graph_out_degree(wide_knn_graph, max_out_degree=1)
        self.assertEqual(
            list(pruned_graph.edges),
            [(0, 3, 0.5), (1, 3, 0.5), (2, 4, 1.5), (3, 0, 0.5), (4, 1, 0.5)],
        )
        self.assertTrue(pruned_graph.metadata.directed)
        self.assertEqual(pruned_graph.metadata.edge_count, len(pruned_graph.edges))
        self.assertEqual(pruned_graph.metadata.k, 2)
        self.assertEqual(pruned_graph.metadata.max_out_degree, 1)
        self.assertEqual(pruned_graph.metadata.sparsification, "out_degree")
        self.assertEqual(pruned_graph.metadata.tie_break, "source_index_then_distance_then_target_index")
        self.assertEqual(prune_graph_out_degree(wide_knn_graph, 0).edges, ())

        degree_diagnostics = graph_degree_diagnostics(knn_graph)
        self.assertIsInstance(degree_diagnostics, GraphDegreeDiagnostics)
        self.assertEqual(degree_diagnostics.record_count, len(records))
        self.assertEqual(degree_diagnostics.edge_count, len(knn_graph.edges))
        self.assertTrue(degree_diagnostics.directed)
        self.assertEqual(degree_diagnostics.out_degrees, (1, 1, 1, 1, 1))
        self.assertEqual(degree_diagnostics.in_degrees, (1, 1, 0, 2, 1))
        self.assertEqual(degree_diagnostics.degrees, (2, 2, 1, 3, 2))
        self.assertEqual(degree_diagnostics.isolated_count, 0)
        self.assertEqual(degree_diagnostics.max_degree, 3)
        self.assertAlmostEqual(degree_diagnostics.average_degree, 2.0)
        self.assertEqual(degree_diagnostics.degree_policy, "directed_in_out")

        union_graph = symmetrize_graph(knn_graph, policy="union", weighting="minimum_distance")
        self.assertEqual(
            list(union_graph.edges),
            [(0, 3, 0.5), (1, 3, 0.5), (1, 4, 0.5), (2, 4, 1.5)],
        )
        self.assertFalse(union_graph.metadata.directed)
        self.assertEqual(union_graph.metadata.edge_count, len(union_graph.edges))
        self.assertEqual(union_graph.metadata.weighting, "minimum_distance")
        self.assertEqual(union_graph.metadata.sparsification, "none")
        self.assertEqual(union_graph.metadata.symmetrization, "union")
        self.assertEqual(union_graph.metadata.tie_break, "source_index_then_target_index")
        with self.assertRaises(ValueError):
            prune_graph_out_degree(union_graph, 1)

        undirected_diagnostics = graph_degree_diagnostics(union_graph)
        self.assertFalse(undirected_diagnostics.directed)
        self.assertEqual(undirected_diagnostics.degrees, (1, 2, 1, 2, 2))
        self.assertEqual(undirected_diagnostics.out_degrees, (0, 0, 0, 0, 0))
        self.assertEqual(undirected_diagnostics.in_degrees, (0, 0, 0, 0, 0))
        self.assertEqual(undirected_diagnostics.edge_count, len(union_graph.edges))
        self.assertEqual(undirected_diagnostics.isolated_count, 0)
        self.assertEqual(undirected_diagnostics.max_degree, 2)
        self.assertAlmostEqual(undirected_diagnostics.average_degree, 1.6)
        self.assertEqual(undirected_diagnostics.degree_policy, "undirected_endpoint")

        mutual_graph = symmetrize_graph(knn_graph, policy="mutual", weighting="minimum_distance")
        self.assertEqual(list(mutual_graph.edges), [(0, 3, 0.5)])
        self.assertEqual(mutual_graph.metadata.symmetrization, "mutual")

        asymmetric_graph = GraphConstructionResult(
            edges=((0, 1, 5), (1, 0, 3), (1, 2, 4)),
            metadata=GraphConstructionMetadata(
                strategy="synthetic",
                record_count=3,
                edge_count=3,
                directed=True,
                self_loops=False,
                exact=True,
                edge_payload="metric_distance",
                weighting="none",
                sparsification="none",
                symmetrization="none",
                normalization="none",
            ),
        )
        self.assertEqual(
            list(symmetrize_graph(asymmetric_graph, policy="union", weighting="minimum_distance").edges),
            [(0, 1, 3), (1, 2, 4)],
        )
        self.assertEqual(
            list(symmetrize_graph(asymmetric_graph, policy="union", weighting="maximum_distance").edges),
            [(0, 1, 5), (1, 2, 4)],
        )
        with self.assertRaises(ValueError):
            symmetrize_graph(asymmetric_graph, policy="invalid", weighting="minimum_distance")
        with self.assertRaises(ValueError):
            symmetrize_graph(asymmetric_graph, policy="union", weighting="average_distance")

        invalid_graph = GraphConstructionResult(
            edges=((0, 3, 1),),
            metadata=GraphConstructionMetadata(
                strategy="synthetic",
                record_count=2,
                edge_count=1,
                directed=True,
                self_loops=False,
                exact=True,
            ),
        )
        with self.assertRaises(ValueError):
            graph_degree_diagnostics(invalid_graph)

        self.assertEqual(
            exact_radius_graph_edges(records, cumulative_transport_distance, 0.5),
            [(0, 3, 0.5), (1, 3, 0.5), (1, 4, 0.5), (3, 0, 0.5), (3, 1, 0.5), (4, 1, 0.5)],
        )
        radius_graph = exact_radius_graph(records, cumulative_transport_distance, 0.5)
        self.assertIsInstance(radius_graph, GraphConstructionResult)
        self.assertEqual(list(radius_graph.edges), exact_radius_graph_edges(records, cumulative_transport_distance, 0.5))
        self.assertEqual(radius_graph.metadata.strategy, "exact_radius")
        self.assertEqual(radius_graph.metadata.record_count, len(records))
        self.assertEqual(radius_graph.metadata.edge_count, len(radius_graph.edges))
        self.assertTrue(radius_graph.metadata.directed)
        self.assertFalse(radius_graph.metadata.self_loops)
        self.assertTrue(radius_graph.metadata.exact)
        self.assertIsNone(radius_graph.metadata.k)
        self.assertEqual(radius_graph.metadata.radius, 0.5)
        self.assertEqual(radius_graph.metadata.edge_payload, "metric_distance")
        self.assertEqual(radius_graph.metadata.weighting, "none")
        self.assertIsNone(radius_graph.metadata.max_out_degree)
        self.assertEqual(radius_graph.metadata.sparsification, "none")
        self.assertEqual(radius_graph.metadata.symmetrization, "none")
        self.assertEqual(radius_graph.metadata.normalization, "none")
        self.assertEqual(radius_graph.metadata.tie_break, "source_then_target_index")
        self.assertEqual(coverage_representative_indices(records, cumulative_transport_distance, 1.5), [0, 2])
        self.assertEqual(
            coverage_representatives(records, cumulative_transport_distance, 1.5),
            [records[0], records[2]],
        )
        self.assertEqual(representative_indices(records, cumulative_transport_distance, 0), [])

    def test_representative_selection_validates_inputs_and_deterministic_ties(self):
        records = [0, 1, 2, 10]

        def absolute_distance(lhs, rhs):
            return abs(lhs - rhs)

        self.assertEqual(representative_indices(records, absolute_distance, 3), [0, 3, 2])
        self.assertEqual(representative_indices(records, absolute_distance, 2, seed_index=1), [1, 3])
        self.assertEqual(medoid_index(records, absolute_distance), 1)
        self.assertEqual(medoid(records, absolute_distance), 1)
        self.assertEqual(separated_representative_indices(records, absolute_distance, 2), [0, 2, 3])
        self.assertEqual(separated_representatives(records, absolute_distance, 2), [0, 2, 10])
        self.assertEqual(separated_representative_indices([], absolute_distance, 2), [])
        self.assertEqual(
            exact_knn_graph_edges(records, absolute_distance, 2),
            [
                (0, 1, 1),
                (0, 2, 2),
                (1, 0, 1),
                (1, 2, 1),
                (2, 1, 1),
                (2, 0, 2),
                (3, 2, 8),
                (3, 1, 9),
            ],
        )
        self.assertEqual(
            exact_radius_graph_edges(records, absolute_distance, 2),
            [(0, 1, 1), (0, 2, 2), (1, 0, 1), (1, 2, 1), (2, 0, 2), (2, 1, 1)],
        )
        self.assertEqual(exact_knn_graph_edges(records, absolute_distance, 0), [])
        self.assertEqual(exact_knn_graph(records, absolute_distance, 0).edges, ())
        self.assertEqual(exact_knn_graph(records, absolute_distance, 0).metadata.k, 0)
        self.assertEqual(coverage_representative_indices(records, absolute_distance, 2), [0, 3])
        self.assertEqual(coverage_representatives(records, absolute_distance, 2), [0, 10])
        self.assertEqual(coverage_representative_indices(records, absolute_distance, 20), [0])
        self.assertEqual(coverage_representative_indices([], absolute_distance, 1), [])

        with self.assertRaises(TypeError):
            representative_indices(records, absolute_distance, 1.5)
        with self.assertRaises(TypeError):
            representative_indices(records, absolute_distance, 1, seed_index=0.5)
        with self.assertRaises(ValueError):
            representative_indices(records, absolute_distance, -1)
        with self.assertRaises(ValueError):
            representative_indices([], absolute_distance, 1)
        with self.assertRaises(ValueError):
            representative_indices(records, absolute_distance, 5)
        with self.assertRaises(ValueError):
            medoid_index([], absolute_distance)
        with self.assertRaises(ValueError):
            medoid([], absolute_distance)
        with self.assertRaises(ValueError):
            separated_representative_indices(records, absolute_distance, -1)
        with self.assertRaises(ValueError):
            exact_knn_graph_edges(records, absolute_distance, 4)
        with self.assertRaises(TypeError):
            exact_knn_graph_edges(records, absolute_distance, 1.5)
        with self.assertRaises(TypeError):
            prune_graph_out_degree(exact_knn_graph(records, absolute_distance, 1), 1.5)
        with self.assertRaises(ValueError):
            exact_radius_graph_edges(records, absolute_distance, -1)
        with self.assertRaises(ValueError):
            prune_graph_out_degree(exact_knn_graph(records, absolute_distance, 1), -1)
        with self.assertRaises(IndexError):
            representative_indices(records, absolute_distance, 1, seed_index=-1)
        with self.assertRaises(IndexError):
            representative_indices(records, absolute_distance, 1, seed_index=len(records))
        with self.assertRaises(ValueError):
            coverage_representative_indices(records, absolute_distance, -1)

    def test_edit_metric_satisfies_metric_contracts(self):
        self.assertMetricContracts(self.records, self.metric)

    def test_space_facade_exposes_intent_names(self):
        space = Space(self.records, self.metric)

        self.assertIsInstance(space, FiniteMetricSpace)
        self.assertEqual(space.neighbors("cut", 2), [(0, 1), (1, 1)])
        self.assertEqual(space.nearest("cut"), (0, 1))
        self.assertEqual(space.within_radius("cut", 1), [(0, 1), (1, 1)])

    def test_numpy_record_arrays_use_custom_metric_callable(self):
        records = np.array([[0.0, 0.0], [3.0, 4.0], [6.0, 8.0]])

        def euclidean(lhs, rhs):
            return float(np.linalg.norm(lhs - rhs))

        space = Space(records, euclidean)

        self.assertEqual(len(space), 3)
        self.assertAlmostEqual(space.distance(0, 1), 5.0)
        self.assertEqual(space.nearest(np.array([3.0, 4.0])), (1, 0.0))
        self.assertEqual(range_neighbors(records, euclidean, np.array([3.0, 4.0]), 0.0), [(1, 0.0)])
        self.assertAlmostEqual(pairwise_distance_matrix(records, euclidean)[1][2], 5.0)
        self.assertMetricContracts(records, euclidean)

    def test_structured_records_use_domain_metric_callable(self):
        records = [
            {"id": "pump-a", "status": "ok", "temperature_c": 62.0, "events": ("start", "load", "idle")},
            {"id": "pump-b", "status": "ok", "temperature_c": 64.5, "events": ("start", "load", "idle")},
            {"id": "valve-c", "status": "warn", "temperature_c": 82.0, "events": ("start", "alarm", "manual")},
            {"id": "pump-d", "status": "ok", "temperature_c": 61.0, "events": ("start", "load", "stop")},
        ]
        query = {"status": "ok", "temperature_c": 63.0, "events": ("start", "load", "idle")}

        def padded_hamming(lhs, rhs):
            width = max(len(lhs), len(rhs))
            padded_lhs = tuple(lhs) + (None,) * (width - len(lhs))
            padded_rhs = tuple(rhs) + (None,) * (width - len(rhs))
            return sum(left != right for left, right in zip(padded_lhs, padded_rhs))

        def structured_record_distance(lhs, rhs):
            status_penalty = 0.0 if lhs["status"] == rhs["status"] else 10.0
            temperature_penalty = abs(lhs["temperature_c"] - rhs["temperature_c"]) / 10.0
            event_penalty = padded_hamming(lhs["events"], rhs["events"])
            return status_penalty + temperature_penalty + event_penalty

        space = Space(records, structured_record_distance)

        nearest_id, nearest_distance = space.nearest(query)
        self.assertEqual(records[nearest_id]["id"], "pump-a")
        self.assertAlmostEqual(nearest_distance, 0.1)
        self.assertGreater(space.distance(0, 2), 10.0)
        self.assertEqual(space.neighbors(query, 2)[0][0], 0)
        self.assertAlmostEqual(pairwise_distance_matrix(records, structured_record_distance)[0][1], 0.25)
        self.assertEqual(representative_indices(records, structured_record_distance, 3), [0, 2, 3])
        self.assertEqual(
            [record["id"] for record in representatives(records, structured_record_distance, 3)],
            ["pump-a", "valve-c", "pump-d"],
        )
        self.assertEqual(separated_representative_indices(records, structured_record_distance, 2.0), [0, 2])
        self.assertEqual(
            [record["id"] for record in separated_representatives(records, structured_record_distance, 2.0)],
            ["pump-a", "valve-c"],
        )
        self.assertEqual(coverage_representative_indices(records, structured_record_distance, 1.5), [0, 2])
        self.assertEqual(
            [record["id"] for record in coverage_representatives(records, structured_record_distance, 1.5)],
            ["pump-a", "valve-c"],
        )
        self.assertMetricContracts(records, structured_record_distance)

    def test_time_series_records_use_alignment_metric_callable(self):
        gap_cost = 2.0
        records = [
            (0, 1, 1, 1, 2, 3),
            (0, 0, 1, 1, 1, 2, 3),
            (2, 2, 2, 2, 2, 2),
            (0, 1, 6, 1, 2, 3),
        ]
        query = (0, 1, 1, 1, 2, 4)

        def aligned_curve_distance(lhs, rhs):
            previous = [index * gap_cost for index in range(len(rhs) + 1)]
            for lhs_index, lhs_value in enumerate(lhs, start=1):
                current = [lhs_index * gap_cost] + [0.0] * len(rhs)
                for rhs_index, rhs_value in enumerate(rhs, start=1):
                    substitute = previous[rhs_index - 1] + min(
                        abs(lhs_value - rhs_value),
                        2 * gap_cost,
                    )
                    delete = previous[rhs_index] + gap_cost
                    insert = current[rhs_index - 1] + gap_cost
                    current[rhs_index] = min(substitute, delete, insert)
                previous = current

            return previous[-1]

        space = Space(records, aligned_curve_distance)

        self.assertEqual(space.nearest(query), (0, 1.0))
        self.assertEqual(space.distance(0, 1), 2.0)
        self.assertEqual(space.distance(0, 2), 6.0)
        self.assertEqual(pairwise_distance_matrix(records, aligned_curve_distance)[3][2], 9.0)
        self.assertEqual(representative_indices(records, aligned_curve_distance, 3), [0, 2, 3])
        self.assertEqual(
            representatives(records, aligned_curve_distance, 3),
            [records[0], records[2], records[3]],
        )
        self.assertEqual(separated_representative_indices(records, aligned_curve_distance, 4.0), [0, 2, 3])
        self.assertEqual(
            separated_representatives(records, aligned_curve_distance, 4.0),
            [records[0], records[2], records[3]],
        )
        self.assertEqual(coverage_representative_indices(records, aligned_curve_distance, 4.0), [0, 2])
        self.assertEqual(
            coverage_representatives(records, aligned_curve_distance, 4.0),
            [records[0], records[2]],
        )
        self.assertGreater(intrinsic_dimension(records, aligned_curve_distance), 0.0)
        self.assertMetricContracts(records, aligned_curve_distance)

    def test_histogram_records_use_transport_metric_callable(self):
        records = [
            (1.0, 0.0, 0.0, 0.0),
            (0.0, 1.0, 0.0, 0.0),
            (0.0, 0.0, 0.0, 1.0),
            (0.5, 0.5, 0.0, 0.0),
            (0.0, 0.5, 0.5, 0.0),
        ]
        query = (0.25, 0.75, 0.0, 0.0)

        def cumulative_transport_distance(lhs, rhs):
            if len(lhs) != len(rhs):
                raise ValueError("histograms must have the same number of bins")

            cumulative_delta = 0.0
            distance = 0.0
            for lhs_mass, rhs_mass in zip(lhs, rhs):
                cumulative_delta += lhs_mass - rhs_mass
                distance += abs(cumulative_delta)

            return distance

        space = Space(records, cumulative_transport_distance)

        self.assertEqual(space.nearest(query), (1, 0.25))
        self.assertEqual(space.distance(0, 1), 1.0)
        self.assertEqual(space.distance(0, 2), 3.0)
        self.assertEqual(space.distance(3, 4), 1.0)
        self.assertEqual(pairwise_distance_matrix(records, cumulative_transport_distance)[1][4], 0.5)
        self.assertGreater(intrinsic_dimension(records, cumulative_transport_distance), 0.0)
        self.assertMetricContracts(records, cumulative_transport_distance)


if __name__ == "__main__":
    unittest.main()
