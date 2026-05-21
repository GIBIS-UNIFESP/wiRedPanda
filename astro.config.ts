import { defineConfig } from 'astro/config';
import tailwindcss from '@tailwindcss/vite';
import icon from 'astro-icon';
import sitemap from '@astrojs/sitemap';

// https://astro.build/config
export default defineConfig({
  output: 'static',
  site: 'https://gibis-unifesp.github.io/wiRedPanda',
  base: '/wiRedPanda/',
  integrations: [icon(), sitemap()],
  i18n: {
    defaultLocale: 'en',
    locales: [
      'en',
      'pt-br',
      'es',
      'zh',
      'ja',
      'fr',
      'de',
      'ar',
      'bg',
      'bn',
      'cs',
      'da',
      'el',
      'et',
      'fa',
      'fi',
      'he',
      'hi',
      'hr',
      'hu',
      'id',
      'it',
      'ko',
      'lt',
      'lv',
      'ms',
      'nb',
      'nl',
      'pl',
      'pt',
      'ro',
      'ru',
      'sk',
      'sv',
      'th',
      'tr',
      'uk',
      'vi',
      'zh-hant',
    ],
    routing: {
      prefixDefaultLocale: false,
      redirectToDefaultLocale: false,
    },
  },
  vite: {
    plugins: [tailwindcss()],
  },
});
