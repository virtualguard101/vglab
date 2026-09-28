import { mountLayout, type ExperimentDef } from '../../app/experiment';
import { getEngine } from '../../core/audio/engine';
import { HARMONIC_PRESETS, Voice } from '../../core/audio/voice';
import { Smooth, loop } from '../../core/anim/tween';
import { createCanvas, PALETTE } from '../../core/viz/canvas';
import { Axes, annotate, stackRects } from '../../core/viz/plots';
import { button, buttonRow, group, hint, playButton, readout, slider, type Slider } from '../../ui/controls';

const N = 8;

export const harmonics: ExperimentDef = {
  id: 'harmonics',
  stage: 'source',
  title: '谐波与音色',
  summary: '拖每一次谐波的电平, 波形与频谱同步变化. 亮暗来自谐波比例, 音高不变.',
  mapping: '谐波亮暗 ≈ SV2 Tension / VOCALOID BRI, CLE; 渲染后 → DAW 里的 EQ, 饱和',
  note: { doc: 'aco-theory', section: '傅里叶声学' },
  mount(root) {
    const { viz, controls } = mountLayout(root, this);
    const cv = createCanvas(viz);
    const eng = getEngine();
    const voice = new Voice(eng.ctx);
    voice.output.connect(eng.master);

    const amps = HARMONIC_PRESETS['暗'].map((v) => new Smooth(v, 14));
    const f0 = new Smooth(220, 14);
    let breath = 0;
    const sliders: Slider[] = [];

    const push = () => voice.setHarmonics(amps.map((a) => a.target));

    const g1 = group(controls, '谐波电平 (k × F0)');
    for (let k = 0; k < N; k++) {
      sliders.push(
        slider(g1, {
          label: k === 0 ? '1 · F0 (基频)' : `${k + 1} · F0`,
          min: 0, max: 1, step: 0.01, value: amps[k].value,
          onInput: (v) => { amps[k].set(v); push(); },
        }),
      );
    }

    const g2 = group(controls, '预设');
    const row = buttonRow(g2);
    for (const [name, preset] of Object.entries(HARMONIC_PRESETS)) {
      button(row, name, () => {
        preset.forEach((v, i) => { amps[i].set(v); sliders[i].set(v); });
        push();
      });
    }
    hint(g2, '<em>锯齿</em>: 1/k 全谐波, 像拉弦. <em>方波</em>: 只有奇次, 空心. 只有第 1 条 = 纯正弦. 音高始终由 F0 决定.');

    const g3 = group(controls, '声源');
    slider(g3, { label: 'F0', min: 110, max: 440, step: 1, value: 220, format: (v) => `${v} Hz`, onInput: (v) => { f0.set(v); voice.setFrequency(v); } });
    slider(g3, { label: '气声 (噪声比例)', min: 0, max: 1, step: 0.01, value: 0, onInput: (v) => { breath = v; voice.setBreath(v); } });
    playButton(g3, () => voice.start(), () => voice.stop());
    const ro = readout(g3);
    push();

    const stop = loop((dt, t) => {
      amps.forEach((a) => a.step(dt));
      f0.step(dt);
      cv.clear();
      const ctx = cv.ctx;
      const [rTop, rBot] = stackRects(cv.width, cv.height, 2);
      const F = f0.value;
      const periodMs = 1000 / F;
      const win = 2 * periodMs;
      const axW = new Axes(rTop, [0, win], [-1.6, 1.6], { title: '波形 (两个周期)', xLabel: 'ms' });
      axW.frame(ctx, [0, periodMs, 2 * periodMs], [-1, 0, 1], (v) => v.toFixed(1));
      const phase = t * 0.4;
      const partial = (k: number, x: number) => amps[k].value * Math.sin(2 * Math.PI * (k + 1) * F * (x / 1000) + phase);
      for (let k = 1; k < N; k++) if (amps[k].value > 0.005) axW.plotFn(ctx, (x) => partial(k, x), 'rgba(255,255,255,0.10)', 1, 300);
      axW.plotFn(ctx, (x) => partial(0, x), 'rgba(88,196,221,0.35)', 1, 300);
      axW.plotFn(ctx, (x) => { let s = 0; for (let k = 0; k < N; k++) s += partial(k, x); return s; }, PALETTE.blue, 2.5, 600);

      const axS = new Axes(rBot, [0, (N + 1) * F], [0, 1.1], { title: '频谱: 每根线是一个谐波', xLabel: 'Hz' });
      const ticks: number[] = [];
      for (let k = 1; k <= N; k++) ticks.push(k * F);
      axS.frame(ctx, ticks, [0, 0.5, 1], (v) => String(Math.round(v)));
      const xs = ticks;
      const ys = amps.map((a) => a.value);
      axS.stems(ctx, xs, ys, PALETTE.yellow, 6);
      axS.stems(ctx, [F], [ys[0]], PALETTE.blue, 6);
      if (breath > 0.01) {
        axS.plotFn(ctx, (f) => (f > 2000 ? breath * 0.25 * (0.6 + 0.4 * Math.sin(f * 0.01 + t * 20)) : 0), 'rgba(252,98,85,0.6)', 1, 300);
      }

      let num = 0, den = 0;
      ys.forEach((a, k) => { num += a * (k + 1) * F; den += a; });
      const centroid = den > 0 ? num / den : F;
      ro.set(`F0 <b>${F.toFixed(0)} Hz</b>  谐波重心 <b>${centroid.toFixed(0)} Hz</b>  → 重心越高越亮`);
      annotate(ctx, rBot.x + rBot.w - 8, rBot.y + 14, `重心 ${centroid.toFixed(0)} Hz`, PALETTE.green, 'right');
      axS.vline(ctx, centroid, 'rgba(131,193,103,0.6)', [4, 4]);
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
