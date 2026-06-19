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


if __name__ == "__main__":
    unittest.main()
