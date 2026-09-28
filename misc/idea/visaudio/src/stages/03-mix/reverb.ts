import { mountLayout, type ExperimentDef } from '../../app/experiment';
import { getEngine } from '../../core/audio/engine';
import { HARMONIC_PRESETS, Voice } from '../../core/audio/voice';
import { loop } from '../../core/anim/tween';
import { convolve } from '../../core/dsp/fft';
import { createCanvas, PALETTE } from '../../core/viz/canvas';
import { Axes, annotate, fmtDb, stackRects } from '../../core/viz/plots';
import { button, buttonRow, group, hint, playButton, readout, slider, toggle } from '../../ui/controls';

const DRAW_RATE = 2000; // samples/s for the drawn envelopes
const VIEW_S = 3;

interface Params { rt60: number; predelay: number; early: number; wet: number }

/** Exponential decay reaching −60 dB at RT60, with a few early reflections after the pre-delay. */
function irEnvelope(p: Params, t: number): number {
  const t0 = p.predelay / 1000;
  if (t < t0) return 0;
  return Math.exp((-6.908 * (t - t0)) / p.rt60);
}

function earlyTaps(p: Params): { t: number; a: number }[] {
  const taps: { t: number; a: number }[] = [];
  const t0 = p.predelay / 1000;
  const n = Math.round(p.early * 8);
  for (let i = 0; i < n; i++) {
    const t = t0 + 0.008 + i * 0.011 + ((i * 7919) % 13) * 0.0008;
    taps.push({ t, a: 0.9 * Math.exp(-i * 0.35) });
  }
  return taps;
}

function buildIR(ctx: AudioContext, p: Params): AudioBuffer {
  const fs = ctx.sampleRate;
  const len = Math.floor(fs * Math.min(8, p.rt60 * 1.1 + p.predelay / 1000 + 0.1));
  const buf = ctx.createBuffer(2, len, fs);
  const taps = earlyTaps(p);
  for (let ch = 0; ch < 2; ch++) {
    const d = buf.getChannelData(ch);
    for (let i = 0; i < len; i++) {
      const t = i / fs;
      d[i] = (Math.random() * 2 - 1) * irEnvelope(p, t) * 0.25;
    }
    for (const tap of taps) {
      const i = Math.floor((tap.t + (ch ? 0.0007 : 0)) * fs);
      if (i < len) d[i] += tap.a * (ch ? -1 : 1);
    }
  }
  return buf;
}

