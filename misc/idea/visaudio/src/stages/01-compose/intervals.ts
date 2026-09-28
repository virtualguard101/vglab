import { mountLayout, type ExperimentDef } from '../../app/experiment';
import { getEngine } from '../../core/audio/engine';
import { HARMONIC_PRESETS, Voice } from '../../core/audio/voice';
import { loop } from '../../core/anim/tween';
import { INTERVALS, midiToHz, noteName } from '../../core/dsp/theory';
import { createCanvas, PALETTE } from '../../core/viz/canvas';
import { Axes, annotate, fmtNum, stackRects } from '../../core/viz/plots';
import { button, buttonRow, group, hint, playButton, readout, select, slider } from '../../ui/controls';

const COLORS = [PALETTE.blue, PALETTE.yellow, PALETTE.red];
const CHORDS: Record<string, number[]> = { 大三和弦: [0, 4, 7], 小三和弦: [0, 3, 7], 增三和弦: [0, 4, 8], 减三和弦: [0, 3, 6], 挂四: [0, 5, 7] };
const NH = 8;

export const intervals: ExperimentDef = {
  id: 'intervals',
  stage: 'compose',
  title: '音程与和弦: 谐波的对齐',
  summary: '两三个音同时响. 谐波重合得越多 (频率比越简单), 叠加波形重复得越快, 听起来越"和".',
  mapping: '编曲时的和声选择; 混音时低频撞车 (大二度 / 小二度低音) 也是这个道理',
  note: { doc: 'basic-music-theory', section: '音程与和弦' },
  mount(root) {
    const { viz, controls } = mountLayout(root, this);
    const cv = createCanvas(viz);
    const eng = getEngine();

    let root0 = 57; // A3
    let offsets: number[] = [0, 7];
    let label = '纯五度';
    const voices = Array.from({ length: 3 }, () => {
      const v = new Voice(eng.ctx);
      v.setHarmonics(HARMONIC_PRESETS['暗']);
      v.setLevel(0.22);
      v.output.connect(eng.master);
      return v;
    });
    let playing = false;
    const applyAudio = () => {
      voices.forEach((v, i) => {
        if (i < offsets.length) {
          v.setFrequency(midiToHz(root0 + offsets[i]), 0.02);
          if (playing) v.start();
        } else v.stop();
      });
    };

    const g1 = group(controls, '选音');
    slider(g1, { label: '根音', min: 48, max: 67, step: 1, value: root0, format: (v) => `${noteName(v)} (${midiToHz(v).toFixed(1)} Hz)`, onInput: (v) => { root0 = v; applyAudio(); } });
    select(g1, '音程', INTERVALS.map((iv) => ({ value: String(iv.semitones), label: `${iv.name} (${iv.semitones} 半音, ≈ ${iv.ratio})` })), '7', (v) => {
      const iv = INTERVALS.find((x) => x.semitones === Number(v))!;
      offsets = [0, iv.semitones]; label = iv.name; applyAudio();
    });
    const row = buttonRow(g1);
    for (const [name, off] of Object.entries(CHORDS)) button(row, name, () => { offsets = off.slice(); label = name; applyAudio(); });
    hint(g1, '小二度 / 大二度的谐波彼此错开一点点 → 拍频 → "打架". 八度 / 纯五度的谐波大量重合 → 融为一体.');
    const g2 = group(controls, '听');
    playButton(g2, () => { playing = true; applyAudio(); }, () => { playing = false; voices.forEach((v) => v.stop()); });
    const ro = readout(g2);

    const stop = loop((_dt, t) => {
      cv.clear();
      const ctx = cv.ctx;
      const [rT, rB] = stackRects(cv.width, cv.height, 2);
      const freqs = offsets.map((o) => midiToHz(root0 + o));
      const f0 = freqs[0];
      const win = (1000 / f0) * 6; // six root periods
      const axW = new Axes(rT, [0, win], [-1.3 * offsets.length, 1.3 * offsets.length], { title: '叠加波形: 重复得越快越协和', xLabel: 'ms' });
      axW.frame(ctx, Array.from({ length: 7 }, (_, i) => (i * win) / 6), [], (v) => v.toFixed(1));
      const ph = t * 0.3;
      const tone = (f: number, x: number) => { let s = 0; for (let k = 1; k <= 4; k++) s += (0.5 ** (k - 1)) * Math.sin(2 * Math.PI * k * f * (x / 1000) + ph); return s / 1.9; };
      freqs.forEach((f, i) => axW.plotFn(ctx, (x) => tone(f, x), COLORS[i] + '66', 1, 400));
      axW.plotFn(ctx, (x) => freqs.reduce((s, f) => s + tone(f, x), 0), PALETTE.white, 2.5, 800);

      const fmax = f0 * (NH + 1) * 1.02 * 2 ** (Math.max(...offsets) / 12);
      const axS = new Axes(rB, [0, Math.min(fmax, f0 * 14)], [0, 1.1], { title: '每个音的谐波 (颜色区分); 重合处标白', xLabel: 'Hz' });
      axS.frame(ctx, Array.from({ length: 8 }, (_, i) => Math.round((i * Math.min(fmax, f0 * 14)) / 7)), [0, 0.5, 1], (v) => fmtNum(Math.round(v)));
      const all: { f: number; i: number; k: number }[] = [];
      freqs.forEach((f, i) => { for (let k = 1; k <= NH; k++) all.push({ f: k * f, i, k }); });
      let coincide = 0;
      for (const h of all) {
        const near = all.find((o) => o.i !== h.i && Math.abs(1200 * Math.log2(o.f / h.f)) < 8);
        const beating = !near && all.some((o) => o.i !== h.i && Math.abs(o.f - h.f) < 30 && o.f !== h.f);
        axS.stems(ctx, [h.f], [1 / h.k], near ? PALETTE.white : beating ? PALETTE.red : COLORS[h.i], near ? 4 : 2.5);
        if (near && h.i === 0) coincide++;
      }
      annotate(ctx, rB.x + rB.w - 8, rB.y + 16, `根音前 ${NH} 次谐波中有 ${coincide} 个被别的音重合`, PALETTE.white, 'right');

      const ratioText = offsets.length === 2 ? (() => {
        const iv = INTERVALS.find((x) => x.semitones === offsets[1]);
        if (!iv) return '';
        const [a, b] = iv.ratio.split(':').map(Number);
        const just = 1200 * Math.log2(a / b);
        return `纯律 ${iv.ratio} = ${just.toFixed(1)} ¢, 平均律 ${offsets[1] * 100} ¢, 差 ${(offsets[1] * 100 - just).toFixed(1)} ¢`;
      })() : offsets.map((o) => noteName(root0 + o)).join(' + ');
      ro.set(`<b>${label}</b>: ${freqs.map((f) => f.toFixed(1)).join(' / ')} Hz\n${ratioText}`);
    });

    return {
      dispose() {
        stop();
        voices.forEach((v) => v.dispose());
        cv.dispose();
      },
    };
  },
};
