import { defineConfig } from 'vite';

// VITE_BASE lets the built site live under a sub-path (e.g. embedded in the wiki).
export default defineConfig(({ mode }) => ({
  base: process.env.VITE_BASE ?? '/',
  build: {
    target: 'es2022',
    sourcemap: mode !== 'production',
  },
  server: {
    port: 5180,
    strictPort: false,
  },
  preview: {
    port: 5180,
    strictPort: false,
  },
}));
