import en from './en';
import ptBr from './pt-br';
import es from './es';

export const languages = {
  en: 'English',
  'pt-br': 'Português',
  es: 'Español',
} as const;

export const defaultLocale = 'en' as const;

export type Locale = keyof typeof languages;
export type TranslationKey = keyof typeof en;

const translations: Record<Locale, Record<string, string>> = {
  en,
  'pt-br': ptBr,
  es,
};

/**
 * Flag icon name for each locale (used by LanguageSwitcher).
 */
export const localeFlags: Record<Locale, string> = {
  en: 'countries/unitedstates',
  'pt-br': 'countries/brazil',
  es: 'countries/spain',
};

/**
 * Extract the locale from the current URL pathname.
 * With Astro's i18n routing (prefixDefaultLocale: false),
 * English pages have no prefix, other locale pages have /{locale}/ prefix.
 */
export function getLocaleFromUrl(url: URL): Locale {
  const base = import.meta.env.BASE_URL.replace(/\/$/, '');
  let pathname = url.pathname;

  // Strip the base path
  if (base && pathname.startsWith(base)) {
    pathname = pathname.slice(base.length);
  }

  // Check if pathname starts with a known locale prefix
  const segments = pathname.split('/').filter(Boolean);
  if (segments.length > 0 && segments[0] in languages && segments[0] !== defaultLocale) {
    return segments[0] as Locale;
  }

  return defaultLocale;
}

/**
 * Get the translation function for a specific locale.
 */
export function useTranslations(locale: Locale) {
  return function t(key: TranslationKey): string {
    return translations[locale]?.[key] ?? translations[defaultLocale][key] ?? key;
  };
}

/**
 * Get the localized path for a given path and locale.
 */
export function getLocalePath(path: string, locale: Locale): string {
  const base = import.meta.env.BASE_URL.replace(/\/$/, '');

  // Normalize the path: strip base and any existing locale prefix
  let cleanPath = path;
  if (base && cleanPath.startsWith(base)) {
    cleanPath = cleanPath.slice(base.length);
  }
  // Strip locale prefix if present
  for (const loc of Object.keys(languages)) {
    if (loc !== defaultLocale && cleanPath.startsWith(`/${loc}`)) {
      cleanPath = cleanPath.slice(`/${loc}`.length);
      break;
    }
  }

  // Ensure cleanPath starts with /
  if (!cleanPath.startsWith('/')) {
    cleanPath = '/' + cleanPath;
  }

  // Build the localized path
  if (locale === defaultLocale) {
    return `${base}${cleanPath}`;
  }
  return `${base}/${locale}${cleanPath}`;
}

/**
 * Get the HTML lang attribute value for a locale.
 */
export function getHtmlLang(locale: Locale): string {
  const map: Record<Locale, string> = {
    en: 'en',
    'pt-br': 'pt-BR',
    es: 'es',
  };
  return map[locale] ?? 'en';
}

/**
 * Get all other locales (for language switcher and hreflang generation).
 */
export function getOtherLocales(locale: Locale): Locale[] {
  return (Object.keys(languages) as Locale[]).filter((l) => l !== locale);
}

/**
 * Get ALL locales.
 */
export function getAllLocales(): Locale[] {
  return Object.keys(languages) as Locale[];
}

/**
 * Generate static paths for [...locale] dynamic routes.
 * Returns an array suitable for Astro's getStaticPaths().
 * The default locale maps to `undefined` (no prefix), others to their locale key.
 */
export function getStaticLocalePaths() {
  return getAllLocales().map((locale) => ({
    params: { locale: locale === defaultLocale ? undefined : locale },
    props: { locale },
  }));
}
