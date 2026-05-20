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
  en: 'countries/unitedstates',
  'pt-br': 'countries/brazil',
  es: 'countries/spain',
  zh: 'countries/china',
  ja: 'countries/japan',
  fr: 'countries/france',
  de: 'countries/germany',
  ar: 'countries/saudiarabia',
  bg: 'countries/bulgaria',
  bn: 'countries/bangladesh',
  cs: 'countries/czechrepublic',
  da: 'countries/denmark',
  el: 'countries/greece',
  et: 'countries/estonia',
  fa: 'countries/iran',
  fi: 'countries/finland',
  he: 'countries/israel',
  hi: 'countries/india',
  hr: 'countries/croatia',
  hu: 'countries/hungary',
  id: 'countries/indonesia',
  it: 'countries/italy',
  ko: 'countries/southkorea',
  lt: 'countries/lithuania',
  lv: 'countries/latvia',
  ms: 'countries/malaysia',
  nb: 'countries/norway',
  nl: 'countries/netherlands',
  pl: 'countries/poland',
  pt: 'countries/portugal',
  ro: 'countries/romania',
  ru: 'countries/russia',
  sk: 'countries/slovakia',
  sv: 'countries/sweden',
  th: 'countries/thailand',
  tr: 'countries/turkey',
  uk: 'countries/ukraine',
  vi: 'countries/vietnam',
  'zh-hant': 'countries/taiwan',
};

const rtlLocales = new Set<Locale>(['ar', 'fa', 'he']);

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
 */
export function getHtmlLang(locale: Locale): string {
  const map: Record<Locale, string> = {
    en: 'en',
    'pt-br': 'pt-BR',
    es: 'es',
    zh: 'zh-CN',
    ja: 'ja',
    fr: 'fr',
    de: 'de',
    ar: 'ar',
    bg: 'bg',
    bn: 'bn',
    cs: 'cs',
    da: 'da',
    el: 'el',
    et: 'et',
    fa: 'fa',
    fi: 'fi',
    he: 'he',
    hi: 'hi',
    hr: 'hr',
    hu: 'hu',
    id: 'id',
    it: 'it',
    ko: 'ko',
    lt: 'lt',
    lv: 'lv',
    ms: 'ms',
    nb: 'nb',
    nl: 'nl',
    pl: 'pl',
    pt: 'pt',
    ro: 'ro',
    ru: 'ru',
    sk: 'sk',
    sv: 'sv',
    th: 'th',
    tr: 'tr',
    uk: 'uk',
    vi: 'vi',
    'zh-hant': 'zh-TW',
  };
  return map[locale];
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
