/**
 * RBJ Audio EQ Cookbook biquads. Coefficients are normalized (a0 = 1).
 * Q is the linear quality factor everywhere here. Note that Web Audio's
 * BiquadFilterNode expresses Q in dB for lowpass / highpass; see `webAudioQ`.
 */
export type FilterType = 'lowpass' | 'highpass' | 'bandpass' | 'notch' | 'peaking' | 'lowshelf' | 'highshelf';

export interface Biquad {
  b0: number;
  b1: number;
  b2: number;
  a1: number;
  a2: number;
}

export function rbj(type: FilterType, f0: number, fs: number, Q: number, gainDb = 0): Biquad {
  const A = 10 ** (gainDb / 40);
  const w0 = (2 * Math.PI * f0) / fs;
  const cos = Math.cos(w0);
  const sin = Math.sin(w0);
  const alpha = sin / (2 * Q);
  const sqA2 = 2 * Math.sqrt(A) * alpha;
  let b0: number, b1: number, b2: number, a0: number, a1: number, a2: number;
  switch (type) {
    case 'lowpass':
      b0 = (1 - cos) / 2; b1 = 1 - cos; b2 = (1 - cos) / 2;
      a0 = 1 + alpha; a1 = -2 * cos; a2 = 1 - alpha;
      break;
    case 'highpass':
      b0 = (1 + cos) / 2; b1 = -(1 + cos); b2 = (1 + cos) / 2;
      a0 = 1 + alpha; a1 = -2 * cos; a2 = 1 - alpha;
      break;
    case 'bandpass':
      b0 = alpha; b1 = 0; b2 = -alpha;
      a0 = 1 + alpha; a1 = -2 * cos; a2 = 1 - alpha;
      break;
    case 'notch':
      b0 = 1; b1 = -2 * cos; b2 = 1;
      a0 = 1 + alpha; a1 = -2 * cos; a2 = 1 - alpha;
      break;
    case 'peaking':
      b0 = 1 + alpha * A; b1 = -2 * cos; b2 = 1 - alpha * A;
      a0 = 1 + alpha / A; a1 = -2 * cos; a2 = 1 - alpha / A;
      break;
    case 'lowshelf':
      b0 = A * (A + 1 - (A - 1) * cos + sqA2);
      b1 = 2 * A * (A - 1 - (A + 1) * cos);
      b2 = A * (A + 1 - (A - 1) * cos - sqA2);
      a0 = A + 1 + (A - 1) * cos + sqA2;
      a1 = -2 * (A - 1 + (A + 1) * cos);
      a2 = A + 1 + (A - 1) * cos - sqA2;
      break;
    case 'highshelf':
      b0 = A * (A + 1 + (A - 1) * cos + sqA2);
      b1 = -2 * A * (A - 1 + (A + 1) * cos);
      b2 = A * (A + 1 + (A - 1) * cos - sqA2);
      a0 = A + 1 - (A - 1) * cos + sqA2;
      a1 = 2 * (A - 1 - (A + 1) * cos);
      a2 = A + 1 - (A - 1) * cos - sqA2;
      break;
  }
  return { b0: b0 / a0, b1: b1 / a0, b2: b2 / a0, a1: a1 / a0, a2: a2 / a0 };
}

/** |H(e^{jw})| at frequency f. */
export function magnitudeAt(bq: Biquad, f: number, fs: number): number {
  const w = (2 * Math.PI * f) / fs;
  const c1 = Math.cos(w), s1 = Math.sin(w);
  const c2 = Math.cos(2 * w), s2 = Math.sin(2 * w);
  const nr = bq.b0 + bq.b1 * c1 + bq.b2 * c2;
  const ni = -(bq.b1 * s1 + bq.b2 * s2);
  const dr = 1 + bq.a1 * c1 + bq.a2 * c2;
  const di = -(bq.a1 * s1 + bq.a2 * s2);
  return Math.hypot(nr, ni) / Math.max(Math.hypot(dr, di), 1e-12);
}

export function responseDb(filters: Biquad[], freqs: ArrayLike<number>, fs: number): Float64Array {
  const out = new Float64Array(freqs.length);
  for (let i = 0; i < freqs.length; i++) {
    let mag = 1;
    for (const bq of filters) mag *= magnitudeAt(bq, freqs[i], fs);
    out[i] = 20 * Math.log10(Math.max(mag, 1e-9));
  }
  return out;
}

export function logspace(fmin: number, fmax: number, n: number): Float64Array {
  const out = new Float64Array(n);
  const r = Math.log(fmax / fmin);
  for (let i = 0; i < n; i++) out[i] = fmin * Math.exp((r * i) / (n - 1));
  return out;
}

/** Web Audio lowpass/highpass take Q in dB; other types take linear Q. */
export function webAudioQ(type: FilterType, Q: number): number {
  return type === 'lowpass' || type === 'highpass' ? 20 * Math.log10(Q) : Q;
}

/** Stateful biquad for running a block of samples (used by the loudness meter). */
export class BiquadRunner {
  private x1 = 0; private x2 = 0; private y1 = 0; private y2 = 0;
  constructor(public coef: Biquad) {}
  process(x: number): number {
    const { b0, b1, b2, a1, a2 } = this.coef;
    const y = b0 * x + b1 * this.x1 + b2 * this.x2 - a1 * this.y1 - a2 * this.y2;
    this.x2 = this.x1; this.x1 = x;
    this.y2 = this.y1; this.y1 = y;
    return y;
  }
}
