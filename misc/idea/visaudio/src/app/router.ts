import type { Experiment, ExperimentDef, StageId } from './experiment';
import type { Shell } from './shell';

/** Hash routes: #/<stage>/<experiment>. Falls back to the first experiment. */
export function startRouter(shell: Shell, defs: ExperimentDef[]): void {
  let current: Experiment | null = null;

  const resolve = (): ExperimentDef => {
    const [, stage, id] = location.hash.replace(/^#/, '').split('/');
    return defs.find((d) => d.stage === (stage as StageId) && d.id === id) ?? defs.find((d) => d.stage === stage) ?? defs[0];
  };

  const navigate = () => {
    const def = resolve();
    const want = `#/${def.stage}/${def.id}`;
    if (location.hash !== want) {
      history.replaceState(null, '', want);
    }
    current?.dispose();
    current = null;
    shell.render(defs, def.stage, def.id);
    current = def.mount(shell.main);
    document.title = `${def.title} · visaudio`;
  };

  window.addEventListener('hashchange', navigate);
  navigate();
}
