import { defineConfig } from 'astro/config';
import tailwindcss from '@tailwindcss/vite';
import icon from 'astro-icon';

// https://astro.build/config
export default defineConfig({
  output: 'static',
  site: 'https://gibis-unifesp.github.io/wiRedPanda',
  base: '/wiRedPanda/',
  integrations: [icon()],
  vite: {
    plugins: [tailwindcss()],
  },
});
