import { PALETTE } from './canvas';

export interface Rect {
  x: number;
  y: number;
  w: number;
  h: number;
}

export interface AxesOptions {
  xLog?: boolean;
  yLog?: boolean;
  xLabel?: string;
  yLabel?: string;
  title?: string;
}

const FONT = '12px "JetBrains Mono", Menlo, monospace';

/** Maps data coordinates into a rectangle and draws frame / ticks / curves. */
export class Axes {
  constructor(
    public rect: Rect,
    public xr: [number, number],
    public yr: [number, number],
    public opts: AxesOptions = {},
  ) {}

  toX(v: number): number {
    const [a, b] = this.xr;
    const t = this.opts.xLog ? Math.log(v / a) / Math.log(b / a) : (v - a) / (b - a);
    return this.rect.x + t * this.rect.w;
  }

  toY(v: number): number {
    const [a, b] = this.yr;
    const t = this.opts.yLog ? Math.log(v / a) / Math.log(b / a) : (v - a) / (b - a);
    return this.rect.y + this.rect.h - t * this.rect.h;
  }

  fromX(px: number): number {
    const t = (px - this.rect.x) / this.rect.w;
    const [a, b] = this.xr;
    return this.opts.xLog ? a * (b / a) ** t : a + t * (b - a);
  }

  fromY(py: number): number {
    const t = (this.rect.y + this.rect.h - py) / this.rect.h;
    const [a, b] = this.yr;
    return this.opts.yLog ? a * (b / a) ** t : a + t * (b - a);
  }

  contains(px: number, py: number): boolean {
    const r = this.rect;
    return px >= r.x && px <= r.x + r.w && py >= r.y && py <= r.y + r.h;
  }

  frame(ctx: CanvasRenderingContext2D, xTicks: number[] = [], yTicks: number[] = [], fmtX = fmtNum, fmtY = fmtNum): void {
    const r = this.rect;
    ctx.save();
    ctx.strokeStyle = PALETTE.greyDark;
    ctx.lineWidth = 1;
    ctx.fillStyle = PALETTE.grey;
    ctx.font = FONT;
    ctx.textAlign = 'center';
    ctx.textBaseline = 'top';
    for (const v of xTicks) {
      const x = this.toX(v);
      ctx.beginPath();
      ctx.moveTo(x, r.y);
      ctx.lineTo(x, r.y + r.h);
      ctx.stroke();
      ctx.fillText(fmtX(v), x, r.y + r.h + 4);
    }
    ctx.textAlign = 'right';
    ctx.textBaseline = 'middle';
    for (const v of yTicks) {
      const y = this.toY(v);
      ctx.beginPath();
      ctx.moveTo(r.x, y);
      ctx.lineTo(r.x + r.w, y);
      ctx.stroke();
      ctx.fillText(fmtY(v), r.x - 6, y);
    }
    ctx.strokeStyle = PALETTE.grey;
    ctx.strokeRect(r.x, r.y, r.w, r.h);
    ctx.fillStyle = PALETTE.grey;
    ctx.textAlign = 'right';
    ctx.textBaseline = 'bottom';
    if (this.opts.xLabel) ctx.fillText(this.opts.xLabel, r.x + r.w, r.y - 4);
    ctx.textAlign = 'left';
    let titleEnd = r.x;
    if (this.opts.title) {
      ctx.fillStyle = PALETTE.white;
      ctx.font = '13px system-ui, sans-serif';
      ctx.fillText(this.opts.title, r.x + 6, r.y - 4);
      titleEnd = r.x + 6 + ctx.measureText(this.opts.title).width + 14;
      ctx.fillStyle = PALETTE.grey;
      ctx.font = FONT;
    }
    if (this.opts.yLabel) ctx.fillText(`↑ ${this.opts.yLabel}`, titleEnd, r.y - 4);
    ctx.restore();
  }

  /** Plot y = fn(x) sampled uniformly (in screen space) across the x range. */
  plotFn(ctx: CanvasRenderingContext2D, fn: (x: number) => number, color: string, width = 2, n = 400): void {
    ctx.save();
    this.clip(ctx);
    ctx.strokeStyle = color;
    ctx.lineWidth = width;
    ctx.lineJoin = 'round';
    ctx.beginPath();
    for (let i = 0; i <= n; i++) {
      const px = this.rect.x + (i / n) * this.rect.w;
      const y = fn(this.fromX(px));
      const py = this.toY(y);
      if (i === 0) ctx.moveTo(px, py);
      else ctx.lineTo(px, py);
    }
    ctx.stroke();
    ctx.restore();
  }

  plotXY(ctx: CanvasRenderingContext2D, xs: ArrayLike<number>, ys: ArrayLike<number>, color: string, width = 2): void {
    ctx.save();
    this.clip(ctx);
    ctx.strokeStyle = color;
    ctx.lineWidth = width;
    ctx.lineJoin = 'round';
    ctx.beginPath();
    for (let i = 0; i < xs.length; i++) {
      const px = this.toX(xs[i]);
      const py = this.toY(ys[i]);
      if (i === 0) ctx.moveTo(px, py);
      else ctx.lineTo(px, py);
    }
    ctx.stroke();
    ctx.restore();
  }

