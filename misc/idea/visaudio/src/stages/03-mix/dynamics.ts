import { mountLayout, type ExperimentDef } from '../../app/experiment';
import { getEngine } from '../../core/audio/engine';
import { HARMONIC_PRESETS, Voice } from '../../core/audio/voice';
import { loop } from '../../core/anim/tween';
import { dbToGain, gainToDb } from '../../core/dsp/theory';
import { createCanvas, PALETTE } from '../../core/viz/canvas';
import { Axes, annotate, columnRects, fmtDb } from '../../core/viz/plots';
import { group, hint, playButton, readout, slider, toggle } from '../../ui/controls';

const LOOP_S = 2.4;
const RATE = 1000; // envelope samples per second
const N = Math.round(LOOP_S * RATE);

/** A phrase: loud – soft – medium – whisper. Values are linear amplitude. */
function phrase(t: number): number {
  const notes: [number, number, number][] = [
    [0.05, 0.5, 1.0],
    [0.65, 1.05, 0.25],
    [1.25, 1.65, 0.6],
    [1.85, 2.25, 0.1],
  ];
  for (const [a, b, lvl] of notes) {
    if (t >= a && t < b) {
      const ramp = Math.min(1, (t - a) / 0.01, (b - t) / 0.03);
      return lvl * Math.max(0, ramp);
    }
  }
  return 0;
}

