/** Exponential smoothing toward a target: manim-style "values glide" instead of jumping. */
export class Smooth {
  target: number;
  constructor(public value: number, public speed = 10) {
    this.target = value;
  }
  set(target: number, immediate = false): void {
    this.target = target;
    if (immediate) this.value = target;
  }
  step(dt: number): number {
    const k = 1 - Math.exp(-this.speed * dt);
    this.value += (this.target - this.value) * k;
    if (Math.abs(this.target - this.value) < 1e-6) this.value = this.target;
    return this.value;
  }
}

export const easeInOut = (t: number): number => (t < 0.5 ? 4 * t * t * t : 1 - (-2 * t + 2) ** 3 / 2);

/** requestAnimationFrame loop with dt (seconds) and elapsed t. Returns a stop function. */
export function loop(fn: (dt: number, t: number) => void): () => void {
  let raf = 0;
  let last = performance.now();
  const start = last;
  const tick = (now: number) => {
    // rAF timestamps can precede the performance.now() taken at scheduling; keep dt and t non-negative.
    const dt = Math.min(Math.max((now - last) / 1000, 0), 0.1);
    last = now;
    fn(dt, Math.max((now - start) / 1000, 0));
    raf = requestAnimationFrame(tick);
  };
  raf = requestAnimationFrame(tick);
  return () => cancelAnimationFrame(raf);
}
