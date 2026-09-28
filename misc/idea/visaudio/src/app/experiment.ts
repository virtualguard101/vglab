export type StageId = 'compose' | 'source' | 'mix' | 'master';

export interface Experiment {
  dispose(): void;
}

export interface ExperimentDef {
  id: string;
  stage: StageId;
  title: string;
  /** One sentence: what to look at. */
  summary: string;
  /** Which knob this is in SV2 / VOCALOID / DAW terms. */
  mapping?: string;
  /** Section heading in the acoustics / theory note. */
  note: { doc: 'aco-theory' | 'basic-music-theory'; section: string };
  mount(root: HTMLElement): Experiment;
}

export interface Layout {
  viz: HTMLElement;
  controls: HTMLElement;
}

const NOTE_TITLES: Record<ExperimentDef['note']['doc'], string> = {
  'aco-theory': '调音混音背后的声学理论基础',
  'basic-music-theory': '乐理基础',
};

/** Standard experiment page: heading, canvas area, control strip. */
export function mountLayout(root: HTMLElement, def: ExperimentDef): Layout {
  root.innerHTML = '';
  const head = document.createElement('div');
  head.className = 'exp-head';
  const h2 = document.createElement('h2');
  h2.textContent = def.title;
  const p = document.createElement('p');
  p.textContent = def.summary;
  head.append(h2, p);
  if (def.mapping) {
    const m = document.createElement('div');
    m.className = 'mapping';
    m.textContent = def.mapping;
    head.appendChild(m);
  }
  const ref = document.createElement('div');
  ref.className = 'note-ref';
  const base = import.meta.env.VITE_WIKI_BASE as string | undefined;
  const text = `${NOTE_TITLES[def.note.doc]} › ${def.note.section}`;
  if (base) {
    const a = document.createElement('a');
    a.href = `${base.replace(/\/$/, '')}/${def.note.doc}/#${encodeURIComponent(def.note.section)}`;
    a.target = '_blank';
    a.rel = 'noreferrer';
    a.textContent = text;
    ref.append('笔记: ', a);
  } else {
    ref.textContent = `笔记: ${text}`;
  }
  head.appendChild(ref);

  const viz = document.createElement('div');
  viz.className = 'viz';
  const controls = document.createElement('div');
  controls.className = 'controls';
  root.append(head, viz, controls);
  return { viz, controls };
}
