import { createNoise, glide, rampGain } from './engine';

/**
 * Additive "singing" source: a PeriodicWave built from harmonic amplitudes,
 * a high-passed noise layer for breathiness, and an LFO on detune for vibrato.
 * This mirrors the note's decomposition of a voice into F0 + harmonics + noise.
 */
export class Voice {
  readonly output: GainNode;
  private readonly osc: OscillatorNode;
  private readonly oscGain: GainNode;
  private readonly noise: AudioBufferSourceNode;
  private readonly noiseGain: GainNode;
  private readonly lfo: OscillatorNode;
  private readonly lfoGain: GainNode;
  private level = 0.3;
  private started = false;
  private harmonics: number[] = [1];

  constructor(private readonly ctx: AudioContext) {
    this.output = ctx.createGain();
    this.output.gain.value = 0;

    this.osc = ctx.createOscillator();
    this.osc.frequency.value = 220;
    this.oscGain = ctx.createGain();
    this.osc.connect(this.oscGain).connect(this.output);

    this.noise = createNoise(ctx);
    const hp = ctx.createBiquadFilter();
    hp.type = 'highpass';
    hp.frequency.value = 2000;
    this.noiseGain = ctx.createGain();
    this.noiseGain.gain.value = 0;
    this.noise.connect(hp).connect(this.noiseGain).connect(this.output);

    this.lfo = ctx.createOscillator();
    this.lfo.frequency.value = 5.5;
    this.lfoGain = ctx.createGain();
    this.lfoGain.gain.value = 0;
    this.lfo.connect(this.lfoGain).connect(this.osc.detune);

    this.setHarmonics([1]);
  }

  /** amps[k-1] is the amplitude of harmonic k (k = 1 is F0). */
  setHarmonics(amps: number[]): void {
    this.harmonics = amps.slice();
    const n = amps.length + 1;
    const real = new Float32Array(n);
    const imag = new Float32Array(n);
    let any = false;
    for (let k = 1; k < n; k++) {
      imag[k] = amps[k - 1];
      if (amps[k - 1] !== 0) any = true;
    }
    if (!any) imag[1] = 1e-4;
    this.osc.setPeriodicWave(this.ctx.createPeriodicWave(real, imag));
  }

  getHarmonics(): number[] {
    return this.harmonics.slice();
  }

  setFrequency(hz: number, tau = 0.02): void {
    glide(this.osc.frequency, this.ctx, hz, tau);
  }

  /** Static detune in cents (on top of vibrato). */
  setDetune(cents: number): void {
    glide(this.osc.detune, this.ctx, cents, 0.02);
  }

  setVibrato(rateHz: number, depthCents: number): void {
    glide(this.lfo.frequency, this.ctx, rateHz, 0.05);
    glide(this.lfoGain.gain, this.ctx, depthCents, 0.05);
  }

  /** 0 = no breath noise, 1 = noise as loud as the harmonics. */
  setBreath(level: number): void {
    glide(this.noiseGain.gain, this.ctx, level * 0.5, 0.05);
  }

  setLevel(level: number): void {
    this.level = level;
    if (this.playing) rampGain(this.output.gain, this.ctx, level);
  }

  get playing(): boolean {
    return this.started && this.output.gain.value > 0.0005;
  }

  start(): void {
    if (!this.started) {
      this.osc.start();
      this.noise.start();
      this.lfo.start();
      this.started = true;
    }
    rampGain(this.output.gain, this.ctx, this.level);
  }

  stop(): void {
    rampGain(this.output.gain, this.ctx, 0);
  }

  /** Short envelope for triggered notes (attack / hold / release in seconds). */
  pluck(attack = 0.01, hold = 0.3, release = 0.15): void {
    this.start();
    const g = this.output.gain;
    const now = this.ctx.currentTime;
    g.cancelScheduledValues(now);
    g.setValueAtTime(0, now);
    g.linearRampToValueAtTime(this.level, now + attack);
    g.setValueAtTime(this.level, now + attack + hold);
    g.linearRampToValueAtTime(0, now + attack + hold + release);
  }

  dispose(): void {
    if (this.started) {
      this.osc.stop();
      this.noise.stop();
      this.lfo.stop();
    }
    this.output.disconnect();
  }
}

/** Handy harmonic presets (8 partials). */
export const HARMONIC_PRESETS: Record<string, number[]> = {
  正弦: [1, 0, 0, 0, 0, 0, 0, 0],
  锯齿: Array.from({ length: 8 }, (_, i) => 1 / (i + 1)),
  方波: Array.from({ length: 8 }, (_, i) => (i % 2 === 0 ? 1 / (i + 1) : 0)),
  暗: [1, 0.5, 0.2, 0.08, 0.03, 0.01, 0, 0],
  亮: [1, 0.8, 0.7, 0.6, 0.55, 0.5, 0.45, 0.4],
};
