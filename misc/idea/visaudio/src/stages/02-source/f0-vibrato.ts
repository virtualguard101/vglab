import { mountLayout, type ExperimentDef } from '../../app/experiment';
import { getEngine } from '../../core/audio/engine';
import { HARMONIC_PRESETS, Voice } from '../../core/audio/voice';
import { loop } from '../../core/anim/tween';
import { midiToHz, noteName } from '../../core/dsp/theory';
import { createCanvas, PALETTE } from '../../core/viz/canvas';
import { Axes, annotate } from '../../core/viz/plots';
import { group, hint, playButton, readout, slider, toggle } from '../../ui/controls';

const HISTORY_S = 4;
const SAMPLES = 480;

export const f0Vibrato: ExperimentDef = {
  id: 'f0-vibrato',
  stage: 'source',
  title: 'F0 音高线: 颤音与量化',
  summary: '看音高线怎么绕着格子摆. 量化把线钉到格子上, 颤音是格子附近的小摆动.',
  mapping: 'F0 曲线 ≈ SV2 Pitch Deviation / VOCALOID PIT; 颤音 ≈ Vibrato; 格子见乐理基础',
  note: { doc: 'aco-theory', section: '频率' },
  mount(root) {
    const { viz, controls } = mountLayout(root, this);
    const cv = createCanvas(viz);
    const eng = getEngine();
    const voice = new Voice(eng.ctx);
    voice.setHarmonics(HARMONIC_PRESETS['暗']);
    voice.output.connect(eng.master);

    const p = { note: 62, offsetCents: 30, rate: 5.5, depth: 40, quantize: false, drift: 0.3 };
    let driftPhase = Math.random() * 100;
    const hist = new Float64Array(SAMPLES).fill(p.note);
    const target = new Float64Array(SAMPLES).fill(p.note);
    let acc = 0;

    const g1 = group(controls, '目标音与偏差');
    slider(g1, { label: '目标音符', min: 55, max: 72, step: 1, value: p.note, format: (v) => `${noteName(v)} (MIDI ${v})`, onInput: (v) => (p.note = v) });
    slider(g1, { label: '歌手偏差 (cent)', min: -100, max: 100, step: 1, value: p.offsetCents, format: (v) => `${v > 0 ? '+' : ''}${v} ¢`, onInput: (v) => (p.offsetCents = v) });
    slider(g1, { label: '慢速漂移', min: 0, max: 1, step: 0.01, value: p.drift, onInput: (v) => (p.drift = v) });
    toggle(g1, '量化到最近的半音 (把线钉到格子上)', p.quantize, (v) => (p.quantize = v));

    const g2 = group(controls, '颤音');
    slider(g2, { label: '速率', min: 0, max: 10, step: 0.1, value: p.rate, format: (v) => `${v.toFixed(1)} Hz`, onInput: (v) => (p.rate = v) });
    slider(g2, { label: '深度', min: 0, max: 120, step: 1, value: p.depth, format: (v) => `±${v} ¢`, onInput: (v) => (p.depth = v) });
    hint(g2, '5–7 Hz 像人声; 太慢像滑音, 太快像羊叫; 深度过大, 音准就发虚. 量化只钉<em>中心</em>, 颤音仍在.');

    const g3 = group(controls, '听');
    playButton(g3, () => voice.start(), () => voice.stop());
    const ro = readout(g3);

    const stop = loop((dt, t) => {
      driftPhase += dt * 0.35;
      const drift = p.drift * 60 * (Math.sin(driftPhase) * 0.6 + Math.sin(driftPhase * 2.3 + 1) * 0.4);
      const centerCents = p.quantize ? 0 : p.offsetCents + drift;
      const center = p.note + centerCents / 100;
      const vib = p.depth * Math.sin(2 * Math.PI * p.rate * t);
      const f0Midi = center + vib / 100;

      voice.setFrequency(midiToHz(center), 0.03);
      voice.setVibrato(p.rate, p.depth);

      acc += dt;
      const step = HISTORY_S / SAMPLES;
      while (acc >= step) {
        acc -= step;
        hist.copyWithin(0, 1);
        target.copyWithin(0, 1);
        hist[SAMPLES - 1] = f0Midi;
        target[SAMPLES - 1] = p.note;
      }

      cv.clear();
      const ctx = cv.ctx;
      const rect = { x: 70, y: 30, w: cv.width - 94, h: cv.height - 58 };
      const lo = p.note - 2.5;
      const hi = p.note + 2.5;
      const ax = new Axes(rect, [-HISTORY_S, 0], [lo, hi], { title: '音高线 (最近 4 秒)', xLabel: 's' });
      const noteTicks: number[] = [];
      for (let n = Math.ceil(lo); n <= Math.floor(hi); n++) noteTicks.push(n);
      ax.frame(ctx, [-4, -3, -2, -1, 0], noteTicks, (v) => String(v), (v) => noteName(v));
      for (const n of noteTicks) ax.hline(ctx, n, n === p.note ? 'rgba(255,255,0,0.35)' : 'rgba(255,255,255,0.08)', n === p.note ? [] : [2, 4]);
      const xs = Array.from({ length: SAMPLES }, (_, i) => -HISTORY_S + (i / (SAMPLES - 1)) * HISTORY_S);
      ax.plotXY(ctx, xs, target, 'rgba(255,255,0,0.5)', 1.5);
      ax.plotXY(ctx, xs, hist, PALETTE.blue, 2.5);
      ax.dot(ctx, 0, f0Midi, PALETTE.blue, 5);

      const devCents = (f0Midi - p.note) * 100;
      const color = Math.abs(devCents) < 20 ? PALETTE.green : Math.abs(devCents) < 50 ? PALETTE.yellow : PALETTE.red;
      annotate(ctx, rect.x + rect.w - 8, rect.y + 16, `此刻偏离目标 ${devCents >= 0 ? '+' : ''}${devCents.toFixed(0)} ¢`, color, 'right');
      if (p.quantize) annotate(ctx, rect.x + 8, rect.y + 16, '已量化: 中心钉在格子上, 只剩颤音', PALETTE.yellow);
      ro.set(`目标 <b>${noteName(p.note)}</b> = ${midiToHz(p.note).toFixed(1)} Hz\n当前中心 <b>${midiToHz(center).toFixed(1)} Hz</b>  (${centerCents >= 0 ? '+' : ''}${centerCents.toFixed(0)} ¢)`);
    });

    return {
      dispose() {
        stop();
        voice.dispose();
        cv.dispose();
      },
    };
  },
};
