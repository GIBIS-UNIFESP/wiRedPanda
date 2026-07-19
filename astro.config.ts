import { defineConfig } from 'astro/config';
import tailwindcss from '@tailwindcss/vite';
import icon from 'astro-icon';
import sitemap from '@astrojs/sitemap';
import { languages, defaultLocale, getAllLocales, getHtmlLang } from './src/i18n';

// https://astro.build/config
export default defineConfig({
  output: 'static',
  site: 'https://gibis-unifesp.github.io/wiRedPanda',
  base: '/wiRedPanda/',
  integrations: [
    icon(),
    sitemap({
      i18n: {
        defaultLocale,
        // Reuses src/i18n/index.ts's own hreflang mapping so the sitemap's
        // <xhtml:link hreflang> alternates can't drift from Base.astro's.
        locales: Object.fromEntries(getAllLocales().map((locale) => [locale, getHtmlLang(locale)])),
      },
    }),
  ],
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
