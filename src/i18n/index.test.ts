import { describe, it, expect } from 'vitest';
import { readFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import {
  getLocaleFromUrl,
  getLocalePath,
  getHtmlLang,
  getLocaleDir,
  useTranslations,
  getOtherLocales,
  getAllLocales,
  getStaticLocalePaths,
  matchBrowserLanguage,
  localeRedirectMap,
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
    ['zh-hant', 'zh-TW'],
    ['ja', 'ja'],
    ['fr', 'fr'],
    ['de', 'de'],
    ['ar', 'ar'],
    ['ru', 'ru'],
    ['uk', 'uk'],
  ];

  for (const [locale, expected] of cases) {
    it(`${locale} → ${expected}`, () => {
      expect(getHtmlLang(locale)).toBe(expected);
    });
  }
});

// ---------------------------------------------------------------------------

describe('getLocaleDir', () => {
  it('returns rtl for Arabic', () => {
    expect(getLocaleDir('ar')).toBe('rtl');
  });

  it('returns rtl for Farsi', () => {
    expect(getLocaleDir('fa')).toBe('rtl');
  });

  it('returns rtl for Hebrew', () => {
    expect(getLocaleDir('he')).toBe('rtl');
  });

  it('returns ltr for all non-RTL locales', () => {
    const ltrLocales: Locale[] = ['en', 'pt-br', 'es', 'zh', 'ja', 'fr', 'de', 'ru', 'ko', 'th'];
    for (const locale of ltrLocales) {
      expect(getLocaleDir(locale)).toBe('ltr');
    }
  });
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
  it('returns all 39 locales', () => {
    expect(getAllLocales()).toHaveLength(39);
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

// ---------------------------------------------------------------------------

describe('localeRedirectMap', () => {
  const targets = new Set(localeRedirectMap.map(([, locale]) => locale));
  const prefixes = localeRedirectMap.map(([prefix]) => prefix);

  it('every non-default locale is reachable as a redirect target', () => {
    const nonDefaultLocales = getAllLocales().filter((l) => l !== defaultLocale);
    for (const locale of nonDefaultLocales) {
      expect(targets, `locale '${locale}' has no entry in localeRedirectMap`).toContain(locale);
    }
  });

  it('has no duplicate browser language prefixes', () => {
    const unique = new Set(prefixes);
    expect(prefixes).toHaveLength(unique.size);
  });

  it('all target locales exist in the languages map', () => {
    for (const [, target] of localeRedirectMap) {
      expect(getAllLocales()).toContain(target);
    }
  });

  it('the default locale is not a target', () => {
    expect(targets).not.toContain(defaultLocale);
  });

  it('specific prefixes appear before their generic equivalents', () => {
    // 'pt-br' must come before 'pt', and 'zh-tw' etc. before 'zh'
    const ptBrIdx = prefixes.indexOf('pt-br');
    const ptIdx = prefixes.indexOf('pt');
    expect(ptBrIdx).toBeLessThan(ptIdx);

    const zhTwIdx = prefixes.indexOf('zh-tw');
    const zhIdx = prefixes.indexOf('zh');
    expect(zhTwIdx).toBeLessThan(zhIdx);

    const zhHantIdx = prefixes.indexOf('zh-hant');
    expect(zhHantIdx).toBeLessThan(zhIdx);
  });
});

// ---------------------------------------------------------------------------

describe('matchBrowserLanguage', () => {
  it('matches pt-BR to pt-br (region code before generic)', () => {
    expect(matchBrowserLanguage('pt-BR')).toBe('pt-br');
  });

  it('matches zh-TW to zh-hant (script/region code before generic zh)', () => {
    expect(matchBrowserLanguage('zh-TW')).toBe('zh-hant');
  });

  it('matches generic pt to pt (European Portuguese)', () => {
    expect(matchBrowserLanguage('pt')).toBe('pt');
  });

  it('matches a bare 2-letter code', () => {
    expect(matchBrowserLanguage('de')).toBe('de');
  });

  it('matches a region-suffixed code via prefix', () => {
    expect(matchBrowserLanguage('de-DE')).toBe('de');
  });

  it('matches Norwegian generic to Bokmål', () => {
    expect(matchBrowserLanguage('no')).toBe('nb');
  });

  it('returns null for an unrecognized language', () => {
    expect(matchBrowserLanguage('xx-YY')).toBeNull();
  });

  it('is case-insensitive', () => {
    expect(matchBrowserLanguage('PT-BR')).toBe('pt-br');
  });
});

// ---------------------------------------------------------------------------

describe('Base.astro locale-matching parity', () => {
  // Base.astro's redirect script must stay `is:inline` (it has to run before
  // first paint to avoid a flash of the wrong locale), so it can't import
  // matchBrowserLanguage directly and carries its own copy of the matching
  // loop. This guards that copy against silently drifting from the canonical
  // implementation above.
  const baseAstroPath = fileURLToPath(new URL('../layouts/Base.astro', import.meta.url));
  const baseAstroSource = readFileSync(baseAstroPath, 'utf-8');
  const scriptMatch =
    /Redirect users to their preferred locale on first visit -->\s*<script[^>]*>([\s\S]*?)<\/script>/.exec(
      baseAstroSource
    );
  if (!scriptMatch?.[1]) {
    throw new Error('Could not find the locale-redirect inline script in Base.astro');
  }
  const redirectScriptBody = scriptMatch[1];

  it('the inline redirect script is present and still matches via localeRedirectMap', () => {
    expect(redirectScriptBody).toContain('localeRedirectMap[j][0]');
    expect(redirectScriptBody).toContain('localeRedirectMap[j][1]');
    expect(redirectScriptBody).toMatch(/lang === prefix/);
    expect(redirectScriptBody).toMatch(/lang\.indexOf\(prefix \+ '-'\) === 0/);
  });

  it('produces the same redirect target as matchBrowserLanguage for representative inputs', () => {
    const cases = ['pt-BR', 'zh-TW', 'zh-Hans-CN', 'de', 'de-DE', 'no', 'xx-YY'];

    for (const language of cases) {
      let redirectedTo: string | null = null;
      const fakeWindow = {
        location: {
          pathname: '/',
          replace: (target: string) => {
            redirectedTo = target;
          },
        },
      };
      const fakeSessionStorage = (() => {
        const store = new Map<string, string>();
        return {
          getItem: (k: string) => store.get(k) ?? null,
          setItem: (k: string, v: string) => void store.set(k, v),
        };
      })();

      // Executing the extracted inline-script text is the point of this test
      // (verifying Base.astro's necessarily-duplicated matching loop behaves
      // identically to matchBrowserLanguage) — not a dynamic/untrusted eval.
      // eslint-disable-next-line @typescript-eslint/no-implied-eval -- see above
      const run = new Function(
        'window',
        'sessionStorage',
        'navigator',
        'siteBase',
        'nonDefaultLocales',
        'localeRedirectMap',
        redirectScriptBody
      );
      // eslint-disable-next-line @typescript-eslint/no-unsafe-call -- see above
      run(fakeWindow, fakeSessionStorage, { language }, '', [], localeRedirectMap);

      const expected = matchBrowserLanguage(language);
      if (expected === null) {
        expect(redirectedTo, `expected no redirect for '${language}'`).toBeNull();
      } else {
        expect(redirectedTo, `expected redirect to '${expected}' for '${language}'`).toBe(
          '/' + expected + '/'
        );
      }
    }
  });
});
