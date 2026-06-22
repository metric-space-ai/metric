#!/usr/bin/env python3
"""Build, install, and smoke-test the Python wheel in an isolated venv."""

from __future__ import annotations

import os
import shutil
import subprocess
import sys
import tempfile
import textwrap
import venv
from pathlib import Path


def run(command: list[str], *, cwd: Path | None = None, env: dict[str, str] | None = None) -> None:
    print("+", " ".join(command), flush=True)
    subprocess.run(command, cwd=cwd, env=env, check=True)


def venv_python(venv_dir: Path) -> Path:
    if os.name == "nt":
        return venv_dir / "Scripts" / "python.exe"
    return venv_dir / "bin" / "python"


def create_venv(venv_dir: Path) -> None:
    try:
        venv.EnvBuilder(with_pip=True, clear=True).create(venv_dir)
        return
    except subprocess.CalledProcessError:
        uv = shutil.which("uv")
        if uv is None:
            raise
        if venv_dir.exists():
            shutil.rmtree(venv_dir)
        run([uv, "venv", "--clear", "--seed", "--python", sys.executable, str(venv_dir)])


def promoted_python_examples(repo: Path) -> list[Path]:
    example_roots = [
        repo / "python" / "examples" / "metric_space",
        repo / "python" / "examples" / "engine",
    ]
    examples: list[Path] = []
    for root in example_roots:
        examples.extend(sorted(root.glob("*.py")))
    return examples


