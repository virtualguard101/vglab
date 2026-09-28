import { glide } from './engine';

export interface Formant {
  freq: number;
  /** Bandwidth in Hz (Q = freq / bw). */
  bw: number;
  /** Linear gain of this resonance. */
  gain: number;
}

/**
 * Parallel band-pass bank approximating the vocal tract: the source's harmonic
 * comb is shaped into a few peaks (F1, F2, F3), which is what decides the vowel.
 */
export class FormantBank {
  readonly input: GainNode;
  readonly output: GainNode;
  private readonly filters: BiquadFilterNode[] = [];
  private readonly gains: GainNode[] = [];

  constructor(private readonly ctx: AudioContext, n = 3) {
    this.input = ctx.createGain();
    this.output = ctx.createGain();
    for (let i = 0; i < n; i++) {
      const f = ctx.createBiquadFilter();
      f.type = 'bandpass';
      const g = ctx.createGain();
      this.input.connect(f).connect(g).connect(this.output);
      this.filters.push(f);
      this.gains.push(g);
    }
  }

  set(formants: Formant[]): void {
    formants.forEach((fm, i) => {
      const f = this.filters[i];
      const g = this.gains[i];
      if (!f || !g) return;
      glide(f.frequency, this.ctx, fm.freq, 0.03);
      glide(f.Q, this.ctx, fm.freq / fm.bw, 0.03);
      glide(g.gain, this.ctx, fm.gain, 0.03);
    });
  }

  dispose(): void {
    this.input.disconnect();
    this.output.disconnect();
  }
}

/** Rough adult-voice vowel targets (F1, F2 in Hz). */
export const VOWELS: { label: string; f1: number; f2: number }[] = [
  { label: '啊 /a/', f1: 800, f2: 1200 },
  { label: '衣 /i/', f1: 300, f2: 2300 },
  { label: '乌 /u/', f1: 350, f2: 800 },
  { label: '哎 /e/', f1: 500, f2: 1900 },
  { label: '哦 /o/', f1: 500, f2: 900 },
];

/** Closed-form spectral envelope of a resonance bank for drawing (sum of Lorentzians). */
export function formantEnvelope(formants: Formant[], f: number): number {
  let e = 0;
  for (const fm of formants) {
    const half = fm.bw / 2;
    e += fm.gain / (1 + ((f - fm.freq) / half) ** 2);
  }
  return e;
}
