export function compileShader(gl, type, source, options = {}) {
  const shader = gl.createShader(type);
  if (!shader) {
    throw new Error("Unable to create WebGL shader.");
  }

  const finalSource = applyShaderDefines(source, options.defines);
  gl.shaderSource(shader, finalSource);
  gl.compileShader(shader);

  if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
    const log = gl.getShaderInfoLog(shader) || "Unknown shader compile error";
    gl.deleteShader(shader);
    throw new Error(formatShaderError(options.label || shaderTypeName(gl, type), log, finalSource));
  }

  return shader;
}

export function createProgram(gl, vertexSource, fragmentSource, options = {}) {
  const vertexShader = compileShader(gl, gl.VERTEX_SHADER, vertexSource, {
    label: `${options.label || "program"} vertex shader`,
    defines: options.defines,
  });
  const fragmentShader = compileShader(gl, gl.FRAGMENT_SHADER, fragmentSource, {
    label: `${options.label || "program"} fragment shader`,
    defines: options.defines,
  });

  const program = linkProgram(gl, vertexShader, fragmentShader, options);
  gl.deleteShader(vertexShader);
  gl.deleteShader(fragmentShader);
  return program;
}

export function linkProgram(gl, vertexShader, fragmentShader, options = {}) {
  const program = gl.createProgram();
  if (!program) {
    throw new Error("Unable to create WebGL program.");
  }

  gl.attachShader(program, vertexShader);
  gl.attachShader(program, fragmentShader);

  if (options.attribLocations) {
    for (const [name, location] of Object.entries(options.attribLocations)) {
      gl.bindAttribLocation(program, location, name);
    }
  }

  gl.linkProgram(program);
  if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
    const log = gl.getProgramInfoLog(program) || "Unknown program link error";
    gl.deleteProgram(program);
    throw new Error(`Unable to link ${options.label || "WebGL program"}:\n${log}`);
  }

  return program;
}

export class ShaderProgram {
  constructor(gl, options = {}) {
    this.gl = gl;
    this.label = options.label || "shader-program";
    this.program = createProgram(gl, options.vertex, options.fragment, options);
    this.attributes = collectActiveAttributes(gl, this.program);
    this.uniforms = collectActiveUniforms(gl, this.program);
    this.disposed = false;
  }

  use() {
    this.gl.useProgram(this.program);
    return this;
  }

  getAttribLocation(name) {
    const attribute = this.attributes[name];
    return attribute ? attribute.location : -1;
  }

  getUniformLocation(name) {
    const uniform = this.uniforms[name] || this.uniforms[`${name}[0]`];
    return uniform ? uniform.location : null;
  }

  setUniform(name, value, explicitType) {
    const uniform = this.uniforms[name] || this.uniforms[`${name}[0]`];
    if (!uniform || uniform.location == null) return this;
    setUniformValue(this.gl, uniform.location, value, explicitType || uniform.type, uniform.size);
    return this;
  }

  bindTexture(name, texture, unit = 0, target = this.gl.TEXTURE_2D) {
    const gl = this.gl;
    gl.activeTexture(gl.TEXTURE0 + unit);
    gl.bindTexture(target, texture);
    return this.setUniform(name, unit, gl.SAMPLER_2D);
  }

  dispose() {
    if (this.disposed) return;
    this.gl.deleteProgram(this.program);
    this.program = null;
    this.disposed = true;
  }
}

export function collectActiveAttributes(gl, program) {
  const count = gl.getProgramParameter(program, gl.ACTIVE_ATTRIBUTES);
  const attributes = {};
  for (let index = 0; index < count; index += 1) {
    const info = gl.getActiveAttrib(program, index);
    if (!info) continue;
    attributes[info.name] = {
      name: info.name,
      size: info.size,
      type: info.type,
      location: gl.getAttribLocation(program, info.name),
    };
  }
  return attributes;
}

export function collectActiveUniforms(gl, program) {
  const count = gl.getProgramParameter(program, gl.ACTIVE_UNIFORMS);
  const uniforms = {};
  for (let index = 0; index < count; index += 1) {
    const info = gl.getActiveUniform(program, index);
    if (!info) continue;
    const name = info.name.replace(/\[0\]$/, "");
    uniforms[name] = {
      name,
      size: info.size,
      type: info.type,
      location: gl.getUniformLocation(program, name),
    };
  }
  return uniforms;
}

