"""Execute the promoted Python notebooks without requiring Jupyter.

The notebook smoke path intentionally uses only the standard library. CI builds
and installs the wheel first, then this script executes each notebook code cell
in order in an isolated namespace.
"""

from __future__ import annotations

import json
from pathlib import Path


NOTEBOOK_DIR = Path(__file__).resolve().parent
REQUIRED_NOTEBOOKS = (
    "00_strings_as_metric_space.ipynb",
    "01_dataframe_records_custom_metric.ipynb",
    "02_neighbors_groups_outliers.ipynb",
    "03_embed_map_density_filter.ipynb",
    "04_compare_aligned_spaces.ipynb",
    "05_strategy_overrides.ipynb",
)
FIRST_CODE_CELL = "from metric import Space\nfrom metric import metrics\n"


def _source(cell):
    source = cell.get("source", "")
    if isinstance(source, list):
        return "".join(source)
    return str(source)


def _code_cells(notebook):
    return [cell for cell in notebook.get("cells", []) if cell.get("cell_type") == "code"]


def _validate_notebook(path, notebook):
    if notebook.get("nbformat") != 4:
        raise AssertionError(f"{path.name}: expected nbformat 4")

    cells = notebook.get("cells")
    if not isinstance(cells, list) or not cells:
        raise AssertionError(f"{path.name}: notebook has no cells")

    code_cells = _code_cells(notebook)
    if not code_cells:
        raise AssertionError(f"{path.name}: notebook has no code cells")

    first_source = _source(code_cells[0])
    if first_source != FIRST_CODE_CELL:
        raise AssertionError(f"{path.name}: first code cell must import only Space and metrics")

    for index, cell in enumerate(code_cells, start=1):
        if cell.get("outputs") not in ([], None):
            raise AssertionError(f"{path.name}: code cell {index} must not store outputs")

        source = _source(cell)
        if "metric.strategies" in source and path.name != "05_strategy_overrides.ipynb":
            raise AssertionError(f"{path.name}: strategy imports belong in 05_strategy_overrides.ipynb")


def _execute_notebook(path, notebook):
    namespace = {"__name__": f"metric_notebook_{path.stem}"}
    for index, cell in enumerate(_code_cells(notebook), start=1):
        source = _source(cell)
        exec(compile(source, f"{path}:cell-{index}", "exec"), namespace)


def main():
    for filename in REQUIRED_NOTEBOOKS:
        path = NOTEBOOK_DIR / filename
        with path.open("r", encoding="utf-8") as handle:
            notebook = json.load(handle)
        _validate_notebook(path, notebook)
        _execute_notebook(path, notebook)
        print(f"notebook smoke ok: {filename}")


if __name__ == "__main__":
    main()