def main() -> int:
    repo = Path(__file__).resolve().parents[1]
    python_project = repo / "python"

    with tempfile.TemporaryDirectory(prefix="metric-wheel-smoke-") as tmp:
        tmp_dir = Path(tmp)
        env_dir = tmp_dir / "venv"
        wheelhouse = tmp_dir / "wheelhouse"
        wheelhouse.mkdir()

        create_venv(env_dir)
        python = venv_python(env_dir)

        run(
            [
                str(python),
                "-m",
                "pip",
                "install",
                "--upgrade",
                "pip",
                "setuptools>=77",
                "wheel",
                "cmake>=3.19",
                "pybind11>=3.0.0",
                "numpy>=1.23",
            ]
        )

        build_env = os.environ.copy()
        build_env.pop("PYTHONPATH", None)
        build_env["METRIC_SOURCE_PATH"] = str(repo)
        build_env.setdefault("CMAKE_COMMON_VARIABLES", "-DMETRIC_PYTHON_BUILD_FULL=OFF")

        run(
            [
                str(python),
                "-m",
                "pip",
                "wheel",
                "--no-build-isolation",
                "--no-deps",
                str(python_project),
                "-w",
                str(wheelhouse),
            ],
            cwd=tmp_dir,
            env=build_env,
        )

        wheels = sorted(wheelhouse.glob("mtrc-*.whl"))
        if not wheels:
            raise RuntimeError("wheel build did not produce an mtrc wheel")
        wheel = wheels[-1]
        run([str(python), "-m", "pip", "install", "--force-reinstall", str(wheel)])

        smoke_env = os.environ.copy()
        smoke_env.pop("PYTHONPATH", None)
        smoke_env["PYTHONDONTWRITEBYTECODE"] = "1"
        smoke_env["METRIC_WHEEL_SMOKE_REPO"] = str(repo)
        smoke = textwrap.dedent(
            r"""
            import os
            from pathlib import Path

            import metric
            import numpy as np
            from metric.exceptions import StrategyUnavailableError
            from metric.metrics import Edit
            from metric.operators import (
                exact_knn_graph,
                graph_connectivity_diagnostics,
                graph_degree_diagnostics,
            )
            from metric.runtime import RuntimePolicy, runtime_diagnostics, using_matrix
            from metric.spaces import Space
            from metric.strategies import KMedoids, MDS, PCFA, PhateAE
            from metric import mappings

            repo = Path(os.environ["METRIC_WHEEL_SMOKE_REPO"]).resolve()
            module_path = Path(metric.__file__).resolve()
            assert repo not in module_path.parents, module_path
            assert "site-packages" in module_path.parts or "dist-packages" in module_path.parts, module_path

            string_space = Space(["cat", "cot", "coat", "dog"], Edit())
            neighbors = string_space.neighbors("cut", count=2)
            assert neighbors.representation == "metric_space"
            assert [neighbor.id for neighbor in neighbors.neighbors] == [0, 1]

            groups = string_space.groups(KMedoids(groups=2), runtime=using_matrix())
            assert groups.representation == "matrix"
            assert groups.record_count == 4

            diagnostics = runtime_diagnostics(RuntimePolicy(cache="materialized"), intent="neighbors")
            assert diagnostics.representation == "matrix"
            assert diagnostics.supported is True

            with np.testing.assert_raises(StrategyUnavailableError):
                string_space.reduce(strategy=PCFA(dimensions=1))

            def absolute_distance(lhs, rhs):
                return abs(lhs - rhs)

            numeric_space = Space([0, 1, 2, 10, 11], absolute_distance)
            assert numeric_space.outliers(count=1).record_count == 5
            assert numeric_space.denoise(count=1).target_record_count == 4
            assert numeric_space.representatives(count=2).requested_count == 2
            assert numeric_space.reduce(count=2).reduced_record_count == 2
            assert numeric_space.compress(count=2).compressed_record_count == 2
            assert numeric_space.embed(dimensions=2, strategy=MDS(dimensions=2)).coordinates.shape == (5, 2)
            mapped_numeric = numeric_space.map(
                transform=lambda record: record // 10,
                metric=absolute_distance,
            )
            assert mapped_numeric.mapping == "deterministic_transform"
            assert mapped_numeric.target_record_count == 5
            assert numeric_space.describe().record_count == 5
            compared = numeric_space.compare(Space([0, 1, 3, 9, 12], absolute_distance))
            assert compared.left_record_count == 5
            assert compared.right_record_count == 5

            graph = exact_knn_graph(numeric_space.records, numeric_space.metric, 2)
            assert graph.metadata.exact is True
            assert graph_degree_diagnostics(graph).record_count == 5
            assert graph_connectivity_diagnostics(graph).component_count == 1

            rows = np.asarray(
                [[0.0, 0.0], [0.5, 0.5], [1.0, 1.0], [1.5, 1.5], [2.0, 2.0]],
                dtype=float,
            )
            model = mappings.native_phate_autoencoder_fit_vectors(
                rows,
                dimensions=1,
                epochs=2,
                learning_rate=0.001,
                distance_provider="matrix_cache_distance_provider",
                affinity_kernel="exponential_affinity_kernel",
                diffusion_operator="lazy_row_normalized_diffusion_operator",
            )
            assert model.mapping == "native_phate_autoencoder"
            assert model.strategy == "native_dnn_phate_ae"
            assert model.training_report()["epoch_count"] == 2
            assert len(model.transform(rows)) == len(rows)

            vector_space = Space.vectors(rows, validate="none", cache="none")
            mapped = vector_space.map(
                strategy=PhateAE(
                    dimensions=1,
                    epochs=2,
                    learning_rate=0.001,
                    distance_provider="matrix_cache_distance_provider",
                    affinity_kernel="exponential_affinity_kernel",
                    diffusion_operator="lazy_row_normalized_diffusion_operator",
                )
            )
            assert mapped.mapping == "native_phate_autoencoder"
            assert mapped.strategy == "native_dnn_phate_ae"
            assert mapped.inverse_supported is True
            assert len(mapped.inverse_transform()) == len(rows)

            custom_metric_space = Space.vectors(
                rows,
                metric=lambda lhs, rhs: float(np.abs(lhs - rhs).sum()),
                validate="none",
                cache="none",
            )
            with np.testing.assert_raises(StrategyUnavailableError):
                custom_metric_space.map(strategy=PhateAE(dimensions=1, epochs=1))
            """
        )
        run([str(python), "-c", smoke], cwd=tmp_dir, env=smoke_env)

        for example in promoted_python_examples(repo):
            run([str(python), str(example)], cwd=tmp_dir, env=smoke_env)

        if os.environ.get("METRIC_WHEEL_SMOKE_KEEP"):
            keep_dir = Path(os.environ["METRIC_WHEEL_SMOKE_KEEP"]).expanduser().resolve()
            if keep_dir.exists():
                shutil.rmtree(keep_dir)
            shutil.copytree(tmp_dir, keep_dir)
            print(f"kept wheel smoke artifacts at {keep_dir}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
