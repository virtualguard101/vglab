import { mountLayout, type ExperimentDef } from '../../app/experiment';
import { getEngine } from '../../core/audio/engine';
import { loop } from '../../core/anim/tween';
import { beatMs } from '../../core/dsp/theory';
import { createCanvas, PALETTE } from '../../core/viz/canvas';
import { annotate } from '../../core/viz/plots';
import { group, hint, playButton, readout, select, slider } from '../../ui/controls';

const DIVISIONS = [
  { value: '4', label: '全音符', div: 4 },
  { value: '2', label: '二分音符', div: 2 },
  { value: '1', label: '四分音符 (一拍)', div: 1 },
  { value: '0.5', label: '八分音符', div: 0.5 },
  { value: '0.333', label: '八分三连音', div: 1 / 3 },
  { value: '0.25', label: '十六分音符', div: 0.25 },
  { value: '0.75', label: '附点八分', div: 0.75 },
];

export const tempo: ExperimentDef = {
  id: 'tempo',
  stage: 'compose',
  title: '拍与时间: BPM → 毫秒',
  summary: '球每拍落地一次. 一拍的毫秒数 = 60000 / BPM; 延迟 / 混响预延迟常常对齐到某个分音符.',
  mapping: 'DAW 的 tempo; 延迟时间 sync; 压缩 release 常对着一拍或半拍来调',
  note: { doc: 'basic-music-theory', section: '节奏与拍' },
  mount(root) {
    const { viz, controls } = mountLayout(root, this);
    const cv = createCanvas(viz);
    const eng = getEngine();

    const p = { bpm: 120, div: 1 };
    let running = false;
    let nextBeatTime = 0;
    let beatIndex = 0;
    let timer = 0;

    const click = (when: number, accent: boolean) => {
      const osc = eng.ctx.createOscillator();
      const g = eng.ctx.createGain();
      osc.frequency.value = accent ? 1500 : 1000;
      g.gain.setValueAtTime(accent ? 0.6 : 0.35, when);
      g.gain.exponentialRampToValueAtTime(0.001, when + 0.06);
      osc.connect(g).connect(eng.master);
      osc.start(when);
      osc.stop(when + 0.07);
    };
    const scheduler = () => {
      const beat = beatMs(p.bpm) / 1000;
      while (nextBeatTime < eng.ctx.currentTime + 0.12) {
        click(nextBeatTime, beatIndex % 4 === 0);
        nextBeatTime += beat;
        beatIndex++;
      }
    };
    const start = () => { running = true; beatIndex = 0; nextBeatTime = eng.ctx.currentTime + 0.05; timer = window.setInterval(scheduler, 25); };
    const stopClicks = () => { running = false; window.clearInterval(timer); };

    const g1 = group(controls, '速度');
    slider(g1, { label: 'BPM', min: 40, max: 240, step: 1, value: p.bpm, format: (v) => `${v} BPM`, onInput: (v) => (p.bpm = v) });
    select(g1, '分音符', DIVISIONS.map((d) => ({ value: d.value, label: d.label })), '1', (v) => (p.div = DIVISIONS.find((d) => d.value === v)!.div));
    hint(g1, 't<sub>ms</sub> = 60000 / BPM × 分音符 (四分 = 1, 八分 = 0.5 …). 120 BPM 四分音符 = 500 ms.');
    const g2 = group(controls, '听节拍器');
    playButton(g2, start, stopClicks, ['开始', '停止']);
    const ro = readout(g2);

    let visualPhase = 0; // beats, fractional
    const stop = loop((dt) => {
      const beatS = beatMs(p.bpm) / 1000;
      if (running) visualPhase = (eng.ctx.currentTime - (nextBeatTime - beatIndex * beatS)) / beatS;
      else visualPhase += dt / beatS;
      const bar = ((visualPhase % 4) + 4) % 4;

      cv.clear();
      const ctx = cv.ctx;
      const W = cv.width, H = cv.height;
      const x0 = 60, x1 = W - 60;
      const ground = H * 0.62;
      const barW = x1 - x0;

      // one bar of 4/4
      ctx.save();
      ctx.strokeStyle = PALETTE.grey;
      ctx.beginPath(); ctx.moveTo(x0, ground); ctx.lineTo(x1, ground); ctx.stroke();
      const divs = Math.round(4 / p.div);
      for (let i = 0; i <= divs; i++) {
        const x = x0 + (i / divs) * barW;
        const onBeat = Math.abs((i * p.div) % 1) < 1e-6;
        ctx.strokeStyle = onBeat ? 'rgba(255,255,255,0.6)' : 'rgba(255,255,255,0.2)';
        ctx.beginPath(); ctx.moveTo(x, ground - (onBeat ? 18 : 10)); ctx.lineTo(x, ground + (onBeat ? 18 : 10)); ctx.stroke();
      }
      ctx.fillStyle = PALETTE.grey;
      ctx.font = '12px system-ui, sans-serif';
      ctx.textAlign = 'center';
      for (let b = 0; b < 4; b++) ctx.fillText(String(b + 1), x0 + ((b + 0.0) / 4) * barW, ground + 36);
      ctx.restore();

      // bouncing ball: parabola within each beat
      const frac = bar % 1;
      const bx = x0 + (bar / 4) * barW;
      const by = ground - 4 * frac * (1 - frac) * H * 0.35 - 10;
      ctx.save();
      ctx.fillStyle = Math.floor(bar) === 0 ? PALETTE.yellow : PALETTE.blue;
      ctx.beginPath(); ctx.arc(bx, by, 12, 0, Math.PI * 2); ctx.fill();
      if (frac < 0.12) {
        ctx.strokeStyle = 'rgba(255,255,255,0.5)';
        ctx.beginPath(); ctx.arc(bx, ground, 14 + frac * 200, 0, Math.PI * 2); ctx.stroke();
      }
      ctx.restore();

      const beat = beatMs(p.bpm);
      const chosen = beatMs(p.bpm, p.div);
      annotate(ctx, x0, 40, `一拍 = 60000 / ${p.bpm} = ${beat.toFixed(1)} ms`, PALETTE.white);
      annotate(ctx, x0, 66, `所选分音符 = ${chosen.toFixed(1)} ms  (${(1000 / chosen).toFixed(2)} Hz)`, PALETTE.yellow);
      annotate(ctx, x1, 40, `一小节 (4/4) = ${(beat * 4 / 1000).toFixed(2)} s`, PALETTE.grey, 'right');

      const rows: [string, number][] = [['1/4', beatMs(p.bpm, 1)], ['1/8', beatMs(p.bpm, 0.5)], ['1/8T', beatMs(p.bpm, 1 / 3)], ['1/16', beatMs(p.bpm, 0.25)], ['1/32', beatMs(p.bpm, 0.125)]];
      ctx.save();
      ctx.font = '12px "JetBrains Mono", Menlo, monospace';
      ctx.textAlign = 'left';
      ctx.fillStyle = PALETTE.grey;
      rows.forEach(([n, ms], i) => {
        ctx.fillStyle = Math.abs(ms - chosen) < 0.01 ? PALETTE.yellow : PALETTE.grey;
        ctx.fillText(`${n.padEnd(5)} ${ms.toFixed(1).padStart(7)} ms`, x0, ground + 70 + i * 18);
      });
      ctx.restore();
      annotate(ctx, x1, ground + 70, '预延迟 / 延迟 / release 对齐这些数, 效果就"在拍上"', PALETTE.green, 'right');

      ro.set(`${p.bpm} BPM: 一拍 <b>${beat.toFixed(1)} ms</b>, 所选 <b>${chosen.toFixed(1)} ms</b>`);
    });

    return {
      dispose() {
        stop();
        stopClicks();
        cv.dispose();
      },
    };
  },
};
