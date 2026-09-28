import { mountLayout, type ExperimentDef } from '../../app/experiment';
import { getEngine } from '../../core/audio/engine';
import { HARMONIC_PRESETS, Voice } from '../../core/audio/voice';
import { Smooth, loop } from '../../core/anim/tween';
import { logspace, magnitudeAt, rbj, responseDb, webAudioQ, type Biquad } from '../../core/dsp/filters';
import { createCanvas, PALETTE } from '../../core/viz/canvas';
import { Axes, annotate, fmtDb, fmtNum, LOG_FREQ_TICKS, stackRects } from '../../core/viz/plots';
import { group, hint, playButton, readout, slider, toggle } from '../../ui/controls';

const FMIN = 20;
const FMAX = 20000;
const SRC_F0 = 200;
const BANDS: [number, number, string][] = [
  [80, 200, '低频 80–200: 厚 / 浑'],
  [200, 500, '200–500: 闷'],
  [1000, 4000, '1–4k: 清晰 / 刺'],
  [5000, 8000, '5–8k: 齿音'],
  [10000, 20000, '>10k: 空气'],
];

export const eq: ExperimentDef = {
  id: 'eq',
  stage: 'mix',
  title: 'EQ: 频响 H(f) 与 Y = X · H',
  summary: '拖三个滤波器, 上图是 H(f), 下图是源频谱 X 被它乘过之后的 Y. 对数频率轴.',
  mapping: 'DAW 里的 EQ 插件: HPF 去底噪, 钟形 (bell) 挖闷 / 提亮, 高架 (shelf) 加空气',
  note: { doc: 'aco-theory', section: 'EQ' },
  mount(root) {
    const { viz, controls } = mountLayout(root, this);
    const cv = createCanvas(viz);
    const eng = getEngine();
    const fs = eng.ctx.sampleRate;

    const p = {
      hpOn: true, hpF: new Smooth(Math.log2(80)),
      bellOn: true, bellF: new Smooth(Math.log2(350)), bellG: new Smooth(-6), bellQ: new Smooth(1.2),
      shOn: true, shF: new Smooth(Math.log2(8000)), shG: new Smooth(3),
      bypass: false,
    };
    const hz = (log2: number) => 2 ** log2;

    const voice = new Voice(eng.ctx);
    voice.setHarmonics(HARMONIC_PRESETS['锯齿'].concat(Array.from({ length: 24 }, (_, i) => 1 / (i + 9))));
    voice.setFrequency(SRC_F0);
    voice.setBreath(0.35);
    const hp = eng.ctx.createBiquadFilter(); hp.type = 'highpass';
    const bell = eng.ctx.createBiquadFilter(); bell.type = 'peaking';
    const sh = eng.ctx.createBiquadFilter(); sh.type = 'highshelf';
    const wet = eng.ctx.createGain();
    const dry = eng.ctx.createGain(); dry.gain.value = 0;
    voice.output.connect(hp).connect(bell).connect(sh).connect(wet).connect(eng.master);
    voice.output.connect(dry).connect(eng.master);

    const applyAudio = () => {
      const now = eng.ctx.currentTime;
      hp.frequency.setTargetAtTime(p.hpOn ? hz(p.hpF.target) : 10, now, 0.02);
      hp.Q.value = webAudioQ('highpass', Math.SQRT1_2);
      bell.frequency.setTargetAtTime(hz(p.bellF.target), now, 0.02);
      bell.gain.setTargetAtTime(p.bellOn ? p.bellG.target : 0, now, 0.02);
      bell.Q.setTargetAtTime(p.bellQ.target, now, 0.02);
      sh.frequency.setTargetAtTime(hz(p.shF.target), now, 0.02);
      sh.gain.setTargetAtTime(p.shOn ? p.shG.target : 0, now, 0.02);
      wet.gain.setTargetAtTime(p.bypass ? 0 : 1, now, 0.02);
      dry.gain.setTargetAtTime(p.bypass ? 1 : 0, now, 0.02);
    };
    applyAudio();

    const fmtHzLog = (v: number) => `${fmtNum(Math.round(hz(v)))} Hz`;
    const g1 = group(controls, '高通 HPF');
    toggle(g1, '启用', p.hpOn, (v) => { p.hpOn = v; applyAudio(); });
    slider(g1, { label: '截止频率', min: Math.log2(20), max: Math.log2(500), step: 0.01, value: p.hpF.value, format: fmtHzLog, onInput: (v) => { p.hpF.set(v); applyAudio(); } });
    const g2 = group(controls, '钟形 Bell');
    toggle(g2, '启用', p.bellOn, (v) => { p.bellOn = v; applyAudio(); });
    slider(g2, { label: '中心频率', min: Math.log2(100), max: Math.log2(10000), step: 0.01, value: p.bellF.value, format: fmtHzLog, onInput: (v) => { p.bellF.set(v); applyAudio(); } });
    slider(g2, { label: '增益', min: -15, max: 15, step: 0.5, value: p.bellG.value, format: (v) => `${fmtDb(v)} dB`, onInput: (v) => { p.bellG.set(v); applyAudio(); } });
    slider(g2, { label: 'Q (越大越窄)', min: 0.3, max: 8, step: 0.1, value: p.bellQ.value, onInput: (v) => { p.bellQ.set(v); applyAudio(); } });
    const g3 = group(controls, '高架 High shelf');
    toggle(g3, '启用', p.shOn, (v) => { p.shOn = v; applyAudio(); });
    slider(g3, { label: '转折频率', min: Math.log2(2000), max: Math.log2(16000), step: 0.01, value: p.shF.value, format: fmtHzLog, onInput: (v) => { p.shF.set(v); applyAudio(); } });
    slider(g3, { label: '增益', min: -12, max: 12, step: 0.5, value: p.shG.value, format: (v) => `${fmtDb(v)} dB`, onInput: (v) => { p.shG.set(v); applyAudio(); } });
    const g4 = group(controls, '听');
    toggle(g4, 'Bypass (对比原声)', p.bypass, (v) => { p.bypass = v; applyAudio(); });
    playButton(g4, () => voice.start(), () => voice.stop());
    const ro = readout(g4);
    hint(g4, 'EQ 是线性时不变系统: 每个频率只被<em>乘一个数</em>, 不会长出新的频率. 所以 Y(f) = X(f)·H(f), 用 dB 表示就是相加.');

    const freqs = logspace(FMIN, FMAX, 400);
    const srcDb = (f: number) => (f < 1500 ? -58 : -58 + 6 * Math.log2(f / 1500) * 0.5); // breath noise floor

    const stop = loop((dt) => {
      for (const s of [p.hpF, p.bellF, p.bellG, p.bellQ, p.shF, p.shG]) s.step(dt);
      const filters: Biquad[] = [];
      if (p.hpOn) filters.push(rbj('highpass', hz(p.hpF.value), fs, Math.SQRT1_2));
      if (p.bellOn) filters.push(rbj('peaking', hz(p.bellF.value), fs, p.bellQ.value, p.bellG.value));
      if (p.shOn) filters.push(rbj('highshelf', hz(p.shF.value), fs, Math.SQRT1_2, p.shG.value));
      const H = responseDb(filters, freqs, fs);

      cv.clear();
      const ctx = cv.ctx;
      const [rT, rB] = stackRects(cv.width, cv.height, 2);
      const axH = new Axes(rT, [FMIN, FMAX], [-24, 24], { xLog: true, title: 'H(f): 每个频率被乘的倍数 (dB)', xLabel: 'Hz', yLabel: 'dB' });
      for (const [a, b, label] of BANDS) axH.band(ctx, a, b, 'rgba(255,255,255,0.03)', label);
      axH.frame(ctx, LOG_FREQ_TICKS, [-18, -12, -6, 0, 6, 12, 18], fmtNum, fmtDb);
      axH.hline(ctx, 0, 'rgba(255,255,255,0.25)');
      axH.plotXY(ctx, freqs, H, p.bypass ? PALETTE.grey : PALETTE.yellow, 2.5);
      if (p.hpOn) { axH.vline(ctx, hz(p.hpF.value), 'rgba(88,196,221,0.4)', [3, 3]); axH.label(ctx, hz(p.hpF.value), -21, 'HPF', PALETTE.blue, 4, 0); }
      if (p.bellOn) { axH.dot(ctx, hz(p.bellF.value), p.bellG.value, PALETTE.red, 5); axH.label(ctx, hz(p.bellF.value), p.bellG.value, `Bell ${fmtDb(Number(p.bellG.value.toFixed(1)))} dB, Q ${p.bellQ.value.toFixed(1)}`, PALETTE.red); }
      if (p.shOn) { axH.vline(ctx, hz(p.shF.value), 'rgba(131,193,103,0.4)', [3, 3]); axH.label(ctx, hz(p.shF.value), -21, 'Shelf', PALETTE.green, 4, 0); }

      const axY = new Axes(rB, [FMIN, FMAX], [-72, 6], { xLog: true, title: `X(f) 源 (灰) 与 Y(f) = X·H (蓝), 源 F0 = ${SRC_F0} Hz`, xLabel: 'Hz', yLabel: 'dB' });
      axY.frame(ctx, LOG_FREQ_TICKS, [-60, -40, -20, 0], fmtNum, fmtDb);
      const hDbAt = (f: number) => {
        let mag = 1;
        for (const bq of filters) mag *= Math.max(1e-9, magnitudeAt(bq, f, fs));
        return p.bypass ? 0 : 20 * Math.log10(mag);
      };
      axY.plotFn(ctx, (f) => srcDb(f), 'rgba(255,255,255,0.15)', 1, 200);
      axY.plotFn(ctx, (f) => srcDb(f) + hDbAt(f), 'rgba(88,196,221,0.35)', 1, 200);
      const xs: number[] = [], ysX: number[] = [], ysY: number[] = [];
      for (let k = 1; k * SRC_F0 < FMAX; k++) {
        const f = k * SRC_F0;
        const x = -20 * Math.log10(k) - 6;
        xs.push(f); ysX.push(x); ysY.push(x + hDbAt(f));
      }
      axY.stems(ctx, xs, ysX, 'rgba(255,255,255,0.22)', 4, -72);
      axY.stems(ctx, xs, ysY, p.bypass ? PALETTE.grey : PALETTE.blue, 2, -72);
      if (p.bypass) annotate(ctx, rB.x + rB.w / 2, rB.y + 16, 'Bypass: H(f) = 0 dB, Y = X', PALETTE.grey, 'center');

      ro.set(`钟形在 <b>${fmtNum(Math.round(hz(p.bellF.value)))} Hz</b> 处 ${fmtDb(Number(p.bellG.value.toFixed(1)))} dB; 高通以下每倍频程掉约 12 dB.`);
    });

    return {
      dispose() {
        stop();
        voice.dispose();
        wet.disconnect(); dry.disconnect();
        cv.dispose();
      },
    };
  },
};