export function setUniformValue(gl, location, value, type, size = 1) {
  if (location == null || value == null) return;

  switch (type) {
    case gl.FLOAT:
      if (size > 1 && (Array.isArray(value) || ArrayBuffer.isView(value))) {
        gl.uniform1fv(location, value);
        return;
      }
      gl.uniform1f(location, Number(value));
      return;
    case gl.FLOAT_VEC2:
      gl.uniform2fv(location, value);
      return;
    case gl.FLOAT_VEC3:
      gl.uniform3fv(location, value);
      return;
    case gl.FLOAT_VEC4:
      gl.uniform4fv(location, value);
      return;
    case gl.INT:
    case gl.BOOL:
    case gl.SAMPLER_2D:
    case gl.SAMPLER_CUBE:
      if (size > 1 && (Array.isArray(value) || ArrayBuffer.isView(value))) {
        gl.uniform1iv(location, value);
        return;
      }
      gl.uniform1i(location, Number(value));
      return;
    case gl.INT_VEC2:
    case gl.BOOL_VEC2:
      gl.uniform2iv(location, value);
      return;
    case gl.INT_VEC3:
    case gl.BOOL_VEC3:
      gl.uniform3iv(location, value);
      return;
    case gl.INT_VEC4:
    case gl.BOOL_VEC4:
      gl.uniform4iv(location, value);
      return;
    case gl.FLOAT_MAT2:
      gl.uniformMatrix2fv(location, false, value);
      return;
    case gl.FLOAT_MAT3:
      gl.uniformMatrix3fv(location, false, value);
      return;
    case gl.FLOAT_MAT4:
      gl.uniformMatrix4fv(location, false, value);
      return;
    default:
      setUniformValueByShape(gl, location, value);
  }
}

function setUniformValueByShape(gl, location, value) {
  if (typeof value === "number" || typeof value === "boolean") {
    gl.uniform1f(location, Number(value));
    return;
  }

  switch (value.length) {
    case 1:
      gl.uniform1fv(location, value);
      break;
    case 2:
      gl.uniform2fv(location, value);
      break;
    case 3:
      gl.uniform3fv(location, value);
      break;
    case 4:
      gl.uniform4fv(location, value);
      break;
    case 9:
      gl.uniformMatrix3fv(location, false, value);
      break;
    case 16:
      gl.uniformMatrix4fv(location, false, value);
      break;
    default:
      throw new Error(`Unsupported uniform value shape: ${value.length}`);
  }
}

function applyShaderDefines(source, defines = {}) {
  const entries = Object.entries(defines).filter(([, value]) => value !== false && value != null);
  if (!entries.length) return source;

  const defineLines = entries.map(([name, value]) => (
    value === true ? `#define ${name}` : `#define ${name} ${value}`
  ));

  if (source.startsWith("#version")) {
    const endOfVersion = source.indexOf("\n");
    return `${source.slice(0, endOfVersion + 1)}${defineLines.join("\n")}\n${source.slice(endOfVersion + 1)}`;
  }

  return `${defineLines.join("\n")}\n${source}`;
}

function formatShaderError(label, log, source) {
  const line = findShaderLogLine(log);
  const sourceContext = line ? `\n${formatSourceLines(source, line)}` : "";
  return `Unable to compile ${label}:\n${log}${sourceContext}`;
}

function findShaderLogLine(log) {
  const match = log.match(/(?:ERROR|WARNING):\s*\d+:(\d+)/);
  return match ? Number(match[1]) : 0;
}

function formatSourceLines(source, line, radius = 3) {
  const lines = source.split("\n");
  const start = Math.max(1, line - radius);
  const end = Math.min(lines.length, line + radius);
  const output = [];
  for (let index = start; index <= end; index += 1) {
    const marker = index === line ? ">" : " ";
    output.push(`${marker} ${String(index).padStart(4, " ")} | ${lines[index - 1]}`);
  }
  return output.join("\n");
}

function shaderTypeName(gl, type) {
  if (type === gl.VERTEX_SHADER) return "vertex shader";
  if (type === gl.FRAGMENT_SHADER) return "fragment shader";
  return "shader";
}
