import { mountLayout, type ExperimentDef } from '../../app/experiment';
import { createNoise, getEngine } from '../../core/audio/engine';
import { HARMONIC_PRESETS, Voice } from '../../core/audio/voice';
import { Smooth, loop } from '../../core/anim/tween';
import { createCanvas, PALETTE } from '../../core/viz/canvas';
import { Axes, annotate, fmtDb, fmtNum, LOG_FREQ_TICKS, stackRects } from '../../core/viz/plots';
import { group, hint, playButton, readout, select, slider, toggle } from '../../ui/controls';

const FMIN = 50;
const FMAX = 20000;

export const phase: ExperimentDef = {
  id: 'phase',
  stage: 'mix',
  title: '相位: 梳状滤波与单声道检查',
  summary: '同一条声音复制一份延迟几毫秒再叠回去, 频响长出一排缺口. 立体声里各放一边听不出, 折成单声道就出现.',
  mapping: '双轨人声 / 双话筒 / 短延迟加宽 (Haas); 检查方法: DAW 里的 mono 按钮',
  note: { doc: 'aco-theory', section: '相位' },
  mount(root) {
    const { viz, controls } = mountLayout(root, this);
    const cv = createCanvas(viz);
    const eng = getEngine();

    const p = { delay: new Smooth(3), level: new Smooth(1), invert: false, mono: true, src: 'noise' as 'noise' | 'voice' };

    const voice = new Voice(eng.ctx);
    voice.setHarmonics(HARMONIC_PRESETS['锯齿']);
    voice.setFrequency(146.8);
    voice.setBreath(0.4);
    const noise = createNoise(eng.ctx);
    const noiseLp = eng.ctx.createBiquadFilter();
    noiseLp.type = 'lowpass'; noiseLp.frequency.value = 8000;
    const noiseGain = eng.ctx.createGain(); noiseGain.gain.value = 0;
    noise.connect(noiseLp).connect(noiseGain);
    noise.start();
    const src = eng.ctx.createGain();
    voice.output.connect(src);
    noiseGain.connect(src);

    const delay = eng.ctx.createDelay(0.1);
    const copyGain = eng.ctx.createGain();
    src.connect(delay).connect(copyGain);
    const merger = eng.ctx.createChannelMerger(2);
    const out = eng.ctx.createGain(); out.gain.value = 0;
    merger.connect(out).connect(eng.master);
    // direct → L, copy → R (stereo); mono sums both into both.
    const monoSum = eng.ctx.createGain();
    src.connect(monoSum); copyGain.connect(monoSum);
    const stL = eng.ctx.createGain(), stR = eng.ctx.createGain(), mo = eng.ctx.createGain();
    src.connect(stL).connect(merger, 0, 0);
    copyGain.connect(stR).connect(merger, 0, 1);
    monoSum.connect(mo);
    mo.connect(merger, 0, 0);
    mo.connect(merger, 0, 1);

    const applyAudio = () => {
      const now = eng.ctx.currentTime;
      delay.delayTime.setTargetAtTime(p.delay.target / 1000, now, 0.02);
      copyGain.gain.setTargetAtTime(p.level.target * (p.invert ? -1 : 1), now, 0.02);
      stL.gain.setTargetAtTime(p.mono ? 0 : 1, now, 0.02);
      stR.gain.setTargetAtTime(p.mono ? 0 : 1, now, 0.02);
      mo.gain.setTargetAtTime(p.mono ? 0.5 : 0, now, 0.02);
      noiseGain.gain.setTargetAtTime(p.src === 'noise' ? 0.25 : 0, now, 0.02);
    };
    applyAudio();

    const g1 = group(controls, '复制的那一条');
    slider(g1, { label: '延迟 τ', min: 0, max: 30, step: 0.05, value: p.delay.value, format: (v) => `${v.toFixed(2)} ms`, onInput: (v) => { p.delay.set(v); applyAudio(); } });
    slider(g1, { label: '电平 g', min: 0, max: 1, step: 0.01, value: p.level.value, onInput: (v) => { p.level.set(v); applyAudio(); } });
    toggle(g1, '极性反转 (Ø)', p.invert, (v) => { p.invert = v; applyAudio(); });
    const g2 = group(controls, '监听方式');
    toggle(g2, '折成单声道 (mono 检查)', p.mono, (v) => { p.mono = v; applyAudio(); });
    select(g2, '声源', [{ value: 'noise', label: '噪声 (最容易听出梳状)' }, { value: 'voice', label: '人声样 (锯齿 + 气声)' }], p.src, (v) => { p.src = v; applyAudio(); if (v === 'voice') voice.start(); else voice.stop(); });
    hint(g2, 'τ = 0 且极性反转 → 完全抵消 (静音). τ 在 1–20 ms 之间, 缺口落在人声频段, 声音"发空 / 像在管子里".');
    const g3 = group(controls, '听');
    playButton(g3, () => { out.gain.setTargetAtTime(1, eng.ctx.currentTime, 0.02); if (p.src === 'voice') voice.start(); }, () => { out.gain.setTargetAtTime(0, eng.ctx.currentTime, 0.02); voice.stop(); });
    const ro = readout(g3);

    const stop = loop((dt, t) => {
      p.delay.step(dt); p.level.step(dt);
      const tau = p.delay.value / 1000;
      const g = p.level.value * (p.invert ? -1 : 1);
      // |1 + g e^{-jωτ}| = sqrt(1 + g² + 2g cos(ωτ))
      const magDb = (f: number) => 20 * Math.log10(Math.max(1e-6, Math.sqrt(1 + g * g + 2 * g * Math.cos(2 * Math.PI * f * tau))));

      cv.clear();
      const ctx = cv.ctx;
      const [rT, rB] = stackRects(cv.width, cv.height, 2);
      const axH = new Axes(rT, [FMIN, FMAX], [-40, 16], { xLog: true, title: p.mono ? '叠加后的频响 |1 + g·e^{−jωτ}| (dB)' : '立体声: 两边各听一条, 没有叠加 → 频响是平的', xLabel: 'Hz', yLabel: 'dB' });
      axH.frame(ctx, LOG_FREQ_TICKS.filter((f) => f >= FMIN), [-36, -24, -12, 0, 6], fmtNum, fmtDb);
      axH.hline(ctx, 0, 'rgba(255,255,255,0.25)');
      if (p.mono) {
        axH.plotFn(ctx, magDb, PALETTE.yellow, 2, 3000);
        if (tau > 1e-5) {
          const notches: number[] = [];
          for (let k = 0; notches.length < 6; k++) {
            const f = p.invert ? k / tau : (2 * k + 1) / (2 * tau);
            if (f > FMAX) break;
            if (f >= FMIN) notches.push(f);
          }
          notches.slice(0, 3).forEach((f, i) => axH.label(ctx, f, -30 - i * 3, `${fmtNum(Math.round(f))} Hz`, PALETTE.red, 4, 0));
          annotate(ctx, rT.x + rT.w - 8, rT.y + 16, `缺口间距 1/τ = ${fmtNum(Math.round(1 / tau))} Hz`, PALETTE.red, 'right');
        } else if (p.invert && Math.abs(p.level.value - 1) < 0.02) {
          annotate(ctx, rT.x + rT.w / 2, rT.y + rT.h / 2, '完全抵消: 相同, 反相, 无延迟 → 静音', PALETTE.red, 'center');
        }
      } else {
        axH.plotFn(ctx, () => 0, PALETTE.green, 2, 2);
        axH.plotFn(ctx, magDb, 'rgba(255,255,0,0.25)', 1, 3000);
        annotate(ctx, rT.x + rT.w - 8, rT.y + 16, '淡黄: 一旦被折成单声道会变成这样', PALETTE.yellow, 'right');
      }

      const win = 12;
      const axT = new Axes(rB, [0, win], [-2.3, 2.3], { title: '时域: 原声 (灰) 与延迟副本 (黄), 叠加 (蓝)', xLabel: 'ms' });
      axT.frame(ctx, [0, 2, 4, 6, 8, 10, 12], [-2, 0, 2]);
      const sig = (x: number) => Math.exp(-x * 0.9) * Math.sin(2 * Math.PI * 0.45 * x + t * 0.5) * (x >= 0 ? 1 : 0);
      const copy = (x: number) => g * sig(x - p.delay.value);
      axT.plotFn(ctx, sig, 'rgba(255,255,255,0.35)', 1.5, 600);
      axT.plotFn(ctx, copy, PALETTE.yellow, 1.5, 600);
      axT.plotFn(ctx, (x) => sig(x) + copy(x), PALETTE.blue, 2.5, 600);
      annotate(ctx, rB.x + 8, rB.y + 16, `τ = ${p.delay.value.toFixed(2)} ms ≈ 声音走 ${(p.delay.value * 0.343).toFixed(1)} m`, PALETTE.yellow);

      if (tau > 1e-5) {
        const first = p.invert ? '0 Hz (直流) 开始' : `${fmtNum(Math.round(1 / (2 * tau)))} Hz`;
        ro.set(`第一道缺口在 <b>${first}</b>, 之后每隔 <b>${fmtNum(Math.round(1 / tau))} Hz</b> 一道. 缺口深度由 g 决定: g = 1 时是 −∞ dB.`);
      } else {
        const sumDb = 20 * Math.log10(Math.max(Math.abs(1 + g), 1e-6));
        ro.set(`τ = 0: 两条完全重合, 只是电平变了 <b>${fmtDb(Number(sumDb.toFixed(1)))} dB</b>${p.invert && p.level.value > 0.98 ? ' → 静音' : ''}.`);
      }
    });

    return {
      dispose() {
        stop();
        noise.stop();
        voice.dispose();
        out.disconnect();
        cv.dispose();
      },
    };
  },
};
