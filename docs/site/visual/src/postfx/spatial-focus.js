import { FullscreenQuadPass } from "../native-postprocess/index.js";
import { MINIATURE_CAMERA_DOF_FRAGMENT_SHADER } from "./shaders.js";

export const PHOTOGRAPHIC_CAMERA_DOF = Object.freeze({
  enabled: true,
  amount: 0.84,
  focusDistance: 9,
  depthNear: 1.2,
  depthFar: 18,
  aperture: 0.92,
  focalLength: 0.08,
  fStop: 1.15,
  sensorScale: 280,
  maxBlur: 6.2,
  focalRange: 0.135,
  quietDesaturation: 0.16,
  quietExposure: 0.09,
  focusClarity: 0.045,
  vignette: 0.16,
});

export class MiniatureCameraDofPass extends FullscreenQuadPass {
  constructor(gl, options = {}) {
    const userBeforeRender = options.beforeRender;
    super(gl, {
      ...options,
      fragmentShader: options.fragmentShader || MINIATURE_CAMERA_DOF_FRAGMENT_SHADER,
      beforeRender: (context, pass) => {
        if (typeof userBeforeRender === "function") userBeforeRender(context, pass);
        const depthTexture = pass.currentDepthTexture || pass.depthTexture;
        const camera = context.camera || context.renderer?.camera || null;
        pass.setUniform("uDepthEnabled", depthTexture ? 1 : 0);
        pass.setUniform("uDepthEncoding", depthEncodingCode(context.cameraDepthEncoding || context.depthEncoding));
        pass.setUniform("uCameraNear", finiteNumber(camera?.near, pass.dof.cameraNear, pass.dof.depthNear));
        pass.setUniform("uCameraFar", finiteNumber(camera?.far, pass.dof.cameraFar, pass.dof.depthFar));
        gl.activeTexture(gl.TEXTURE1);
        gl.bindTexture(gl.TEXTURE_2D, depthTexture || pass.currentFallbackTexture || null);
        if (pass.uniforms.uDepthTexture) gl.uniform1i(pass.uniforms.uDepthTexture, 1);
        gl.activeTexture(gl.TEXTURE0);
      },
    });
    this.depthTexture = options.depthTexture || null;
    this.currentDepthTexture = null;
    this.currentFallbackTexture = null;
    this.dof = createPhotographicCameraDof(options);
    this.setDof(this.dof);
  }

  setDof(options = {}) {
    this.dof = createPhotographicCameraDof({
      ...this.dof,
      ...options,
    });
    this.enabled = this.dof.enabled !== false;
    return this
      .setUniform("uAmount", this.dof.amount)
      .setUniform("uFocusDistance", this.dof.focusDistance)
      .setUniform("uAperture", this.dof.aperture)
      .setUniform("uFocalLength", this.dof.focalLength)
      .setUniform("uFStop", this.dof.fStop)
      .setUniform("uSensorScale", this.dof.sensorScale)
      .setUniform("uMaxBlur", this.dof.maxBlur)
      .setUniform("uFocalRange", this.dof.focalRange)
      .setUniform("uDepthNear", this.dof.depthNear)
      .setUniform("uDepthFar", this.dof.depthFar)
      .setUniform("uQuietDesaturation", this.dof.quietDesaturation)
      .setUniform("uQuietExposure", this.dof.quietExposure)
      .setUniform("uFocusClarity", this.dof.focusClarity)
      .setUniform("uVignette", this.dof.vignette);
  }

  setFocusDepth(focusDepth) {
    const focusDistance = this.dof.depthNear + focusDepth * (this.dof.depthFar - this.dof.depthNear);
    return this.setDof({ focusDistance });
  }

  setFocusDistance(focusDistance) {
    return this.setDof({ focusDistance });
  }

  setDepthTexture(texture) {
    this.depthTexture = texture || null;
    return this;
  }

  render(context, inputTexture, outputTarget = null) {
    const gl = this.gl;
    this.currentDepthTexture = resolveCameraDepthTexture(context) || this.depthTexture;
    this.currentFallbackTexture = inputTexture || null;
    super.render(context, inputTexture, outputTarget);
    gl.activeTexture(gl.TEXTURE1);
    gl.bindTexture(gl.TEXTURE_2D, null);
    gl.activeTexture(gl.TEXTURE0);
    this.currentDepthTexture = null;
    this.currentFallbackTexture = null;
  }
}

