import type { ExperimentDef, StageId } from './experiment';
import { pitchGrid } from '../stages/01-compose/pitch-grid';
import { intervals } from '../stages/01-compose/intervals';
import { tempo } from '../stages/01-compose/tempo';
import { waveParams } from '../stages/02-source/wave-params';
import { harmonics } from '../stages/02-source/harmonics';
import { formant } from '../stages/02-source/formant';
import { f0Vibrato } from '../stages/02-source/f0-vibrato';
import { eq } from '../stages/03-mix/eq';
import { dynamics } from '../stages/03-mix/dynamics';
import { reverb } from '../stages/03-mix/reverb';
import { phase } from '../stages/03-mix/phase';
import { loudness } from '../stages/04-master/loudness';

export interface StageMeta {
  id: StageId;
  index: string;
  title: string;
  subtitle: string;
}

/** Ordered like a production pipeline: write → synthesize/tune → mix → master. */
export const STAGES: StageMeta[] = [
  { id: 'compose', index: '01', title: '作曲编曲', subtitle: '格子' },
  { id: 'source', index: '02', title: '声源调音', subtitle: '一条干声' },
  { id: 'mix', index: '03', title: '混音', subtitle: '几条声部' },
  { id: 'master', index: '04', title: '母带', subtitle: '整体电平' },
];

export const EXPERIMENTS: ExperimentDef[] = [
  pitchGrid,
  intervals,
  tempo,
  waveParams,
  harmonics,
  formant,
  f0Vibrato,
  eq,
  dynamics,
  reverb,
  phase,
  loudness,
];
