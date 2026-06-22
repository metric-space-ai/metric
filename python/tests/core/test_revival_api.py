import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

import metric
import metric.distance as distance_module
import metric.spaces as spaces_module
import numpy as np
from metric import compat, core, exceptions, intent, mappings, representations, runtime, transforms
from metric.exceptions import (
    AmbiguousIntentError,
    IncompatibleSpaceError,
    MetricContractError,
    MetricError,
    MetricInputError,
    MissingMetricError,
    OptionalDependencyError,
    StaleRepresentationError,
    StrategyParameterError,
    StrategyUnavailableError,
    UnsupportedOperationError,
)
from metric.metrics import Edit, Metric, available
from metric.operators import (
    ClusteringResult,
    CompressionResult,
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
    Neighbor,
    NeighborResult,
    Outlier,
    OutlierResult,
    RepresentativeSet,
    ReductionResult,
    StructureDescription,
    coverage_representative_indices,
    coverage_representatives,
    compress_space,
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
from metric.representations import GraphIndex, TreeIndex, graph, tree
from metric.runtime import CachePolicy, RuntimeDiagnostics, RuntimePolicy, runtime_diagnostics
from metric.spaces import FiniteMetricSpace, MatrixSpace, RecordId, Space
from metric.strategies import (
    ClassicMDS,
    DBSCAN,
    DSPCC,
    DiffusionEmbedding,
    DistanceProfileCorrelation,
    FarthestFirst,
    KOC,
    KMedoids,
    MDS,
    PCFA,
    PhateAE,
    SOM,
)


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

    def assertRequiresNative(self, fn, *args, **kwargs):
        with self.assertRaisesRegex(StrategyUnavailableError, r"native C\+\+ binding"):
            fn(*args, **kwargs)

    def test_metric_concepts_are_importable(self):
        self.assertIn("Edit", available())
        self.assertTrue(hasattr(distance_module, "_install_compatibility_aliases"))
        self.assertIs(MatrixSpace, FiniteMetricSpace)
        self.assertIs(metric.metrics.Edit, Edit)
        self.assertIs(metric.metrics.Metric, Metric)
        self.assertIs(metric.core, core)
        self.assertIs(metric.core.Edit, Edit)
        self.assertIs(metric.core.Metric, Metric)
        self.assertIs(metric.core.Space, Space)
        self.assertIs(metric.core.FiniteMetricSpace, FiniteMetricSpace)
        self.assertIs(metric.core.MatrixSpace, MatrixSpace)
        self.assertIs(metric.core.RuntimePolicy, RuntimePolicy)
        self.assertIs(metric.core.RuntimeDiagnostics, RuntimeDiagnostics)
        self.assertIs(metric.core.MetricError, MetricError)
        self.assertIs(metric.core.MissingMetricError, MissingMetricError)
        self.assertIs(metric.core.StaleRepresentationError, StaleRepresentationError)
        self.assertIs(metric.core.UnsupportedOperationError, UnsupportedOperationError)
        self.assertEqual(metric.core.make_space(self.records, self.metric).records, self.records)
        for name in ("Manhattan", "Minkowski", "ThresholdedEuclidean"):
            self.assertTrue(hasattr(metric.metrics, name))
        self.assertIs(metric.spaces.FiniteMetricSpace, FiniteMetricSpace)
        self.assertIs(metric.intent.find_groups, find_groups)
        self.assertIs(metric.intent.find_neighbors, nearest_neighbors)
        self.assertIs(metric.intent.denoise, denoise_space)
        self.assertIs(metric.intent.embed, embed_space)
        self.assertIs(metric.intent.compress, compress_space)
        self.assertIs(metric.exceptions, exceptions)
        self.assertIs(metric.exceptions.UnsupportedOperationError, UnsupportedOperationError)
        self.assertIs(metric.exceptions.StaleRepresentationError, StaleRepresentationError)
        self.assertIs(metric.runtime, runtime)
        self.assertIs(metric.runtime.RuntimePolicy, RuntimePolicy)
        self.assertIs(metric.runtime.CachePolicy, CachePolicy)
        self.assertIs(metric.runtime.RuntimeDiagnostics, RuntimeDiagnostics)
        self.assertIs(metric.runtime.runtime_diagnostics, runtime_diagnostics)
        self.assertTrue(issubclass(UnsupportedOperationError, MetricError))
        self.assertTrue(issubclass(StrategyUnavailableError, MetricError))
        self.assertTrue(issubclass(StrategyParameterError, MetricError))
        self.assertIs(metric.representations.MatrixSpace, MatrixSpace)
        self.assertIs(metric.representations.GraphIndex, GraphIndex)
        self.assertIs(metric.representations.TreeIndex, TreeIndex)
        self.assertIs(metric.representations.graph, graph)
        self.assertIs(metric.representations.tree, tree)
        self.assertIs(metric.operators.nearest_neighbors, nearest_neighbors)
        self.assertIs(metric.operators.range_neighbors, range_neighbors)
        self.assertIs(metric.operators.GraphConnectivityDiagnostics, GraphConnectivityDiagnostics)
        self.assertIs(metric.operators.GraphDegreeDiagnostics, GraphDegreeDiagnostics)
        self.assertIs(metric.operators.GraphStretchDiagnostics, GraphStretchDiagnostics)
        self.assertIs(metric.operators.GraphConstructionMetadata, GraphConstructionMetadata)
        self.assertIs(metric.operators.GraphConstructionResult, GraphConstructionResult)
        self.assertIs(metric.operators.Neighbor, Neighbor)
        self.assertIs(metric.operators.NeighborResult, NeighborResult)
        self.assertIs(metric.operators.CompressionResult, CompressionResult)
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
        self.assertIs(metric.operators.compress_space, compress_space)
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
        self.assertIs(metric.strategies.MDS, MDS)
        self.assertIs(metric.strategies.DiffusionEmbedding, DiffusionEmbedding)
        self.assertIs(metric.strategies.PCFA, PCFA)
        self.assertIs(metric.strategies.SOM, SOM)
        self.assertIs(metric.strategies.KOC, KOC)
        self.assertIs(metric.strategies.DSPCC, DSPCC)
        self.assertIs(metric.strategies.PhateAE, PhateAE)
        self.assertIs(metric.transforms, transforms)
        self.assertIs(metric.Edit, Edit)
        self.assertIs(metric.Metric, Metric)
        self.assertIs(metric.GraphConnectivityDiagnostics, GraphConnectivityDiagnostics)
        self.assertIs(metric.GraphDegreeDiagnostics, GraphDegreeDiagnostics)
        self.assertIs(metric.GraphStretchDiagnostics, GraphStretchDiagnostics)
        self.assertIs(metric.GraphConstructionMetadata, GraphConstructionMetadata)
        self.assertIs(metric.GraphConstructionResult, GraphConstructionResult)
        self.assertIs(metric.Neighbor, Neighbor)
        self.assertIs(metric.NeighborResult, NeighborResult)
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
        for strategy_name in (
            "ClassicMDS",
            "DBSCAN",
            "DSPCC",
            "DiffusionEmbedding",
            "DistanceProfileCorrelation",
            "FarthestFirst",
            "KOC",
            "KMedoids",
            "MDS",
            "PCFA",
            "PhateAE",
            "SOM",
        ):
            self.assertFalse(hasattr(metric, strategy_name), strategy_name)
        with self.assertRaises(ImportError):
            exec("from metric import KMedoids", {})
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
        self.assertIs(metric.RecordId, RecordId)
        self.assertIsInstance(Space(self.records, self.metric).ids[0], RecordId)
        self.assertEqual(Space(self.records, self.metric).ids, [0, 1, 2, 3])
        self.assertIs(metric.MetricError, MetricError)
        self.assertIs(metric.MissingMetricError, MissingMetricError)
        self.assertIs(metric.StaleRepresentationError, StaleRepresentationError)
        self.assertIs(metric.UnsupportedOperationError, UnsupportedOperationError)
        self.assertIn("FiniteMetricSpace", metric.__all__)
        self.assertIn("Space", metric.__all__)
        self.assertIn("RecordId", metric.__all__)
        self.assertIn("core", metric.__all__)
        self.assertIn("exceptions", metric.__all__)
        self.assertIn("MetricError", metric.__all__)
        self.assertIn("Metric", metric.__all__)
        self.assertIn("MissingMetricError", metric.__all__)
        self.assertIn("StaleRepresentationError", metric.__all__)
        self.assertIn("UnsupportedOperationError", metric.__all__)
        self.assertIn("intent", metric.__all__)
        self.assertIn("mappings", metric.__all__)
        self.assertIn("representations", metric.__all__)
        self.assertIn("transforms", metric.__all__)
        self.assertIn("GraphConnectivityDiagnostics", metric.__all__)
        self.assertIn("GraphDegreeDiagnostics", metric.__all__)
        self.assertIn("GraphStretchDiagnostics", metric.__all__)
        self.assertIn("GraphConstructionMetadata", metric.__all__)
        self.assertIn("GraphConstructionResult", metric.__all__)
        self.assertIn("Neighbor", metric.__all__)
        self.assertIn("NeighborResult", metric.__all__)
        self.assertIn("CompressionResult", metric.__all__)
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
        self.assertIn("compress_space", metric.__all__)
        self.assertIn("kmedoids", metric.__all__)
        self.assertIn("dbscan", metric.__all__)
        for strategy_name in (
            "ClassicMDS",
            "DBSCAN",
            "DSPCC",
            "DiffusionEmbedding",
            "DistanceProfileCorrelation",
            "FarthestFirst",
            "KOC",
            "KMedoids",
            "MDS",
            "PCFA",
            "PhateAE",
            "SOM",
        ):
            self.assertNotIn(strategy_name, metric.__all__)
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
        self.assertIn("RuntimePolicy", metric.__all__)
        self.assertIn("CachePolicy", metric.__all__)
        self.assertIn("RuntimeDiagnostics", metric.__all__)
        self.assertIn("runtime_diagnostics", metric.__all__)
        self.assertIn("compat", metric.__all__)
        self.assertEqual(compat.STABILITY, "compatibility")
        self.assertIn("distance", compat.LEGACY_MODULES)
        self.assertIs(compat.legacy_module("distance"), distance_module)
        self.assertIsInstance(compat.available_modules(), tuple)
        self.assertEqual(mappings.STABILITY, "beta")
        self.assertTrue(callable(mappings.load_native_mapping_artifact))
        self.assertTrue(callable(mappings.native_mapping_artifact))
        self.assertTrue(callable(mappings.native_phate_autoencoder_fit_vectors))
        self.assertIs(metric.mappings.NativeMappingArtifact, mappings.NativeMappingArtifact)
        self.assertIs(metric.mappings.NativePhateAutoencoderModel, mappings.NativePhateAutoencoderModel)
        self.assertTrue(callable(mappings.clustered_space))
        self.assertTrue(callable(mappings.make_clustered_space_mapping))
        self.assertTrue(callable(mappings.fit))
        self.assertTrue(callable(mappings.transform))
        self.assertEqual(transforms.STABILITY, "beta")
        self.assertIsInstance(representations.matrix(Space(self.records, self.metric)), MatrixSpace)
        self.assertIsInstance(representations.matrix_space(self.records, self.metric), MatrixSpace)
        self.assertIsInstance(mappings.available(), tuple)
        self.assertIsInstance(transforms.available(), tuple)

        with self.assertRaisesRegex(ValueError, "unknown legacy METRIC module"):
            compat.legacy_module("unknown")

    def test_twed_signature_metric_is_promoted_in_default_wheel(self):
        # TWED is the one promoted signature metric in the default wheel; it is
        # compiled from python/src/distance/TWED.cpp (native mtrc::TWED<double>).
        self.assertIn("TWED", available())
        self.assertIsNotNone(metric.metrics.TWED)
        self.assertIn("TWED", metric.metrics.__all__)
        twed = metric.metrics.TWED(penalty=0, elastic=1)
        self.assertEqual(twed.penalty, 0)
        self.assertEqual(twed.elastic, 1)
        # Native evaluation: identity is zero and symmetry holds (no Python math).
        self.assertEqual(twed([0.0, 1.0, 2.0], [0.0, 1.0, 2.0]), 0.0)
        self.assertEqual(
            twed([0.0, 1.0, 2.0, 3.0], [0.0, 1.0, 2.0, 4.0]),
            twed([0.0, 1.0, 2.0, 4.0], [0.0, 1.0, 2.0, 3.0]),
        )

    def test_c31_available_capability_flags_are_dynamic_bools(self):
        required_keys = {
            "native_core",
            "distance_metrics",
            "neighbors",
            "pairwise",
            "representatives",
            "reduce_compress",
            "structure",
            "groups",
            "outliers",
            "denoise",
            "embed",
            "compare_correlate",
            "correlation_package",
        }

        result = metric.available()
        self.assertIsInstance(result, dict)
        self.assertTrue(set(result) >= required_keys)
        self.assertTrue(all(isinstance(value, bool) for value in result.values()))

        self.assertIs(result["native_core"], True)
        self.assertIs(result["distance_metrics"], True)
        for key in (
            "neighbors",
            "pairwise",
            "representatives",
            "reduce_compress",
            "structure",
            "groups",
            "outliers",
            "denoise",
        ):
            self.assertIs(result[key], True, key)
        self.assertIs(result["embed"], False)

        # The dynamic flags must equal their live probes so the test passes both
        # standalone and after a parallel task promotes compare/correlation.
        import importlib

        try:
            native_metric = importlib.import_module("metric._impl.metric")
        except (ImportError, ModuleNotFoundError):
            native_metric = None
        self.assertEqual(
            result["compare_correlate"],
            bool(native_metric is not None and hasattr(native_metric, "compare_spaces")),
        )

        try:
            importlib.import_module("metric.correlation")
            correlation_importable = True
        except Exception:
            correlation_importable = False
        self.assertEqual(result["correlation_package"], correlation_importable)

        self.assertEqual(metric.available(), metric.capabilities())
        self.assertIn("available", metric.__all__)

    def test_native_record_file_io_builds_spaces_without_python_parsing(self):
        from metric._impl import metric as native_metric

        with tempfile.TemporaryDirectory() as tmpdir:
            csv_path = Path(tmpdir) / "vectors.csv"
            native_metric.write_csv_double_records(
                str(csv_path),
                [[0.0, 0.0], [3.0, 4.0]],
                header=["x", "y"],
            )

            rows = native_metric.read_csv_double_records(str(csv_path), has_header=True)
            self.assertEqual(rows, [[0.0, 0.0], [3.0, 4.0]])

            space = Space.from_csv(csv_path, has_header=True)
            self.assertEqual(space.records, rows)
            self.assertAlmostEqual(space.distance(0, 1), 5.0)

            exported_csv = Path(tmpdir) / "exported_vectors.csv"
            returned_path = space.to_csv(
                exported_csv,
                include_ids=True,
                header=["x", "y"],
            )
            self.assertEqual(returned_path, exported_csv)
            exported_space = Space.from_csv(exported_csv, has_header=True, id_column=0)
            self.assertEqual(exported_space.ids, [0.0, 1.0])
            self.assertEqual(exported_space.records, rows)

            npy_path = Path(tmpdir) / "vectors.npy"
            np.save(npy_path, np.asarray(rows, dtype=float))
            numpy_space = Space.from_numpy_file(npy_path)
            self.assertEqual(numpy_space.records, rows)
            self.assertAlmostEqual(numpy_space.distance(0, 1), 5.0)

            csv_with_id = Path(tmpdir) / "vectors_with_id.csv"
            csv_with_id.write_text("id,x,y\n100,1,2\n101,4,6\n", encoding="utf-8")
            identified = Space.from_csv(csv_with_id, has_header=True, id_column=0)
            self.assertEqual(identified.ids, [100.0, 101.0])
            self.assertEqual(identified.records, [[1.0, 2.0], [4.0, 6.0]])

            tsv_path = Path(tmpdir) / "tokens.tsv"
            native_metric.write_tsv_string_records(
                str(tsv_path),
                [["cat"], ["coat"]],
                header=["token"],
            )
            strings = Space.from_tsv(
                tsv_path,
                metric=self.metric,
                value_type="string",
                has_header=True,
                as_scalar=True,
            )
            self.assertEqual(strings.records, ["cat", "coat"])
            self.assertEqual(strings.distance(0, 1), self.metric("cat", "coat"))

            exported_tsv = Path(tmpdir) / "exported_tokens.tsv"
            strings.to_tsv(
                exported_tsv,
                value_type="string",
                as_scalar=True,
                header=["token"],
            )
            exported_strings = Space.from_tsv(
                exported_tsv,
                metric=self.metric,
                value_type="string",
                has_header=True,
                as_scalar=True,
            )
            self.assertEqual(exported_strings.records, ["cat", "coat"])

            with self.assertRaisesRegex(MissingMetricError, "requires metric"):
                Space.from_tsv(tsv_path, value_type="string", has_header=True, as_scalar=True)

            invalid_npy_path = Path(tmpdir) / "invalid.npy"
            np.save(invalid_npy_path, np.asarray([1.0, 2.0]))
            with self.assertRaisesRegex(ValueError, "2D array"):
                Space.from_numpy_file(invalid_npy_path)

    def test_native_mapping_artifact_projection_is_metadata_only(self):
        manifest = {
            "format": "metric.native_phate_autoencoder_artifact",
            "format_version": 1,
            "backend": "native_dnn",
            "scalar_type": "double",
            "mapping": {
                "name": "native_phate_autoencoder",
                "strategy": "native_dnn_phate_ae",
                "target": "metric_space",
            },
            "pipeline": {
                "name": "native_phate_autoencoder_pipeline",
                "components": (
                    {"name": "feature_record_codec", "role": "codec"},
                    {"name": "matrix_cache_distance_provider", "role": "distance_provider"},
                    {"name": "adaptive_gaussian_affinity_kernel", "role": "affinity_kernel"},
                    {"name": "lazy_row_normalized_diffusion_operator", "role": "diffusion_operator"},
                ),
            },
            "phate_geometry": {
                "spec": {"dimensions": 1, "diffusion_steps": 2, "max_dense_records": 5},
                "targets": {"target_count": 5, "dense_distance_evaluations": 25},
            },
            "source": {"record_count": 5, "feature_count": 2, "space_version": 7},
            "codec": {"type": "VectorRecordCodec", "feature_count": 2},
            "network": {"serialization": "cereal_binary", "byte_count": 18},
            "loss": {"terms": ({"name": "reconstruction_mse"}, {"name": "bottleneck_geometry"})},
            "training_spec": {"epochs": 80, "seed": 23},
        }

        artifact = mappings.load_native_mapping_artifact(
            {
                "manifest": manifest,
                "diagnostics": {"epoch_count": 80},
                "network_cereal": b"native-network-bytes",
            }
        )

        self.assertIsInstance(artifact, mappings.NativeMappingArtifact)
        self.assertEqual(artifact.format, "metric.native_phate_autoencoder_artifact")
        self.assertEqual(artifact.format_version, 1)
        self.assertEqual(artifact.backend, "native_dnn")
        self.assertEqual(artifact.mapping, "native_phate_autoencoder")
        self.assertEqual(artifact.strategy, "native_dnn_phate_ae")
        self.assertEqual(artifact.target, "metric_space")
        self.assertEqual(artifact.pipeline_name, "native_phate_autoencoder_pipeline")
        self.assertEqual(
            artifact.component_names,
            (
                "feature_record_codec",
                "matrix_cache_distance_provider",
                "adaptive_gaussian_affinity_kernel",
                "lazy_row_normalized_diffusion_operator",
            ),
        )
        self.assertEqual(artifact.source_record_count, 5)
        self.assertEqual(artifact.source_feature_count, 2)
        self.assertEqual(artifact.source_space_version, 7)
        self.assertEqual(artifact.network_byte_count, len(b"native-network-bytes"))
        self.assertFalse(artifact.transform_supported)
        self.assertFalse(artifact.inverse_supported)
        self.assertEqual(artifact.to_dict()["diagnostics"], {"epoch_count": 80})

        json_artifact = mappings.load_native_mapping_artifact(json.dumps(manifest))
        self.assertEqual(json_artifact.mapping, "native_phate_autoencoder")
        self.assertEqual(json_artifact.network_byte_count, 18)
        self.assertEqual(mappings.native_mapping_artifact(manifest).strategy, "native_dnn_phate_ae")

        with self.assertRaisesRegex(StrategyUnavailableError, "native C\\+\\+ binding"):
            artifact.transform(Space.vectors([[0.0, 0.0]], metric=lambda lhs, rhs: 0.0, validate="none"))
        with self.assertRaisesRegex(StrategyUnavailableError, "native C\\+\\+ binding"):
            artifact.inverse_transform([[0.0]])

        invalid = dict(manifest)
        invalid["format"] = "metric.unknown_artifact"
        with self.assertRaisesRegex(MetricInputError, "unsupported native mapping artifact format"):
            mappings.load_native_mapping_artifact(invalid)

    def test_native_phate_autoencoder_vector_binding_delegates_to_cpp(self):
        records = np.asarray(
            [[0.0, 0.0], [0.5, 0.5], [1.0, 1.0], [1.5, 1.5], [2.0, 2.0]],
            dtype=float,
        )

        model = mappings.native_phate_autoencoder_fit_vectors(
            records,
            dimensions=1,
            epochs=3,
            learning_rate=0.001,
            diffusion_steps=2,
            kernel_scale=1.0,
            distance_provider="matrix_cache_distance_provider",
            affinity_kernel="exponential_affinity_kernel",
            diffusion_operator="lazy_row_normalized_diffusion_operator",
        )

        self.assertIsInstance(model, mappings.NativePhateAutoencoderModel)
        self.assertEqual(model.mapping, "native_phate_autoencoder")
        self.assertEqual(model.strategy, "native_dnn_phate_ae")
        self.assertEqual(model.source_record_count, 5)
        self.assertEqual(model.latent_dimension, 1)
        self.assertEqual(model.distance_provider, "matrix_cache_distance_provider")
        self.assertEqual(model.affinity_kernel, "exponential_affinity_kernel")
        self.assertEqual(model.diffusion_operator, "lazy_row_normalized_diffusion_operator")
        self.assertTrue(model.transform_supported)
        self.assertTrue(model.inverse_supported)
        self.assertEqual(model.training_report()["epoch_count"], 3)

        latent = model.transform(records)
        self.assertEqual(len(latent), 5)
        self.assertEqual(len(latent[0]), 1)
        restored = model.inverse_transform(latent)
        self.assertEqual(len(restored), 5)
        self.assertEqual(len(restored[0]), 2)
        summary = model.to_dict()
        self.assertEqual(summary["mapping"], "native_phate_autoencoder")
        self.assertEqual(summary["affinity_kernel"], "exponential_affinity_kernel")
        self.assertEqual(summary["diffusion_operator"], "lazy_row_normalized_diffusion_operator")
        self.assertEqual(summary["training_report"]["epoch_count"], 3)

        with self.assertRaisesRegex(ValueError, "unsupported native PHATE-AE distance provider"):
            mappings.native_phate_autoencoder_fit_vectors(records, distance_provider="unknown")
        with self.assertRaisesRegex(ValueError, "unsupported native PHATE-AE affinity kernel"):
            mappings.native_phate_autoencoder_fit_vectors(records, affinity_kernel="unknown")
        with self.assertRaisesRegex(ValueError, "unsupported native PHATE-AE diffusion operator"):
            mappings.native_phate_autoencoder_fit_vectors(records, diffusion_operator="unknown")

    def test_space_map_phateae_delegates_only_for_native_vector_spaces(self):
        try:
            from metric.distance import Euclidean
            Euclidean()
        except Exception as exc:
            self.skipTest(f"native Euclidean binding unavailable: {exc}")

        records = [[0.0, 0.0], [0.5, 0.5], [1.0, 1.0], [1.5, 1.5], [2.0, 2.0]]
        space = Space.vectors(records, validate="none", cache="none")

        mapped = space.map(
            strategy=PhateAE(
                dimensions=1,
                epochs=3,
                learning_rate=0.001,
                kernel_scale=1.0,
                distance_provider="matrix_cache_distance_provider",
                affinity_kernel="exponential_affinity_kernel",
                diffusion_operator="lazy_row_normalized_diffusion_operator",
            )
        )

        self.assertIsInstance(mapped, MappingResult)
        self.assertEqual(mapped.mapping, "native_phate_autoencoder")
        self.assertEqual(mapped.strategy, "native_dnn_phate_ae")
        self.assertTrue(mapped.inverse_supported)
        self.assertEqual(mapped.source_record_ids, tuple(space.ids))
        self.assertEqual(mapped.source_records, tuple((record_id,) for record_id in space.ids))
        self.assertEqual(mapped.representative_records, tuple(space.ids))
        self.assertEqual(mapped.space.ids, space.ids)
        self.assertEqual(len(mapped.space.records), len(records))
        self.assertEqual(len(mapped.space.records[0]), 1)
        self.assertEqual(
            mapped.space.metadata["distance_provider"],
            "matrix_cache_distance_provider",
        )
        self.assertEqual(mapped.space.metadata["affinity_kernel"], "exponential_affinity_kernel")
        self.assertEqual(
            mapped.space.metadata["diffusion_operator"],
            "lazy_row_normalized_diffusion_operator",
        )

        restored = mapped.inverse_transform()
        self.assertEqual(len(restored), len(records))
        self.assertEqual(len(restored[0]), 2)

        custom_metric_space = Space.vectors(
            records,
            metric=lambda lhs, rhs: abs(lhs[0] - rhs[0]) + abs(lhs[1] - rhs[1]),
            validate="none",
            cache="none",
        )
        with self.assertRaisesRegex(StrategyUnavailableError, "Custom Python metrics are not ignored"):
            custom_metric_space.map(strategy=PhateAE(dimensions=1, epochs=1))

    def test_distance_compatibility_aliases_are_lazy(self):
        manhattan = object()
        p_norm = object()
        thresholded = object()
        namespace = {
            "Manhattan": manhattan,
            "P_norm": p_norm,
            "Euclidean_thresholded": thresholded,
        }

        distance_module._install_compatibility_aliases(namespace)

        self.assertIs(namespace["Manhattan"], manhattan)
        self.assertIs(namespace["Minkowski"], p_norm)
        self.assertIs(namespace["ThresholdedEuclidean"], thresholded)

        explicit = object()
        namespace["Manhattan"] = explicit
        distance_module._install_compatibility_aliases(namespace)
        self.assertIs(namespace["Manhattan"], explicit)

        for alias, historical_name in (
            ("Minkowski", "P_norm"),
            ("ThresholdedEuclidean", "Euclidean_thresholded"),
        ):
            historical = getattr(distance_module, historical_name, None)
            if historical is not None:
                self.assertIs(getattr(distance_module, alias), historical)

    def test_correlation_import_is_clean_adapter_boundary(self):
        import importlib
        import importlib.util

        from metric.exceptions import OptionalDependencyError

        # Importing the top-level package must keep succeeding; it does not
        # import correlation.
        self.assertTrue(hasattr(metric, "OptionalDependencyError"))

        native_available = all(
            importlib.util.find_spec(name) is not None
            for name in ("metric._impl.entropy", "metric._impl.mgc")
        )
        if native_available:
            self.skipTest("native correlation bindings are present; adapter-boundary raise does not apply")

        with self.assertRaises(OptionalDependencyError) as ctx:
            importlib.import_module("metric.correlation")
        self.assertIn("metric._impl.entropy", str(ctx.exception))
        self.assertNotIsInstance(ctx.exception, ModuleNotFoundError)
        self.assertIsNone(ctx.exception.__cause__)

    def test_top_level_hides_debug_extension_exports(self):
        for name in ("DoubleVector", "LongVector", "test"):
            self.assertFalse(hasattr(metric, name), name)

    def test_legacy_tree_binding_uses_boolean_empty_and_named_insert_if(self):
        from metric.space import Tree

        tree = Tree()
        self.assertIs(tree.empty(), True)
        self.assertEqual(tree.size(), 0)
        self.assertEqual(len(tree), 0)

        first_id = tree.insert([1.0, 2.0])
        self.assertEqual(first_id, 0)
        self.assertIs(tree.empty(), False)
        self.assertEqual(tree.size(), 1)
        self.assertEqual(len(tree), 1)

        second_id, inserted = tree.insert_if([2.0, 3.0], 0.1)
        self.assertEqual(second_id, 1)
        self.assertIs(inserted, True)

        duplicate_id, inserted = tree.insert_if([2.0, 3.0], 10.0)
        self.assertEqual(duplicate_id, 1)
        self.assertIs(inserted, False)

        with self.assertRaises(TypeError):
            tree.insert([3.0, 4.0], 0.1)

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

    def test_promoted_notebooks_run(self):
        smoke_script = Path(__file__).resolve().parents[2] / "notebooks" / "smoke_notebooks.py"
        result = subprocess.run(
            [sys.executable, str(smoke_script)],
            check=False,
            env={**os.environ, "PYTHONDONTWRITEBYTECODE": "1"},
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        self.assertEqual(result.returncode, 0, result.stdout)

    def test_finite_metric_space_is_an_adapter_container(self):
        space = FiniteMetricSpace(self.records, self.metric)

        self.assertEqual(len(space), len(self.records))
        self.assertEqual(space.ids, [0, 1, 2, 3])
        self.assertEqual(space.version(), 0)
        self.assertEqual(space.validate, "sample")
        self.assertEqual(space.cache, CachePolicy("auto"))
        self.assertIn("FiniteMetricSpace(size=4, metric=", repr(space))
        named_space = Space(self.records, self.metric, name="words")
        self.assertIn("Space(size=4, metric=", repr(named_space))
        self.assertFalse(space.is_stale())
        self.assertIs(space.ensure_fresh(), space)
        self.assertEqual(space[0], "cat")
        self.assertEqual(space.record(0), "cat")
        self.assertEqual(space(0, 1), 1)
        self.assertEqual(space.distance(0, 2), 1)

        # pairwise_distances() materializes the explicit representation; it
        # returns a copy and only invokes the caller's metric (an adapter).
        distances = space.pairwise_distances()
        distances[0][1] = 999
        self.assertEqual(space(0, 1), 1)
        self.assertEqual(space.pairwise(ids=[0, 2]), [[0, 1], [1, 0]])

        matrix = space.to_matrix()
        self.assertIsInstance(matrix, MatrixSpace)
        self.assertIsNot(matrix, space)
        self.assertIs(matrix.source_space, space)
        self.assertEqual(matrix.representation, "matrix")
        self.assertEqual(matrix.pairwise_distances(), space.pairwise_distances())

        # Exact-scan search now runs the pair loop through the native binding.
        self.assertEqual([neighbor.record for neighbor in space.knn("cut", 2).neighbors], ["cat", "cot"])
        self.assertEqual(space.nn("cut").record, "cat")
        self.assertEqual([neighbor.record for neighbor in space.rnn("cut", 1).neighbors], ["cat", "cot"])
        self.assertEqual([neighbor.id for neighbor in space.neighbors("cut", 2).neighbors], [0, 1])
        identified = Space(self.records, self.metric, ids=["cat-id", "cot-id", "coat-id", "dog-id"])
        self.assertEqual(identified.nearest("cut").id, "cat-id")

        # A tree index is a stored representation; indexed search is native-only.
        tree_index = space.to_tree()
        self.assertIsInstance(tree_index, TreeIndex)
        self.assertIs(tree_index.source_space, space)
        self.assertEqual(tree_index.source_version, space.version())
        self.assertEqual(tree_index.record_count, len(space))
        self.assertTrue(tree_index.exact)
        self.assertEqual(tree_index.representation, "exact_tree_index")
        self.assertFalse(tree_index.is_stale())
        self.assertIs(tree_index.ensure_fresh(), tree_index)
        self.assertEqual(tree_index.distance(0, 1), 1)
        self.assertRequiresNative(tree_index.knn, "cut", 2)
        self.assertRequiresNative(tree_index.neighbors, "cut", 2)
        self.assertRequiresNative(tree_index.rnn, "cut", 1)
        self.assertIs(tree(space).source_space, space)

        # Building an exact kNN graph is a native-only construction algorithm.
        self.assertRequiresNative(space.to_graph, 1)
        self.assertRequiresNative(graph, space, 1)

        # Matrix staleness bookkeeping stays in Python (it runs no algorithm).
        self.assertEqual(space.touch(), 1)
        self.assertEqual(space.version(), 1)
        self.assertTrue(matrix.is_stale())
        self.assertTrue(tree_index.is_stale())
        with self.assertRaisesRegex(StaleRepresentationError, "source version 1"):
            matrix.distance(0, 1)
        fresh_matrix = space.to_matrix()
        self.assertFalse(fresh_matrix.is_stale())
        self.assertEqual(fresh_matrix.distance(0, 1), 1)

        class CallableMetric:
            def __call__(self, lhs, rhs):
                return 0 if lhs == rhs else 1

        self.assertIsInstance(CallableMetric(), Metric)
        with self.assertRaisesRegex(MissingMetricError, "explicit metric"):
            Space(self.records)
    def test_space_constructor_runtime_options_are_explicit(self):
        def numeric_distance(lhs, rhs):
            return abs(lhs["value"] - rhs["value"])

        records = [{"value": 0}, {"value": 2}]

        shared = Space(records, metric=numeric_distance, validate="none", copy=False, cache="none")
        copied = Space(records, metric=numeric_distance, validate="none", copy=True, cache="none")

        self.assertIs(shared.records[0], records[0])
        self.assertIsNot(copied.records[0], records[0])
        self.assertEqual(copied.distance(0, 1), 2)
        self.assertEqual(copied.cache, CachePolicy("none"))

        uncached_calls = []

        def uncached_distance(lhs, rhs):
            uncached_calls.append((lhs, rhs))
            return abs(lhs - rhs)

        uncached = Space([0, 2], metric=uncached_distance, validate="none", cache="none")
        self.assertEqual(uncached_calls, [])
        self.assertEqual(uncached.distance(0, 1), 2)
        self.assertEqual(uncached.distance(0, 1), 2)
        self.assertEqual(len(uncached_calls), 2)

        lazy_calls = []

        def lazy_distance(lhs, rhs):
            lazy_calls.append((lhs, rhs))
            return abs(lhs - rhs)

        lazy_space = Space([0, 2], metric=lazy_distance, validate="none", cache=CachePolicy("lazy"))
        self.assertEqual(lazy_space.cache, CachePolicy("lazy"))
        self.assertEqual(lazy_space.distance(0, 1), 2)
        self.assertEqual(lazy_space.distance(0, 1), 2)
        self.assertEqual(lazy_calls, [(0, 2)])

        def negative_tail(lhs, rhs):
            return -1 if lhs == rhs == 3 else 0

        Space([0, 1, 2, 3], metric=negative_tail, validate="sample", cache="none")
        with self.assertRaisesRegex(MetricContractError, "non-negative"):
            Space([0, 1, 2, 3], metric=negative_tail, validate="strict", cache="none")
        with self.assertRaisesRegex(ValueError, "validate must be one of"):
            Space(records, metric=numeric_distance, validate="full")
        with self.assertRaisesRegex(TypeError, "copy must be a bool"):
            Space(records, metric=numeric_distance, copy="yes")
        with self.assertRaisesRegex(StrategyParameterError, "cache mode"):
            Space(records, metric=numeric_distance, cache="forever")

    def test_space_from_dataframe_uses_metric_and_id_column(self):
        class DataFrameLike:
            def __init__(self, rows):
                self._rows = rows

            def to_dict(self, orient):
                if orient != "records":
                    raise AssertionError(orient)
                return list(self._rows)

        rows = [
            {"sample_id": "pump-a", "temperature": 62.0, "events": ("start", "idle")},
            {"sample_id": "pump-b", "temperature": 65.0, "events": ("start", "idle")},
            {"sample_id": "valve-c", "temperature": 82.0, "events": ("start", "alarm")},
        ]

        def row_distance(lhs, rhs):
            event_penalty = 0.0 if lhs["events"] == rhs["events"] else 10.0
            return abs(lhs["temperature"] - rhs["temperature"]) + event_penalty

        dataframe = DataFrameLike(rows)
        space = Space.from_dataframe(dataframe, metric=row_distance, id_column="sample_id", name="equipment")

        self.assertEqual(space.ids, ["pump-a", "pump-b", "valve-c"])
        self.assertEqual(space.name, "equipment")
        self.assertEqual(space.record("pump-a"), {"temperature": 62.0, "events": ("start", "idle")})
        self.assertAlmostEqual(space.distance(0, 1), 3.0)
        self.assertEqual(space.pairwise(ids=["pump-a", "valve-c"]), [[0.0, 30.0], [30.0, 0.0]])
        self.assertNotIn("sample_id", space.records[0])

        with self.assertRaisesRegex(MissingMetricError, "explicit metric"):
            Space.from_dataframe(dataframe)
        with self.assertRaises(ValueError):
            Space.from_dataframe(dataframe, metric=row_distance, id_column="sample_id", ids=["a", "b", "c"])
        with self.assertRaises(KeyError):
            Space.from_dataframe(DataFrameLike([{"sample_id": "a"}, {"other": "b"}]), metric=row_distance, id_column="sample_id")

    def test_nearest_neighbor_helpers_use_native_exact_scan(self):
        space = FiniteMetricSpace(self.records, self.metric)
        self.assertEqual([neighbor.record for neighbor in space.knn("cut", 2).neighbors], ["cat", "cot"])
        self.assertEqual(space.nn("cut").record, "cat")
        self.assertEqual([neighbor.record for neighbor in space.rnn("cut", 1).neighbors], ["cat", "cot"])
        self.assertEqual([neighbor.record for neighbor in nearest_neighbors(self.records, self.metric, "cut", 2).neighbors], ["cat", "cot"])
        self.assertEqual([neighbor.record for neighbor in range_neighbors(self.records, self.metric, "cut", 1).neighbors], ["cat", "cot"])
        with self.assertRaises(TypeError):
            nearest_neighbors(self.records, self.metric, "cut", 2.5)
        # pairwise_distance_matrix uses the native all-pairs loop over the caller's metric.
        self.assertEqual(pairwise_distance_matrix(self.records, self.metric)[0][1], 1)
    def test_space_intent_methods_require_native(self):
        space = Space([0, 1, 2, 3, 4], metric=lambda lhs, rhs: abs(lhs - rhs))

        self.assertEqual([neighbor.id for neighbor in space.neighbors(0, 2).neighbors], [0, 1])
        self.assertEqual(space.nearest(0).id, 0)
        self.assertEqual([neighbor.id for neighbor in space.within_radius(0, 1).neighbors], [0, 1])

        groups = space.groups(KMedoids(groups=2))
        self.assertIsInstance(groups, ClusteringResult)
        self.assertEqual(groups.assignments, (0, 0, 0, 1, 1))
        self.assertEqual(groups.medoids, (1, 3))
        self.assertEqual(groups.cluster_sizes, (3, 2))
        sparse_space = Space([0, 1, 2, 10, 11], metric=lambda lhs, rhs: abs(lhs - rhs))
        density_groups = sparse_space.groups(DBSCAN(radius=1, min_points=3))
        self.assertEqual(density_groups.assignments, (0, 0, 0, -1, -1))
        self.assertEqual(density_groups.noise_records, (3, 4))
        outliers = sparse_space.outliers(DBSCAN(radius=1, min_points=3))
        self.assertIsInstance(outliers, OutlierResult)
        self.assertEqual([outlier.record_id for outlier in outliers.outliers], [4, 3])
        denoised = sparse_space.denoise(DBSCAN(radius=1, min_points=3))
        self.assertIsInstance(denoised, MappingResult)
        self.assertEqual(denoised.space.records, [0, 1, 2])
        self.assertEqual(denoised.source_record_ids, (0, 1, 2))
        representatives_result = space.representatives(3)
        self.assertIsInstance(representatives_result, RepresentativeSet)
        self.assertEqual(representatives_result.representatives, (0, 4, 2))
        self.assertEqual(representatives_result.nearest_representative_distances, (0.0, 1.0, 0.0, 1.0, 0.0))
        reduced = space.reduce(3)
        self.assertIsInstance(reduced, ReductionResult)
        self.assertEqual(reduced.space.records, [0, 4, 2])
        self.assertEqual(reduced.source_record_ids, (0, 4, 2))
        compressed = space.compress(3)
        self.assertIsInstance(compressed, CompressionResult)
        self.assertEqual(compressed.compressed_record_count, 3)
        self.assertAlmostEqual(compressed.compression_ratio, 0.6)
        self.assertRequiresNative(space.reduce, None, KMedoids(groups=2))
        self.assertRequiresNative(space.embed)
        self.assertRequiresNative(space.embed, 1)
        description = space.describe()
        self.assertIsInstance(description, StructureDescription)
        self.assertEqual(description.record_count, 5)
        self.assertEqual(description.pair_count, 10)
        self.assertEqual(description.zero_distance_pair_count, 0)
        self.assertEqual(description.minimum_nonzero_distance, 1.0)
        self.assertEqual(description.maximum_distance, 4.0)
        self.assertAlmostEqual(description.average_distance, 2.0)
        self.assertAlmostEqual(description.intrinsic_dimension, np.log(5.0 / 3.0) / np.log(2.0))
        self.assertTrue(description.has_nonzero_distances)
        self.assertEqual(space.describe_structure(), description)
        space_comparison = space.compare(space)
        self.assertIsInstance(space_comparison, CorrelationResult)
        self.assertEqual(space_comparison.left_record_count, 5)
        self.assertEqual(space_comparison.right_record_count, 5)
        self.assertEqual(space_comparison.pair_count, 10)
        self.assertAlmostEqual(space_comparison.value, 1.0)
        self.assertEqual(space_comparison.align, "position")
        self.assertEqual(space_comparison.statistic_name, "distance_profile_correlation")
        self.assertTrue(space_comparison.diagnostics["defined"])
        self.assertIsInstance(space.correlate(space), CorrelationResult)

        # The matching operator free functions also raise.
        metric = lambda lhs, rhs: abs(lhs - rhs)
        records = [0, 1, 2, 10, 11]
        self.assertEqual(kmedoids(records, metric, 2).assignments, (0, 0, 0, 1, 1))
        self.assertEqual(find_groups(records, metric, 2).medoids, (1, 3))
        self.assertEqual(dbscan(records, metric, 1, 3).noise_records, (3, 4))
        self.assertEqual(
            [outlier.record_id for outlier in find_outliers(records, metric, DBSCAN(radius=1, min_points=3)).outliers],
            [4, 3],
        )
        self.assertEqual(denoise_space(records, metric, DBSCAN(radius=1, min_points=3)).space.records, [0, 1, 2])
        self.assertEqual(find_representatives(records, metric, 2).representatives, (0, 4))
        self.assertEqual(reduce_space(records, metric, 2).source_record_ids, (0, 4))
        self.assertEqual(compress_space(records, metric, 2).compressed_record_count, 2)
        self.assertRequiresNative(embed_space, records, metric)
        free_comparison = compare_spaces(records, metric, records, metric)
        self.assertIsInstance(free_comparison, CorrelationResult)
        self.assertAlmostEqual(free_comparison.value, 1.0)
        self.assertEqual(free_comparison.pair_count, 10)
        self.assertEqual(free_comparison.matched_ids, (0, 1, 2, 3, 4))
        self.assertIsInstance(correlate_spaces(records, metric, records, metric), CorrelationResult)
        structure = describe_structure(records, metric)
        self.assertEqual(structure.record_count, 5)
        self.assertEqual(structure.pair_count, 10)
        self.assertEqual(structure.maximum_distance, 11.0)

        # Space.map applies the caller's own deterministic transform (adapter).
        mapped = space.map(
            lambda record: {"value": record},
            metric=lambda lhs, rhs: abs(lhs["value"] - rhs["value"]),
        )
        self.assertIsInstance(mapped, MappingResult)
        self.assertEqual(mapped.space.records, [{"value": value} for value in range(5)])
        self.assertEqual(mapped.mapping, "deterministic_transform")
        self.assertEqual(mapped.strategy, "deterministic_transform")
        self.assertFalse(mapped.inverse_supported)
        self.assertEqual(mapped.space.distance(0, 4), 4)
        self.assertEqual(space.map(transform=lambda record: record * record).space.records, [0, 1, 4, 9, 16])
        self.assertEqual(
            map_space([0, 1, 2], lambda record: record * record, lambda lhs, rhs: abs(lhs - rhs)).space.records,
            [0, 1, 4],
        )
        with self.assertRaisesRegex(UnsupportedOperationError, "inverse_supported=False"):
            mapped.inverse_transform()

        # Space.map still rejects ambiguous / unpromoted mapping intents.
        with self.assertRaisesRegex(AmbiguousIntentError, "requires transform"):
            space.map()
        with self.assertRaisesRegex(AmbiguousIntentError, "either transform or target"):
            space.map(transform=lambda record: record, target=space)
        with self.assertRaisesRegex(StrategyUnavailableError, "strategy mappings are not promoted"):
            space.map(transform=lambda record: record, strategy=KMedoids(groups=2))
        with self.assertRaisesRegex(AmbiguousIntentError, "omit transform and target"):
            space.map(transform=lambda record: record, strategy=PhateAE(dimensions=2))
        with self.assertRaisesRegex(StrategyUnavailableError, r"native C\+\+ binding"):
            space.map(strategy=PhateAE(dimensions=1))
        with self.assertRaisesRegex(StrategyUnavailableError, "target mappings are not promoted"):
            space.map(target=space)

        # mappings.clustered_space marshals a clustering RESULT (built directly
        # here, not computed in Python) into a derived Space using only the
        # adapter-level space.distance lookups.
        clustering = ClusteringResult(
            assignments=(0, 0, 0, 1, 1),
            medoids=(1, 3),
            core_records=(),
            noise_records=(),
            cluster_sizes=(3, 2),
            record_count=5,
            cluster_count=2,
            noise_count=0,
            iterations=2,
            converged=True,
            algorithm="kmedoids",
            representation="metric_space",
        )
        group_space = Space([0, 1, 2, 10, 11], metric=lambda lhs, rhs: abs(lhs - rhs))
        clustered = mappings.clustered_space(group_space, clustering)
        self.assertIsInstance(clustered, MappingResult)
        self.assertEqual(clustered.mapping, "clustered_space")
        self.assertEqual(clustered.strategy, "kmedoids")
        self.assertEqual(clustered.source_records, ((0, 1, 2), (3, 4)))
        self.assertEqual(clustered.representative_records, (1, 3))
        self.assertEqual(clustered.space.distance(0, 1), group_space.distance(1, 3))
        model = mappings.fit(mappings.make_clustered_space_mapping(clustering), group_space)
        self.assertFalse(model.inverse_supported())
        self.assertEqual(mappings.transform(model, group_space).source_records, clustered.source_records)

        keyed_space = Space(
            [0, 1, 2, 10, 11],
            metric=lambda lhs, rhs: abs(lhs - rhs),
            ids=[10, 11, 12, 20, 21],
        )
        keyed_groups = keyed_space.groups(KMedoids(groups=2))
        self.assertEqual(keyed_groups.medoids, (11, 20))
        keyed_clustered = mappings.clustered_space(keyed_space, keyed_groups)
        self.assertEqual(keyed_clustered.source_records, ((10, 11, 12), (20, 21)))
        self.assertEqual(keyed_clustered.representative_records, (11, 20))
    def test_intrinsic_dimension_uses_native_binding(self):
        import metric.operators as operators

        records = [0, 1, 2, 3, 4]
        expected = np.log(5.0 / 3.0) / np.log(2.0)
        self.assertAlmostEqual(intrinsic_dimension(records, lambda lhs, rhs: abs(lhs - rhs)), expected)
        self.assertEqual(operators.intrinsic_dimension_from_distances([[0.0, 1.0], [1.0, 0.0]]), 0.0)
        with self.assertRaisesRegex(ValueError, "row 0"):
            operators.intrinsic_dimension_from_distances([[0.0, 1.0, 2.0], [1.0, 0.0]])
    def test_representative_and_graph_operators_require_native(self):
        records = [(1.0, 0.0), (0.0, 1.0), (0.5, 0.5)]

        def manhattan(lhs, rhs):
            return abs(lhs[0] - rhs[0]) + abs(lhs[1] - rhs[1])

        self.assertEqual(representative_indices(records, manhattan, 2), (0, 1))
        self.assertEqual(representatives(records, manhattan, 2), (records[0], records[1]))
        with self.assertRaisesRegex(ValueError, "positive"):
            find_representatives(records, manhattan, 0)
        self.assertEqual(medoid_index(records, manhattan), 2)
        self.assertEqual(medoid(records, manhattan), records[2])
        self.assertEqual(separated_representative_indices(records, manhattan, 1.0), (0, 1, 2))
        self.assertEqual(separated_representatives(records, manhattan, 1.0), tuple(records))
        self.assertEqual(coverage_representative_indices(records, manhattan, 1.0), (0, 1))
        self.assertEqual(coverage_representatives(records, manhattan, 1.0), (records[0], records[1]))

        for fn, args in [
            (exact_knn_graph, (records, manhattan, 1)),
            (exact_knn_graph_edges, (records, manhattan, 1)),
            (exact_radius_graph, (records, manhattan, 1.0)),
            (exact_radius_graph_edges, (records, manhattan, 1.0)),
        ]:
            self.assertRequiresNative(fn, *args)

        # Graph post-processing / diagnostics operate on a marshaled graph
        # result, which can still be constructed directly; the analyses are
        # native-only.
        graph_result = GraphConstructionResult(
            edges=((0, 1, 1.0), (1, 2, 1.0)),
            metadata=GraphConstructionMetadata(
                strategy="synthetic",
                record_count=3,
                edge_count=2,
                directed=True,
                self_loops=False,
                exact=True,
            ),
        )
        self.assertRequiresNative(symmetrize_graph, graph_result)
        self.assertRequiresNative(prune_graph_out_degree, graph_result, 1)
        self.assertRequiresNative(graph_degree_diagnostics, graph_result)
        self.assertRequiresNative(graph_connectivity_diagnostics, graph_result)
        self.assertRequiresNative(graph_stretch_diagnostics, records, manhattan, graph_result)

    def test_result_objects_marshal_native_payloads(self):
        import metric.operators as operators

        graph_result = GraphConstructionResult(
            edges=((0, 1, 0.5), (1, 0, 0.5)),
            metadata=GraphConstructionMetadata(
                strategy="exact_knn",
                record_count=2,
                edge_count=2,
                directed=True,
                self_loops=False,
                exact=True,
                k=1,
            ),
        )
        self.assertEqual(graph_result.to_dict()["metadata"]["strategy"], "exact_knn")
        np.testing.assert_array_equal(graph_result.to_numpy(), np.asarray(graph_result.edges, dtype=object))

        clustering = ClusteringResult(
            assignments=(0, 0, 1),
            medoids=(0, 2),
            core_records=(),
            noise_records=(),
            cluster_sizes=(2, 1),
            record_count=3,
            cluster_count=2,
            noise_count=0,
            iterations=2,
            converged=True,
            algorithm="kmedoids",
            representation="metric_space",
        )
        self.assertEqual(clustering.to_dict()["assignments"], (0, 0, 1))
        np.testing.assert_array_equal(clustering.to_numpy(), np.asarray([0, 0, 1]))

        neighbors = operators.neighbor_result([10, 20, 30], query=15, neighbors=[(0, 5), (1, 5)])
        self.assertIsInstance(neighbors, NeighborResult)
        self.assertEqual(neighbors.neighbors[0].record, 10)
        self.assertEqual(neighbors.distances, (5, 5))
        self.assertEqual(neighbors.strategy, "exact_scan")

        outliers = OutlierResult(
            outliers=(Outlier(record_id=2, score=9),),
            record_count=3,
            cluster_count=1,
            noise_count=1,
            exact=True,
            operator_name="find_outliers",
            strategy="dbscan_noise",
            representation="metric_space",
        )
        self.assertEqual(outliers.to_dict()["outliers"], [{"record_id": 2, "score": 9}])
        np.testing.assert_array_equal(outliers.to_numpy(), np.asarray([9.0]))
    def test_edit_metric_satisfies_metric_contracts(self):
        self.assertMetricContracts(self.records, self.metric)

    def test_space_facade_exposes_intent_names(self):
        space = Space(self.records, self.metric)

        self.assertIsInstance(space, FiniteMetricSpace)
        for name in (
            "neighbors",
            "nearest",
            "within_radius",
            "groups",
            "outliers",
            "denoise",
            "representatives",
            "reduce",
            "compress",
            "embed",
            "describe",
            "describe_structure",
            "compare",
            "correlate",
            "map",
            "runtime_diagnostics",
            "knn",
            "nn",
            "rnn",
        ):
            self.assertTrue(callable(getattr(space, name)), name)

        # The intent vocabulary stays importable and exact search is bound natively.
        self.assertEqual([neighbor.record for neighbor in space.neighbors("cut", 2).neighbors], ["cat", "cot"])
        self.assertEqual(space.nearest("cut").record, "cat")
        self.assertEqual([neighbor.record for neighbor in space.within_radius("cut", 1).neighbors], ["cat", "cot"])

    def test_compare_correlate_aligned_distance_profile_is_native(self):
        absolute = lambda lhs, rhs: abs(lhs - rhs)
        process = Space([0.0, 1.0, 2.0, 3.0], metric=absolute, ids=["a", "b", "c", "d"])
        # Monotone transform keeps the distance profile perfectly correlated.
        scaled = Space([0.0, 2.0, 4.0, 6.0], metric=absolute, ids=["a", "b", "c", "d"])

        comparison = process.compare(scaled)
        self.assertIsInstance(comparison, CorrelationResult)
        self.assertEqual(comparison.left_record_count, 4)
        self.assertEqual(comparison.right_record_count, 4)
        self.assertEqual(comparison.pair_count, 6)
        self.assertTrue(comparison.exact)
        self.assertEqual(comparison.algorithm, "distance_profile_correlation")
        self.assertEqual(comparison.strategy, "distance_profile_correlation")
        self.assertEqual(comparison.statistic_name, "distance_profile_correlation")
        self.assertEqual(comparison.align, "position")
        self.assertEqual(comparison.matched_ids, (0, 1, 2, 3))
        self.assertAlmostEqual(comparison.value, 1.0)
        self.assertTrue(comparison.diagnostics["defined"])

        # correlate_spaces is the operator-level alias of compare_spaces.
        free = correlate_spaces([0.0, 1.0, 3.0], absolute, [0.0, 2.0, 1.0], absolute)
        self.assertIsInstance(free, CorrelationResult)
        self.assertEqual(free.pair_count, 3)
        self.assertTrue(free.diagnostics["defined"])

        # The DistanceProfileCorrelation strategy token selects the same path.
        strategy_comparison = process.compare(scaled, strategy=DistanceProfileCorrelation())
        self.assertAlmostEqual(strategy_comparison.value, 1.0)

    def test_compare_mismatched_counts_names_the_mismatch(self):
        absolute = lambda lhs, rhs: abs(lhs - rhs)
        left = Space([0.0, 1.0, 2.0], metric=absolute)
        right = Space([0.0, 1.0], metric=absolute)

        with self.assertRaisesRegex(IncompatibleSpaceError, r"3 records.*2 records"):
            left.compare(right)
        with self.assertRaisesRegex(IncompatibleSpaceError, r"3 records.*2 records"):
            compare_spaces([0.0, 1.0, 2.0], absolute, [0.0, 1.0], absolute)

    def test_compare_degenerate_profile_returns_undefined_sentinel(self):
        absolute = lambda lhs, rhs: abs(lhs - rhs)

        # Fewer than two records yields no pairs -> correlation is undefined.
        single = Space([5.0], metric=absolute)
        empty_profile = single.compare(single)
        self.assertEqual(empty_profile.pair_count, 0)
        self.assertEqual(empty_profile.value, 0.0)
        self.assertFalse(empty_profile.diagnostics["defined"])
        self.assertTrue(empty_profile.diagnostics["degenerate"])

        # A constant (zero-variance) distance profile is also undefined.
        constant = Space([0.0, 1.0, 2.0], metric=lambda lhs, rhs: 0.0 if lhs == rhs else 1.0)
        varied = Space([0.0, 1.0, 2.0], metric=absolute)
        degenerate = constant.compare(varied)
        self.assertEqual(degenerate.pair_count, 3)
        self.assertEqual(degenerate.value, 0.0)
        self.assertFalse(degenerate.diagnostics["defined"])

    def test_embed_stays_unpromoted_after_compare_is_promoted(self):
        space = Space([0.0, 1.0, 2.0], metric=lambda lhs, rhs: abs(lhs - rhs))
        self.assertRequiresNative(space.embed)
        self.assertRequiresNative(embed_space, [0.0, 1.0], lambda lhs, rhs: abs(lhs - rhs))
    def test_runtime_policy_is_explicit_and_exact(self):
        space = Space(self.records, self.metric)
        policy = RuntimePolicy(exact=True, parallel=True, cache="materialized")

        self.assertEqual(policy.cache_mode, "materialized")
        self.assertEqual(policy.name, "exact_materialized_parallel")
        self.assertEqual(policy.cache.to_dict(), {"mode": "materialized"})
        self.assertEqual(policy.to_dict()["representation_preference"], "matrix")
        self.assertEqual(runtime.materialized(runtime.parallel()).name, "exact_materialized_parallel")
        self.assertEqual(runtime.using_matrix().representation_preference, "matrix")
        self.assertEqual(runtime.using_tree().representation_preference, "exact_tree_index")
        self.assertEqual(runtime.using_graph(1).representation_preference, "exact_knn_graph")
        diagnostics = runtime_diagnostics(policy, representation="matrix", intent="neighbors")
        self.assertIsInstance(diagnostics, RuntimeDiagnostics)
        self.assertEqual(diagnostics.policy_name, "exact_materialized_parallel")
        self.assertTrue(diagnostics.exact)
        self.assertTrue(diagnostics.parallel)
        self.assertEqual(diagnostics.cache_mode, "materialized")
        self.assertEqual(diagnostics.representation, "matrix")
        self.assertEqual(diagnostics.intent, "neighbors")
        self.assertTrue(diagnostics.supported)
        self.assertEqual(diagnostics.reason, "")
        self.assertEqual(diagnostics.to_dict()["policy_name"], "exact_materialized_parallel")

        # Space.runtime_diagnostics reports policy metadata only (an adapter).
        space_diagnostics = space.runtime_diagnostics(
            representation=space.to_matrix(),
            runtime=policy,
            intent="neighbors",
        )
        self.assertEqual(space_diagnostics.representation, "matrix")
        self.assertEqual(space_diagnostics.policy_name, policy.name)
        approximate_diagnostics = runtime_diagnostics(RuntimePolicy(exact=False), representation="tree")
        self.assertFalse(approximate_diagnostics.supported)
        self.assertIn("approximate", approximate_diagnostics.reason)

        self.assertEqual([neighbor.record for neighbor in space.neighbors("cut", 2).neighbors], ["cat", "cot"])
        self.assertEqual(space.groups(count=2).cluster_count, 2)
        described = space.describe(representation=space.to_matrix(), runtime=policy)
        self.assertEqual(described.record_count, len(self.records))
        self.assertEqual(described.representation, "matrix")
        self_comparison = space.compare(space, runtime=policy)
        self.assertIsInstance(self_comparison, CorrelationResult)
        self.assertAlmostEqual(self_comparison.value, 1.0)

        with self.assertRaises(StrategyParameterError):
            RuntimePolicy(cache="unknown")
        with self.assertRaises(StrategyParameterError):
            RuntimePolicy(representation="unknown")
        with self.assertRaises(StrategyParameterError):
            RuntimePolicy(representation="graph", graph_count=-1)
        with self.assertRaises(StrategyParameterError):
            RuntimePolicy(parallel="yes")
    def test_numpy_record_arrays_use_custom_metric_callable(self):
        records = np.array([[0.0, 0.0], [3.0, 4.0], [6.0, 8.0]])

        def euclidean(lhs, rhs):
            return float(np.linalg.norm(lhs - rhs))

        space = Space(records, euclidean)
        vector_space = Space.vectors(
            records,
            metric=euclidean,
            ids=["origin", "middle", "far"],
            name="vectors",
        )

        self.assertEqual(len(space), 3)
        self.assertAlmostEqual(space.distance(0, 1), 5.0)
        self.assertAlmostEqual(pairwise_distance_matrix(records, euclidean)[1][2], 5.0)
        self.assertMetricContracts(records, euclidean)
        self.assertEqual(vector_space.ids, ["origin", "middle", "far"])
        self.assertEqual(vector_space.name, "vectors")
        self.assertIs(vector_space.metric, euclidean)
        self.assertAlmostEqual(vector_space.distance(0, 1), 5.0)
        self.assertMetricContracts(records, vector_space.metric)

        self.assertEqual(space.nearest(np.array([3.0, 4.0])).id, 1)
        self.assertEqual(range_neighbors(records, euclidean, np.array([3.0, 4.0]), 0.0).neighbors[0].id, 1)

        manhattan_space = Space.vectors(
            records,
            metric=lambda lhs, rhs: float(np.abs(lhs - rhs).sum()),
        )
        self.assertEqual(manhattan_space.distance(0, 1), 7.0)
        def checked_vector_metric(lhs, rhs):
            if len(lhs) != len(rhs):
                raise MetricInputError("vector records must have the same length")
            return float(np.linalg.norm(np.asarray(lhs) - np.asarray(rhs)))

        with self.assertRaisesRegex(MetricInputError, "same length"):
            Space.vectors([[0.0], [0.0, 1.0]], metric=checked_vector_metric)
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

        # Adapter surface: distances and the explicit matrix over a domain metric.
        self.assertGreater(space.distance(0, 2), 10.0)
        self.assertAlmostEqual(pairwise_distance_matrix(records, structured_record_distance)[0][1], 0.25)
        self.assertMetricContracts(records, structured_record_distance)

        self.assertEqual(space.nearest(query).record["id"], "pump-a")
        self.assertEqual([neighbor.record["id"] for neighbor in space.neighbors(query, 2).neighbors], ["pump-a", "pump-b"])
        self.assertEqual(len(representative_indices(records, structured_record_distance, 3)), 3)
        self.assertGreaterEqual(len(separated_representative_indices(records, structured_record_distance, 2.0)), 1)
        self.assertGreaterEqual(len(coverage_representative_indices(records, structured_record_distance, 1.5)), 1)
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

        self.assertEqual(space.distance(0, 1), 2.0)
        self.assertEqual(space.distance(0, 2), 6.0)
        self.assertEqual(pairwise_distance_matrix(records, aligned_curve_distance)[3][2], 9.0)
        self.assertMetricContracts(records, aligned_curve_distance)

        self.assertEqual(space.nearest(query).id, 0)
        self.assertEqual(len(representative_indices(records, aligned_curve_distance, 3)), 3)
        self.assertGreaterEqual(len(separated_representative_indices(records, aligned_curve_distance, 4.0)), 1)
        self.assertGreaterEqual(len(coverage_representative_indices(records, aligned_curve_distance, 4.0)), 1)
        self.assertGreaterEqual(intrinsic_dimension(records, aligned_curve_distance), 0.0)
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

        self.assertEqual(space.distance(0, 1), 1.0)
        self.assertEqual(space.distance(0, 2), 3.0)
        self.assertEqual(space.distance(3, 4), 1.0)
        self.assertEqual(pairwise_distance_matrix(records, cumulative_transport_distance)[1][4], 0.5)
        self.assertMetricContracts(records, cumulative_transport_distance)

        self.assertEqual(space.nearest(query).id, 1)
        self.assertGreaterEqual(intrinsic_dimension(records, cumulative_transport_distance), 0.0)

    def test_distance_matrix_numpy_marshals_pairwise_in_record_order(self):
        records = [0.0, 1.0, 4.0]
        space = Space(records, metric=lambda lhs, rhs: abs(lhs - rhs))

        matrix = space.distance_matrix_numpy()

        self.assertIsInstance(matrix, np.ndarray)
        self.assertEqual(matrix.shape, (len(records), len(records)))
        # Record order is preserved: [i][j] is the known |records[i] - records[j]|.
        for i, lhs in enumerate(records):
            for j, rhs in enumerate(records):
                self.assertEqual(matrix[i][j], abs(lhs - rhs))
        # Symmetric metric -> symmetric matrix; identity distance -> zero diagonal.
        self.assertTrue(np.array_equal(matrix, matrix.T))
        self.assertTrue(np.array_equal(np.diagonal(matrix), np.zeros(len(records))))

    def test_distance_matrix_numpy_requires_numpy(self):
        space = Space([0.0, 1.0], metric=lambda lhs, rhs: abs(lhs - rhs))
        original_numpy = spaces_module._numpy

        def _raise_missing_numpy():
            raise OptionalDependencyError(
                "Space.distance_matrix_numpy() requires numpy."
            ) from ModuleNotFoundError("numpy")

        spaces_module._numpy = _raise_missing_numpy
        try:
            with self.assertRaises(OptionalDependencyError):
                space.distance_matrix_numpy()
        finally:
            spaces_module._numpy = original_numpy


if __name__ == "__main__":
    unittest.main()
