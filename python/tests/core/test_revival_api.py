import unittest

import metric
import numpy as np
from metric.metrics import Edit, available
from metric import mappings, transforms
from metric.operators import nearest_neighbors, pairwise_distance_matrix, range_neighbors
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
        self.assertIs(metric.mappings, mappings)
        self.assertIs(metric.transforms, transforms)
        self.assertIs(metric.Edit, Edit)
        self.assertIs(metric.range_neighbors, range_neighbors)
        self.assertIs(metric.FiniteMetricSpace, FiniteMetricSpace)
        self.assertIs(metric.Space, Space)
        self.assertIn("FiniteMetricSpace", metric.__all__)
        self.assertIn("Space", metric.__all__)
        self.assertIn("mappings", metric.__all__)
        self.assertIn("transforms", metric.__all__)
        self.assertEqual(mappings.STABILITY, "beta")
        self.assertEqual(transforms.STABILITY, "beta")
        self.assertIsInstance(mappings.available(), tuple)
        self.assertIsInstance(transforms.available(), tuple)

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
        self.assertMetricContracts(records, structured_record_distance)


if __name__ == "__main__":
    unittest.main()
