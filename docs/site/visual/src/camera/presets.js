import { PerspectiveCamera } from "./PerspectiveCamera.js";
import { TargetCameraState } from "./TargetCameraState.js";

export const MINIATURE_PERSPECTIVE_PRESET = Object.freeze({
  mode: "miniaturePerspective",
  fov: 50,
  aspect: 1,
  near: 0.05,
  far: 2000,
  position: Object.freeze([0, -20, 10]),
  target: Object.freeze([0, 0, 0]),
  up: Object.freeze([0, 1, 0]),
  minDistance: 1,
  maxDistance: 2000,
  minPitch: -1.42,
  maxPitch: 1.42,
});

export const ORBIT_INSPECT_PRESET = Object.freeze({
  mode: "orbitInspect",
  fov: 50,
  aspect: 1,
  near: 0.05,
  far: 2000,
  target: Object.freeze([0, 0, 0]),
  up: Object.freeze([0, 1, 0]),
  yaw: -Math.PI / 4,
  pitch: 0.48,
  radius: 3.2,
  minDistance: 0.2,
  maxDistance: 2000,
  minPitch: -1.42,
  maxPitch: 1.42,
});

export function miniaturePerspective(options = {}) {
  return new TargetCameraState({
    ...MINIATURE_PERSPECTIVE_PRESET,
    ...options,
    mode: "miniaturePerspective",
  });
}

export function orbitInspect(options = {}) {
  return new TargetCameraState({
    ...ORBIT_INSPECT_PRESET,
    ...options,
    mode: "orbitInspect",
  });
}

export function createMiniaturePerspectiveCamera(options = {}) {
  const state = miniaturePerspective(options);
  const camera = new PerspectiveCamera(state.toJSON());
  state.applyTo(camera);
  return { camera, state };
}

export function createOrbitInspectCamera(options = {}) {
  const state = orbitInspect(options);
  const camera = new PerspectiveCamera(state.toJSON());
  state.applyTo(camera);
  return { camera, state };
}
