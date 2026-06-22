# Babyplots Reference Notes

Babyplots is a technical reference for METRIC visual, not a product dependency.

Relevant strengths:

- large browser-side point-cloud rendering
- turntable-style 3D interaction
- animated transitions between coordinate states
- point metadata and picking-oriented presentation
- practical Canvas/WebGL performance tradeoffs

METRIC visual must not expose Babyplots:

- no Babyplots dependency
- no Babyplots adapter
- no Babyplots namespace
- no Babyplots JSON contract
- no Babyplots package structure
- no Babylon.js or other transitive renderer dependency

Code may be ported or used as a starting point only when it is fully
assimilated into native METRIC visual code. If substantial Babyplots source is
ported, Apache-2.0 license notices must be preserved.

The useful idea for the first implementation is coordinate-state animation: the
same records have stable identity and multiple coordinate states, such as a 2D
state and a 3D state. The visual engine interpolates between already exported
states. It does not compute the states.
