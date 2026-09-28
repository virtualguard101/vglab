import { EXPERIMENTS } from './app/registry';
import { startRouter } from './app/router';
import { createShell } from './app/shell';
import { getEngine } from './core/audio/engine';

const root = document.getElementById('app');
if (!root) throw new Error('#app missing');

// Autoplay policy: the context is created lazily and resumed on the first gesture.
const unlock = () => {
  void getEngine().ensureRunning();
  document.removeEventListener('pointerdown', unlock);
  document.removeEventListener('keydown', unlock);
};
document.addEventListener('pointerdown', unlock);
document.addEventListener('keydown', unlock);

startRouter(createShell(root), EXPERIMENTS);
