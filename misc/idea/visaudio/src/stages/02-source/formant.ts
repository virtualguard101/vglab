import { mountLayout, type ExperimentDef } from '../../app/experiment';
import { getEngine } from '../../core/audio/engine';
import { formantEnvelope, FormantBank, VOWELS, type Formant } from '../../core/audio/formant';
import { HARMONIC_PRESETS, Voice } from '../../core/audio/voice';
import { Smooth, loop } from '../../core/anim/tween';
import { clamp } from '../../core/dsp/theory';
import { createCanvas, PALETTE, pointerPos } from '../../core/viz/canvas';
import { Axes, annotate, columnRects } from '../../core/viz/plots';
import { button, buttonRow, group, hint, playButton, readout, slider } from '../../ui/controls';

const F1_RANGE: [number, number] = [200, 1000];
const F2_RANGE: [number, number] = [600, 3000];
const F3_BASE = 2600;

export const formant: ExperimentDef = {
  id: 'formant',
  stage: 'source',
  title: '共振峰与元音',
  summary: '在 F1–F2 平面上拖动: 同一个 F0, 元音变了, 谐波串没变, 变的是包络.',
  mapping: '共振峰整体平移 ≈ SV2 Gender / VOCALOID GEN, Character; 开口度 ≈ Mouth Opening / OPE',
  note: { doc: 'aco-theory', section: '共鸣与共振峰' },
  mount(root) {
    const { viz, controls } = mountLayout(root, this);
    const cv = createCanvas(viz);
    const eng = getEngine();
    const voice = new Voice(eng.ctx);
    voice.setHarmonics(HARMONIC_PRESETS['锯齿'].concat(Array.from({ length: 16 }, (_, i) => 1 / (i + 9))));
    const bank = new FormantBank(eng.ctx, 3);
    voice.output.connect(bank.input);
    bank.output.connect(eng.master);
    bank.output.gain.value = 2.5;

    const f1 = new Smooth(VOWELS[0].f1, 12);
    const f2 = new Smooth(VOWELS[0].f2, 12);
    const gender = new Smooth(1, 8);
    const f0 = new Smooth(160, 12);
    voice.setFrequency(f0.value);

    const formants = (): Formant[] => {
      const g = gender.value;
      return [
        { freq: f1.value * g, bw: 90, gain: 1 },
        { freq: f2.value * g, bw: 120, gain: 0.6 },
        { freq: F3_BASE * g, bw: 180, gain: 0.35 },
      ];
    };

    const g1 = group(controls, '元音预设');
    const row = buttonRow(g1);
    for (const v of VOWELS) button(row, v.label, () => { f1.set(v.f1); f2.set(v.f2); });
    hint(g1, '也可以直接在左图<em>拖动黄点</em>. F1 大致对应开口度, F2 对应舌位前后.');

    const g2 = group(controls, '声道与声源');
    slider(g2, { label: 'Gender (共振峰整体平移)', min: 0.75, max: 1.25, step: 0.01, value: 1, format: (v) => `× ${v.toFixed(2)}`, onInput: (v) => gender.set(v) });
    slider(g2, { label: 'F0 (音高, 不动元音)', min: 100, max: 400, step: 1, value: 160, format: (v) => `${v} Hz`, onInput: (v) => { f0.set(v); voice.setFrequency(v); } });
    playButton(g2, () => voice.start(), () => voice.stop());
    const ro = readout(g2);

    let dragging = false;
    let plane: Axes | null = null;
    const onDown = (ev: PointerEvent) => {
      if (!plane) return;
      const { x, y } = pointerPos(cv.el, ev);
      if (!plane.contains(x, y)) return;
      dragging = true;
      cv.el.setPointerCapture(ev.pointerId);
      moveTo(x, y);
    };
    const moveTo = (x: number, y: number) => {
      if (!plane) return;
      f1.set(clamp(plane.fromX(x), ...F1_RANGE));
      f2.set(clamp(plane.fromY(y), ...F2_RANGE));
    };
    const onMove = (ev: PointerEvent) => { if (dragging) { const { x, y } = pointerPos(cv.el, ev); moveTo(x, y); } };
    const onUp = () => (dragging = false);
    cv.el.addEventListener('pointerdown', onDown);
    cv.el.addEventListener('pointermove', onMove);
    cv.el.addEventListener('pointerup', onUp);
    cv.el.style.cursor = 'crosshair';

    const stop = loop((dt) => {
      f1.step(dt); f2.step(dt); gender.step(dt); f0.step(dt);
      const fm = formants();
      bank.set(fm);

      cv.clear();
      const ctx = cv.ctx;
      const [rL, rR] = columnRects(cv.width, cv.height, 2);
      plane = new Axes(rL, F1_RANGE, F2_RANGE, { title: '元音平面', xLabel: 'F1 (Hz)', yLabel: 'F2 (Hz)' });
      plane.frame(ctx, [200, 400, 600, 800, 1000], [1000, 1500, 2000, 2500, 3000]);
      for (const v of VOWELS) {
        plane.dot(ctx, v.f1, v.f2, PALETTE.grey, 4);
        plane.label(ctx, v.f1, v.f2, v.label, PALETTE.grey, 8, -10);
      }
      plane.dot(ctx, f1.value, f2.value, PALETTE.yellow, 7);
      if (Math.abs(gender.value - 1) > 0.01) {
        plane.dot(ctx, fm[0].freq, fm[1].freq, PALETTE.red, 5);
        plane.label(ctx, fm[0].freq, fm[1].freq, 'Gender 后', PALETTE.red, 8, 10);
      }

      const axS = new Axes(rR, [0, 4000], [0, 1.15], { title: '频谱: 谐波串 × 声道包络', xLabel: 'Hz' });
      axS.frame(ctx, [0, 1000, 2000, 3000, 4000], [0, 0.5, 1]);
      const env = (f: number) => formantEnvelope(fm, f);
      let envMax = 0.001;
      for (let f = 0; f < 4000; f += 20) envMax = Math.max(envMax, env(f));
      axS.plotFn(ctx, (f) => env(f) / envMax, PALETTE.yellow, 2, 500);
      const xs: number[] = [];
      const ys: number[] = [];
      for (let k = 1; k * f0.value < 4000; k++) {
        xs.push(k * f0.value);
        ys.push(env(k * f0.value) / envMax);
      }
      axS.stems(ctx, xs, ys, PALETTE.blue, 3);
      fm.forEach((f, i) => { axS.vline(ctx, f.freq, 'rgba(255,255,0,0.25)', [3, 3]); axS.label(ctx, f.freq, 1.08, `F${i + 1}`, PALETTE.yellow, 0, 0, 'center'); });
      annotate(ctx, rR.x + rR.w - 8, rR.y + rR.h - 16, `谐波间距 = F0 = ${f0.value.toFixed(0)} Hz`, PALETTE.blue, 'right');

      const nearest = VOWELS.reduce((best, v) => {
        const d = Math.hypot((v.f1 - f1.value) / 100, (v.f2 - f2.value) / 300);
        return d < best.d ? { v, d } : best;
      }, { v: VOWELS[0], d: Infinity });
      ro.set(`F1 <b>${fm[0].freq.toFixed(0)}</b>  F2 <b>${fm[1].freq.toFixed(0)}</b>  F3 <b>${fm[2].freq.toFixed(0)}</b> Hz\n最接近 <b>${nearest.v.label}</b>${gender.value !== 1 ? '  (Gender 改的是包络位置, 不是音高)' : ''}`);
    });

    return {
      dispose() {
        stop();
        cv.el.removeEventListener('pointerdown', onDown);
        cv.el.removeEventListener('pointermove', onMove);
        cv.el.removeEventListener('pointerup', onUp);
        voice.dispose();
        bank.dispose();
        cv.dispose();
      },
    };
  },
};