export const reverb: ExperimentDef = {
  id: 'reverb',
  stage: 'mix',
  title: '混响: 脉冲响应 h(t) 与卷积',
  summary: '房间被压缩成一条脉冲响应. 干声 * h(t) 就是湿声. RT60 是掉 60 dB 的时间.',
  mapping: 'DAW 里的 Reverb: size / decay (RT60), pre-delay, early reflections, wet / dry',
  note: { doc: 'aco-theory', section: '空间与混响' },
  mount(root) {
    const { viz, controls } = mountLayout(root, this);
    const cv = createCanvas(viz);
    const eng = getEngine();

    const p: Params = { rt60: 1.6, predelay: 20, early: 0.6, wet: 0.35 };
    let auto = true;

    const voice = new Voice(eng.ctx);
    voice.setHarmonics(HARMONIC_PRESETS['暗']);
    voice.setFrequency(261.63);
    voice.setLevel(0.5);
    const conv = eng.ctx.createConvolver();
    const wet = eng.ctx.createGain();
    const dry = eng.ctx.createGain();
    voice.output.connect(dry).connect(eng.master);
    voice.output.connect(conv).connect(wet).connect(eng.master);

    let irTimer = 0;
    const rebuild = () => {
      window.clearTimeout(irTimer);
      irTimer = window.setTimeout(() => { conv.buffer = buildIR(eng.ctx, p); }, 120);
    };
    const applyMix = () => {
      const now = eng.ctx.currentTime;
      wet.gain.setTargetAtTime(p.wet * 1.6, now, 0.02);
      dry.gain.setTargetAtTime(1 - p.wet, now, 0.02);
    };
    conv.buffer = buildIR(eng.ctx, p);
    applyMix();

    const g1 = group(controls, '房间');
    const sRt = slider(g1, { label: 'RT60 (衰减时间)', min: 0.2, max: 6, step: 0.1, value: p.rt60, format: (v) => `${v.toFixed(1)} s`, onInput: (v) => { p.rt60 = v; rebuild(); } });
    const sPre = slider(g1, { label: 'Pre-delay', min: 0, max: 120, step: 1, value: p.predelay, format: (v) => `${v} ms`, onInput: (v) => { p.predelay = v; rebuild(); } });
    const sEarly = slider(g1, { label: '早期反射密度', min: 0, max: 1, step: 0.05, value: p.early, onInput: (v) => { p.early = v; rebuild(); } });
    const set = (rt60: number, predelay: number, early: number) => {
      Object.assign(p, { rt60, predelay, early });
      sRt.set(rt60); sPre.set(predelay); sEarly.set(early);
      rebuild();
    };
    const row = buttonRow(g1);
    button(row, '小房间', () => set(0.4, 5, 0.9));
    button(row, '录音棚', () => set(1.0, 15, 0.6));
    button(row, '音乐厅', () => set(2.4, 35, 0.4));
    button(row, '教堂', () => set(5.5, 60, 0.2));

    const g2 = group(controls, '混合');
    slider(g2, { label: 'Wet (湿声比例)', min: 0, max: 1, step: 0.01, value: p.wet, onInput: (v) => { p.wet = v; applyMix(); } });
    hint(g2, 'Pre-delay 把干声和湿声拉开一点, 人声更清楚. RT60 长 = 空间大, 但快歌会糊.');
    const g3 = group(controls, '听');
    const pluck = () => voice.pluck(0.01, 0.25, 0.1);
    let autoTimer = 0;
    const play = playButton(g3, () => { pluck(); if (auto) autoTimer = window.setInterval(pluck, 1800); }, () => { window.clearInterval(autoTimer); voice.stop(); }, ['开始弹音', '停止']);
    button(g3, '弹一下', () => { void eng.ensureRunning(); pluck(); });
    toggle(g3, '每 1.8 s 自动弹', auto, (v) => { auto = v; window.clearInterval(autoTimer); if (v && play.playing()) autoTimer = window.setInterval(pluck, 1800); });
    const ro = readout(g3);

    const M = VIEW_S * DRAW_RATE;
    const xs = Array.from({ length: M }, (_, i) => i / DRAW_RATE);
    const dryEnv = new Float64Array(M);
    for (let i = 0; i < M; i++) {
      const t = xs[i];
      dryEnv[i] = t < 0.01 ? t / 0.01 : t < 0.26 ? 1 : t < 0.36 ? 1 - (t - 0.26) / 0.1 : 0;
    }

    const stop = loop(() => {
      cv.clear();
      const ctx = cv.ctx;
      const [rT, rB] = stackRects(cv.width, cv.height, 2);
      const irEnv = new Float64Array(M);
      for (let i = 0; i < M; i++) irEnv[i] = irEnvelope(p, xs[i]);
      const taps = earlyTaps(p);

      const axH = new Axes(rT, [0, VIEW_S], [-60, 3], { title: 'h(t) 脉冲响应的包络 (dB)', xLabel: 's', yLabel: 'dB' });
      axH.frame(ctx, [0, 0.5, 1, 1.5, 2, 2.5, 3], [-60, -40, -20, 0], (v) => v.toFixed(1), fmtDb);
      axH.hline(ctx, -60, 'rgba(252,98,85,0.5)', [3, 3]);
      axH.plotFn(ctx, (t) => { const e = irEnvelope(p, t); return e > 0 ? 20 * Math.log10(e) : -100; }, PALETTE.yellow, 2.5, 500);
      axH.stems(ctx, taps.map((k) => k.t), taps.map((k) => 20 * Math.log10(k.a)), 'rgba(88,196,221,0.8)', 2, -60);
      const tEnd = p.predelay / 1000 + p.rt60;
      axH.vline(ctx, tEnd, 'rgba(252,98,85,0.7)', [4, 4]);
      axH.dot(ctx, tEnd, -60, PALETTE.red, 4);
      axH.label(ctx, tEnd, -50, `RT60 = ${p.rt60.toFixed(1)} s: 掉到 −60 dB`, PALETTE.red);
      if (p.predelay > 2) axH.label(ctx, p.predelay / 1000, -6, `pre-delay ${p.predelay} ms`, PALETTE.green, 6, 0);
      if (taps.length) annotate(ctx, rT.x + rT.w - 8, rT.y + 16, '蓝色竖线: 早期反射 (墙面第一次回来)', PALETTE.blue, 'right');

      // Wet envelope: dry envelope convolved with the IR envelope (drawn at low rate).
      const irShort = irEnv.subarray(0, Math.min(M, Math.ceil((tEnd + 0.1) * DRAW_RATE)));
      const wetEnv = convolve(dryEnv, irShort);
      let mx = 1e-6;
      for (let i = 0; i < M; i++) mx = Math.max(mx, wetEnv[i]);
      const axT = new Axes(rB, [0, VIEW_S], [0, 1.15], { title: '一个音: 干声 (灰) 与 干声 * h(t) (蓝)', xLabel: 's' });
      axT.frame(ctx, [0, 0.5, 1, 1.5, 2, 2.5, 3], [0, 0.5, 1], (v) => v.toFixed(1));
      axT.plotXY(ctx, xs, dryEnv, 'rgba(255,255,255,0.35)', 1.5);
      const wetScaled = new Float64Array(M);
      for (let i = 0; i < M; i++) wetScaled[i] = (1 - p.wet) * dryEnv[i] + p.wet * (wetEnv[i] / mx);
      axT.plotXY(ctx, xs, wetScaled, PALETTE.blue, 2.5);
      annotate(ctx, rB.x + rB.w - 8, rB.y + 16, `wet ${(p.wet * 100).toFixed(0)}%`, PALETTE.blue, 'right');

      ro.set(`RT60 <b>${p.rt60.toFixed(1)} s</b>: 每秒掉 <b>${(60 / p.rt60).toFixed(0)} dB</b>. 尾巴 ${p.rt60 > 2.5 ? '长, 适合慢歌 / 铺底' : p.rt60 > 0.8 ? '中等, 常规人声' : '短, 只是"有个房间"'}.`);
    });

    return {
      dispose() {
        stop();
        window.clearInterval(autoTimer);
        window.clearTimeout(irTimer);
        voice.dispose();
        wet.disconnect(); dry.disconnect();
        cv.dispose();
      },
    };
  },
};
