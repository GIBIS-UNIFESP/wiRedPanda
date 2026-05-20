import eslint from '@eslint/js';
import tseslint from 'typescript-eslint';
import eslintPluginAstro from 'eslint-plugin-astro';
import eslintConfigPrettier from 'eslint-config-prettier';
import { defineConfig } from 'eslint/config';

export default defineConfig(
  eslint.configs.recommended,

  {
    rules: {
      eqeqeq: ['error', 'always'],
      'no-console': ['error', { allow: ['error', 'warn', 'info'] }],
    },
  },

  // TypeScript files with full type-aware rules (excludes astro virtual files)
  {
    files: ['**/*.ts'],
    ignores: ['**/*.astro/*.ts'],
    extends: [...tseslint.configs.strictTypeChecked, ...tseslint.configs.stylisticTypeChecked],
    languageOptions: {
      parserOptions: {
        project: true,
        tsconfigRootDir: import.meta.dirname,
      },
    },
    rules: {
      '@typescript-eslint/switch-exhaustiveness-check': 'error',
      '@typescript-eslint/no-shadow': 'error',
    },
  },

  // Astro files with recommended rules (no type-aware rules)
  ...eslintPluginAstro.configs['flat/recommended'],

  // Ban `as any` inside .astro script blocks (no-explicit-any doesn't need type info)
  {
    files: ['**/*.astro'],
    plugins: { '@typescript-eslint': tseslint.plugin },
    rules: {
      '@typescript-eslint/no-explicit-any': 'error',
    },
  },

  eslintConfigPrettier,

  {
    ignores: ['dist/', 'public/', '.astro/'],
  }
);
