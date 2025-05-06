import { defineConfig } from 'astro/config';

import icon from "astro-icon";

// https://astro.build/config
export default defineConfig({
  site: 'https://gibis-unifesp.github.io',
  base: 'wiRedPanda',
  integrations: [icon()]
});
