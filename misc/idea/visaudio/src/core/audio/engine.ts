/**
 * Single AudioContext for the whole app. Browsers require a user gesture before
 * audio can start, so `ensureRunning` is called from play buttons and a one-shot
 * document listener.
 */
export interface Engine {
  ctx: AudioContext;
  master: GainNode;
  analyser: AnalyserNode;
  ensureRunning(): Promise<void>;
}

let engine: Engine | null = null;

export function getEngine(): Engine {
  if (engine) return engine;
  const ctx = new AudioContext({ latencyHint: 'interactive' });
  const master = ctx.createGain();
  master.gain.value = 0.5;
  const analyser = ctx.createAnalyser();
  analyser.fftSize = 2048;
  analyser.smoothingTimeConstant = 0.6;
  master.connect(analyser);
  analyser.connect(ctx.destination);
  engine = {
    ctx,
    master,
    analyser,
    async ensureRunning() {
      if (ctx.state !== 'running') await ctx.resume();
    },
  };
  return engine;
}

/** Fade a gain param to a value; avoids clicks when toggling sources. */
export function rampGain(param: AudioParam, ctx: AudioContext, value: number, seconds = 0.03): void {
  const now = ctx.currentTime;
  param.cancelScheduledValues(now);
  param.setValueAtTime(param.value, now);
  param.linearRampToValueAtTime(value, now + seconds);
}

/** Smoothly move any AudioParam (frequency, gain ...) toward a value. */
export function glide(param: AudioParam, ctx: AudioContext, value: number, tau = 0.02): void {
  param.setTargetAtTime(value, ctx.currentTime, tau);
}

/** Looping white-noise source. */
export function createNoise(ctx: AudioContext, seconds = 2): AudioBufferSourceNode {
  const buf = ctx.createBuffer(1, Math.floor(ctx.sampleRate * seconds), ctx.sampleRate);
  const data = buf.getChannelData(0);
  for (let i = 0; i < data.length; i++) data[i] = Math.random() * 2 - 1;
  const src = ctx.createBufferSource();
  src.buffer = buf;
  src.loop = true;
  return src;
}
