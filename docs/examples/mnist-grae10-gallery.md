# MNIST Dimension Reduction

This gallery uses the full MNIST reference set: `60,000` digit image records.

The reference workflow is the parametric diffusion coordinate/GRAE part of `GRAE10`: two geometry-regularized coordinate solvers are calibrated over MNIST image records, one for the 3D view and one for the 2D animation target. In each case parametric diffusion coordinates supplies the target geometry and the coordinate solver is calibrated against that geometry while reconstructing the original image records.

The checked reference parameters are:

- records: full MNIST train split, normalized to `60,000 x 784` image records
- 3D view: geometry-regularized coordinate solver, `30` calibration steps, `3` components, step size `.0001`, batch size `128`, automatic diffusion-coordinate time, `8` neighbors
- 2D target: geometry-regularized coordinate solver, `30` calibration steps, `2` components, step size `.0001`, batch size `128`, automatic diffusion-coordinate time, `8` neighbors
- animation: 3D metric-space coordinates with 2D target coordinates

The project page renders the native METRIC Visual Engine 60k GRAE10 plot:

- visual: [MNIST Dimension Reduction](../../visual/examples/grae10-metric-engine/index.html)
- visual contract: [MNIST Dimension Reduction README](../../visual/examples/grae10-metric-engine/README.md)
- integrity check: [mnist_grae10_integrity.cpp](../../examples/engine/mnist_grae10_integrity.cpp)

The integrity check verifies that the visual dataset contains `60,000` records, has one label for every MNIST reference record, matches the raw MNIST IDX labels exactly, and contains finite 2D and 3D coordinates for all records.

Current gap: the repository does not yet contain a native C++ executable that
recalibrates the full `60,000` record parametric diffusion coordinate/GRAE10 workflow and exports this
visual dataset from scratch. The native parametric diffusion coordinate gallery executable currently
proves the C++ path on a balanced MNIST subset only.
