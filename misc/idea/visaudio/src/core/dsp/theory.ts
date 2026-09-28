export const A4_HZ = 440;
export const A4_MIDI = 69;

export const midiToHz = (n: number): number => A4_HZ * 2 ** ((n - A4_MIDI) / 12);
export const hzToMidi = (f: number): number => A4_MIDI + 12 * Math.log2(f / A4_HZ);
export const centsBetween = (f1: number, f2: number): number => 1200 * Math.log2(f2 / f1);

export const NOTE_NAMES = ['C', 'C♯', 'D', 'D♯', 'E', 'F', 'F♯', 'G', 'G♯', 'A', 'A♯', 'B'] as const;

export function noteName(midi: number): string {
  const n = Math.round(midi);
  const name = NOTE_NAMES[((n % 12) + 12) % 12];
  return `${name}${Math.floor(n / 12) - 1}`;
}

export const isBlackKey = (midi: number): boolean => [1, 3, 6, 8, 10].includes(((midi % 12) + 12) % 12);

export interface Interval {
  semitones: number;
  name: string;
  ratio: string;
}

export const INTERVALS: Interval[] = [
  { semitones: 0, name: '同度', ratio: '1:1' },
  { semitones: 2, name: '大二度', ratio: '9:8' },
  { semitones: 3, name: '小三度', ratio: '6:5' },
  { semitones: 4, name: '大三度', ratio: '5:4' },
  { semitones: 5, name: '纯四度', ratio: '4:3' },
  { semitones: 6, name: '三全音', ratio: '45:32' },
  { semitones: 7, name: '纯五度', ratio: '3:2' },
  { semitones: 12, name: '八度', ratio: '2:1' },
];

/** Milliseconds for one note of the given division at a tempo (1 = quarter, 0.5 = eighth ...). */
export const beatMs = (bpm: number, division = 1): number => (60000 / bpm) * division;

export const dbToGain = (db: number): number => 10 ** (db / 20);
export const gainToDb = (g: number): number => 20 * Math.log10(Math.max(Math.abs(g), 1e-9));

export const clamp = (v: number, lo: number, hi: number): number => Math.min(hi, Math.max(lo, v));
export const lerp = (a: number, b: number, t: number): number => a + (b - a) * t;
