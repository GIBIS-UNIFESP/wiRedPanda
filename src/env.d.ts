/// <reference types="astro/client" />

export {};

declare global {
  interface Window {
    umami?: {
      track: (event: string, data?: Record<string, unknown>) => void;
    };
    /** Guards the astro:before-swap theme listener against re-registration on SPA navigations. */
    __themeTransitionSetup?: true;
  }
}
