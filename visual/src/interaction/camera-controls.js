import { OrbitInspectMode } from "../camera/orbit-inspect.js";
import { orbitInspect } from "../camera/presets.js";
import { finiteNumber } from "../camera/math.js";
import { createPointerSnapshot, normalizePointerEvent } from "./pointer.js";

export class CameraControls {
  constructor(options = {}) {
    this.element = options.element;
    this.camera = options.camera;
    this.state = options.state || orbitInspect(options);
    this.mode = options.mode || this.state.mode || "orbitInspect";
    this.focusLine = options.focusLine;
    this.focusOptions = {
      axis: options.focusAxis || options.focusLine?.axis || "horizontal",
      radius: options.focusRadius,
      ...(options.focusOptions || {}),
    };
    this.enabled = options.enabled !== false;
    this.preventDefault = options.preventDefault !== false;
    this.updateFocusOnMove = options.updateFocusOnMove !== false;
    this.onChange = options.onChange || (() => {});
    this.onFocusChange = options.onFocusChange || (() => {});
    this.pointer = createPointerSnapshot();
    this.lastPointer = { x: 0, y: 0 };
    this.dragging = false;
    this.activeAction = "rotate";
    this.orbitMode = new OrbitInspectMode(this.state, options);

    this.handlePointerDown = (event) => this.pointerDown(event);
    this.handlePointerMove = (event) => this.pointerMove(event);
    this.handlePointerUp = (event) => this.pointerUp(event);
    this.handlePointerLeave = (event) => this.pointerLeave(event);
    this.handleWheel = (event) => this.wheel(event);
    this.handleContextMenu = (event) => {
      if (this.preventDefault) event.preventDefault();
    };

    if (options.autoAttach !== false) this.attach();
    this.updateCamera();
  }

  attach() {
    if (!this.element || this.attached) return this;
    this.element.addEventListener("pointerdown", this.handlePointerDown);
    this.element.addEventListener("pointermove", this.handlePointerMove);
    this.element.addEventListener("pointerup", this.handlePointerUp);
    this.element.addEventListener("pointercancel", this.handlePointerUp);
    this.element.addEventListener("pointerleave", this.handlePointerLeave);
    this.element.addEventListener("wheel", this.handleWheel, { passive: false });
    this.element.addEventListener("contextmenu", this.handleContextMenu);
    this.attached = true;
    return this;
  }

  detach() {
    if (!this.element || !this.attached) return this;
    this.element.removeEventListener("pointerdown", this.handlePointerDown);
    this.element.removeEventListener("pointermove", this.handlePointerMove);
    this.element.removeEventListener("pointerup", this.handlePointerUp);
    this.element.removeEventListener("pointercancel", this.handlePointerUp);
    this.element.removeEventListener("pointerleave", this.handlePointerLeave);
    this.element.removeEventListener("wheel", this.handleWheel);
    this.element.removeEventListener("contextmenu", this.handleContextMenu);
    this.attached = false;
    return this;
  }

  setCamera(camera) {
    this.camera = camera;
    this.updateCamera();
    return this;
  }

  setState(state) {
    this.state = state;
    this.orbitMode.state = state;
    this.updateCamera();
    return this;
  }

  setFocusLine(focusLine) {
    this.focusLine = focusLine;
    return this;
  }

  setFocusOptions(options = {}) {
    this.focusOptions = {
      ...this.focusOptions,
      ...options,
    };
    if (typeof options.updateFocusOnMove === "boolean") {
      this.updateFocusOnMove = options.updateFocusOnMove;
    }
    return this;
  }

  setSize(width, height, pixelRatio = 1) {
    this.state.setViewport(width, height, pixelRatio);
    if (this.camera) this.camera.setViewport(width, height, pixelRatio);
    if (this.focusLine) {
      this.focusLine.setSize(
        Math.max(1, Math.round(finiteNumber(width, 1) * finiteNumber(pixelRatio, 1))),
        Math.max(1, Math.round(finiteNumber(height, 1) * finiteNumber(pixelRatio, 1))),
      );
    }
    this.updateCamera();
    return this;
  }

  pointerDown(event) {
    if (!this.enabled) return;
    if (this.preventDefault) event.preventDefault();
    const pointer = normalizePointerEvent(event, this.element, this.pointer);
    this.dragging = true;
    this.activeAction = this.resolveAction(event);
    this.lastPointer.x = pointer.pixel.x;
    this.lastPointer.y = pointer.pixel.y;
    if (this.element?.setPointerCapture && event.pointerId !== undefined) {
      this.element.setPointerCapture(event.pointerId);
    }
  }

  pointerMove(event) {
    const pointer = normalizePointerEvent(event, this.element, this.pointer);
    if (this.updateFocusOnMove && this.focusLine) {
      this.focusLine.setFromPointer(pointer, this.focusOptions);
      this.onFocusChange(this.focusLine, pointer);
    }

    if (!this.enabled || !this.dragging) return;
    if (this.preventDefault) event.preventDefault();

    const dx = pointer.pixel.x - this.lastPointer.x;
    const dy = pointer.pixel.y - this.lastPointer.y;
    this.lastPointer.x = pointer.pixel.x;
    this.lastPointer.y = pointer.pixel.y;

    if (this.mode === "orbitInspect") {
      if (this.activeAction === "pan") {
        this.orbitMode.panPixels(dx, dy, this.state.viewport.pixelHeight);
      } else {
        this.orbitMode.orbitPixels(dx, dy);
      }
      this.updateCamera();
      this.onChange(this.state, { type: this.activeAction, pointer });
    }
  }

  pointerUp(event) {
    if (!this.dragging) return;
    if (this.preventDefault) event.preventDefault();
    this.dragging = false;
    if (this.element?.releasePointerCapture && event.pointerId !== undefined) {
      try {
        this.element.releasePointerCapture(event.pointerId);
      } catch {
        // The pointer may already be released by the browser.
      }
    }
  }

  pointerLeave(event) {
    if (this.updateFocusOnMove && this.focusLine) {
      const pointer = normalizePointerEvent(event, this.element, this.pointer);
      this.onFocusChange(this.focusLine, pointer);
    }
  }

  wheel(event) {
    if (!this.enabled || this.mode !== "orbitInspect") return;
    if (this.preventDefault) event.preventDefault();
    this.orbitMode.dollyWheel(event.deltaY);
    this.updateCamera();
    this.onChange(this.state, { type: "dolly", sourceEvent: event });
  }

  updateCamera() {
    if (this.camera) {
      this.state.applyTo(this.camera);
      this.camera.updateMatrices();
    }
    return this;
  }

  resolveAction(event) {
    if (event.button === 1 || event.button === 2 || event.altKey || event.shiftKey) {
      return "pan";
    }
    return "rotate";
  }

  dispose() {
    return this.detach();
  }
}

export function createCameraControls(options = {}) {
  return new CameraControls(options);
}
