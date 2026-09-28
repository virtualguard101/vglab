# visaudio 架构

## 分层

```
src/
├── main.ts                  入口: 建 shell, 起路由, 首次手势解锁 AudioContext
├── app/                     壳
│   ├── experiment.ts        ExperimentDef / Experiment 契约, mountLayout (标题 / 画布 / 控件区)
│   ├── registry.ts          STAGES (流程条四站) 与 EXPERIMENTS (有序实验列表)
│   ├── router.ts            hash 路由 #/<stage>/<exp>, 切换时 dispose 旧实验
│   └── shell.ts             顶部流程条 + 左侧实验列表 + 主区域
├── core/                    与具体实验无关的可复用层
│   ├── dsp/                 纯函数, 不依赖浏览器 API
│   │   ├── theory.ts        midi ↔ Hz, cent, 音程表, BPM → ms, dB ↔ gain
│   │   ├── fft.ts           radix-2 FFT, 幅度谱, FFT 卷积
│   │   └── filters.ts       RBJ biquad 系数, 闭式幅频响应, Web Audio Q 换算, BiquadRunner
│   ├── audio/               Web Audio
│   │   ├── engine.ts        单例 AudioContext + master + analyser; rampGain / glide / createNoise
│   │   ├── voice.ts         加法合成"歌声": PeriodicWave 谐波 + 气声噪声 + 颤音 LFO
│   │   └── formant.ts       并联带通共振峰组; 元音 F1/F2 表; 画图用的洛伦兹包络
│   ├── anim/tween.ts        Smooth (指数逼近, manim 式滑动) 与 rAF loop
│   └── viz/
│       ├── canvas.ts        DPR 自适应 Canvas, 调色板 (3b1b 蓝 / 黄 / 红 / 绿)
│       └── plots.ts         Axes (线性 / 对数坐标, 网格, 曲线, 竖线, 色带, 标注), 布局辅助
├── ui/controls.ts           slider / toggle / select / button / playButton / readout / hint
└── stages/                  每个实验一个文件, 导出一个 ExperimentDef
    ├── 01-compose/          pitch-grid, intervals, tempo
    ├── 02-source/           wave-params, harmonics, formant, f0-vibrato
    ├── 03-mix/              eq, dynamics, reverb, phase
    └── 04-master/           loudness
```

依赖方向自上而下: `stages` → `app` / `core` / `ui`; `core/dsp` 不依赖任何浏览器 API, 可以在 Node 里单测.

## 实验契约

```ts
interface ExperimentDef {
  id: string;                 // 路由片段
  stage: StageId;             // compose | source | mix | master
  title: string;
  summary: string;            // 一句话: 看什么
  mapping?: string;           // 对应 SV2 / VOCALOID / DAW 里的哪个旋钮
  note: { doc; section };     // 对应笔记的小节
  mount(root: HTMLElement): { dispose(): void };
}
```

`mount` 内部的固定套路:

1. `mountLayout(root, this)` 拿到 `viz` 与 `controls` 两个容器
2. `createCanvas(viz)` 建画布; 参数放在一个 `p` 对象里, 需要平滑的用 `Smooth`
3. 建 Web Audio 图 (Voice → 处理节点 → `engine.master`), 写一个 `applyAudio()` 把 `p` 推到 AudioParam
4. 用 `ui/controls` 建滑块, `onInput` 同时改 `p` 和调 `applyAudio()`
5. `loop((dt, t) => { ...step Smooth; cv.clear(); 用 Axes 画 })`
6. `dispose()` 里停 loop, 停振荡器, 断开输出, 释放画布

## 两个原则

**图由公式画, 声由节点出.** 频响曲线来自 `filters.ts` 的闭式 `|H(e^{jω})|`, 梳状滤波来自 `|1 + g e^{-jωτ}|`, 混响包络来自 `exp(-6.9 t / RT60)` 与 FFT 卷积. 不用 AnalyserNode 反推曲线 (会抖, 且分辩率随窗长变), 只在母带页用它做电平表, 因为那里"实测"本身就是主题.

**Web Audio 的坑集中在 core.** `BiquadFilterNode` 的 lowpass / highpass 用 dB 表示 Q → `webAudioQ()`; 自动播放策略 → `engine.ensureRunning()` 由播放按钮与首个手势调用; 切参数点击声 → `rampGain` / `glide`; rAF 首帧时间戳可能早于调度时刻 → `loop` 把 dt / t 夹到非负.

## 扩展一个实验

1. 在对应 `stages/0x-*/` 下新建文件, 导出 `ExperimentDef`
2. 在 `app/registry.ts` 的 `EXPERIMENTS` 中按流程顺序插入
3. 在 README 的表格里补一行

不需要改路由或壳.
