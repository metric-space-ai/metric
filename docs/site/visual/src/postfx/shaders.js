/*
 * METRIC Visual post-FX shaders.
 *
 * These shaders are embedded so the post-FX subsystem stays dependency-free
 * and can be bundled by the existing browser module loader without build steps.
 */

export const COLOR_GRADE_FRAGMENT_SHADER = `
  precision mediump float;

  uniform sampler2D uTexture;
  uniform float uExposure;
  uniform float uContrast;
  uniform float uContrastPivot;
  uniform float uSaturation;
  uniform float uVibrance;
  uniform float uTemperature;
  uniform float uTint;
  uniform float uGamma;
  uniform float uBlackPoint;
  uniform float uWhitePoint;
  uniform float uHighlightCompression;
  uniform vec3 uLift;
  uniform vec3 uGain;
  uniform vec3 uShadowTint;
  uniform vec3 uHighlightTint;
  uniform float uShadowTintStrength;
  uniform float uHighlightTintStrength;
  varying vec2 vUv;

  float luma(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
  }

  vec3 applyTemperatureTint(vec3 color, float temperature, float tint) {
    vec3 warm = vec3(1.0 + temperature * 0.12, 1.0, 1.0 - temperature * 0.10);
    vec3 greenMagenta = vec3(1.0 + tint * 0.035, 1.0 - tint * 0.08, 1.0 + tint * 0.035);
    return color * warm * greenMagenta;
  }

  vec3 applyVibrance(vec3 color, float amount) {
    float average = (color.r + color.g + color.b) * 0.333333;
    float strongest = max(color.r, max(color.g, color.b));
    float mask = clamp(strongest - average, 0.0, 1.0);
    float boost = amount * (1.0 - mask);
    return mix(vec3(luma(color)), color, 1.0 + boost);
  }

  vec3 compressHighlights(vec3 color, float amount) {
    float compression = clamp(amount, 0.0, 1.0);
    vec3 shoulder = color / (color + vec3(1.0));
    return mix(color, shoulder * 1.65, compression);
  }

  void main() {
    vec4 source = texture2D(uTexture, vUv);
    vec3 color = max(source.rgb, vec3(0.0));

    color *= exp2(uExposure);
    color = (color - vec3(uBlackPoint)) / max(uWhitePoint - uBlackPoint, 0.0001);
    color = max(color, vec3(0.0));

    color += uLift;
    color *= uGain;
    color = applyTemperatureTint(color, uTemperature, uTint);

    float luminance = luma(color);
    float shadowMask = 1.0 - smoothstep(0.18, 0.55, luminance);
    float highlightMask = smoothstep(0.45, 0.92, luminance);
    color = mix(color, color * uShadowTint, shadowMask * uShadowTintStrength);
    color = mix(color, color * uHighlightTint, highlightMask * uHighlightTintStrength);

    color = (color - vec3(uContrastPivot)) * uContrast + vec3(uContrastPivot);
    color = mix(vec3(luma(color)), color, uSaturation);
    color = applyVibrance(color, uVibrance);
    color = compressHighlights(max(color, vec3(0.0)), uHighlightCompression);
    color = pow(max(color, vec3(0.0)), vec3(1.0 / max(uGamma, 0.0001)));

    gl_FragColor = vec4(clamp(color, 0.0, 1.0), source.a);
  }
`;

export const VIGNETTE_FRAGMENT_SHADER = `
  precision mediump float;

  uniform sampler2D uTexture;
  uniform float uAmount;
  uniform float uRadius;
  uniform float uSoftness;
  uniform float uRoundness;
  uniform vec2 uCenter;
  uniform vec3 uColor;
  uniform float uExposurePreserve;
  varying vec2 vUv;

  void main() {
    vec4 source = texture2D(uTexture, vUv);
    vec2 position = vUv - uCenter;
    position.x *= mix(1.0, 0.72, clamp(uRoundness, 0.0, 1.0));

    float distanceFromCenter = length(position) * 1.41421356;
    float edge = smoothstep(
      clamp(uRadius, 0.0, 1.5),
      clamp(uRadius + uSoftness, 0.0, 2.0),
      distanceFromCenter
    );
    float shade = edge * clamp(uAmount, 0.0, 1.0);
    vec3 darkened = mix(source.rgb, source.rgb * uColor, shade);
    vec3 preserved = mix(darkened, source.rgb, clamp(uExposurePreserve, 0.0, 1.0) * (1.0 - edge));

    gl_FragColor = vec4(preserved, source.a);
  }
`;

