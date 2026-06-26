import unittest

from metric.operators import RedifMeasureResult, add_noise_space, find_outliers, redif_dynamics, remove_noise_space
from metric.spaces import Space
from metric.strategies import ForwardDynamics, InverseDynamics, RedifDynamics, TransportPath


class RedifPythonSurfaceTest(unittest.TestCase):
    def test_redif_measure_dynamics_and_transport_path_strategy(self):
        records = [0, 1, 2, 100]
        metric = lambda lhs, rhs: abs(lhs - rhs)

        inverse = remove_noise_space(
            records,
            metric,
            InverseDynamics(neighbors=1, iterations=2, euler_step=0.25, adaptive_geometry=False),
        )
        self.assertIsInstance(inverse, RedifMeasureResult)
        self.assertEqual(inverse.operator_name, "redif_remove_noise")
        self.assertEqual(len(inverse.paths), len(records))
        self.assertEqual(len(inverse.step_diagnostics), 2)
        self.assertEqual(inverse.transport_diagnostics["solver"], "exact_discrete_wasserstein")
        self.assertEqual(
            inverse.operator_diagnostics[0]["spectral_gap_proxy"],
            "minimum_transition_escape_probability",
        )
        self.assertGreaterEqual(inverse.operator_diagnostics[0]["spectral_gap_proxy_value"], 0.0)
        self.assertIn("minimum_transition_escape_probability", inverse.step_diagnostics[0]["operator_diagnostics"])
        self.assertEqual(inverse.operator_diagnostics[0]["local_relation_representation"], "dense_distance_matrix")
        self.assertEqual(inverse.operator_diagnostics[0]["local_relation_exactness"], "exact")
        self.assertTrue(inverse.operator_diagnostics[0]["local_relation_exact"])
        self.assertGreater(inverse.operator_diagnostics[0]["local_relation_distance_evaluations"], 0)
        self.assertEqual(inverse.transport_diagnostics["transport_problem_count"], len(records) * 2)
        self.assertEqual(inverse.transport_diagnostics["exactness"], "exact")
        self.assertFalse(inverse.transport_diagnostics["support_truncated"])

        forward = add_noise_space(
            records,
            metric,
            ForwardDynamics(neighbors=1, iterations=2, euler_step=0.25, adaptive_geometry=False),
        )
        self.assertEqual(forward.operator_name, "redif_add_noise")
        self.assertGreater(forward.summaries[0].terminal_shannon_entropy, 0.0)

        generic = redif_dynamics(
            records,
            metric,
            RedifDynamics(neighbors=1, iterations=1, euler_step=0.25, adaptive_geometry=False),
            direction="forward",
        )
        self.assertEqual(generic.operator_name, "redif_add_noise")

        ranked = find_outliers(
            records,
            metric,
            TransportPath(neighbors=1, iterations=2, euler_step=0.25, adaptive_geometry=True),
        )
        self.assertEqual(ranked.strategy, "redif_transport_path_length")
        self.assertEqual(ranked.outliers[0].record_id, 3)

    def test_redif_rejects_ambiguous_boolean_configuration(self):
        records = [0, 1, 2]
        metric = lambda lhs, rhs: abs(lhs - rhs)

        with self.assertRaises(TypeError):
            remove_noise_space(
                records,
                metric,
                InverseDynamics(neighbors=1, iterations=1, euler_step=0.25, adaptive_geometry="false"),
            )

    def test_space_methods_preserve_space_record_ids(self):
        records = [0, 1, 2, 100]
        metric = lambda lhs, rhs: abs(lhs - rhs)
        space = Space(records, metric=metric, ids=("a", "b", "c", "d"))

        inverse = space.remove_noise(
            InverseDynamics(neighbors=1, iterations=1, euler_step=0.25, adaptive_geometry=False)
        )
        self.assertEqual(inverse.paths[0].record_id, "a")
        self.assertEqual(inverse.summaries[-1].record_id, "d")

        forward = space.add_noise(
            ForwardDynamics(neighbors=1, iterations=1, euler_step=0.25, adaptive_geometry=False)
        )
        self.assertEqual(forward.operator_name, "redif_add_noise")

        dynamics = space.dynamics(
            RedifDynamics(neighbors=1, iterations=1, euler_step=0.25, adaptive_geometry=False),
            direction="inverse",
        )
        self.assertEqual(dynamics.operator_name, "redif_remove_noise")

        ranked = space.outliers(
            TransportPath(neighbors=1, iterations=2, euler_step=0.25, adaptive_geometry=True)
        )
        self.assertEqual(ranked.outliers[0].record_id, "d")

    def test_redif_surface_uses_only_the_supplied_metric(self):
        def histogram_distance(lhs, rhs):
            cumulative = 0.0
            total = 0.0
            for left, right in zip(lhs, rhs):
                cumulative += left - right
                total += abs(cumulative)
            return total

        cases = [
            (
                ["a", "aa", "aaa", "aaaaaaaa"],
                lambda lhs, rhs: abs(len(lhs) - len(rhs)),
            ),
            (
                [(1.0, 0.0, 0.0), (0.8, 0.2, 0.0), (0.0, 0.2, 0.8), (0.0, 0.0, 1.0)],
                histogram_distance,
            ),
            (
                [(0.0, 0.0), (1.0, 0.0), (2.0, 0.0), (10.0, 0.0)],
                lambda lhs, rhs: ((lhs[0] - rhs[0]) ** 2 + (lhs[1] - rhs[1]) ** 2) ** 0.5,
            ),
            (
                [("pump", 0), ("pump", 1), ("valve", 0), ("valve", 5)],
                lambda lhs, rhs: (0 if lhs[0] == rhs[0] else 3) + abs(lhs[1] - rhs[1]),
            ),
        ]

        for records, metric in cases:
            with self.subTest(records=records):
                result = remove_noise_space(
                    records,
                    metric,
                    InverseDynamics(neighbors=1, iterations=1, euler_step=0.25, adaptive_geometry=False),
                )
                self.assertEqual(result.record_count, len(records))
                self.assertEqual(result.operator_name, "redif_remove_noise")
                self.assertEqual(len(result.paths), len(records))


if __name__ == "__main__":
    unittest.main()
