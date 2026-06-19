import os
import subprocess
import sys
import unittest
from pathlib import Path

import metric
import numpy as np
from metric.metrics import Edit, available
from metric import intent, mappings, representations, transforms
from metric.operators import (
    ClusteringResult,
    CorrelationResult,
    EmbeddingDiagnostics,
    EmbeddingModel,
    EmbeddingResult,
    GraphConnectivityDiagnostics,
    GraphDegreeDiagnostics,
    GraphStretchDiagnostics,
    GraphConstructionMetadata,
    GraphConstructionResult,
    MappingResult,
    Outlier,
    OutlierResult,
    RepresentativeSet,
    ReductionResult,
    StructureDescription,
    coverage_representative_indices,
    coverage_representatives,
    compare_spaces,
    correlate_spaces,
    dbscan,
    denoise_space,
    describe_structure,
    embed_space,
    exact_knn_graph,
    exact_knn_graph_edges,
    exact_radius_graph,
    exact_radius_graph_edges,
    find_groups,
    find_outliers,
    find_representatives,
    graph_connectivity_diagnostics,
    graph_degree_diagnostics,
    graph_stretch_diagnostics,
    intrinsic_dimension,
    kmedoids,
    map_space,
    medoid,
    medoid_index,
    nearest_neighbors,
    pairwise_distance_matrix,
    prune_graph_out_degree,
    range_neighbors,
    reduce_space,
    representative_indices,
    representatives,
    separated_representative_indices,
    separated_representatives,
    symmetrize_graph,
)
from metric.spaces import FiniteMetricSpace, MatrixSpace, Space
from metric.strategies import ClassicMDS, DBSCAN, DistanceProfileCorrelation, FarthestFirst, KMedoids


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
        self.assertIs(metric.intent.find_groups, find_groups)
        self.assertIs(metric.intent.find_neighbors, nearest_neighbors)
        self.assertIs(metric.intent.denoise, denoise_space)
        self.assertIs(metric.intent.embed, embed_space)
        self.assertIs(metric.representations.MatrixSpace, MatrixSpace)
        self.assertIs(metric.operators.nearest_neighbors, nearest_neighbors)
        self.assertIs(metric.operators.range_neighbors, range_neighbors)
        self.assertIs(metric.operators.GraphConnectivityDiagnostics, GraphConnectivityDiagnostics)
        self.assertIs(metric.operators.GraphDegreeDiagnostics, GraphDegreeDiagnostics)
        self.assertIs(metric.operators.GraphStretchDiagnostics, GraphStretchDiagnostics)
        self.assertIs(metric.operators.GraphConstructionMetadata, GraphConstructionMetadata)
        self.assertIs(metric.operators.GraphConstructionResult, GraphConstructionResult)
        self.assertIs(metric.operators.MappingResult, MappingResult)
        self.assertIs(metric.operators.EmbeddingDiagnostics, EmbeddingDiagnostics)
        self.assertIs(metric.operators.EmbeddingModel, EmbeddingModel)
        self.assertIs(metric.operators.EmbeddingResult, EmbeddingResult)
        self.assertIs(metric.operators.ClusteringResult, ClusteringResult)
        self.assertIs(metric.operators.Outlier, Outlier)
        self.assertIs(metric.operators.OutlierResult, OutlierResult)
        self.assertIs(metric.operators.CorrelationResult, CorrelationResult)
        self.assertIs(metric.operators.RepresentativeSet, RepresentativeSet)
        self.assertIs(metric.operators.ReductionResult, ReductionResult)
        self.assertIs(metric.operators.StructureDescription, StructureDescription)
        self.assertIs(metric.operators.find_groups, find_groups)
        self.assertIs(metric.operators.find_outliers, find_outliers)
        self.assertIs(metric.operators.denoise_space, denoise_space)
        self.assertIs(metric.operators.embed_space, embed_space)
        self.assertIs(metric.operators.compare_spaces, compare_spaces)
        self.assertIs(metric.operators.correlate_spaces, correlate_spaces)
        self.assertIs(metric.operators.describe_structure, describe_structure)
        self.assertIs(metric.operators.find_representatives, find_representatives)
        self.assertIs(metric.operators.reduce_space, reduce_space)
        self.assertIs(metric.operators.kmedoids, kmedoids)
        self.assertIs(metric.operators.dbscan, dbscan)
        self.assertIs(metric.operators.exact_knn_graph, exact_knn_graph)
        self.assertIs(metric.operators.exact_knn_graph_edges, exact_knn_graph_edges)
        self.assertIs(metric.operators.exact_radius_graph, exact_radius_graph)
        self.assertIs(metric.operators.exact_radius_graph_edges, exact_radius_graph_edges)
        self.assertIs(metric.operators.graph_connectivity_diagnostics, graph_connectivity_diagnostics)
        self.assertIs(metric.operators.graph_degree_diagnostics, graph_degree_diagnostics)
        self.assertIs(metric.operators.graph_stretch_diagnostics, graph_stretch_diagnostics)
        self.assertIs(metric.operators.intrinsic_dimension, intrinsic_dimension)
        self.assertIs(metric.operators.map_space, map_space)
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
        self.assertIs(metric.intent, intent)
        self.assertIs(metric.representations, representations)
        self.assertIs(metric.strategies.FarthestFirst, FarthestFirst)
        self.assertIs(metric.transforms, transforms)
        self.assertIs(metric.Edit, Edit)
        self.assertIs(metric.GraphConnectivityDiagnostics, GraphConnectivityDiagnostics)
        self.assertIs(metric.GraphDegreeDiagnostics, GraphDegreeDiagnostics)
        self.assertIs(metric.GraphStretchDiagnostics, GraphStretchDiagnostics)
        self.assertIs(metric.GraphConstructionMetadata, GraphConstructionMetadata)
        self.assertIs(metric.GraphConstructionResult, GraphConstructionResult)
        self.assertIs(metric.MappingResult, MappingResult)
        self.assertIs(metric.EmbeddingDiagnostics, EmbeddingDiagnostics)
        self.assertIs(metric.EmbeddingModel, EmbeddingModel)
        self.assertIs(metric.EmbeddingResult, EmbeddingResult)
        self.assertIs(metric.ClusteringResult, ClusteringResult)
        self.assertIs(metric.Outlier, Outlier)
        self.assertIs(metric.OutlierResult, OutlierResult)
        self.assertIs(metric.RepresentativeSet, RepresentativeSet)
        self.assertIs(metric.ReductionResult, ReductionResult)
        self.assertIs(metric.StructureDescription, StructureDescription)
        self.assertIs(metric.find_groups, find_groups)
        self.assertIs(metric.find_outliers, find_outliers)
        self.assertIs(metric.denoise_space, denoise_space)
        self.assertIs(metric.embed_space, embed_space)
        self.assertIs(metric.describe_structure, describe_structure)
        self.assertIs(metric.find_representatives, find_representatives)
        self.assertIs(metric.reduce_space, reduce_space)
        self.assertIs(metric.kmedoids, kmedoids)
        self.assertIs(metric.dbscan, dbscan)
        self.assertIs(metric.KMedoids, KMedoids)
        self.assertIs(metric.DBSCAN, DBSCAN)
        self.assertIs(metric.DistanceProfileCorrelation, DistanceProfileCorrelation)
        self.assertIs(metric.FarthestFirst, FarthestFirst)
        self.assertIs(metric.ClassicMDS, ClassicMDS)
        self.assertIs(metric.CorrelationResult, CorrelationResult)
        self.assertIs(metric.compare_spaces, compare_spaces)
        self.assertIs(metric.correlate_spaces, correlate_spaces)
        self.assertIs(metric.exact_knn_graph, exact_knn_graph)
        self.assertIs(metric.exact_knn_graph_edges, exact_knn_graph_edges)
        self.assertIs(metric.exact_radius_graph, exact_radius_graph)
        self.assertIs(metric.exact_radius_graph_edges, exact_radius_graph_edges)
        self.assertIs(metric.graph_connectivity_diagnostics, graph_connectivity_diagnostics)
        self.assertIs(metric.graph_degree_diagnostics, graph_degree_diagnostics)
        self.assertIs(metric.graph_stretch_diagnostics, graph_stretch_diagnostics)
        self.assertIs(metric.intrinsic_dimension, intrinsic_dimension)
        self.assertIs(metric.map_space, map_space)
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
        self.assertIn("intent", metric.__all__)
        self.assertIn("mappings", metric.__all__)
        self.assertIn("representations", metric.__all__)
        self.assertIn("transforms", metric.__all__)
        self.assertIn("GraphConnectivityDiagnostics", metric.__all__)
        self.assertIn("GraphDegreeDiagnostics", metric.__all__)
        self.assertIn("GraphStretchDiagnostics", metric.__all__)
        self.assertIn("GraphConstructionMetadata", metric.__all__)
        self.assertIn("GraphConstructionResult", metric.__all__)
        self.assertIn("MappingResult", metric.__all__)
        self.assertIn("EmbeddingDiagnostics", metric.__all__)
        self.assertIn("EmbeddingModel", metric.__all__)
        self.assertIn("EmbeddingResult", metric.__all__)
        self.assertIn("ClusteringResult", metric.__all__)
        self.assertIn("Outlier", metric.__all__)
        self.assertIn("OutlierResult", metric.__all__)
        self.assertIn("CorrelationResult", metric.__all__)
        self.assertIn("RepresentativeSet", metric.__all__)
        self.assertIn("ReductionResult", metric.__all__)
        self.assertIn("StructureDescription", metric.__all__)
        self.assertIn("find_groups", metric.__all__)
        self.assertIn("find_outliers", metric.__all__)
        self.assertIn("denoise_space", metric.__all__)
        self.assertIn("embed_space", metric.__all__)
        self.assertIn("compare_spaces", metric.__all__)
        self.assertIn("correlate_spaces", metric.__all__)
        self.assertIn("describe_structure", metric.__all__)
        self.assertIn("find_representatives", metric.__all__)
        self.assertIn("reduce_space", metric.__all__)
        self.assertIn("kmedoids", metric.__all__)
        self.assertIn("dbscan", metric.__all__)
        self.assertIn("KMedoids", metric.__all__)
        self.assertIn("DBSCAN", metric.__all__)
        self.assertIn("DistanceProfileCorrelation", metric.__all__)
        self.assertIn("FarthestFirst", metric.__all__)
        self.assertIn("ClassicMDS", metric.__all__)
        self.assertIn("exact_knn_graph", metric.__all__)
        self.assertIn("exact_knn_graph_edges", metric.__all__)
        self.assertIn("exact_radius_graph", metric.__all__)
        self.assertIn("exact_radius_graph_edges", metric.__all__)
        self.assertIn("graph_connectivity_diagnostics", metric.__all__)
        self.assertIn("graph_degree_diagnostics", metric.__all__)
        self.assertIn("graph_stretch_diagnostics", metric.__all__)
        self.assertIn("map_space", metric.__all__)
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
        self.assertIsInstance(representations.matrix(Space(self.records, self.metric)), MatrixSpace)
        self.assertIsInstance(representations.matrix_space(self.records, self.metric), MatrixSpace)
        self.assertIsInstance(mappings.available(), tuple)
        self.assertIsInstance(transforms.available(), tuple)

    def test_promoted_metric_space_examples_run(self):
        examples_root = Path(__file__).resolve().parents[2] / "examples"
        examples = sorted((examples_root / "metric_space").glob("*.py"))
        examples.extend(sorted((examples_root / "engine").glob("*.py")))

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

        matrix = space.to_matrix()
        self.assertIsInstance(matrix, MatrixSpace)
        self.assertIsNot(matrix, space)
        self.assertEqual(matrix.pairwise_distances(), space.pairwise_distances())

    def test_nearest_neighbor_helpers_use_record_ids(self):
        space = FiniteMetricSpace(self.records, self.metric)

        self.assertEqual(space.knn("cut", 2), [(0, 1), (1, 1)])
        self.assertEqual(space.nn("cut"), (0, 1))
        self.assertEqual(space.rnn("cut", 1), [(0, 1), (1, 1)])
        self.assertEqual(nearest_neighbors(self.records, self.metric, "cut", 2), [(0, 1), (1, 1)])
        self.assertEqual(range_neighbors(self.records, self.metric, "cut", 1), [(0, 1), (1, 1)])
        self.assertEqual(pairwise_distance_matrix(self.records, self.metric)[0][1], 1)

    def test_space_intent_methods_return_named_results(self):
        space = Space([0, 1, 2, 3, 4], metric=lambda lhs, rhs: abs(lhs - rhs))
        group_space = Space([0, 1, 2, 10, 11], metric=lambda lhs, rhs: abs(lhs - rhs))

        groups = group_space.groups(KMedoids(groups=2))
        self.assertIsInstance(groups, ClusteringResult)
        self.assertEqual(groups.assignments, (0, 0, 0, 1, 1))
        self.assertEqual(groups.medoids, (1, 3))
        self.assertEqual(groups.cluster_sizes, (3, 2))
        self.assertEqual(groups.record_count, 5)
        self.assertEqual(groups.cluster_count, 2)
        self.assertEqual(groups.noise_count, 0)
        self.assertEqual(groups.iterations, 2)
        self.assertTrue(groups.converged)
        self.assertEqual(groups.algorithm, "kmedoids")
        self.assertEqual(groups.representation, "metric_space")
        self.assertEqual(find_groups([0, 1, 2, 10, 11], lambda lhs, rhs: abs(lhs - rhs), 2), groups)

        density_groups = group_space.groups(DBSCAN(radius=1, min_points=2))
        self.assertIsInstance(density_groups, ClusteringResult)
        self.assertEqual(density_groups.assignments, (0, 0, 0, 1, 1))
        self.assertEqual(density_groups.medoids, (1, 3))
        self.assertEqual(density_groups.core_records, (0, 1, 2, 3, 4))
        self.assertEqual(density_groups.noise_records, ())
        self.assertEqual(density_groups.cluster_sizes, (3, 2))
        self.assertEqual(density_groups.cluster_count, 2)
        self.assertEqual(density_groups.noise_count, 0)
        self.assertEqual(density_groups.algorithm, "dbscan")

        outlier_space = Space([0, 1, 10, 11, 30], metric=lambda lhs, rhs: abs(lhs - rhs))
        outliers = outlier_space.outliers(DBSCAN(radius=2, min_points=2))
        self.assertIsInstance(outliers, OutlierResult)
        self.assertEqual(outliers.outliers, (Outlier(record_id=4, score=19),))
        self.assertEqual(outliers.record_count, 5)
        self.assertEqual(outliers.cluster_count, 2)
        self.assertEqual(outliers.noise_count, 1)
        self.assertTrue(outliers.exact)
        self.assertEqual(outliers.operator_name, "find_outliers")
        self.assertEqual(outliers.strategy, "dbscan_noise")
        self.assertEqual(outliers.representation, "metric_space")
        self.assertEqual(
            find_outliers([0, 1, 10, 11, 30], lambda lhs, rhs: abs(lhs - rhs), DBSCAN(radius=2, min_points=2)),
            outliers,
        )
        self.assertEqual(outlier_space.outliers(DBSCAN(radius=100, min_points=2)).outliers, ())

        denoised = outlier_space.denoise(DBSCAN(radius=2, min_points=2))
        self.assertIsInstance(denoised, MappingResult)
        self.assertIsInstance(denoised.space, Space)
        self.assertEqual(denoised.space.records, [0, 1, 10, 11])
        self.assertEqual(denoised.source_record_ids, (0, 1, 2, 3))
        self.assertEqual(denoised.source_record_count, 5)
        self.assertEqual(denoised.target_record_count, 4)
        self.assertTrue(denoised.exact)
        self.assertEqual(denoised.operator_name, "denoise")
        self.assertEqual(denoised.mapping, "density_denoise")
        self.assertEqual(denoised.strategy, "dbscan_noise_filter")
        self.assertEqual(denoised.representation, "metric_space")
        self.assertFalse(denoised.inverse_supported)
        self.assertEqual(denoised.space.distance(0, 3), 11)
        self.assertEqual(
            denoise_space(
                [0, 1, 10, 11, 30],
                lambda lhs, rhs: abs(lhs - rhs),
                DBSCAN(radius=2, min_points=2),
            ).source_record_ids,
            denoised.source_record_ids,
        )
        self.assertEqual(
            outlier_space.denoise(DBSCAN(radius=100, min_points=2)).source_record_ids,
            (0, 1, 2, 3, 4),
        )
        self.assertEqual(
            Space([0, 10], metric=lambda lhs, rhs: abs(lhs - rhs)).denoise(DBSCAN(radius=1, min_points=2)).space.records,
            [],
        )

        representatives_result = space.representatives(3)
        self.assertIsInstance(representatives_result, RepresentativeSet)
        self.assertEqual(representatives_result.representatives, (0, 4, 2))
        self.assertEqual(representatives_result.nearest_representative_distances, (0, 1, 0, 1, 0))
        self.assertEqual(representatives_result.record_count, 5)
        self.assertEqual(representatives_result.requested_count, 3)
        self.assertEqual(representatives_result.coverage_radius, 1)
        self.assertAlmostEqual(representatives_result.average_nearest_distance, 0.4)
        self.assertEqual(representatives_result.strategy, "farthest_first")
        self.assertEqual(representatives_result.representation, "metric_space")

        seeded = space.representatives(2, strategy=FarthestFirst(seed_index=2))
        self.assertEqual(seeded.representatives, (2, 0))

        reduction = space.reduce(3)
        self.assertIsInstance(reduction, ReductionResult)
        self.assertIsInstance(reduction.space, Space)
        self.assertEqual(reduction.source_record_ids, (0, 4, 2))
        self.assertEqual(reduction.assignments, (0, 0, 2, 2, 1))
        self.assertEqual(reduction.nearest_representative_distances, (0, 1, 0, 1, 0))
        self.assertEqual(reduction.source_record_count, 5)
        self.assertEqual(reduction.reduced_record_count, 3)
        self.assertEqual(reduction.space.records, [0, 4, 2])
        self.assertTrue(reduction.exact)
        self.assertEqual(reduction.operator_name, "reduce")
        self.assertEqual(reduction.strategy, "farthest_first")
        self.assertEqual(reduction.representation, "metric_space")
        self.assertFalse(reduction.inverse_supported)
        self.assertEqual(
            reduce_space([0, 1, 2, 3, 4], lambda lhs, rhs: abs(lhs - rhs), 3).source_record_ids,
            reduction.source_record_ids,
        )

        medoid_reduction = group_space.reduce(strategy=KMedoids(groups=2))
        self.assertEqual(medoid_reduction.source_record_ids, (1, 3))
        self.assertEqual(medoid_reduction.assignments, (0, 0, 0, 1, 1))
        self.assertEqual(medoid_reduction.nearest_representative_distances, (1, 0, 1, 0, 1))
        self.assertEqual(medoid_reduction.strategy, "kmedoids")
        self.assertEqual(medoid_reduction.space.records, [1, 10])

        mapped = space.map(lambda record: {"value": record}, metric=lambda lhs, rhs: abs(lhs["value"] - rhs["value"]))
        self.assertIsInstance(mapped, MappingResult)
        self.assertIsInstance(mapped.space, Space)
        self.assertEqual(mapped.space.records, [{"value": 0}, {"value": 1}, {"value": 2}, {"value": 3}, {"value": 4}])
        self.assertEqual(mapped.source_record_ids, (0, 1, 2, 3, 4))
        self.assertEqual(mapped.source_record_count, 5)
        self.assertEqual(mapped.target_record_count, 5)
        self.assertTrue(mapped.exact)
        self.assertEqual(mapped.operator_name, "map")
        self.assertEqual(mapped.mapping, "deterministic_transform")
        self.assertEqual(mapped.strategy, "deterministic_transform")
        self.assertEqual(mapped.representation, "metric_space")
        self.assertFalse(mapped.inverse_supported)
        self.assertEqual(mapped.space.distance(0, 4), 4)
        self.assertEqual(
            map_space([0, 1, 2], lambda record: record * record, lambda lhs, rhs: abs(lhs - rhs)).space.records,
            [0, 1, 4],
        )

        embedding = space.embed(dimensions=1)
        self.assertIsInstance(embedding, EmbeddingResult)
        self.assertIsInstance(embedding.diagnostics, EmbeddingDiagnostics)
        self.assertIsInstance(embedding.model, EmbeddingModel)
        self.assertIsInstance(embedding.embedded_space, Space)
        self.assertIsInstance(embedding.source_space, Space)
        self.assertEqual(embedding.coordinates.shape, (5, 1))
        self.assertEqual(embedding.dimensions, 1)
        self.assertEqual(embedding.model.method, "classic_mds")
        self.assertEqual(embedding.model.dimensions, 1)
        self.assertEqual(embedding.model.source_record_ids, (0, 1, 2, 3, 4))
        self.assertEqual(embedding.source_record_ids, (0, 1, 2, 3, 4))
        self.assertEqual(embedding.source_record_count, 5)
        self.assertTrue(embedding.exact)
        self.assertEqual(embedding.operator_name, "embed")
        self.assertEqual(embedding.strategy, "classic_mds")
        self.assertEqual(embedding.representation, "metric_space")
        self.assertLess(embedding.stress, 1e-12)
        self.assertLess(embedding.diagnostics.raw_stress, 1e-12)
        self.assertGreaterEqual(embedding.diagnostics.distance_correlation, 0.999999)
        self.assertEqual(embedding.trustworthiness, 1.0)
        self.assertEqual(embedding.diagnostics.neighbor_k, 2)
        self.assertTrue(embedding.diagnostics.finite_coordinates)
        self.assertEqual(embedding.source_space.records, [0, 1, 2, 3, 4])
        self.assertAlmostEqual(embedding.embedded_space.distance(0, 4), 4.0)
        self.assertTrue(np.isfinite(embedding.coordinates).all())

        strategy_embedding = embed_space(
            [0, 1, 2],
            lambda lhs, rhs: abs(lhs - rhs),
            strategy=ClassicMDS(dimensions=1),
        )
        self.assertEqual(strategy_embedding.coordinates.shape, (3, 1))
        self.assertLess(strategy_embedding.stress, 1e-12)

        description = space.describe()
        self.assertIsInstance(description, StructureDescription)
        self.assertEqual(description.record_count, 5)
        self.assertEqual(description.pair_count, 10)
        self.assertEqual(description.zero_distance_pair_count, 0)
        self.assertEqual(description.minimum_nonzero_distance, 1)
        self.assertEqual(description.maximum_distance, 4)
        self.assertAlmostEqual(description.average_distance, 2.0)
        self.assertAlmostEqual(description.intrinsic_dimension, np.log2(5.0 / 3.0))
        self.assertEqual(space.describe_structure(), description)

        process_space = Space([0, 1, 2, 3, 4, 5], metric=lambda lhs, rhs: abs(lhs - rhs))
        quality_space = Space([0, 1, 4, 9, 16, 25], metric=lambda lhs, rhs: abs(lhs - rhs))
        dependency = process_space.compare(quality_space)
        self.assertIsInstance(dependency, CorrelationResult)
        self.assertAlmostEqual(dependency.value, 0.8500255011475573)
        self.assertEqual(dependency.left_record_count, 6)
        self.assertEqual(dependency.right_record_count, 6)
        self.assertEqual(dependency.pair_count, 15)
        self.assertTrue(dependency.exact)
        self.assertEqual(dependency.algorithm, "distance_profile_correlation")
        self.assertEqual(dependency.strategy, "distance_profile_correlation")
        self.assertEqual(dependency.left_representation, "metric_space")
        self.assertEqual(dependency.right_representation, "metric_space")
        self.assertEqual(process_space.correlate(quality_space), dependency)
        self.assertEqual(
            compare_spaces(process_space.records, process_space.metric, quality_space.records, quality_space.metric),
            correlate_spaces(process_space.records, process_space.metric, quality_space.records, quality_space.metric),
        )
        with self.assertRaises(ValueError):
            process_space.compare(Space([0, 1, 2], metric=lambda lhs, rhs: abs(lhs - rhs)))
        with self.assertRaises(TypeError):
            process_space.compare(quality_space, strategy=KMedoids(groups=2))
        with self.assertRaises(ValueError):
            process_space.compare(quality_space, strategy=DistanceProfileCorrelation(method="spearman"))
        with self.assertRaises(ValueError):
            space.reduce(0)
        with self.assertRaises(ValueError):
            space.reduce(6)
        with self.assertRaises(TypeError):
            space.reduce(strategy=FarthestFirst())
        with self.assertRaises(ValueError):
            space.reduce(3, strategy=KMedoids(groups=2))
        with self.assertRaises(TypeError):
            space.map(3)
        with self.assertRaises(TypeError):
            map_space([0, 1], lambda record: record, metric=3)
        with self.assertRaises(TypeError):
            space.denoise(KMedoids(groups=2))
        with self.assertRaises(ValueError):
            space.embed(dimensions=0)
        with self.assertRaises(TypeError):
            space.embed(strategy=KMedoids(groups=2))
        with self.assertRaises(TypeError):
            embed_space([0, 1], metric=3)

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

        connectivity_diagnostics = graph_connectivity_diagnostics(knn_graph)
        self.assertIsInstance(connectivity_diagnostics, GraphConnectivityDiagnostics)
        self.assertEqual(connectivity_diagnostics.record_count, len(records))
        self.assertEqual(connectivity_diagnostics.edge_count, len(knn_graph.edges))
        self.assertTrue(connectivity_diagnostics.directed)
        self.assertEqual(connectivity_diagnostics.component_labels, (0, 0, 0, 0, 0))
        self.assertEqual(connectivity_diagnostics.component_count, 1)
        self.assertEqual(connectivity_diagnostics.isolated_count, 0)
        self.assertEqual(connectivity_diagnostics.largest_component_size, 5)
        self.assertTrue(connectivity_diagnostics.connected)
        self.assertEqual(connectivity_diagnostics.connectivity_policy, "weak_undirected_reachability")

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

        undirected_connectivity = graph_connectivity_diagnostics(union_graph)
        self.assertFalse(undirected_connectivity.directed)
        self.assertEqual(undirected_connectivity.component_labels, (0, 0, 0, 0, 0))
        self.assertEqual(undirected_connectivity.component_count, 1)
        self.assertEqual(undirected_connectivity.isolated_count, 0)
        self.assertEqual(undirected_connectivity.largest_component_size, 5)
        self.assertTrue(undirected_connectivity.connected)
        self.assertEqual(undirected_connectivity.connectivity_policy, "undirected_reachability")

        undirected_stretch = graph_stretch_diagnostics(records, cumulative_transport_distance, union_graph)
        self.assertIsInstance(undirected_stretch, GraphStretchDiagnostics)
        self.assertFalse(undirected_stretch.directed)
        self.assertEqual(undirected_stretch.record_count, len(records))
        self.assertEqual(undirected_stretch.edge_count, len(union_graph.edges))
        self.assertEqual(undirected_stretch.pair_count, 10)
        self.assertEqual(undirected_stretch.reachable_pair_count, 10)
        self.assertEqual(undirected_stretch.unreachable_pair_count, 0)
        self.assertEqual(undirected_stretch.zero_metric_pair_count, 0)
        self.assertAlmostEqual(undirected_stretch.max_stretch, 1.0)
        self.assertAlmostEqual(undirected_stretch.average_stretch, 1.0)
        self.assertEqual(undirected_stretch.stretch_policy, "undirected_shortest_path")

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
        with self.assertRaises(ValueError):
            graph_connectivity_diagnostics(invalid_graph)
        with self.assertRaises(ValueError):
            graph_stretch_diagnostics([0, 1], cumulative_transport_distance, invalid_graph)
        with self.assertRaises(ValueError):
            graph_stretch_diagnostics(records, cumulative_transport_distance, invalid_graph)

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
        complete_radius_graph = exact_radius_graph(records, cumulative_transport_distance, 3.0)
        directed_stretch = graph_stretch_diagnostics(records, cumulative_transport_distance, complete_radius_graph)
        self.assertTrue(directed_stretch.directed)
        self.assertEqual(directed_stretch.pair_count, 20)
        self.assertEqual(directed_stretch.reachable_pair_count, 20)
        self.assertEqual(directed_stretch.unreachable_pair_count, 0)
        self.assertEqual(directed_stretch.zero_metric_pair_count, 0)
        self.assertAlmostEqual(directed_stretch.max_stretch, 1.0)
        self.assertAlmostEqual(directed_stretch.average_stretch, 1.0)
        self.assertEqual(directed_stretch.stretch_policy, "directed_shortest_path")
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
        disconnected_graph = exact_radius_graph([0, 1, 10], absolute_distance, 1)
        disconnected_connectivity = graph_connectivity_diagnostics(disconnected_graph)
        self.assertEqual(disconnected_connectivity.component_labels, (0, 0, 1))
        self.assertEqual(disconnected_connectivity.component_count, 2)
        self.assertEqual(disconnected_connectivity.isolated_count, 1)
        self.assertEqual(disconnected_connectivity.largest_component_size, 2)
        self.assertFalse(disconnected_connectivity.connected)
        disconnected_stretch = graph_stretch_diagnostics([0, 1, 10], absolute_distance, disconnected_graph)
        self.assertEqual(disconnected_stretch.pair_count, 6)
        self.assertEqual(disconnected_stretch.reachable_pair_count, 2)
        self.assertEqual(disconnected_stretch.unreachable_pair_count, 4)
        self.assertEqual(disconnected_stretch.zero_metric_pair_count, 0)
        self.assertAlmostEqual(disconnected_stretch.max_stretch, 1.0)
        self.assertAlmostEqual(disconnected_stretch.average_stretch, 1.0)
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
        self.assertTrue(callable(space.compare))
        self.assertTrue(callable(space.correlate))
        self.assertTrue(callable(space.embed))
        self.assertTrue(callable(space.outliers))
        self.assertTrue(callable(space.denoise))
        self.assertTrue(callable(space.reduce))
        self.assertTrue(callable(space.map))

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