export const dynamics: ExperimentDef = {
  id: 'dynamics',
  stage: 'mix',
  title: '压缩: 静态曲线与包络',
  summary: '左图是输入电平到输出电平的映射; 右图是一句话的响度包络被压之前 / 之后.',
  mapping: 'DAW 里的 Compressor: threshold / ratio / attack / release / makeup; 声源阶段的 Loudness 曲线是它的手动版',
  note: { doc: 'aco-theory', section: '动态处理' },
  mount(root) {
    const { viz, controls } = mountLayout(root, this);
    const cv = createCanvas(viz);
    const eng = getEngine();

    const p = { thr: -18, ratio: 4, attack: 10, release: 150, makeup: 6, bypass: false };

    const voice = new Voice(eng.ctx);
    voice.setHarmonics(HARMONIC_PRESETS['暗']);
    voice.setFrequency(196);
    voice.setLevel(1);
    const env = eng.ctx.createGain();
    env.gain.value = 0;
    const comp = eng.ctx.createDynamicsCompressor();
    comp.knee.value = 0;
    const wet = eng.ctx.createGain();
    const dry = eng.ctx.createGain(); dry.gain.value = 0;
    const trim = eng.ctx.createGain(); trim.gain.value = 0.4;
    voice.output.connect(env);
    env.connect(comp).connect(wet).connect(trim);
    env.connect(dry).connect(trim);
    trim.connect(eng.master);

    const applyAudio = () => {
      const now = eng.ctx.currentTime;
      comp.threshold.setTargetAtTime(p.thr, now, 0.02);
      comp.ratio.setTargetAtTime(p.ratio, now, 0.02);
      comp.attack.setTargetAtTime(p.attack / 1000, now, 0.02);
      comp.release.setTargetAtTime(p.release / 1000, now, 0.02);
      wet.gain.setTargetAtTime(p.bypass ? 0 : dbToGain(p.makeup), now, 0.02);
      dry.gain.setTargetAtTime(p.bypass ? 1 : 0, now, 0.02);
    };
    applyAudio();

    // Loop the phrase on the gain node while playing.
    const curve = new Float32Array(N);
    for (let i = 0; i < N; i++) curve[i] = phrase(i / RATE);
    let loopTimer = 0;
    let loopStart = 0;
    const schedule = () => {
      const t0 = eng.ctx.currentTime + 0.05;
      loopStart = t0;
      env.gain.cancelScheduledValues(t0);
      env.gain.setValueCurveAtTime(curve, t0, LOOP_S);
      loopTimer = window.setTimeout(schedule, LOOP_S * 1000 - 20);
    };
    const startAudio = () => { voice.start(); schedule(); };
    const stopAudio = () => { window.clearTimeout(loopTimer); voice.stop(); env.gain.cancelScheduledValues(eng.ctx.currentTime); env.gain.setValueAtTime(0, eng.ctx.currentTime); };

    const g1 = group(controls, '静态曲线');
    slider(g1, { label: 'Threshold', min: -50, max: 0, step: 1, value: p.thr, format: (v) => `${v} dB`, onInput: (v) => { p.thr = v; applyAudio(); } });
    slider(g1, { label: 'Ratio', min: 1, max: 20, step: 0.5, value: p.ratio, format: (v) => `${v}:1`, onInput: (v) => { p.ratio = v; applyAudio(); } });
    slider(g1, { label: 'Makeup gain', min: 0, max: 24, step: 0.5, value: p.makeup, format: (v) => `+${v} dB`, onInput: (v) => { p.makeup = v; applyAudio(); } });
    const g2 = group(controls, '时间');
    slider(g2, { label: 'Attack', min: 0.1, max: 200, step: 0.1, value: p.attack, format: (v) => `${v.toFixed(1)} ms`, onInput: (v) => { p.attack = v; applyAudio(); } });
    slider(g2, { label: 'Release', min: 10, max: 1500, step: 10, value: p.release, format: (v) => `${v} ms`, onInput: (v) => { p.release = v; applyAudio(); } });
    hint(g2, 'Attack 太快: 音头被削平, 没冲击; Release 太快: 音尾抽气. Ratio ≥ 10 且 attack 极快就是<em>限制器</em>.');
    const g3 = group(controls, '听');
    toggle(g3, 'Bypass', p.bypass, (v) => { p.bypass = v; applyAudio(); });
    const play = playButton(g3, startAudio, stopAudio);
    const ro = readout(g3);

    const inDb = new Float64Array(N);
    for (let i = 0; i < N; i++) inDb[i] = gainToDb(Math.max(curve[i], 1e-4));
    const outDb = new Float64Array(N);
    const grDb = new Float64Array(N);

    const simulate = () => {
      const aA = Math.exp(-1 / (RATE * Math.max(p.attack, 0.05) / 1000));
      const aR = Math.exp(-1 / (RATE * p.release / 1000));
      let gr = 0;
      for (let i = 0; i < N; i++) {
        const x = inDb[i];
        const y = x > p.thr ? p.thr + (x - p.thr) / p.ratio : x;
        const want = y - x;
        gr = want < gr ? aA * gr + (1 - aA) * want : aR * gr + (1 - aR) * want;
        grDb[i] = gr;
        outDb[i] = p.bypass ? x : x + gr + p.makeup;
      }
    };

    const stop = loop((_dt, t) => {
      simulate();
      cv.clear();
      const ctx = cv.ctx;
      const [rL, rR] = columnRects(cv.width, cv.height, 2);
      const rLeft = { ...rL, w: Math.min(rL.w, rL.h) };
      const axS = new Axes(rLeft, [-60, 0], [-60, 0], { title: '静态曲线', xLabel: '输入 dB', yLabel: '输出 dB' });
      axS.frame(ctx, [-60, -40, -20, 0], [-60, -40, -20, 0], fmtDb, fmtDb);
      axS.plotFn(ctx, (x) => x, 'rgba(255,255,255,0.2)', 1, 2);
      axS.plotFn(ctx, (x) => (p.bypass ? x : (x > p.thr ? p.thr + (x - p.thr) / p.ratio : x) + p.makeup), PALETTE.yellow, 2.5, 200);
      axS.vline(ctx, p.thr, 'rgba(252,98,85,0.5)', [3, 3]);
      axS.label(ctx, p.thr, -58, `thr ${p.thr}`, PALETTE.red, 4, 0);
      const cursor = ((t % LOOP_S) + LOOP_S) % LOOP_S;
      const idx = Math.max(0, Math.min(N - 1, Math.floor(cursor * RATE)));
      axS.dot(ctx, inDb[idx], outDb[idx], PALETTE.blue, 5);
      annotate(ctx, rLeft.x + 8, rLeft.y + 16, `斜率 1/${p.ratio}: 超过阈值 ${p.ratio} dB 只出来 1 dB`, PALETTE.yellow);

      const rRight = { x: rLeft.x + rLeft.w + 70, y: rR.y, w: cv.width - (rLeft.x + rLeft.w + 70) - 24, h: rR.h };
      const axE = new Axes(rRight, [0, LOOP_S], [-50, 6], { title: '包络: 输入 (灰) → 输出 (蓝), 增益衰减 (红)', xLabel: 's', yLabel: 'dB' });
      axE.frame(ctx, [0, 0.5, 1, 1.5, 2], [-40, -30, -20, -10, 0], (v) => v.toFixed(1), fmtDb);
      const xs = Array.from({ length: N }, (_, i) => i / RATE);
      axE.hline(ctx, p.thr, 'rgba(252,98,85,0.5)', [3, 3]);
      axE.plotXY(ctx, xs, inDb, 'rgba(255,255,255,0.3)', 1.5);
      axE.plotXY(ctx, xs, outDb, PALETTE.blue, 2.5);
      axE.plotXY(ctx, xs, grDb, PALETTE.red, 1.5);
      const playhead = play.playing() ? ((eng.ctx.currentTime - loopStart) % LOOP_S + LOOP_S) % LOOP_S : cursor;
      axE.vline(ctx, playhead, 'rgba(255,255,255,0.5)');
      const gr = grDb[Math.max(0, Math.min(N - 1, Math.floor(playhead * RATE)))];
      annotate(ctx, rRight.x + rRight.w - 8, rRight.y + 16, `此刻增益衰减 ${gr.toFixed(1)} dB`, PALETTE.red, 'right');

      // Steady-state level of each of the four notes (sampled just before each note ends).
      const probes = [0.48, 1.03, 1.63, 2.23].map((s) => Math.round(s * RATE));
      const inLv = probes.map((i) => inDb[i]);
      const outLv = probes.map((i) => outDb[i]);
      const inRange = Math.max(...inLv) - Math.min(...inLv);
      const outRange = Math.max(...outLv) - Math.min(...outLv);
      ro.set(`四个音的电平跨度: 输入 <b>${inRange.toFixed(1)} dB</b> → 输出 <b>${outRange.toFixed(1)} dB</b>. 越小越"平", 也越少呼吸.`);
    });

    return {
      dispose() {
        stop();
        window.clearTimeout(loopTimer);
        voice.dispose();
        trim.disconnect();
        cv.dispose();
      },
    };
  },
};
