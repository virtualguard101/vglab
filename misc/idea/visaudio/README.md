# visaudio

**成熟度: mvp**

3b1b / manim 风格的调音混音可视化: 12 个可交互的小实验, 每个实验一张实时重绘的 Canvas 加一组滑块, 并且**能听**. 帮助初学者把[调音混音背后的声学理论基础](../../../docs/wiki/docs/obsidian/音视频/调音混音背后的声学理论基础.md)和[乐理基础](../../../docs/wiki/docs/obsidian/音视频/乐理基础.md)里的公式与 DAW / 歌声合成器里的旋钮对应起来.

页面顶部是音乐作品制作的流程条, 实验按流程分成四站:

| 站 | 实验 | 看什么 |
|---|---|---|
| 01 作曲编曲 | 音高格子 / 音程与和弦 / 拍与时间 | 十二平均律在对数轴上等距; 协和音程的谐波重合; BPM → ms |
| 02 声源调音 | 声波参数 / 谐波与音色 / 共振峰与元音 / F0 音高线 | 振幅·频率·相位; 谐波比例决定亮暗; F1–F2 决定元音; 颤音与量化 |
| 03 混音 | EQ / 压缩 / 混响 / 相位 | H(f) 与 Y = X·H; 静态曲线与包络; 脉冲响应与 RT60; 梳状滤波与 mono 检查 |
| 04 母带 | 电平与响度 | dBFS / RMS / 近似 LUFS, +6 dB = ×2, 削波与限制器 |

## 依赖

本地开发需要自行安装:

- Node.js ≥ 20, [pnpm](https://pnpm.io/) 11
- 可选 [just](https://github.com/casey/just)
- 浏览器需支持 Web Audio API; 首次点击页面后才会出声 (自动播放策略)

## 本地

```bash
cd misc/idea/visaudio
just bootstrap   # pnpm install
just dev         # http://127.0.0.1:5180
just check       # tsc --noEmit
just build       # 与 Vercel 相同的生产构建, 产物 dist/
just preview     # 预览 dist/
```

## Vercel

仓库根目录不是这个应用. 新建项目时把 **Root Directory** 设为 `misc/idea/visaudio`. 构建由 [vercel.json](vercel.json) 指定: `pnpm install --frozen-lockfile`, 然后 `bash scripts/up.sh` (只打包 `dist/` 并退出). Node 选 20 或以上.

路由是 hash, 例如 `#/mix/eq`, 不需要重写规则. `VITE_BASE` 保持默认 `/`. 设置 `VITE_WIKI_BASE=<mkdocs 站点地址>` 后页面上的"笔记"链接会指向对应文档.

## 技术选型

Vite + TypeScript, 原生 Canvas 2D 与 Web Audio, 无 UI 框架. 画面上的曲线一律由闭式公式或离线 DSP (`src/core/dsp/`) 计算, 音频引擎只负责"听"; 这样图不抖, 数值可以直接对照笔记里的公式. 结构说明见 [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## 数值对照

- A4 = 440 Hz, C4 = 261.63 Hz, 半音比 2^(1/12) ≈ 1.05946 (音高格子)
- 纯五度: 平均律 700 ¢, 纯律 3:2 = 701.96 ¢ (音程与和弦)
- 120 BPM 四分音符 = 500 ms (拍与时间)
- 同频同幅相位差 180° → 和为 0 (声波参数)
- 延迟 τ 的梳状缺口在 (2k+1)/(2τ), 间距 1/τ (相位)
- 增益 +6 dB → 峰值幅度 ×2.00 (电平与响度)
