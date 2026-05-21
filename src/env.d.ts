/// <reference types="astro/client" />

export {};

declare global {
  interface Window {
    umami?: {
      track: (event: string, data?: Record<string, unknown>) => void;
    };
    /** Guards the astro:before-swap theme listener against re-registration on SPA navigations. */
    __themeTransitionSetup?: true;
    /** Guards the Umami analytics script injection against re-injection on SPA navigations. */
    __umamiLoaded?: true;
  }
}
