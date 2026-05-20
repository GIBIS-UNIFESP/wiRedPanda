import { describe, it, expect } from 'vitest';
import {
  getLocaleFromUrl,
  getLocalePath,
  getHtmlLang,
  useTranslations,
  getOtherLocales,
  getAllLocales,
  getStaticLocalePaths,
  defaultLocale,
  languages,
  type Locale,
} from './index';

// In the test environment (Vitest/Node), import.meta.env.BASE_URL defaults to
// '/', so the functions operate with an empty base prefix. The logic tested
// here is identical to production; only the /wiRedPanda prefix is absent.
const url = (path: string) => new URL(`https://x.com${path}`);

// ---------------------------------------------------------------------------

describe('getLocaleFromUrl', () => {
  it('returns default locale for root', () => {
    expect(getLocaleFromUrl(url('/'))).toBe('en');
  });

  it('returns default locale for English path (no prefix)', () => {
    expect(getLocaleFromUrl(url('/about'))).toBe('en');
  });

  it('detects pt-br at root', () => {
    expect(getLocaleFromUrl(url('/pt-br/'))).toBe('pt-br');
  });

  it('detects pt-br on nested path', () => {
    expect(getLocaleFromUrl(url('/pt-br/about'))).toBe('pt-br');
  });

  it('detects es', () => {
    expect(getLocaleFromUrl(url('/es/'))).toBe('es');
  });

  it('detects zh on nested path', () => {
    expect(getLocaleFromUrl(url('/zh/download'))).toBe('zh');
  });

  it('detects ja', () => {
    expect(getLocaleFromUrl(url('/ja/'))).toBe('ja');
  });

  it('detects fr', () => {
    expect(getLocaleFromUrl(url('/fr/about'))).toBe('fr');
  });

  it('detects de', () => {
    expect(getLocaleFromUrl(url('/de/'))).toBe('de');
  });

  it('does not treat /en/ as a locale prefix (default has no prefix)', () => {
    expect(getLocaleFromUrl(url('/en/'))).toBe('en');
  });

  it('returns default locale for unknown path segment', () => {
    expect(getLocaleFromUrl(url('/notareal/'))).toBe('en');
  });
});

// ---------------------------------------------------------------------------

describe('getLocalePath', () => {
  it('root for default locale', () => {
    expect(getLocalePath('/', 'en')).toBe('/');
  });

  it('nested path for default locale', () => {
    expect(getLocalePath('/about', 'en')).toBe('/about');
  });

  it('root for pt-br', () => {
    expect(getLocalePath('/', 'pt-br')).toBe('/pt-br/');
  });

  it('nested path for pt-br', () => {
    expect(getLocalePath('/about', 'pt-br')).toBe('/pt-br/about');
  });

  it('nested path for de', () => {
    expect(getLocalePath('/about', 'de')).toBe('/de/about');
  });

  it('strips existing locale prefix when switching to default', () => {
    expect(getLocalePath('/pt-br/about', 'en')).toBe('/about');
  });

  it('switches locale prefix from pt-br to es', () => {
    expect(getLocalePath('/pt-br/about', 'es')).toBe('/es/about');
  });

  it('normalises path without leading slash', () => {
    expect(getLocalePath('about', 'en')).toBe('/about');
  });

  it('normalises path without leading slash for non-default locale', () => {
    expect(getLocalePath('about', 'fr')).toBe('/fr/about');
  });

  it('does not strip locale prefix from a path that merely starts with the same letters', () => {
    // '/design' starts with '/de' but is not a locale-prefixed path
    expect(getLocalePath('/design', 'en')).toBe('/design');
  });
});

// ---------------------------------------------------------------------------

describe('getHtmlLang', () => {
  const cases: [Locale, string][] = [
    ['en', 'en'],
    ['pt-br', 'pt-BR'],
    ['es', 'es'],
    ['zh', 'zh-CN'],
    ['ja', 'ja'],
    ['fr', 'fr'],
    ['de', 'de'],
  ];

  for (const [locale, expected] of cases) {
    it(`${locale} → ${expected}`, () => {
      expect(getHtmlLang(locale)).toBe(expected);
    });
  }
});

// ---------------------------------------------------------------------------

describe('useTranslations', () => {
  it('returns a function', () => {
    expect(typeof useTranslations('en')).toBe('function');
  });

  it('returns correct English translation', () => {
    expect(useTranslations('en')('nav.home')).toBe('Home');
  });

  it('returns locale-specific translation for pt-br', () => {
    const ptHome = useTranslations('pt-br')('nav.home');
    const enHome = useTranslations('en')('nav.home');
    expect(ptHome).not.toBe(enHome);
  });

  it('returns a string for every locale', () => {
    for (const locale of getAllLocales()) {
      const t = useTranslations(locale);
      expect(typeof t('meta.description')).toBe('string');
    }
  });
});

// ---------------------------------------------------------------------------

describe('getOtherLocales', () => {
  it('excludes the active locale', () => {
    expect(getOtherLocales('en')).not.toContain('en');
  });

  it('has length totalLocales - 1', () => {
    expect(getOtherLocales('en')).toHaveLength(Object.keys(languages).length - 1);
  });

  it('excludes pt-br when that is the active locale', () => {
    expect(getOtherLocales('pt-br')).not.toContain('pt-br');
  });

  it('contains all other locales', () => {
    const others = getOtherLocales('en');
    for (const locale of Object.keys(languages) as Locale[]) {
      if (locale !== 'en') expect(others).toContain(locale);
    }
  });
});

// ---------------------------------------------------------------------------

describe('getAllLocales', () => {
  it('returns all 7 locales', () => {
    expect(getAllLocales()).toHaveLength(7);
  });

  it('contains all expected locale codes', () => {
    const locales = getAllLocales();
    for (const locale of Object.keys(languages) as Locale[]) {
      expect(locales).toContain(locale);
    }
  });

  it('includes the default locale', () => {
    expect(getAllLocales()).toContain(defaultLocale);
  });
});

// ---------------------------------------------------------------------------

describe('getStaticLocalePaths', () => {
  const paths = getStaticLocalePaths();

  it('returns one entry per locale', () => {
    expect(paths).toHaveLength(getAllLocales().length);
  });

  it('default locale entry has params.locale === undefined', () => {
    const entry = paths.find((p) => p.props.locale === defaultLocale);
    expect(entry).toBeDefined();
    expect(entry?.params.locale).toBeUndefined();
  });

  it('non-default locale entries have params.locale matching props.locale', () => {
    const nonDefault = paths.filter((p) => p.props.locale !== defaultLocale);
    for (const entry of nonDefault) {
      expect(entry.params.locale).toBe(entry.props.locale);
    }
  });

  it('all entries have props.locale defined', () => {
    for (const entry of paths) {
      expect(entry.props.locale).toBeDefined();
    }
  });

  it('non-default entries cover all non-default locales', () => {
    const nonDefaultLocales = getAllLocales().filter((l) => l !== defaultLocale);
    type LocaleParam = (typeof paths)[number]['params']['locale'];
    const nonDefaultParams = paths
      .map((p) => p.params.locale)
      .filter((l): l is Exclude<LocaleParam, undefined> => l !== undefined);
    expect(nonDefaultParams.sort()).toEqual(nonDefaultLocales.sort());
  });
});
