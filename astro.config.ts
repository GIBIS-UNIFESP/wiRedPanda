import { defineConfig } from 'astro/config';
import tailwindcss from '@tailwindcss/vite';
import icon from 'astro-icon';
import sitemap from '@astrojs/sitemap';
import { languages, defaultLocale } from './src/i18n';

// https://astro.build/config
export default defineConfig({
  output: 'static',
  site: 'https://gibis-unifesp.github.io/wiRedPanda',
  base: '/wiRedPanda/',
  integrations: [icon(), sitemap()],
  i18n: {
    defaultLocale,
    // Single source of truth: src/i18n/index.ts's `languages` map, so this
    // list can't silently drift from the one routing/translations actually use.
    locales: Object.keys(languages),
    routing: {
      prefixDefaultLocale: false,
      redirectToDefaultLocale: false,
    },
  },
  vite: {
    plugins: [tailwindcss()],
  },
});
