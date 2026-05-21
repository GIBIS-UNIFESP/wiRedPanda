import en from './en';
import ptBr from './pt-br';
import es from './es';
import zh from './zh';
import ja from './ja';
import fr from './fr';
import de from './de';
import ar from './ar';
import bg from './bg';
import bn from './bn';
import cs from './cs';
import da from './da';
import el from './el';
import et from './et';
import fa from './fa';
import fi from './fi';
import he from './he';
import hi from './hi';
import hr from './hr';
import hu from './hu';
import id from './id';
import it from './it';
import ko from './ko';
import lt from './lt';
import lv from './lv';
import ms from './ms';
import nb from './nb';
import nl from './nl';
import pl from './pl';
import pt from './pt';
import ro from './ro';
import ru from './ru';
import sk from './sk';
import sv from './sv';
import th from './th';
import tr from './tr';
import uk from './uk';
import vi from './vi';
import zhHant from './zh-hant';

export const languages = {
  en: 'English',
  'pt-br': 'Português (BR)',
  es: 'Español',
  zh: '中文 (简体)',
  ja: '日本語',
  fr: 'Français',
  de: 'Deutsch',
  ar: 'العربية',
  bg: 'Български',
  bn: 'বাংলা',
  cs: 'Čeština',
  da: 'Dansk',
  el: 'Ελληνικά',
  et: 'Eesti',
  fa: 'فارسی',
  fi: 'Suomi',
  he: 'עברית',
  hi: 'हिंदी',
  hr: 'Hrvatski',
  hu: 'Magyar',
  id: 'Bahasa Indonesia',
  it: 'Italiano',
  ko: '한국어',
  lt: 'Lietuvių',
  lv: 'Latviešu',
  ms: 'Bahasa Melayu',
  nb: 'Norsk',
  nl: 'Nederlands',
  pl: 'Polski',
  pt: 'Português (PT)',
  ro: 'Română',
  ru: 'Русский',
  sk: 'Slovenčina',
  sv: 'Svenska',
  th: 'ภาษาไทย',
  tr: 'Türkçe',
  uk: 'Українська',
  vi: 'Tiếng Việt',
  'zh-hant': '中文 (繁體)',
} as const;

export const defaultLocale = 'en' as const;

export type Locale = keyof typeof languages;
export type TranslationKey = keyof typeof en;

const translations: Record<Locale, Record<string, string>> = {
  en,
  'pt-br': ptBr,
  es,
  zh,
  ja,
  fr,
  de,
  ar,
  bg,
  bn,
  cs,
  da,
  el,
  et,
  fa,
  fi,
  he,
  hi,
  hr,
  hu,
  id,
  it,
  ko,
  lt,
  lv,
  ms,
  nb,
  nl,
  pl,
  pt,
  ro,
  ru,
  sk,
  sv,
  th,
  tr,
  uk,
  vi,
  'zh-hant': zhHant,
};

export const localeFlags: Record<Locale, string> = {
  en: 'circle-flags:us',
  'pt-br': 'circle-flags:br',
  es: 'circle-flags:es',
  zh: 'circle-flags:cn',
  ja: 'circle-flags:jp',
  fr: 'circle-flags:fr',
  de: 'circle-flags:de',
  ar: 'circle-flags:sa',
  bg: 'circle-flags:bg',
  bn: 'circle-flags:bd',
  cs: 'circle-flags:cz',
  da: 'circle-flags:dk',
  el: 'circle-flags:gr',
  et: 'circle-flags:ee',
  fa: 'circle-flags:ir',
  fi: 'circle-flags:fi',
  he: 'circle-flags:il',
  hi: 'circle-flags:in',
  hr: 'circle-flags:hr',
  hu: 'circle-flags:hu',
  id: 'circle-flags:id',
  it: 'circle-flags:it',
  ko: 'circle-flags:kr',
  lt: 'circle-flags:lt',
  lv: 'circle-flags:lv',
  ms: 'circle-flags:my',
  nb: 'circle-flags:no',
  nl: 'circle-flags:nl',
  pl: 'circle-flags:pl',
  pt: 'circle-flags:pt',
  ro: 'circle-flags:ro',
  ru: 'circle-flags:ru',
  sk: 'circle-flags:sk',
  sv: 'circle-flags:se',
  th: 'circle-flags:th',
  tr: 'circle-flags:tr',
  uk: 'circle-flags:ua',
  vi: 'circle-flags:vn',
  'zh-hant': 'circle-flags:tw',
};

