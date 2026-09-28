export const PALETTE = {
  bg: '#000000',
  blue: '#58C4DD',
  yellow: '#FFFF00',
  red: '#FC6255',
  green: '#83C167',
  purple: '#9A72AC',
  teal: '#5CD0B3',
  white: '#ECECEC',
  grey: '#5C5C66',
  greyDark: '#26262E',
  orange: '#FF862F',
} as const;

export interface Canvas2D {
  el: HTMLCanvasElement;
  ctx: CanvasRenderingContext2D;
  /** CSS pixel size (drawing space is already scaled by DPR). */
  readonly width: number;
  readonly height: number;
  clear(): void;
  dispose(): void;
}

/** DPR-aware canvas that fills its parent and tracks parent resize. */
export function createCanvas(parent: HTMLElement): Canvas2D {
  const el = document.createElement('canvas');
  parent.appendChild(el);
  const ctx = el.getContext('2d');
  if (!ctx) throw new Error('2D context unavailable');
  let width = 0;
  let height = 0;
  const resize = () => {
    const dpr = window.devicePixelRatio || 1;
    width = parent.clientWidth;
    height = parent.clientHeight;
    el.width = Math.max(1, Math.round(width * dpr));
    el.height = Math.max(1, Math.round(height * dpr));
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
  };
  resize();
  const ro = new ResizeObserver(resize);
  ro.observe(parent);
  return {
    el,
    ctx,
    get width() {
      return width;
    },
    get height() {
      return height;
    },
    clear() {
      ctx.fillStyle = PALETTE.bg;
      ctx.fillRect(0, 0, width, height);
    },
    dispose() {
      ro.disconnect();
      el.remove();
    },
  };
}

/** Pointer position in CSS pixels relative to the canvas. */
export function pointerPos(el: HTMLCanvasElement, ev: PointerEvent): { x: number; y: number } {
  const r = el.getBoundingClientRect();
  return { x: ev.clientX - r.left, y: ev.clientY - r.top };
}