export function createMiniatureCameraDofPass(gl, options = {}) {
  return new MiniatureCameraDofPass(gl, options);
}

export function createPhotographicCameraDof(options = {}) {
  const depthNear = clampNumber(options.depthNear, 0, 100000, PHOTOGRAPHIC_CAMERA_DOF.depthNear);
  const depthFarCandidate = clampNumber(options.depthFar, 0.0001, 100000, PHOTOGRAPHIC_CAMERA_DOF.depthFar);
  const depthFar = Math.max(depthFarCandidate, depthNear + 0.0001);
  const focusDistanceFallback = options.focusDepth != null
    ? depthNear + clampNumber(options.focusDepth, 0, 1, 0.52) * (depthFar - depthNear)
    : PHOTOGRAPHIC_CAMERA_DOF.focusDistance;
  const cameraNear = clampNumber(options.cameraNear, 0.0001, 100000, depthNear);
  const cameraFar = Math.max(
    clampNumber(options.cameraFar, 0.0002, 100000, depthFar),
    cameraNear + 0.0001,
  );
  return {
    model: "camera-depth-circle-of-confusion",
    focusModel: "camera-space-distance",
    cocModel: "thin-lens-circle-of-confusion",
    depthMode: "raw-camera-depth",
    requiresDepthTexture: true,
    usesCameraNearFar: true,
    enabled: options.enabled !== false,
    amount: clampNumber(options.amount, 0, 1, PHOTOGRAPHIC_CAMERA_DOF.amount),
    focusDistance: clampNumber(options.focusDistance, 0.0001, 100000, focusDistanceFallback),
    depthNear,
    depthFar,
    cameraNear,
    cameraFar,
    aperture: clampNumber(options.aperture, 0, 4, PHOTOGRAPHIC_CAMERA_DOF.aperture),
    focalLength: clampNumber(options.focalLength, 0.0001, 100, PHOTOGRAPHIC_CAMERA_DOF.focalLength),
    fStop: clampNumber(options.fStop, 0.1, 64, PHOTOGRAPHIC_CAMERA_DOF.fStop),
    sensorScale: clampNumber(options.sensorScale, 0, 10000, PHOTOGRAPHIC_CAMERA_DOF.sensorScale),
    maxBlur: clampNumber(options.maxBlur ?? options.blurRadius, 0, 48, PHOTOGRAPHIC_CAMERA_DOF.maxBlur),
    focalRange: clampNumber(options.focalRange, 0.0001, 1, PHOTOGRAPHIC_CAMERA_DOF.focalRange),
    quietDesaturation: clampNumber(options.quietDesaturation, 0, 1, PHOTOGRAPHIC_CAMERA_DOF.quietDesaturation),
    quietExposure: clampNumber(options.quietExposure, 0, 1, PHOTOGRAPHIC_CAMERA_DOF.quietExposure),
    focusClarity: clampNumber(options.focusClarity, 0, 2, PHOTOGRAPHIC_CAMERA_DOF.focusClarity),
    vignette: clampNumber(options.vignette, 0, 1, PHOTOGRAPHIC_CAMERA_DOF.vignette),
  };
}

export function resolveCameraDepthTexture(context = {}) {
  return context.cameraDepthTexture
    || context.depthTexture
    || context.depth?.texture
    || context.cameraDepth?.texture
    || context.depthTarget?.texture
    || context.cameraDepthTarget?.texture
    || null;
}

export {
  MiniatureCameraDofPass as MiniatureSpatialFocusPass,
  PHOTOGRAPHIC_CAMERA_DOF as PHOTOGRAPHIC_SPATIAL_FOCUS,
  createMiniatureCameraDofPass as createMiniatureSpatialFocusPass,
  createPhotographicCameraDof as createPhotographicSpatialFocus,
  resolveCameraDepthTexture as resolveFocusDepthTexture,
};

function clampNumber(value, min, max, fallback) {
  const number = Number(value);
  if (!Number.isFinite(number)) return fallback;
  return Math.min(max, Math.max(min, number));
}

function finiteNumber(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return 0;
}

function depthEncodingCode(value) {
  return value === "raw-depth" || value === "depth-texture" || value === "depth-buffer" ? 1 : 0;
}
