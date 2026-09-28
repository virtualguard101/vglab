import { mountLayout, type ExperimentDef } from '../../app/experiment';
import { getEngine } from '../../core/audio/engine';
import { HARMONIC_PRESETS, Voice } from '../../core/audio/voice';
import { Smooth, loop } from '../../core/anim/tween';
import { isBlackKey, noteName } from '../../core/dsp/theory';
import { createCanvas, PALETTE, pointerPos } from '../../core/viz/canvas';
import { Axes, annotate, fmtNum, type Rect } from '../../core/viz/plots';
import { group, hint, readout, slider, toggle } from '../../ui/controls';

const LO = 48; // C3
const HI = 72; // C5

export const pitchGrid: ExperimentDef = {
  id: 'pitch-grid',
  stage: 'compose',
  title: '音高格子: 十二平均律',
  summary: '点琴键. 每个半音是频率乘 2^(1/12); 在对数轴上它们等距, 在线性轴上越来越稀.',
  mapping: '钢琴卷帘的每一行 = 一个格子; 音符之间的偏差用 cent 说, 100 cent = 1 格',
  note: { doc: 'basic-music-theory', section: '音高格子' },
  mount(root) {
    const { viz, controls } = mountLayout(root, this);
    const cv = createCanvas(viz);
    const eng = getEngine();
    const voice = new Voice(eng.ctx);
    voice.setHarmonics(HARMONIC_PRESETS['暗']);
    voice.setLevel(0.5);
    voice.output.connect(eng.master);

    const p = { a4: 440, cents: 0, logAxis: true };
    let current = 69;
    const marker = new Smooth(69, 14);
    const hz = (midi: number) => p.a4 * 2 ** ((midi - 69) / 12);
    const playNote = (midi: number) => {
      current = midi;
      marker.set(midi);
      void eng.ensureRunning();
      voice.setFrequency(hz(midi + p.cents / 100), 0.005);
      voice.pluck(0.01, 0.35, 0.25);
    };

    const g1 = group(controls, '格子的参数');
    slider(g1, { label: '参考音高 A4', min: 415, max: 466, step: 1, value: p.a4, format: (v) => `${v} Hz`, onInput: (v) => { p.a4 = v; } });
    slider(g1, { label: '偏离格子 (cent)', min: -100, max: 100, step: 1, value: p.cents, format: (v) => `${v > 0 ? '+' : ''}${v} ¢`, onInput: (v) => { p.cents = v; voice.setFrequency(hz(current + v / 100), 0.02); } });
    toggle(g1, '下方频率轴用对数刻度', p.logAxis, (v) => (p.logAxis = v));
    hint(g1, 'A4 = 440 Hz 是约定; 巴洛克常用 415, 一些乐团用 442. 格子整体移动, 相对关系不变.');
    const g2 = group(controls, '读数');
    const ro = readout(g2);
    hint(g2, 'f = 440 · 2^((n − 69) / 12). 相差 1200 · log2(f2 / f1) cent. 人耳大约在 5–10 cent 开始察觉走音.');

    let keyRects: { midi: number; r: Rect }[] = [];
    const onDown = (ev: PointerEvent) => {
      const { x, y } = pointerPos(cv.el, ev);
      // black keys are drawn on top, so test them first
      const hit = [...keyRects].sort((a, b) => Number(isBlackKey(b.midi)) - Number(isBlackKey(a.midi))).find(({ r }) => x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h);
      if (hit) playNote(hit.midi);
    };
    cv.el.addEventListener('pointerdown', onDown);
    cv.el.style.cursor = 'pointer';

    const stop = loop((dt) => {
      marker.step(dt);
      cv.clear();
      const ctx = cv.ctx;
      const W = cv.width, H = cv.height;
      const kb: Rect = { x: 40, y: 34, w: W - 80, h: Math.min(150, H * 0.36) };
      const whites: number[] = [];
      for (let n = LO; n <= HI; n++) if (!isBlackKey(n)) whites.push(n);
      const ww = kb.w / whites.length;
      keyRects = [];
      ctx.save();
      ctx.font = '11px system-ui, sans-serif';
      ctx.textAlign = 'center';
      ctx.textBaseline = 'bottom';
      whites.forEach((n, i) => {
        const r = { x: kb.x + i * ww, y: kb.y, w: ww - 2, h: kb.h };
        keyRects.push({ midi: n, r });
        ctx.fillStyle = n === current ? PALETTE.blue : '#ECECEC';
        ctx.fillRect(r.x, r.y, r.w, r.h);
        ctx.fillStyle = n === current ? '#000' : '#555';
        ctx.fillText(noteName(n), r.x + r.w / 2, r.y + r.h - 6);
      });
      whites.forEach((n, i) => {
        if (n + 1 <= HI && isBlackKey(n + 1)) {
          const r = { x: kb.x + (i + 1) * ww - ww * 0.3 - 1, y: kb.y, w: ww * 0.6, h: kb.h * 0.6 };
          keyRects.push({ midi: n + 1, r });
          ctx.fillStyle = n + 1 === current ? PALETTE.blue : '#111';
          ctx.fillRect(r.x, r.y, r.w, r.h);
        }
      });
      ctx.restore();
      annotate(ctx, kb.x, kb.y - 12, '点琴键 (C3 – C5)', PALETTE.white);

      const axRect: Rect = { x: 60, y: kb.y + kb.h + 60, w: W - 90, h: H - (kb.y + kb.h + 60) - 40 };
      const fLo = hz(LO) * 0.97, fHi = hz(HI) * 1.03;
      const ax = new Axes(axRect, [fLo, fHi], [0, 1], { xLog: p.logAxis, title: p.logAxis ? '频率轴 (对数): 每个半音等距' : '频率轴 (线性): 高音区半音越来越宽', xLabel: 'Hz' });
      const ticks = [hz(48), hz(55), hz(60), hz(64), hz(67), hz(69), hz(72)];
      ax.frame(ctx, ticks, [], (v) => fmtNum(Math.round(v)));
      for (let n = LO; n <= HI; n++) {
        const f = hz(n);
        const black = isBlackKey(n);
        ax.stems(ctx, [f], [black ? 0.35 : 0.55], black ? 'rgba(255,255,255,0.35)' : 'rgba(255,255,255,0.7)', 2, 0);
        if (!black && n % 12 === 0) ax.label(ctx, f, 0.62, noteName(n), PALETTE.grey, 0, 0, 'center');
      }
      const fm = hz(marker.value);
      ax.stems(ctx, [fm], [0.8], PALETTE.blue, 4, 0);
      ax.label(ctx, fm, 0.9, `${noteName(current)} = ${hz(current).toFixed(1)} Hz`, PALETTE.blue, 0, 0, 'center');
      if (p.cents !== 0) {
        const fd = hz(current + p.cents / 100);
        ax.stems(ctx, [fd], [0.7], PALETTE.red, 2, 0);
        ax.label(ctx, fd, 0.75, `${p.cents > 0 ? '+' : ''}${p.cents} ¢ → ${fd.toFixed(1)} Hz`, PALETTE.red, p.cents > 0 ? 8 : -8, 0, p.cents > 0 ? 'left' : 'right');
      }
      // octave bracket
      const oc0 = hz(60), oc1 = hz(72);
      ax.band(ctx, oc0, oc1, 'rgba(88,196,221,0.06)', '一个八度 = 频率 × 2 = 1200 ¢');

      ro.set(`当前 <b>${noteName(current)}</b> (MIDI ${current}) = <b>${hz(current).toFixed(2)} Hz</b>\n上一半音 × 2^(1/12) = ${(hz(current) * 2 ** (1 / 12)).toFixed(2)} Hz  (${(2 ** (1 / 12)).toFixed(5)} 倍, ≈ 6%)${p.cents !== 0 ? `\n偏离 ${p.cents} ¢ → ${hz(current + p.cents / 100).toFixed(2)} Hz` : ''}`);
    });

    return {
      dispose() {
        stop();
        cv.el.removeEventListener('pointerdown', onDown);
        voice.dispose();
        cv.dispose();
      },
    };
  },
};
