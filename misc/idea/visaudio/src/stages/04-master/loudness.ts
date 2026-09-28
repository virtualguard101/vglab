import { mountLayout, type ExperimentDef } from '../../app/experiment';
import { createNoise, getEngine } from '../../core/audio/engine';
import { HARMONIC_PRESETS, Voice } from '../../core/audio/voice';
import { loop } from '../../core/anim/tween';
import { BiquadRunner, rbj } from '../../core/dsp/filters';
import { dbToGain, gainToDb } from '../../core/dsp/theory';
import { createCanvas, PALETTE } from '../../core/viz/canvas';
import { Axes, annotate, fmtDb, type Rect } from '../../core/viz/plots';
import { group, hint, playButton, readout, select, slider, toggle } from '../../ui/controls';

type Src = 'sine' | 'voice' | 'noise';

export const loudness: ExperimentDef = {
  id: 'loudness',
  stage: 'master',
  title: '电平与响度: dBFS, RMS, LUFS',
  summary: '推增益看三根表. +6 dB 幅度翻倍; 顶到 0 dBFS 就削波; 限制器把峰值按在天花板下.',
  mapping: '母带的最后一步: 增益 / 限制器 / 目标响度 (流媒体约 −14 LUFS)',
  note: { doc: 'aco-theory', section: '声压级与响度' },
  mount(root) {
    const { viz, controls } = mountLayout(root, this);
    const cv = createCanvas(viz);
    const eng = getEngine();
    const fs = eng.ctx.sampleRate;

    const p = { gainDb: 0, ceiling: -1, limiter: false, src: 'voice' as Src };

    const voice = new Voice(eng.ctx);
    voice.setHarmonics(HARMONIC_PRESETS['暗']);
    voice.setFrequency(220);
    voice.setLevel(0.35);
    voice.setVibrato(5.5, 25);
    const sine = eng.ctx.createOscillator(); sine.frequency.value = 1000; sine.start();
    const sineG = eng.ctx.createGain(); sineG.gain.value = 0;
    sine.connect(sineG);
    const noise = createNoise(eng.ctx); noise.start();
    const noiseG = eng.ctx.createGain(); noiseG.gain.value = 0;
    noise.connect(noiseG);
    const bus = eng.ctx.createGain();
    voice.output.connect(bus); sineG.connect(bus); noiseG.connect(bus);

    const gain = eng.ctx.createGain();
    const limiter = eng.ctx.createDynamicsCompressor();
    limiter.knee.value = 0; limiter.ratio.value = 20; limiter.attack.value = 0.001; limiter.release.value = 0.12;
    const clipper = eng.ctx.createWaveShaper();
    const curve = new Float32Array(4096);
    for (let i = 0; i < curve.length; i++) curve[i] = Math.max(-1, Math.min(1, (i / (curve.length - 1)) * 2 - 1));
    clipper.curve = curve; // hard clip at 0 dBFS, what a DAC / file does anyway
    const limOn = eng.ctx.createGain(); const limOff = eng.ctx.createGain();
    bus.connect(gain);
    gain.connect(limiter).connect(limOn).connect(clipper);
    gain.connect(limOff).connect(clipper);
    const meterTap = eng.ctx.createAnalyser(); meterTap.fftSize = 2048;
    const out = eng.ctx.createGain(); out.gain.value = 0;
    clipper.connect(meterTap).connect(out).connect(eng.master);

    const applyAudio = () => {
      const now = eng.ctx.currentTime;
      gain.gain.setTargetAtTime(dbToGain(p.gainDb), now, 0.02);
      limiter.threshold.setTargetAtTime(p.ceiling, now, 0.02);
      limOn.gain.setTargetAtTime(p.limiter ? 1 : 0, now, 0.02);
      limOff.gain.setTargetAtTime(p.limiter ? 0 : 1, now, 0.02);
      sineG.gain.setTargetAtTime(p.src === 'sine' ? 0.5 : 0, now, 0.02);
      noiseG.gain.setTargetAtTime(p.src === 'noise' ? 0.25 : 0, now, 0.02);
      if (p.src === 'voice') voice.start(); else voice.stop();
    };

    const g1 = group(controls, '电平');
    slider(g1, { label: '增益', min: -30, max: 18, step: 0.5, value: p.gainDb, format: (v) => `${fmtDb(v)} dB  (× ${dbToGain(v).toFixed(2)})`, onInput: (v) => { p.gainDb = v; applyAudio(); } });
    slider(g1, { label: '限制器天花板', min: -12, max: 0, step: 0.5, value: p.ceiling, format: (v) => `${v} dBFS`, onInput: (v) => { p.ceiling = v; applyAudio(); } });
    toggle(g1, '限制器 (ratio 20:1, attack 1 ms)', p.limiter, (v) => { p.limiter = v; applyAudio(); });
    hint(g1, '+6 dB = 幅度 × 2; +20 dB = × 10. 响度感大约每 +10 dB 翻一倍. 超过 0 dBFS 的样本会被<em>削平</em>.');
    const g2 = group(controls, '声源与听');
    select(g2, '声源', [{ value: 'voice', label: '人声样 (有颤音)' }, { value: 'sine', label: '1 kHz 正弦' }, { value: 'noise', label: '白噪声' }], p.src, (v) => { p.src = v; applyAudio(); });
    playButton(g2, () => { applyAudio(); out.gain.setTargetAtTime(1, eng.ctx.currentTime, 0.02); }, () => { out.gain.setTargetAtTime(0, eng.ctx.currentTime, 0.02); voice.stop(); });
    const ro = readout(g2);
    hint(g2, 'RMS 是均方根 (能量), LUFS 是加了 K 权重 (人耳对中高频更敏感) 的 RMS, 这里用瞬时 (~400 ms) 近似.');

    // K-weighting per ITU-R BS.1770: high shelf then high pass.
    const kShelf = new BiquadRunner(rbj('highshelf', 1681.97, fs, 0.7071, 3.99984));
    const kHp = new BiquadRunner(rbj('highpass', 38.13, fs, 0.5));
    const buf = new Float32Array(meterTap.fftSize);
    let peakHold = -100, peakHoldT = 0;
    let rmsSmooth = 0, lufsMs = 0, clipCount = 0;
    const history: number[] = [];

    const stop = loop((dt, t) => {
      meterTap.getFloatTimeDomainData(buf);
      let peak = 0, sq = 0, ksq = 0, clips = 0;
      for (let i = 0; i < buf.length; i++) {
        const x = buf[i];
        const a = Math.abs(x);
        if (a > peak) peak = a;
        if (a >= 0.999) clips++;
        sq += x * x;
        const k = kHp.process(kShelf.process(x));
        ksq += k * k;
      }
      const alpha = 1 - Math.exp(-dt / 0.4);
      rmsSmooth += (sq / buf.length - rmsSmooth) * alpha;
      lufsMs += (ksq / buf.length - lufsMs) * alpha;
      clipCount = clipCount * 0.9 + clips;
      const peakDb = gainToDb(peak);
      const rmsDb = gainToDb(Math.sqrt(rmsSmooth));
      const lufs = -0.691 + 10 * Math.log10(Math.max(lufsMs, 1e-10));
      if (peakDb >= peakHold || t - peakHoldT > 1.5) { peakHold = peakDb; peakHoldT = t; }
      history.push(peak); if (history.length > 300) history.shift();

      cv.clear();
      const ctx = cv.ctx;
      const W = cv.width, H = cv.height;
      const meterRect: Rect = { x: 70, y: 40, w: Math.min(300, W * 0.32), h: H - 80 };
      const ax = new Axes(meterRect, [0, 3], [-60, 6], { title: '表', yLabel: 'dB' });
      ax.frame(ctx, [], [-60, -48, -36, -24, -14, -6, 0], undefined, fmtDb);
      ax.hline(ctx, 0, 'rgba(252,98,85,0.8)');
      ax.label(ctx, 3, 0, '0 dBFS: 数字满刻度', PALETTE.red, 6, 0);
      ax.hline(ctx, -14, 'rgba(131,193,103,0.5)', [3, 3]);
      ax.label(ctx, 3, -14, '−14 LUFS: 流媒体常见目标', PALETTE.green, 6, 0);
      if (p.limiter) { ax.hline(ctx, p.ceiling, 'rgba(255,255,0,0.7)', [2, 2]); ax.label(ctx, 3, p.ceiling, `天花板 ${p.ceiling}`, PALETTE.yellow, 6, 10); }
      const bars: [string, number, string][] = [['Peak', peakDb, PALETTE.blue], ['RMS', rmsDb, PALETTE.teal], ['LUFS', lufs, PALETTE.yellow]];
      bars.forEach(([name, v, color], i) => {
        const x0 = ax.toX(i + 0.15), x1 = ax.toX(i + 0.85);
        const yTop = ax.toY(Math.max(-60, Math.min(6, v)));
        const yBase = ax.toY(-60);
        ctx.fillStyle = color;
        ctx.globalAlpha = 0.85;
        ctx.fillRect(x0, yTop, x1 - x0, yBase - yTop);
        ctx.globalAlpha = 1;
        ctx.fillStyle = PALETTE.white;
        ctx.font = '12px system-ui, sans-serif';
        ctx.textAlign = 'center';
        ctx.fillText(name, (x0 + x1) / 2, yBase + 16);
        ctx.fillText(Number.isFinite(v) && v > -99 ? v.toFixed(1) : '−∞', (x0 + x1) / 2, Math.max(meterRect.y + 12, yTop - 8));
      });
      const hx0 = ax.toX(0.15), hx1 = ax.toX(0.85);
      ctx.fillStyle = clipCount > 1 ? PALETTE.red : PALETTE.white;
      ctx.fillRect(hx0, ax.toY(Math.min(6, peakHold)) - 1, hx1 - hx0, 2);

      const waveRect: Rect = { x: meterRect.x + meterRect.w + 90, y: 40, w: W - (meterRect.x + meterRect.w + 90) - 30, h: (H - 80) * 0.55 };
      const axW = new Axes(waveRect, [0, buf.length], [-1.25, 1.25], { title: '输出波形 (最近 ~43 ms)' });
      axW.frame(ctx, [], [-1, 0, 1]);
      axW.hline(ctx, 1, 'rgba(252,98,85,0.6)', [3, 3]); axW.hline(ctx, -1, 'rgba(252,98,85,0.6)', [3, 3]);
      if (p.limiter) { const c = dbToGain(p.ceiling); axW.hline(ctx, c, 'rgba(255,255,0,0.5)', [2, 2]); axW.hline(ctx, -c, 'rgba(255,255,0,0.5)', [2, 2]); }
      axW.plotXY(ctx, Array.from({ length: buf.length }, (_, i) => i), buf, clipCount > 1 ? PALETTE.red : PALETTE.blue, 1.5);
      if (clipCount > 1) annotate(ctx, waveRect.x + waveRect.w / 2, waveRect.y + waveRect.h / 2, '削波: 波峰被切平, 长出刺耳的高次谐波', PALETTE.red, 'center');

      const histRect: Rect = { x: waveRect.x, y: waveRect.y + waveRect.h + 50, w: waveRect.w, h: H - (waveRect.y + waveRect.h + 50) - 40 };
      const axH = new Axes(histRect, [0, 300], [-40, 3], { title: '峰值历史 (dBFS)' });
      axH.frame(ctx, [], [-36, -24, -12, 0], undefined, fmtDb);
      axH.hline(ctx, 0, 'rgba(252,98,85,0.6)');
      axH.plotXY(ctx, Array.from({ length: history.length }, (_, i) => 300 - history.length + i), history.map((v) => gainToDb(v)), PALETTE.blue, 1.5);

      const crest = peakDb - rmsDb;
      ro.set(`Peak <b>${peakDb.toFixed(1)} dBFS</b>  RMS <b>${rmsDb.toFixed(1)} dB</b>  ≈ <b>${lufs.toFixed(1)} LUFS</b>  峰均比 ${Number.isFinite(crest) ? crest.toFixed(1) : '–'} dB${clipCount > 1 ? '  <b style="color:#fc6255">CLIP</b>' : ''}\n增益 ${fmtDb(p.gainDb)} dB → 幅度 × ${dbToGain(p.gainDb).toFixed(2)}${Math.abs(p.gainDb - 6) < 0.26 ? '  (+6 dB 恰好 × 2)' : ''}`);
    });

    return {
      dispose() {
        stop();
        sine.stop(); noise.stop();
        voice.dispose();
        out.disconnect();
        cv.dispose();
      },
    };
  },
};