const rtlLocales = new Set<Locale>(['ar', 'fa', 'he']);

/**
 * Ordered list of [browserLangPrefix, targetLocale] pairs for the first-visit
 * locale auto-redirect in Base.astro.
 *
 * Rules:
 * - More specific prefixes (region/script codes) must come before shorter ones
 *   so that e.g. "pt-BR" matches before the generic "pt" entry.
 * - Every non-default locale must appear as a target at least once; the test
 *   suite enforces this so adding a locale without updating this map fails CI.
 */
export const localeRedirectMap: readonly [string, Locale][] = [
  // ── Region / script codes (specific → generic) ──────────────────────────
  ['pt-br', 'pt-br'], // pt-BR  → Brazilian Portuguese
  ['zh-tw', 'zh-hant'], // zh-TW  → Traditional Chinese
  ['zh-hk', 'zh-hant'], // zh-HK  → Traditional Chinese
  ['zh-mo', 'zh-hant'], // zh-MO  → Traditional Chinese
  ['zh-hant', 'zh-hant'], // zh-Hant, zh-Hant-TW → Traditional Chinese
  ['zh-hans', 'zh'], // zh-Hans, zh-Hans-CN → Simplified Chinese
  ['no', 'nb'], // Norwegian (generic) → Bokmål
  // ── Generic 2-letter language codes ─────────────────────────────────────
  ['ar', 'ar'],
  ['bg', 'bg'],
  ['bn', 'bn'],
  ['cs', 'cs'],
  ['da', 'da'],
  ['de', 'de'],
  ['el', 'el'],
  ['es', 'es'],
  ['et', 'et'],
  ['fa', 'fa'],
  ['fi', 'fi'],
  ['fr', 'fr'],
  ['he', 'he'],
  ['hi', 'hi'],
  ['hr', 'hr'],
  ['hu', 'hu'],
  ['id', 'id'],
  ['it', 'it'],
  ['ja', 'ja'],
  ['ko', 'ko'],
  ['lt', 'lt'],
  ['lv', 'lv'],
  ['ms', 'ms'],
  ['nb', 'nb'],
  ['nl', 'nl'],
  ['pl', 'pl'],
  ['pt', 'pt'], // generic pt / pt-PT → European Portuguese
  ['ro', 'ro'],
  ['ru', 'ru'],
  ['sk', 'sk'],
  ['sv', 'sv'],
  ['th', 'th'],
  ['tr', 'tr'],
  ['uk', 'uk'],
  ['vi', 'vi'],
  ['zh', 'zh'], // generic zh / zh-CN / zh-Hans → Simplified Chinese
];

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
  const [first] = pathname.split('/').filter(Boolean);
  if (first !== undefined && first in languages && first !== defaultLocale) {
    return first as Locale;
  }

  return defaultLocale;
}

/**
 * Get the translation function for a specific locale.
 */
export function useTranslations(locale: Locale) {
  return function t(key: TranslationKey): string {
    return translations[locale][key] ?? translations[defaultLocale][key] ?? key;
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
    if (loc !== defaultLocale && (cleanPath === `/${loc}` || cleanPath.startsWith(`/${loc}/`))) {
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
 * Most locale keys map directly to a BCP 47 tag; only three need remapping.
 */
export function getHtmlLang(locale: Locale): string {
  const overrides: Partial<Record<Locale, string>> = {
    'pt-br': 'pt-BR',
    zh: 'zh-CN',
    'zh-hant': 'zh-TW',
  };
  return overrides[locale] ?? locale;
}

/**
 * Get the text direction for a locale ('ltr' or 'rtl').
 */
export function getLocaleDir(locale: Locale): 'ltr' | 'rtl' {
  return rtlLocales.has(locale) ? 'rtl' : 'ltr';
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