  /** Vertical stems (e.g. spectral lines). */
  stems(ctx: CanvasRenderingContext2D, xs: ArrayLike<number>, ys: ArrayLike<number>, color: string, width = 3, base = this.yr[0]): void {
    ctx.save();
    this.clip(ctx);
    ctx.strokeStyle = color;
    ctx.lineWidth = width;
    ctx.lineCap = 'round';
    const y0 = this.toY(base);
    for (let i = 0; i < xs.length; i++) {
      const px = this.toX(xs[i]);
      ctx.beginPath();
      ctx.moveTo(px, y0);
      ctx.lineTo(px, this.toY(ys[i]));
      ctx.stroke();
    }
    ctx.restore();
  }

  vline(ctx: CanvasRenderingContext2D, x: number, color: string, dash: number[] = [], width = 1): void {
    ctx.save();
    ctx.strokeStyle = color;
    ctx.lineWidth = width;
    ctx.setLineDash(dash);
    const px = this.toX(x);
    ctx.beginPath();
    ctx.moveTo(px, this.rect.y);
    ctx.lineTo(px, this.rect.y + this.rect.h);
    ctx.stroke();
    ctx.restore();
  }

  hline(ctx: CanvasRenderingContext2D, y: number, color: string, dash: number[] = [], width = 1): void {
    ctx.save();
    ctx.strokeStyle = color;
    ctx.lineWidth = width;
    ctx.setLineDash(dash);
    const py = this.toY(y);
    ctx.beginPath();
    ctx.moveTo(this.rect.x, py);
    ctx.lineTo(this.rect.x + this.rect.w, py);
    ctx.stroke();
    ctx.restore();
  }

  band(ctx: CanvasRenderingContext2D, x0: number, x1: number, color: string, label?: string): void {
    ctx.save();
    this.clip(ctx);
    const a = this.toX(x0);
    const b = this.toX(x1);
    ctx.fillStyle = color;
    ctx.fillRect(a, this.rect.y, b - a, this.rect.h);
    if (label) {
      ctx.fillStyle = PALETTE.grey;
      ctx.font = FONT;
      ctx.textAlign = 'center';
      ctx.textBaseline = 'top';
      ctx.fillText(label, (a + b) / 2, this.rect.y + 4);
    }
    ctx.restore();
  }

  dot(ctx: CanvasRenderingContext2D, x: number, y: number, color: string, r = 5): void {
    ctx.save();
    ctx.fillStyle = color;
    ctx.beginPath();
    ctx.arc(this.toX(x), this.toY(y), r, 0, Math.PI * 2);
    ctx.fill();
    ctx.restore();
  }

  label(ctx: CanvasRenderingContext2D, x: number, y: number, text: string, color: string = PALETTE.white, dx = 8, dy = -8, align: CanvasTextAlign = 'left'): void {
    ctx.save();
    ctx.fillStyle = color;
    ctx.font = FONT;
    ctx.textAlign = align;
    ctx.textBaseline = 'middle';
    ctx.fillText(text, this.toX(x) + dx, this.toY(y) + dy);
    ctx.restore();
  }

  private clip(ctx: CanvasRenderingContext2D): void {
    ctx.beginPath();
    ctx.rect(this.rect.x, this.rect.y, this.rect.w, this.rect.h);
    ctx.clip();
  }
}

export function fmtNum(v: number): string {
  if (Math.abs(v) >= 1000) return `${(v / 1000).toFixed(v % 1000 === 0 ? 0 : 1)}k`;
  if (Number.isInteger(v)) return String(v);
  return v.toFixed(Math.abs(v) < 1 ? 2 : 1);
}

export const fmtHz = (v: number): string => `${fmtNum(v)}`;
export const fmtDb = (v: number): string => `${v > 0 ? '+' : ''}${v}`;
export const fmtMs = (v: number): string => `${fmtNum(v)}`;

export const LOG_FREQ_TICKS = [20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000];

/** Text with a subtle dark backdrop, for annotations over curves. */
export function annotate(ctx: CanvasRenderingContext2D, x: number, y: number, text: string, color: string = PALETTE.white, align: CanvasTextAlign = 'left'): void {
  ctx.save();
  ctx.font = '13px system-ui, sans-serif';
  ctx.textAlign = align;
  ctx.textBaseline = 'middle';
  const w = ctx.measureText(text).width;
  const x0 = align === 'left' ? x - 4 : align === 'right' ? x - w - 4 : x - w / 2 - 4;
  ctx.fillStyle = 'rgba(0,0,0,0.7)';
  ctx.fillRect(x0, y - 10, w + 8, 20);
  ctx.fillStyle = color;
  ctx.fillText(text, x, y);
  ctx.restore();
}

/** Split a rect into vertical stacks with margins for labels. */
export function stackRects(width: number, height: number, n: number, pad = { l: 56, r: 24, t: 30, b: 28 }, gap = 40): Rect[] {
  const usable = height - pad.t - pad.b - gap * (n - 1);
  const h = usable / n;
  const out: Rect[] = [];
  for (let i = 0; i < n; i++) out.push({ x: pad.l, y: pad.t + i * (h + gap), w: width - pad.l - pad.r, h });
  return out;
}

/** Split a rect into side-by-side columns. */
export function columnRects(width: number, height: number, n: number, pad = { l: 56, r: 24, t: 30, b: 28 }, gap = 70): Rect[] {
  const usable = width - pad.l - pad.r - gap * (n - 1);
  const w = usable / n;
  const out: Rect[] = [];
  for (let i = 0; i < n; i++) out.push({ x: pad.l + i * (w + gap), y: pad.t, w, h: height - pad.t - pad.b });
  return out;
}
