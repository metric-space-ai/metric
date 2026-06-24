# MNIST Dimension Reduction Visual

This is the original standalone GRAE10/Babyplot visual used as the public MNIST dimension-reduction reference.

Visible behavior:

- `60,000` MNIST training records
- automatic turntable rotation
- delayed, quick 3D-to-2D coordinate morph
- category coloring and legend
- axes/floor from the original Babyplot scene

Reference parameters encoded in `index.html`:

- `turntable: true`
- `rotationRate: 0.0035`
- `animationDelay: 460`
- `animationDuration: 20`
- `animationLoop: true`
- `showAxes: [true, true, true]`
- `showLegend: true`

The native METRIC visual engine must match this behavior before it can replace this reference as the public hero.
