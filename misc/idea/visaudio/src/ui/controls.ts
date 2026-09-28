import { getEngine } from '../core/audio/engine';

export function group(parent: HTMLElement, title?: string): HTMLElement {
  const el = document.createElement('div');
  el.className = 'ctl-group';
  if (title) {
    const t = document.createElement('div');
    t.className = 'ctl-title';
    t.textContent = title;
    el.appendChild(t);
  }
  parent.appendChild(el);
  return el;
}

export interface SliderOpts {
  label: string;
  min: number;
  max: number;
  step?: number;
  value: number;
  format?: (v: number) => string;
  onInput: (v: number) => void;
}

export interface Slider {
  el: HTMLElement;
  get(): number;
  set(v: number, emit?: boolean): void;
}

export function slider(parent: HTMLElement, o: SliderOpts): Slider {
  const el = document.createElement('div');
  el.className = 'ctl';
  const label = document.createElement('label');
  label.textContent = o.label;
  const val = document.createElement('span');
  val.className = 'val';
  const input = document.createElement('input');
  input.type = 'range';
  input.min = String(o.min);
  input.max = String(o.max);
  input.step = String(o.step ?? (o.max - o.min) / 200);
  input.value = String(o.value);
  const fmt = o.format ?? ((v: number) => (Number.isInteger(v) ? String(v) : v.toFixed(2)));
  const render = () => (val.textContent = fmt(Number(input.value)));
  render();
  input.addEventListener('input', () => {
    render();
    o.onInput(Number(input.value));
  });
  el.append(label, val, input);
  parent.appendChild(el);
  return {
    el,
    get: () => Number(input.value),
    set(v, emit = false) {
      input.value = String(v);
      render();
      if (emit) o.onInput(v);
    },
  };
}

export function toggle(parent: HTMLElement, label: string, value: boolean, onChange: (v: boolean) => void): HTMLLabelElement {
  const el = document.createElement('label');
  el.className = 'toggle';
  const input = document.createElement('input');
  input.type = 'checkbox';
  input.checked = value;
  input.addEventListener('change', () => onChange(input.checked));
  const span = document.createElement('span');
  span.textContent = label;
  el.append(input, span);
  parent.appendChild(el);
  return el;
}

export function select<T extends string>(
  parent: HTMLElement,
  label: string,
  options: { value: T; label: string }[],
  value: T,
  onChange: (v: T) => void,
): HTMLSelectElement {
  const el = document.createElement('div');
  el.className = 'ctl';
  const lab = document.createElement('label');
  lab.textContent = label;
  const sel = document.createElement('select');
  for (const opt of options) {
    const o = document.createElement('option');
    o.value = opt.value;
    o.textContent = opt.label;
    sel.appendChild(o);
  }
  sel.value = value;
  sel.addEventListener('change', () => onChange(sel.value as T));
  el.append(lab, sel);
  parent.appendChild(el);
  return sel;
}

export function button(parent: HTMLElement, label: string, onClick: () => void, primary = false): HTMLButtonElement {
  const b = document.createElement('button');
  b.className = primary ? 'btn primary' : 'btn';
  b.textContent = label;
  b.addEventListener('click', onClick);
  parent.appendChild(b);
  return b;
}

export function buttonRow(parent: HTMLElement): HTMLElement {
  const el = document.createElement('div');
  el.className = 'btn-row';
  parent.appendChild(el);
  return el;
}

/** Play / stop toggle that also unlocks the AudioContext. */
export function playButton(parent: HTMLElement, onStart: () => void, onStop: () => void, labels = ['播放', '停止']): { el: HTMLButtonElement; playing: () => boolean; stop: () => void } {
  let on = false;
  const b = button(
    parent,
    labels[0],
    async () => {
      await getEngine().ensureRunning();
      on = !on;
      b.textContent = on ? labels[1] : labels[0];
      b.classList.toggle('on', on);
      if (on) onStart();
      else onStop();
    },
    true,
  );
  return {
    el: b,
    playing: () => on,
    stop() {
      if (on) {
        on = false;
        b.textContent = labels[0];
        b.classList.remove('on');
        onStop();
      }
    },
  };
}

export function readout(parent: HTMLElement): { set(html: string): void } {
  const el = document.createElement('div');
  el.className = 'readout';
  parent.appendChild(el);
  return { set: (html) => (el.innerHTML = html) };
}

export function hint(parent: HTMLElement, html: string): HTMLElement {
  const el = document.createElement('div');
  el.className = 'hint';
  el.innerHTML = html;
  parent.appendChild(el);
  return el;
}
