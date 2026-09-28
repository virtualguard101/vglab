import { mountLayout, type ExperimentDef } from '../../app/experiment';
import { getEngine, rampGain } from '../../core/audio/engine';
import { Smooth, loop } from '../../core/anim/tween';
import { createCanvas, PALETTE } from '../../core/viz/canvas';
import { Axes, annotate, stackRects } from '../../core/viz/plots';
import { group, playButton, readout, slider, hint } from '../../ui/controls';

const WINDOW_MS = 20;

export const waveParams: ExperimentDef = {
  id: 'wave-params',
  stage: 'source',
  title: '声波参数: 振幅 / 频率 / 相位',
  summary: '两列正弦叠加. 同频同幅相差 180° 时完全抵消.',
  mapping: '振幅 ≈ SV2 Loudness / VOCALOID DYN / 推子; 频率 ≈ F0; 相位 → 混音里的对齐与抵消',
  note: { doc: 'aco-theory', section: '声波的构成参数' },
  mount(root) {
    const { viz, controls } = mountLayout(root, this);
    const cv = createCanvas(viz);

    const p = {
      a1: new Smooth(1), f1: new Smooth(220), ph1: new Smooth(0),
      a2: new Smooth(1), f2: new Smooth(220), ph2: new Smooth(180),
    };

    const eng = getEngine();
    const out = eng.ctx.createGain();
    out.gain.value = 0;
    out.connect(eng.master);
    const mk = () => {
      const osc = eng.ctx.createOscillator();
      const delay = eng.ctx.createDelay(1);
      const g = eng.ctx.createGain();
      osc.connect(delay).connect(g).connect(out);
      osc.start();
      return { osc, delay, g };
    };
    const v1 = mk();
    const v2 = mk();
    const applyAudio = () => {
      const now = eng.ctx.currentTime;
      v1.osc.frequency.setTargetAtTime(p.f1.target, now, 0.02);
      v2.osc.frequency.setTargetAtTime(p.f2.target, now, 0.02);
      v1.g.gain.setTargetAtTime(p.a1.target * 0.3, now, 0.02);
      v2.g.gain.setTargetAtTime(p.a2.target * 0.3, now, 0.02);
      // A phase offset is a delay of φ/360 periods.
      v1.delay.delayTime.setTargetAtTime(((p.ph1.target % 360) / 360) / p.f1.target, now, 0.02);
      v2.delay.delayTime.setTargetAtTime(((p.ph2.target % 360) / 360) / p.f2.target, now, 0.02);
    };
    applyAudio();

    const mkGroup = (title: string, a: Smooth, f: Smooth, ph: Smooth) => {
      const g = group(controls, title);
      slider(g, { label: '振幅 A', min: 0, max: 1, step: 0.01, value: a.value, onInput: (v) => { a.set(v); applyAudio(); } });
      slider(g, { label: '频率 f (Hz)', min: 110, max: 440, step: 1, value: f.value, format: (v) => `${v} Hz`, onInput: (v) => { f.set(v); applyAudio(); } });
      slider(g, { label: '相位 φ (°)', min: 0, max: 360, step: 1, value: ph.value, format: (v) => `${v}°`, onInput: (v) => { ph.set(v); applyAudio(); } });
    };
    mkGroup('波 1 (蓝)', p.a1, p.f1, p.ph1);
    mkGroup('波 2 (黄)', p.a2, p.f2, p.ph2);

    const g3 = group(controls, '叠加');
    playButton(g3, () => rampGain(out.gain, eng.ctx, 1), () => rampGain(out.gain, eng.ctx, 0));
    const ro = readout(g3);
    hint(g3, '试试: 同频, 同幅, φ 差 <em>180°</em> → 和为零. 频率差几 Hz → 和的包络慢慢起伏, 这就是<em>拍频</em>.');

    const wave = (a: number, f: number, ph: number, tMs: number, t0: number) => a * Math.sin(2 * Math.PI * f * ((tMs + t0 * 1000) / 1000) + (ph * Math.PI) / 180);

    const stop = loop((dt, t) => {
      for (const s of Object.values(p)) s.step(dt);
      cv.clear();
      const ctx = cv.ctx;
      const rects = stackRects(cv.width, cv.height, 3);
      const xr: [number, number] = [0, WINDOW_MS];
      const ax1 = new Axes(rects[0], xr, [-1.1, 1.1], { title: '波 1', xLabel: 'ms' });
      const ax2 = new Axes(rects[1], xr, [-1.1, 1.1], { title: '波 2' });
      const ax3 = new Axes(rects[2], xr, [-2.2, 2.2], { title: '叠加 = 波 1 + 波 2' });
      const xt = [0, 5, 10, 15, 20];
      ax1.frame(ctx, xt, [-1, 0, 1]);
      ax2.frame(ctx, xt, [-1, 0, 1]);
      ax3.frame(ctx, xt, [-2, 0, 2]);
      const slow = t * 0.15; // slow drift so the waves visibly travel
      const w1 = (x: number) => wave(p.a1.value, p.f1.value, p.ph1.value, x, slow);
      const w2 = (x: number) => wave(p.a2.value, p.f2.value, p.ph2.value, x, slow);
      ax1.plotFn(ctx, w1, PALETTE.blue);
      ax2.plotFn(ctx, w2, PALETTE.yellow);
      ax3.plotFn(ctx, w1, 'rgba(88,196,221,0.25)', 1);
      ax3.plotFn(ctx, w2, 'rgba(255,255,0,0.25)', 1);
      ax3.plotFn(ctx, (x) => w1(x) + w2(x), PALETTE.green, 2.5);

      let peak = 0;
      for (let i = 0; i < 400; i++) {
        const x = (i / 400) * WINDOW_MS;
        peak = Math.max(peak, Math.abs(w1(x) + w2(x)));
      }
      const sameF = Math.abs(p.f1.value - p.f2.value) < 0.5;
      const dph = Math.abs(((((p.ph1.value - p.ph2.value) % 360) + 360) % 360) - 180);
      const cancel = sameF && dph < 2 && Math.abs(p.a1.value - p.a2.value) < 0.02;
      if (cancel) annotate(ctx, rects[2].x + rects[2].w / 2, rects[2].y + rects[2].h / 2, '相位抵消: 两列波完全互相抹掉', PALETTE.red, 'center');
      else if (!sameF) annotate(ctx, rects[2].x + rects[2].w - 8, rects[2].y + 14, `拍频 ≈ |f1 − f2| = ${Math.abs(p.f1.value - p.f2.value).toFixed(1)} Hz`, PALETTE.yellow, 'right');
      ro.set(`叠加峰值 <b>${peak.toFixed(2)}</b>  (A1 + A2 = ${(p.a1.value + p.a2.value).toFixed(2)})`);
    });

    return {
      dispose() {
        stop();
        v1.osc.stop();
        v2.osc.stop();
        out.disconnect();
        cv.dispose();
      },
    };
  },
};
