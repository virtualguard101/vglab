import type { ExperimentDef, StageId } from './experiment';
import { STAGES } from './registry';

export interface Shell {
  main: HTMLElement;
  render(defs: ExperimentDef[], stage: StageId, current: string): void;
}

export function createShell(root: HTMLElement): Shell {
  root.innerHTML = `
    <header class="topbar">
      <div class="brand">visaudio<small>调音 · 混音 · 可视化</small></div>
      <nav class="stages"></nav>
    </header>
    <div class="body">
      <aside class="sidebar"></aside>
      <main class="main"></main>
    </div>`;
  const stagesEl = root.querySelector<HTMLElement>('.stages')!;
  const sidebar = root.querySelector<HTMLElement>('.sidebar')!;
  const main = root.querySelector<HTMLElement>('.main')!;

  return {
    main,
    render(defs, stage, current) {
      stagesEl.innerHTML = '';
      STAGES.forEach((s, i) => {
        if (i > 0) {
          const arrow = document.createElement('span');
          arrow.className = 'stage-arrow';
          arrow.textContent = '→';
          stagesEl.appendChild(arrow);
        }
        const b = document.createElement('button');
        b.className = 'stage-btn' + (s.id === stage ? ' active' : '');
        b.innerHTML = `<span class="idx">${s.index}</span><span>${s.title}</span>`;
        b.addEventListener('click', () => {
          const first = defs.find((d) => d.stage === s.id);
          if (first) location.hash = `#/${s.id}/${first.id}`;
        });
        stagesEl.appendChild(b);
      });

      const stageMeta = STAGES.find((s) => s.id === stage)!;
      sidebar.innerHTML = `<h3>${stageMeta.title} · ${stageMeta.subtitle}</h3>`;
      for (const d of defs.filter((d) => d.stage === stage)) {
        const a = document.createElement('a');
        a.className = 'exp-link' + (d.id === current ? ' active' : '');
        a.href = `#/${d.stage}/${d.id}`;
        a.innerHTML = `${d.title}<small>${d.summary}</small>`;
        sidebar.appendChild(a);
      }
    },
  };
}