export const HIGHLIGHT_EXTRACT_FRAGMENT_SHADER = `
  precision mediump float;

  uniform sampler2D uTexture;
  uniform float uThreshold;
  uniform float uKnee;
  uniform float uSourceIntensity;
  varying vec2 vUv;

  float luma(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
  }

  void main() {
    vec4 source = texture2D(uTexture, vUv);
    float brightness = luma(source.rgb);
    float knee = max(uKnee, 0.0001);
    float soft = clamp((brightness - uThreshold + knee) / (2.0 * knee), 0.0, 1.0);
    float contribution = max(brightness - uThreshold, 0.0) + soft * soft * knee;
    float weight = contribution / max(brightness, 0.0001);
    gl_FragColor = vec4(source.rgb * clamp(weight, 0.0, 1.0) * uSourceIntensity, source.a);
  }
`;

export const SEPARABLE_BLUR_FRAGMENT_SHADER = `
  precision mediump float;

  uniform sampler2D uTexture;
  uniform vec2 uTexelSize;
  uniform vec2 uDirection;
  uniform float uRadius;
  varying vec2 vUv;

  void main() {
    vec2 delta = uDirection * uTexelSize * max(uRadius, 0.0);
    vec4 color = texture2D(uTexture, vUv) * 0.227027;
    color += texture2D(uTexture, clamp(vUv + delta * 1.384615, vec2(0.0), vec2(1.0))) * 0.316216;
    color += texture2D(uTexture, clamp(vUv - delta * 1.384615, vec2(0.0), vec2(1.0))) * 0.316216;
    color += texture2D(uTexture, clamp(vUv + delta * 3.230769, vec2(0.0), vec2(1.0))) * 0.070270;
    color += texture2D(uTexture, clamp(vUv - delta * 3.230769, vec2(0.0), vec2(1.0))) * 0.070270;
    gl_FragColor = color;
  }
`;

export const BLOOM_COMPOSITE_FRAGMENT_SHADER = `
  precision mediump float;

  uniform sampler2D uTexture;
  uniform sampler2D uBloomTexture;
  uniform float uBloomIntensity;
  uniform float uBloomSaturation;
  uniform float uBloomWarmth;
  uniform float uBloomClamp;
  varying vec2 vUv;

  float luma(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
  }

  void main() {
    vec4 source = texture2D(uTexture, vUv);
    vec3 bloom = texture2D(uBloomTexture, vUv).rgb;
    bloom = min(bloom, vec3(max(uBloomClamp, 0.0)));
    bloom = mix(vec3(luma(bloom)), bloom, uBloomSaturation);
    bloom *= vec3(1.0 + uBloomWarmth * 0.08, 1.0 + uBloomWarmth * 0.02, 1.0 - uBloomWarmth * 0.05);

    vec3 color = source.rgb + bloom * uBloomIntensity;
    gl_FragColor = vec4(clamp(color, 0.0, 1.0), source.a);
  }
`;

export const FILM_GRAIN_FRAGMENT_SHADER = `
  precision mediump float;

  uniform sampler2D uTexture;
  uniform vec2 uTextureSize;
  uniform float uTime;
  uniform float uIntensity;
  uniform float uSize;
  uniform float uLumaResponse;
  varying vec2 vUv;

  float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
  }

  float luma(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
  }

  void main() {
    vec4 source = texture2D(uTexture, vUv);
    vec2 grainUv = floor(vUv * uTextureSize / max(uSize, 1.0));
    float noise = hash(grainUv + vec2(uTime * 37.0, uTime * 11.0)) - 0.5;
    float response = mix(1.0, 1.0 - smoothstep(0.15, 0.95, luma(source.rgb)), clamp(uLumaResponse, 0.0, 1.0));
    vec3 color = source.rgb + noise * uIntensity * response;
    gl_FragColor = vec4(clamp(color, 0.0, 1.0), source.a);
  }
`;

export const MINIATURE_CAMERA_DOF_FRAGMENT_SHADER = `
  precision mediump float;

  uniform sampler2D uTexture;
  uniform sampler2D uDepthTexture;
  uniform vec2 uTexelSize;
  uniform float uAmount;
  uniform float uDepthEnabled;
  uniform float uDepthEncoding;
  uniform float uFocusDistance;
  uniform float uAperture;
  uniform float uFocalLength;
  uniform float uFStop;
  uniform float uSensorScale;
  uniform float uMaxBlur;
  uniform float uFocalRange;
  uniform float uDepthNear;
  uniform float uDepthFar;
  uniform float uCameraNear;
  uniform float uCameraFar;
  uniform float uQuietDesaturation;
  uniform float uQuietExposure;
  uniform float uFocusClarity;
  uniform float uVignette;
  varying vec2 vUv;

  float luma(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
  }

  float rawDepthToCameraDistance(float depth) {
    float z = depth * 2.0 - 1.0;
    float nearPlane = max(uCameraNear, 0.0001);
    float farPlane = max(uCameraFar, nearPlane + 0.0001);
    return (2.0 * nearPlane * farPlane) / max(farPlane + nearPlane - z * (farPlane - nearPlane), 0.0001);
  }

  float cameraDistanceAt(vec2 uv, out float hasDepth) {
    vec4 depthSample = texture2D(uDepthTexture, uv);
    float rawDepthPath = step(0.5, uDepthEncoding);
    float linearCoverage = smoothstep(0.015, 0.18, depthSample.a);
    float rawCoverage = 1.0 - smoothstep(0.999, 1.0, depthSample.r);
    hasDepth = mix(linearCoverage, rawCoverage, rawDepthPath) * uDepthEnabled;
    float linearDistance = mix(uDepthNear, uDepthFar, depthSample.r);
    float rawDistance = rawDepthToCameraDistance(depthSample.r);
    return mix(linearDistance, rawDistance, rawDepthPath);
  }

  float circleOfConfusion(vec2 uv) {
    float hasDepth = 0.0;
    float cameraDistance = cameraDistanceAt(uv, hasDepth);
    float focusDistance = max(uFocusDistance, 0.0001);
    float focalLength = clamp(uFocalLength, 0.0001, focusDistance * 0.95);
    float fStop = max(uFStop, 0.1);
    float apertureDiameter = (focalLength / fStop) * max(uAperture, 0.0);
    float denominator = max(cameraDistance * (focusDistance - focalLength), 0.0001);
    float sceneCoC = abs(apertureDiameter * focalLength * (cameraDistance - focusDistance) / denominator);
    float pixelCoC = sceneCoC * max(uSensorScale, 0.0);
    float normalizedCoC = pixelCoC / max(uMaxBlur, 0.0001);
    float coc = smoothstep(0.0, max(uFocalRange, 0.0002), normalizedCoC);
    return clamp(coc * hasDepth, 0.0, 1.0);
  }

  vec3 blurSample(vec2 uv, float radius) {
    vec2 r = uTexelSize * radius;
    vec3 color = texture2D(uTexture, uv).rgb * 0.227027;
    color += texture2D(uTexture, clamp(uv + vec2( r.x, 0.0) * 1.384615, vec2(0.0), vec2(1.0))).rgb * 0.158108;
    color += texture2D(uTexture, clamp(uv + vec2(-r.x, 0.0) * 1.384615, vec2(0.0), vec2(1.0))).rgb * 0.158108;
    color += texture2D(uTexture, clamp(uv + vec2(0.0,  r.y) * 1.384615, vec2(0.0), vec2(1.0))).rgb * 0.158108;
    color += texture2D(uTexture, clamp(uv + vec2(0.0, -r.y) * 1.384615, vec2(0.0), vec2(1.0))).rgb * 0.158108;
    color += texture2D(uTexture, clamp(uv + vec2( r.x,  r.y) * 2.384615, vec2(0.0), vec2(1.0))).rgb * 0.035635;
    color += texture2D(uTexture, clamp(uv + vec2(-r.x,  r.y) * 2.384615, vec2(0.0), vec2(1.0))).rgb * 0.035635;
    color += texture2D(uTexture, clamp(uv + vec2( r.x, -r.y) * 2.384615, vec2(0.0), vec2(1.0))).rgb * 0.035635;
    color += texture2D(uTexture, clamp(uv + vec2(-r.x, -r.y) * 2.384615, vec2(0.0), vec2(1.0))).rgb * 0.035635;
    return color;
  }

  void main() {
    vec3 sharp = texture2D(uTexture, vUv).rgb;
    if (uDepthEnabled < 0.5) {
      gl_FragColor = vec4(sharp, 1.0);
      return;
    }
    float coc = circleOfConfusion(vUv);
    vec3 blurred = blurSample(vUv, uMaxBlur * coc);
    vec3 color = mix(sharp, blurred, coc * clamp(uAmount, 0.0, 1.0));

    float luminance = luma(color);
    vec3 quietTone = vec3(luminance) * vec3(0.78, 0.84, 0.88);
    color = mix(color, quietTone, coc * clamp(uQuietDesaturation, 0.0, 1.0));
    color *= 1.0 - coc * clamp(uQuietExposure, 0.0, 1.0);
    color += (sharp - blurred) * (1.0 - coc) * clamp(uFocusClarity, 0.0, 2.0);

    float edge = smoothstep(0.46, 0.92, length((vUv - vec2(0.5)) * vec2(0.74, 1.0)));
    color *= 1.0 - edge * clamp(uVignette, 0.0, 1.0) * (0.58 + coc * 0.42);
    gl_FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
  }
`;
